/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) 2026 Guangzhou Anyka Microelectronics Co., Ltd. */

/*
 * How the Linux UVC gadget function talks to user space (linux/usb/g_uvc.h):
 * it is a V4L2 video-output node. Class-specific control requests from the
 * host arrive as V4L2 events and are answered with UVCIOC_SEND_RESPONSE; a
 * negative response length makes the driver stall the request. Frames go out
 * as queued V4L2 output buffers, and the driver adds the UVC payload headers.
 * Probe/commit negotiation follows the USB Video Class 1.1 specification,
 * section 4.3.1.1. USB is little-endian, like the KM02G, so the request and
 * control fields are used as they are.
 *
 * Camera-terminal and processing-unit controls are not implemented: those
 * requests are stalled, and the VideoControl request error code control says
 * why.
 */

#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <linux/usb/ch9.h>
#include <linux/usb/g_uvc.h>
#include <linux/usb/video.h>

#include "usb_webcam.h"

#define LOG(fmt, ...) printf("[webcam] " fmt, ##__VA_ARGS__)

#define MAX_BUFS        4
#define STALL           (-1)
#define PROBE_MIN_LEN   26      /* UVC 1.0 probe/commit size; UVC 1.1 adds 8 bytes */
#define UVC_ERR_INVALID_CONTROL 0x06    /* UVC 1.1 request error code */

struct webcam {
    int fd;
    struct webcam_config cfg;
    unsigned int max_frame;
    unsigned int wait_ms;       /* how long webcam_send_frame waits for a buffer */
    int streaming_intf;         /* VideoStreaming interface number, -1 = unknown */
    unsigned char vc_error;     /* why the last VideoControl request was stalled */
    pthread_mutex_t lock;       /* the event and frame threads share the buffers */
    int streaming;
    unsigned int nbufs;         /* buffers mapped for the current stream */
    unsigned int fresh;         /* buffers [fresh, nbufs) were never queued this stream */
    void *mem[MAX_BUFS];
    size_t len[MAX_BUFS];
    struct uvc_streaming_control probe;
    struct uvc_streaming_control commit;
    unsigned int set_cs;        /* control whose SET_CUR data stage is due, 0 = none */
};

/* The one stream we offer; only the frame interval is left to the host. */
static void default_control(const struct webcam *cam, struct uvc_streaming_control *c)
{
    memset(c, 0, sizeof(*c));
    c->bFormatIndex = 1;
    c->bFrameIndex = 1;
    c->dwFrameInterval = 10000000 / cam->cfg.fps;   /* 100 ns units */
    c->dwMaxVideoFrameSize = cam->max_frame;
    c->dwMaxPayloadTransferSize = cam->cfg.max_payload;
    c->bmFramingInfo = 3;                           /* FID and EOF in every payload header */
}

/*
 * Is this request for the VideoStreaming interface? Use its interface number
 * when configfs gave it. Otherwise tell probe and commit apart from the
 * VideoControl interface's own controls, which reuse selectors 1 and 2 but are
 * 1 byte long, by size: that leaves only their 1- and 2-byte GET_INFO and
 * GET_LEN queries ambiguous.
 */
static int is_streaming_request(const struct webcam *cam, const struct usb_ctrlrequest *req)
{
    unsigned int cs = req->wValue >> 8;

    if (cam->streaming_intf >= 0)
        return (req->wIndex & 0xff) == (unsigned int)cam->streaming_intf;
    return (req->wIndex >> 8) == 0 && (cs == UVC_VS_PROBE_CONTROL || cs == UVC_VS_COMMIT_CONTROL) &&
           (req->wLength >= PROBE_MIN_LEN || req->bRequest == UVC_GET_INFO || req->bRequest == UVC_GET_LEN);
}

/*
 * VideoControl interface: no camera or processing-unit controls are offered.
 * Only the request error code control is answered, so a host can ask why its
 * last request was stalled.
 */
static void handle_control_request(struct webcam *cam, const struct usb_ctrlrequest *req,
                                   struct uvc_request_data *resp)
{
    if ((req->wIndex >> 8) == 0 && (req->wValue >> 8) == UVC_VC_REQUEST_ERROR_CODE_CONTROL) {
        if (req->bRequest == UVC_GET_CUR) {
            resp->data[0] = cam->vc_error;
            resp->length = 1;
            return;
        }
        if (req->bRequest == UVC_GET_INFO) {
            resp->data[0] = UVC_CONTROL_CAP_GET;
            resp->length = 1;
            return;
        }
    }
    cam->vc_error = UVC_ERR_INVALID_CONTROL;
}

