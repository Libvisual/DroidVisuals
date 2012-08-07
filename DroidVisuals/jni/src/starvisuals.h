#ifndef __STARVIS
#define __STARVIS

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <jni.h>
#include <time.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <libvisual/libvisual.h>
#include <math.h>
#include <sys/types.h>

#ifdef HAVE_NEON
#   include <arm_neon.h>
#endif

#include "starvisuals.h"

#define DEVICE_DEPTH VISUAL_VIDEO_DEPTH_32BIT

#define  LOG_TAG    "StarVisuals"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

// Initial plugins. Preferences should override these.
#define MORPH "tentacle"
#define ACTOR "lv_gltest"
#define INPUT "debug"

#define URL_GPLv2 "http://www.gnu.org/licenses/gpl-2.0.txt"
#define URL_GPLv3 "http://www.gnu.org/licenses/gpl-3.0.txt"
#define URL_LGPL "http://www.gnu.org/licenses/lgpl-3.0.txt"
#define URL_BSD "http://www.opensource.org/licenses/bsd-license.php"

#define MAX_PCM 1024

struct {
    int16_t pcm_data[MAX_PCM]; // FIXME grow this with audio buffer size taken from java-side. later.
    int size;
    VisAudioSampleRateType rate;
    VisAudioSampleChannelType channels;
    VisAudioSampleFormatType encoding;
} pcm_ref;

/* LIBVISUAL */
struct {
    LV::VideoPtr   video;
    //VisPalette  *pal;
    LV::Bin     *bin;
    pthread_mutex_t mutex;
    const char *actor_name;
    const char *morph_name;
    const char *input_name;
    bool pluginIsGL;
    int is_active;
} v;

#endif
