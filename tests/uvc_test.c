/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) 2026 Guangzhou Anyka Microelectronics Co., Ltd. */

/*
 * Host check of the UVC control-request handling in usb_webcam.c (no device
 * I/O). The module is included so its static functions can be called.
 */

#include <assert.h>
#include <stdio.h>
#include "usb_webcam.c"

static const unsigned char IN = 0xA1, OUT = 0x21;     /* class request, interface recipient */
static const unsigned short PROBE = UVC_VS_PROBE_CONTROL << 8, COMMIT = UVC_VS_COMMIT_CONTROL << 8;

static struct uvc_request_data setup(struct webcam *cam, unsigned char type, unsigned char req,
                                     unsigned short value, unsigned short index, unsigned short len)
{
    struct usb_ctrlrequest r = { type, req, value, index, len };
    struct uvc_request_data resp;

    memset(&resp, 0xAA, sizeof(resp));
    handle_setup(cam, &r, &resp);
    return resp;
}

static void init(struct webcam *cam, int streaming_intf)
{
    memset(cam, 0, sizeof(*cam));
    cam->cfg.fps = 10;
    cam->cfg.max_payload = 512;
    cam->max_frame = 6096384;
    cam->streaming_intf = streaming_intf;
    default_control(cam, &cam->probe);
    cam->commit = cam->probe;
}

/* Probe/commit on interface 1, the same in both modes */
static void check_negotiation(struct webcam *cam)
{
    struct uvc_request_data d;
    struct uvc_streaming_control *c, host;

    d = setup(cam, IN, UVC_GET_DEF, PROBE, 1, 34);
    c = (struct uvc_streaming_control *)d.data;
    assert(d.length == 34 && c->bFormatIndex == 1 && c->bFrameIndex == 1);
    assert(c->dwFrameInterval == 1000000 && c->dwMaxVideoFrameSize == 6096384 && c->dwMaxPayloadTransferSize == 512);

    d = setup(cam, OUT, UVC_SET_CUR, PROBE, 1, 26);           /* UVC 1.0 host: 26-byte SET_CUR */
    assert(d.length == 26 && cam->set_cs == UVC_VS_PROBE_CONTROL);
    memset(&host, 0, sizeof(host));
    host.bFormatIndex = 3;
    host.bFrameIndex = 7;
    host.dwFrameInterval = 333333;
    host.dwMaxPayloadTransferSize = 3072;
    memcpy(d.data, &host, 26);
    d.length = 26;
    handle_data(cam, &d);                                      /* host asks for format 3 / frame 7 */
    assert(cam->set_cs == 0 && cam->probe.bFormatIndex == 1 && cam->probe.bFrameIndex == 1);
    assert(cam->probe.dwFrameInterval == 333333 && cam->probe.dwMaxPayloadTransferSize == 512);

    d = setup(cam, IN, UVC_GET_CUR, PROBE, 1, 26);            /* reads back what we accepted */
    assert(d.length == 34 && memcmp(d.data, &cam->probe, 34) == 0);

    d = setup(cam, OUT, UVC_SET_CUR, COMMIT, 1, 34);
    memcpy(d.data, &cam->probe, 34);
    d.length = 34;
    handle_data(cam, &d);
    assert(cam->commit.dwFrameInterval == 333333 && cam->commit.bFrameIndex == 1);

    d = setup(cam, IN, UVC_GET_LEN, PROBE, 1, 2);
    assert(d.length == 2 && d.data[0] == 34 && d.data[1] == 0);
    d = setup(cam, IN, UVC_GET_INFO, COMMIT, 1, 1);
    assert(d.length == 1 && d.data[0] == 3);

    assert(setup(cam, OUT, UVC_SET_CUR, PROBE, 1, 61).length < 0);     /* longer than the reply buffer */
    assert(setup(cam, IN, UVC_GET_CUR, 0x0300, 1, 34).length < 0);     /* other VS selectors */

    d.length = 34;
    cam->set_cs = 0;
    handle_data(cam, &d);                                      /* stray DATA event: ignored */
    assert(cam->commit.dwFrameInterval == 333333);
}

/* VideoControl on interface 0: units stalled, the error code control answered */
static void check_control(struct webcam *cam)
{
    struct uvc_request_data d;

    assert(setup(cam, IN, UVC_GET_CUR, 0x0200, 0x0200, 2).length < 0); /* processing unit 2 */
    d = setup(cam, IN, UVC_GET_CUR, UVC_VC_REQUEST_ERROR_CODE_CONTROL << 8, 0, 1);
    assert(d.length == 1 && d.data[0] == UVC_ERR_INVALID_CONTROL);
}

int main(void)
{
    struct webcam cam;
    struct uvc_request_data d;

    init(&cam, -1);                 /* interface numbers unknown: told apart by size */
    check_negotiation(&cam);
    check_control(&cam);

    init(&cam, 1);                  /* streaming interface 1, from configfs */
    check_negotiation(&cam);
    check_control(&cam);
    /* now unambiguous: VideoControl GET_INFO/GET_LEN get VideoControl answers, not probe ones */
    d = setup(&cam, IN, UVC_GET_INFO, UVC_VC_REQUEST_ERROR_CODE_CONTROL << 8, 0, 1);
    assert(d.length == 1 && d.data[0] == UVC_CONTROL_CAP_GET);
    assert(setup(&cam, IN, UVC_GET_INFO, 0x0100, 0, 1).length < 0);
    assert(setup(&cam, IN, UVC_GET_LEN, 0x0100, 0, 2).length < 0);
    assert(setup(&cam, IN, UVC_GET_CUR, PROBE, 0, 34).length < 0);    /* probe sent to interface 0 */

    puts("uvc_test: PASS");
    return 0;
}