static void handle_setup(struct webcam *cam, const struct usb_ctrlrequest *req,
                         struct uvc_request_data *resp)
{
    unsigned int cs = req->wValue >> 8;
    struct uvc_streaming_control *cur = cs == UVC_VS_PROBE_CONTROL ? &cam->probe : &cam->commit;

    resp->length = STALL;
    if (req->wLength > sizeof(resp->data))
        return;
    if (!is_streaming_request(cam, req)) {
        handle_control_request(cam, req, resp);
        return;
    }
    /* VideoStreaming interface: serve probe and commit only */
    if ((cs != UVC_VS_PROBE_CONTROL && cs != UVC_VS_COMMIT_CONTROL) ||
        (req->wLength < PROBE_MIN_LEN && req->bRequest != UVC_GET_INFO && req->bRequest != UVC_GET_LEN))
        return;

    switch (req->bRequest) {
    case UVC_SET_CUR:
        cam->set_cs = cs;                       /* the values follow as UVC_EVENT_DATA */
        resp->length = req->wLength;
        break;
    case UVC_GET_CUR:
        memcpy(resp->data, cur, sizeof(*cur));  /* the driver trims replies to wLength */
        resp->length = sizeof(*cur);
        break;
    case UVC_GET_MIN:
    case UVC_GET_MAX:
    case UVC_GET_DEF:
    case UVC_GET_RES:
        default_control(cam, (struct uvc_streaming_control *)resp->data);
        resp->length = sizeof(struct uvc_streaming_control);
        break;
    case UVC_GET_LEN:
        resp->data[0] = sizeof(struct uvc_streaming_control);
        resp->data[1] = 0;
        resp->length = 2;
        break;
    case UVC_GET_INFO:
        resp->data[0] = UVC_CONTROL_CAP_GET | UVC_CONTROL_CAP_SET;
        resp->length = 1;
        break;
    }
}

/* SET_CUR data stage: accept the host's frame interval, keep everything else ours. */
static void handle_data(struct webcam *cam, const struct uvc_request_data *data)
{
    struct uvc_streaming_control host;
    struct uvc_streaming_control *dst = cam->set_cs == UVC_VS_PROBE_CONTROL ? &cam->probe : &cam->commit;
    size_t n = data->length > 0 ? (size_t)data->length : 0;

    if (cam->set_cs == 0)
        return;
    cam->set_cs = 0;
    memset(&host, 0, sizeof(host));
    memcpy(&host, data->data, n < sizeof(host) ? n : sizeof(host));
    default_control(cam, dst);
    if (host.dwFrameInterval)
        dst->dwFrameInterval = host.dwFrameInterval;
}

static void release_buffers(struct webcam *cam)
{
    struct v4l2_requestbuffers rb;
    unsigned int i;

    for (i = 0; i < cam->nbufs; i++)
        munmap(cam->mem[i], cam->len[i]);
    cam->nbufs = 0;
    memset(&rb, 0, sizeof(rb));
    rb.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    rb.memory = V4L2_MEMORY_MMAP;
    ioctl(cam->fd, VIDIOC_REQBUFS, &rb);
}

static void stream_off(struct webcam *cam)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

    if (!cam->streaming)
        return;
    cam->streaming = 0;
    ioctl(cam->fd, VIDIOC_STREAMOFF, &type);
    release_buffers(cam);
}

/*
 * The driver holds the host's SET_INTERFACE request until VIDIOC_STREAMON, so
 * start right away. Buffers are queued as frames arrive (webcam_send_frame).
 */
static int stream_on(struct webcam *cam)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    struct v4l2_requestbuffers rb;
    struct v4l2_buffer b;
    unsigned int i;

    stream_off(cam);
    memset(&rb, 0, sizeof(rb));
    rb.count = cam->cfg.nbufs;
    rb.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    rb.memory = V4L2_MEMORY_MMAP;
    if (ioctl(cam->fd, VIDIOC_REQBUFS, &rb) < 0)
        goto fail;
    for (i = 0; i < rb.count && i < MAX_BUFS; i++) {
        memset(&b, 0, sizeof(b));
        b.index = i;
        b.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        b.memory = V4L2_MEMORY_MMAP;
        if (ioctl(cam->fd, VIDIOC_QUERYBUF, &b) < 0)
            goto fail;
        cam->mem[i] = mmap(NULL, b.length, PROT_READ | PROT_WRITE, MAP_SHARED, cam->fd, b.m.offset);
        if (cam->mem[i] == MAP_FAILED)
            goto fail;
        cam->len[i] = b.length;
        cam->nbufs++;
    }
    if (cam->nbufs == 0 || ioctl(cam->fd, VIDIOC_STREAMON, &type) < 0)
        goto fail;
    cam->fresh = 0;
    cam->streaming = 1;
    return 0;

