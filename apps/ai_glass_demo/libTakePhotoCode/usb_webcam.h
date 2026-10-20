/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) 2026 Guangzhou Anyka Microelectronics Co., Ltd. */

/*
 * USB webcam output: serves one MJPEG stream to a USB host through the Linux
 * UVC gadget function.
 *
 * The gadget's USB descriptors are set up outside this module (configfs) and
 * must declare exactly one MJPEG format with one frame size: the host is
 * always answered with format 1, frame 1.
 *
 * Threading: webcam_handle_events() and webcam_send_frame() may run in
 * different threads.
 */

#ifndef USB_WEBCAM_H
#define USB_WEBCAM_H

#include <stddef.h>

struct webcam_config {
    const char *devnode;        /* UVC gadget video node; NULL = first V4L2 output node */
    unsigned int width;         /* frame size the descriptors declare */
    unsigned int height;
    unsigned int fps;           /* frame rate offered to the host */
    unsigned int nbufs;         /* frame buffers, 1-4; 0 = 2 */
    unsigned int max_payload;   /* isochronous bytes per (micro)frame: wMaxPacketSize * (mult + 1) */
    unsigned int max_frame;     /* largest JPEG in bytes; 0 = width * height / 2 (4 bits per pixel) */
};

enum webcam_event {
    WEBCAM_EVENT_NONE,          /* streaming state unchanged */
    WEBCAM_EVENT_STREAM_ON,     /* the host started streaming: start sending frames */
    WEBCAM_EVENT_STREAM_OFF,    /* the host stopped streaming or went away */
    WEBCAM_EVENT_ERROR,         /* the gadget node failed (printed); stop using it */
};

struct webcam;

/* Returns NULL on error (the reason is printed). */
struct webcam *webcam_open(const struct webcam_config *cfg);
void webcam_close(struct webcam *cam);

/* Poll this for POLLPRI (select() exceptfds): gadget events are pending. */
int webcam_fd(const struct webcam *cam);

/* Handle every pending gadget event; returns the last streaming change, or WEBCAM_EVENT_ERROR. */
enum webcam_event webcam_handle_events(struct webcam *cam);

/* Non-zero while the host is streaming. */
int webcam_is_streaming(struct webcam *cam);

/*
 * Copy one JPEG into a free buffer and queue it, waiting for the host to hand
 * a buffer back for about as long as the largest frame takes on the bus.
 * Returns 0, -EAGAIN when the host is not streaming or no buffer came back,
 * -EMSGSIZE when the frame is empty or larger than max_frame, or another -errno.
 */
int webcam_send_frame(struct webcam *cam, const void *jpeg, size_t len);

#endif /* USB_WEBCAM_H */
