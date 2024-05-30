/**
 * RPiPlay - An open-source AirPlay mirroring server for Raspberry Pi
 * Copyright (C) 2019 Florian Draschbacher
 * Modified for:
 * UxPlay - An open-source AirPlay mirroring server
 * Copyright (C) 2021-23 F. Duncanh
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include "video_renderer.h"


#define SECOND_IN_NSECS 1000000000UL


static video_renderer_t *renderer = NULL;
static logger_t *logger = NULL;
static unsigned short width, height, width_source, height_source;  /* not currently used */
static bool first_packet = false;
static bool sync = false;



static void append_videoflip (char *launch, const videoflip_t *flip, const videoflip_t *rot) {

}

/* apple uses colorimetry=1:3:5:1                                *
 * (not recognized by v4l2 plugin in Gstreamer  < 1.20.4)        *
 * See .../gst-libs/gst/video/video-color.h in gst-plugins-base  *
 * range = 1   -> GST_VIDEO_COLOR_RANGE_0_255      ("full RGB")  *
 * matrix = 3  -> GST_VIDEO_COLOR_MATRIX_BT709                   *
 * transfer = 5 -> GST_VIDEO_TRANSFER_BT709                      *
 * primaries = 1 -> GST_VIDEO_COLOR_PRIMARIES_BT709              *
 * closest used by  GStreamer < 1.20.4 is BT709, 2:3:5:1 with    *                            *
 * range = 2 -> GST_VIDEO_COLOR_RANGE_16_235 ("limited RGB")     */

static const char h264_caps[]="video/x-h264,stream-format=(string)byte-stream,alignment=(string)au";

void video_renderer_size(float *f_width_source, float *f_height_source, float *f_width, float *f_height) {
    logger_log(logger, LOGGER_DEBUG, "ENTER");
    width_source = (unsigned short) *f_width_source;
    height_source = (unsigned short) *f_height_source;
    width = (unsigned short) *f_width;
    height = (unsigned short) *f_height;
    logger_log(logger, LOGGER_DEBUG, "begin video stream wxh = %dx%d; source %dx%d", width, height, width_source, height_source);
}

void  video_renderer_init(logger_t *render_logger, const char *server_name, videoflip_t videoflip[2], const char *parser,
                          const char *decoder, const char *converter, const char *videosink, const bool *initial_fullscreen,
                          const bool *video_sync) {

}

void video_renderer_pause() {
    logger_log(logger, LOGGER_DEBUG, "video renderer paused");

}

void video_renderer_resume() {
    if (video_renderer_is_paused()) {
        logger_log(logger, LOGGER_DEBUG, "video renderer resumed");
    }
}

bool video_renderer_is_paused() {
    return 1;
}

void video_renderer_start() {

}

void video_renderer_render_buffer(unsigned char* data, int *data_len, int *nal_count, uint64_t *ntp_time) {


}

void video_renderer_flush() {
}

void video_renderer_stop() {
  if (renderer) {
  }
}

void video_renderer_destroy() {

}

/* not implemented for gstreamer */
void video_renderer_update_background(int type) {
}

unsigned int video_renderer_listen(void *loop) {

}