fail:
    LOG("cannot start streaming: %s\n", strerror(errno));
    release_buffers(cam);
    return -1;
}

enum webcam_event webcam_handle_events(struct webcam *cam)
{
    enum webcam_event result = WEBCAM_EVENT_NONE;
    struct v4l2_event ev;
    struct uvc_event *uev = (struct uvc_event *)&ev.u.data;
    struct uvc_request_data resp;

    pthread_mutex_lock(&cam->lock);
    for (;;) {
        memset(&ev, 0, sizeof(ev));
        if (ioctl(cam->fd, VIDIOC_DQEVENT, &ev) < 0) {
            /* ENOENT: the queue is empty (some drivers say EAGAIN) */
            if (errno != ENOENT && errno != EAGAIN) {
                LOG("cannot read gadget events: %s\n", strerror(errno));
                stream_off(cam);
                result = WEBCAM_EVENT_ERROR;
            }
            break;
        }
        switch (ev.type) {
        case UVC_EVENT_SETUP:
            memset(&resp, 0, sizeof(resp));
            handle_setup(cam, &uev->req, &resp);
            if (ioctl(cam->fd, UVCIOC_SEND_RESPONSE, &resp) < 0)
                LOG("cannot answer request 0x%02x: %s\n", uev->req.bRequest, strerror(errno));
            break;
        case UVC_EVENT_DATA:
            handle_data(cam, &uev->data);
            break;
        case UVC_EVENT_STREAMON:
            if (stream_on(cam) == 0)
                result = WEBCAM_EVENT_STREAM_ON;
            break;
        case UVC_EVENT_STREAMOFF:
        case UVC_EVENT_DISCONNECT:
            stream_off(cam);
            result = WEBCAM_EVENT_STREAM_OFF;
            break;
        }
    }
    pthread_mutex_unlock(&cam->lock);
    return result;
}

int webcam_is_streaming(struct webcam *cam)
{
    int on;

    pthread_mutex_lock(&cam->lock);
    on = cam->streaming;
    pthread_mutex_unlock(&cam->lock);
    return on;
}

int webcam_send_frame(struct webcam *cam, const void *jpeg, size_t len)
{
    struct pollfd pfd = { .fd = cam->fd, .events = POLLOUT };
    struct v4l2_buffer b;
    int wait, ret = 0;

    if (len == 0 || len > cam->max_frame)
        return -EMSGSIZE;

    /* Wait outside the lock so stream on/off handling is never held up. */
    pthread_mutex_lock(&cam->lock);
    wait = cam->streaming && cam->fresh == cam->nbufs;
    pthread_mutex_unlock(&cam->lock);
    if (wait)
        poll(&pfd, 1, cam->wait_ms);

    pthread_mutex_lock(&cam->lock);
    memset(&b, 0, sizeof(b));
    b.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    b.memory = V4L2_MEMORY_MMAP;
    if (!cam->streaming)
        ret = -EAGAIN;
    else if (cam->fresh < cam->nbufs)
        b.index = cam->fresh++;
    else if (ioctl(cam->fd, VIDIOC_DQBUF, &b) < 0)
        ret = -errno;                           /* EAGAIN: the host still has them all */
    if (ret == 0) {
        memcpy(cam->mem[b.index], jpeg, len);
        b.bytesused = len;
        if (ioctl(cam->fd, VIDIOC_QBUF, &b) < 0)
            ret = -errno;
    }
    pthread_mutex_unlock(&cam->lock);
    return ret;
}

/* The UVC gadget is a V4L2 video *output* node: frames go out to the host. */
static int open_node(const char *path)
{
    struct v4l2_capability cap;
    int fd = open(path, O_RDWR | O_NONBLOCK);

    if (fd < 0)
        return -1;
    memset(&cap, 0, sizeof(cap));
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == 0 &&
        (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT) && (cap.capabilities & V4L2_CAP_STREAMING))
        return fd;
    close(fd);
    return -1;
}

