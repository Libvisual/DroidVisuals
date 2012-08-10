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
#include <string>
#include <cstring>

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
#define ACTOR "lv_scope"
#define INPUT "debug"

#define URL_GPLv2 "http://www.gnu.org/licenses/gpl-2.0.txt"
#define URL_GPLv3 "http://www.gnu.org/licenses/gpl-3.0.txt"
#define URL_LGPL "http://www.gnu.org/licenses/lgpl-3.0.txt"
#define URL_BSD "http://www.opensource.org/licenses/bsd-license.php"

#define MAX_PCM 1024

int v_upload_callback (VisInput* input, VisAudio *audio, void* unused);

typedef struct PcmRef_ {
    int16_t pcm_data[MAX_PCM]; // FIXME grow this with audio buffer size taken from java-side. later.
    int size;
    VisAudioSampleRateType rate;
    VisAudioSampleChannelType channels;
    VisAudioSampleFormatType encoding;
} PcmRef;

/* LIBVISUAL */
class V {
    private:
    pthread_mutex_t mutex;
    public:
    LV::Bin     *bin;
    LV::VideoPtr   video;
    //VisPalette  *pal;
    std::string actor_name;
    std::string input_name;
    std::string morph_name;
    bool pluginIsGL;
    int is_active;
    PcmRef pcm_ref;
    V(int w, int h, std::string actor_, std::string input_, std::string morph_, VisBinSwitchStyle switch_style);

    ~V() ;

    void realize();
    void set_actor(std::string act);

    void set_input(std::string inp);
    void set_morph(std::string mor);

    void lock();
    void unlock();
};

LV::PluginRef &get_input(int index);
void v_cycleInput(int prev);
int get_input_index();
void finalizeInput(std::string input);

LV::PluginRef &get_actor(int index);
void v_cycleActor(int prev, bool nogl);
int get_actor_index();
void finalizeActor(std::string actor);

LV::PluginRef &get_morph(int index);
void v_cycleMorph();
int get_input_morph();
void finalizeMorph(std::string morph);

#endif
