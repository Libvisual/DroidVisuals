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

typedef struct PcmRef {
    int16_t pcm_data[MAX_PCM]; // FIXME grow this with audio buffer size taken from java-side. later.
    int size;
    VisAudioSampleRateType rate;
    VisAudioSampleChannelType channels;
    VisAudioSampleFormatType encoding;
};

/* LIBVISUAL */
class V {
    public:
    LV::VideoPtr   video;
    //VisPalette  *pal;
    LV::Bin     *bin;
    pthread_mutex_t mutex;
    std::string actor_name;
    std::string input_name;
    std::string morph_name;
    bool pluginIsGL;
    int is_active;
    PcmRef pcm_ref;
    V(int w, int h, std::string actor_, std::string input_, std::string morph_, VisBinSwitchStyle switch_style)
        {
        int depthflag;
        VisVideoDepth depth;

        pthread_mutex_init(&mutex, NULL);

        actor_name = actor_;
        input_name = input_;
        morph_name = morph_;

        bin = new LV::Bin();
        bin->set_supported_depth(VISUAL_VIDEO_DEPTH_ALL);

        VisActor *actor = visual_actor_new(actor_name.c_str());
        VisInput *input = visual_input_new(input_name.c_str());
        depthflag = visual_actor_get_supported_depth(actor);
    
        if((depthflag == VISUAL_VIDEO_DEPTH_GL))
        {
            
            depth = visual_video_depth_get_highest(depthflag);
        }
        else
        {
            depth = visual_video_depth_get_highest_nogl(depthflag);
        }
    
        bin->set_depth(depth);
    
        bin->switch_set_style(switch_style);
        bin->switch_set_steps (12);
        bin->switch_set_automatic(true);
        bin->switch_set_rate(1.0f);
        bin->switch_set_mode(VISUAL_MORPH_MODE_TIME);

        LV::Time time(1, 10000);
        bin->switch_set_time(time);
    
        video = LV::Video::create(w, h, depth);
    
        bin->set_video(video);
    
        bin->connect(actor, input);
    
        bin->set_morph(morph_);

        bin->realize();
    
        bin->sync(false);
    
        pluginIsGL = (depth == VISUAL_VIDEO_DEPTH_GL);

    }

    ~V() {
        visual_object_unref(VISUAL_OBJECT(bin->get_actor()));
        visual_object_unref(VISUAL_OBJECT(bin->get_input()));
        visual_object_unref(VISUAL_OBJECT(bin->get_morph()));
        video->free_buffer();
        video->unref();
        delete bin;
    }

    void set_actor(std::string act)
    {
        int depthflag;
        VisVideoDepth depth;
        bin->switch_actor(act);
        // handle depth of new actor
        depthflag = visual_actor_get_supported_depth(bin->get_actor());
        if (depthflag == VISUAL_VIDEO_DEPTH_GL)
        {
            bin->set_depth(VISUAL_VIDEO_DEPTH_GL);
        }
        else
        {
            depth = visual_video_depth_get_highest(depthflag);
            if ((bin->get_supported_depth() & depth) > 0)
                bin->set_depth(depth);
            else
                bin->set_depth(visual_video_depth_get_highest_nogl(bin->get_supported_depth()));
        }
        bin->force_actor_depth (bin->get_depth ());
    }

    void set_input(std::string inp)
    {
        VisInput *input = visual_input_new(inp.c_str());
        bin->set_input(input);
    }

    void set_morph(std::string mor)
    {
        bin->set_morph(mor);
    }

};

extern "C" {
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
}

#endif