static int find_node(void)
{
    char path[32];
    int i, fd;

    for (i = 0; i < 32; i++) {
        snprintf(path, sizeof(path), "/dev/video%d", i);
        fd = open_node(path);
        if (fd >= 0) {
            LOG("using %s\n", path);
            return fd;
        }
    }
    return -1;
}

/* From the gadget's configfs; -1 when it isn't there or there are several UVC functions. */
static int find_streaming_intf(void)
{
    glob_t g;
    FILE *f;
    int n = -1;

    if (glob("/sys/kernel/config/usb_gadget/*/functions/uvc.*/streaming/bInterfaceNumber", 0, NULL, &g) != 0)
        return -1;
    if (g.gl_pathc == 1 && (f = fopen(g.gl_pathv[0], "r")) != NULL) {
        if (fscanf(f, "%d", &n) != 1)
            n = -1;
        fclose(f);
    }
    globfree(&g);
    return n;
}

struct webcam *webcam_open(const struct webcam_config *cfg)
{
    static const unsigned int events[] = {
        UVC_EVENT_SETUP, UVC_EVENT_DATA, UVC_EVENT_STREAMON, UVC_EVENT_STREAMOFF, UVC_EVENT_DISCONNECT,
    };
    struct v4l2_event_subscription sub;
    struct v4l2_format fmt;
    struct webcam *cam;
    unsigned int i;

    if (!cfg->width || !cfg->height || !cfg->fps || !cfg->max_payload || cfg->nbufs > MAX_BUFS) {
        LOG("invalid configuration\n");
        return NULL;
    }
    cam = calloc(1, sizeof(*cam));
    if (cam == NULL)
        return NULL;
    cam->cfg = *cfg;
    if (cam->cfg.nbufs == 0)
        cam->cfg.nbufs = 2;
    cam->fd = cfg->devnode ? open_node(cfg->devnode) : find_node();
    if (cam->fd < 0) {
        LOG("no UVC gadget video node at %s\n", cfg->devnode ? cfg->devnode : "/dev/video*");
        free(cam);
        return NULL;
    }

    /*
     * The driver sizes its buffers from sizeimage and may adjust it. Default:
     * 4 bits per pixel, a generous ceiling for a high-quality JPEG; raise
     * max_frame if frames are dropped with -EMSGSIZE.
     */
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    fmt.fmt.pix.width = cfg->width;
    fmt.fmt.pix.height = cfg->height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    fmt.fmt.pix.sizeimage = cfg->max_frame ? cfg->max_frame : cfg->width * cfg->height / 2;
    if (ioctl(cam->fd, VIDIOC_S_FMT, &fmt) < 0) {
        LOG("cannot set MJPEG %ux%u: %s\n", cfg->width, cfg->height, strerror(errno));
        goto fail;
    }
    cam->max_frame = fmt.fmt.pix.sizeimage;
    /*
     * High-speed isochronous moves max_payload bytes every 125 us (full speed
     * is 8 times slower). Wait for the largest frame's bus time plus 0.5 s.
     */
    cam->wait_ms = cam->max_frame / (cam->cfg.max_payload * 8) + 500;

    cam->streaming_intf = find_streaming_intf();
    if (cam->streaming_intf >= 0)
        LOG("VideoStreaming interface %d\n", cam->streaming_intf);
    else
        LOG("VideoStreaming interface number unknown; telling requests apart by size\n");

    for (i = 0; i < sizeof(events) / sizeof(events[0]); i++) {
        memset(&sub, 0, sizeof(sub));
        sub.type = events[i];
        if (ioctl(cam->fd, VIDIOC_SUBSCRIBE_EVENT, &sub) < 0) {
            LOG("cannot subscribe to gadget events: %s\n", strerror(errno));
            goto fail;
        }
    }

    pthread_mutex_init(&cam->lock, NULL);
    default_control(cam, &cam->probe);
    cam->commit = cam->probe;
    return cam;

fail:
    close(cam->fd);
    free(cam);
    return NULL;
}

void webcam_close(struct webcam *cam)
{
    if (cam == NULL)
        return;
    pthread_mutex_lock(&cam->lock);
    stream_off(cam);
    pthread_mutex_unlock(&cam->lock);
    pthread_mutex_destroy(&cam->lock);
    close(cam->fd);
    free(cam);
}

int webcam_fd(const struct webcam *cam)
{
    return cam->fd;
}
