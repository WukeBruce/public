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

#include <math.h>
#include "audio_renderer.h"
#define SECOND_IN_NSECS 1000000000UL
#define FALSE 0
#define TRUE  1

#define NFORMATS 2     /* set to 4 to enable AAC_LD and PCM:  allowed, but  never seen in real-world use */

static logger_t *logger = NULL;
const char * format[NFORMATS];

static const char *avdec_aac = "avdec_aac";
static const char *avdec_alac = "avdec_alac";
static bool aac = FALSE;
static bool alac = FALSE;
static bool render_audio = FALSE;
static bool async = FALSE;
static bool vsync = FALSE;
static bool sync = FALSE;

typedef struct audio_renderer_s {
    unsigned char ct;
} audio_renderer_t ;
static audio_renderer_t *renderer_type[NFORMATS];
static audio_renderer_t *renderer = NULL;

/* GStreamer Caps strings for Airplay-defined audio compression types (ct) */

/* ct = 1; linear PCM (uncompressed): 44100/16/2, S16LE */
static const char lpcm_caps[]="audio/x-raw,rate=(int)44100,channels=(int)2,format=S16LE,layout=interleaved";

/* ct = 2; codec_data is ALAC magic cookie:  44100/16/2 spf = 352 */
static const char alac_caps[] = "audio/x-alac,mpegversion=(int)4,channnels=(int)2,rate=(int)44100,stream-format=raw,codec_data=(buffer)"
                           "00000024""616c6163""00000000""00000160""0010280a""0e0200ff""00000000""00000000""0000ac44";

/* ct = 4; codec_data from MPEG v4 ISO 14996-3 Section 1.6.2.1:  AAC-LC 44100/2 spf = 1024 */
static const char aac_lc_caps[] ="audio/mpeg,mpegversion=(int)4,channnels=(int)2,rate=(int)44100,stream-format=raw,codec_data=(buffer)1210";

/* ct = 8; codec_data from MPEG v4 ISO 14996-3 Section 1.6.2.1: AAC_ELD 44100/2  spf = 480 */
static const char aac_eld_caps[] ="audio/mpeg,mpegversion=(int)4,channnels=(int)2,rate=(int)44100,stream-format=raw,codec_data=(buffer)f8e85000";

static bool check_plugins (void)
{
    bool ret;

    return ret;
}

static bool check_plugin_feature (const char *needed_feature)
{
    bool ret;
    ret = TRUE;
    return ret;
}

bool gstreamer_init(){
    return (bool) check_plugins ();
}

void audio_renderer_init(logger_t *render_logger, const char* audiosink, const bool* audio_sync, const bool* video_sync) {


}

void audio_renderer_stop() {

}

static void get_renderer_type(unsigned char *ct, int *id) {
    render_audio = FALSE;
    *id = -1;
    for (int i = 0; i < NFORMATS; i++) {
        if (renderer_type[i]->ct == *ct) {
	    *id = i;
            break;
        }
    }
    switch (*id) {
    case 2:
    case 0:
        if (aac) {
            render_audio = TRUE;
        } else {
            logger_log(logger, LOGGER_INFO, "*** GStreamer libav plugin feature avdec_aac is missing, cannot decode AAC audio");
        }
        sync = vsync;
        break;
    case 1:
        if (alac) {
            render_audio = TRUE;
        } else {
            logger_log(logger, LOGGER_INFO, "*** GStreamer libav plugin feature avdec_alac is missing, cannot decode ALAC audio");
        }
        sync = async;
        break;
    case 3:
        render_audio = TRUE;
	sync = FALSE;
        break;
    default:
        break;
    }
}

void  audio_renderer_start(unsigned char *ct) {
    int id = -1;
    get_renderer_type(ct, &id);
}

void audio_renderer_render_buffer(unsigned char* data, int *data_len, unsigned short *seqnum, uint64_t *ntp_time) {
    printf("\n");
}

void audio_renderer_set_volume(double volume) {

}

void audio_renderer_flush() {
}

void audio_renderer_destroy() {
    audio_renderer_stop();

}
