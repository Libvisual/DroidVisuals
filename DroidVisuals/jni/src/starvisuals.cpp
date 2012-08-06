
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include <jni.h>
#include <time.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <libvisual/libvisual.h>
//#include <tinyalsa/asoundlib.h>
//#include <asound.h>
#include <math.h>
#include <sys/types.h>

#ifdef HAVE_NEON
#   include <arm_neon.h>
#endif

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
    int do_beat;
    int is_beat;
    int beat_hold;
    int min_beat;
    int stuck_beat;
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

static int my_error_handler (int error, void *priv)
{
    LOGE(visual_error_to_string(error));
    return 0;
}

static void my_log_handler(VisLogSeverity severity, const char *msg, const VisLogSource *source, void *priv)
{
    switch(severity)
    {
        case VISUAL_LOG_DEBUG:
            LOGI("lvDEBUG: (%s) line # %d (%s) : %s", source->file, source->line, source->func, msg);
            break;
        case VISUAL_LOG_INFO:
            LOGI("lvINFO: %s", msg);
            break;
        case VISUAL_LOG_WARNING:
            LOGW("lvWARNING: %s", msg);
            break;
        case VISUAL_LOG_ERROR:
            LOGE("lvERROR: (%s) line # %d (%s) : %s\n", source->file, source->line, source->func, msg);
            break;
        case VISUAL_LOG_CRITICAL:
            LOGE("lvCRITICAL: (%s) line # %d (%s) : %s\n", source->file, source->line, source->func, msg);
            break;
    }

}

void v_cycleActor (int prev)
{
    auto name = prev ? visual_actor_get_prev_by_name(v.actor_name)
                     : visual_actor_get_next_by_name(v.actor_name);

    if (!name) {
        name = prev ? visual_actor_get_prev_by_name(0)
                    : visual_actor_get_next_by_name(0);
    }

    v.actor_name = name;
}


void v_cycleMorph ()
{
    auto name = visual_morph_get_next_by_name(v.morph_name);

    if(!name) {
        name = visual_morph_get_next_by_name(0);
    }

    v.morph_name = name;
}


static void v_cycleInput(int prev)
{
/*
    v.input_name = visual_input_get_next_by_name(v.input_name);
    if(!v.input_name)
    {
        v.input_name = visual_input_get_next_by_name(0);
    }
*/
}

extern "C" {

JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_getIsBeat(JNIEnv *env, jobject obj)
{
    return pcm_ref.is_beat;
}

JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_setDoBeat(JNIEnv *env, jobject obj, jboolean do_beat)
{
    pcm_ref.do_beat = (int)do_beat;
    return 0;
}

JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_setStuckBeat(JNIEnv *env, jobject obj, jboolean stuck_beat)
{
    pcm_ref.stuck_beat = (int)stuck_beat;
    return 0;
}

JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_setMinBeat(JNIEnv *env, jobject obj, jlong timemil)
{
    pcm_ref.min_beat = (int)timemil;
    return 0;
}

JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_setBeatHold(JNIEnv *env, jobject obj, jint timemil)
{
    pcm_ref.beat_hold = (int)timemil;
    return 0;
}


JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_setIsActive(JNIEnv *env, jobject obj, jboolean is_active)
{
    v.is_active = (int)is_active;

    return 0;
}

JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_getIsActive(JNIEnv *env, jobject obj)
{
    return v.is_active;
}

static int v_upload_callback (VisInput* input, VisAudio *audio, void* unused)
{

/*
    static VisTimer *timer = NULL;
    static VisTime *then;
    static VisTime *now;

    if(timer == NULL)
    {
        now = visual_time_new();
        then = visual_time_new();
        timer = visual_timer_new();
        visual_timer_start(timer);
        visual_time_get(then);
    }

    visual_time_get(now);

    visual_return_val_if_fail(input != NULL, VISUAL_ERROR_GENERAL);
    visual_return_val_if_fail(audio != NULL, VISUAL_ERROR_GENERAL);
    visual_return_val_if_fail(pcm_ref.pcm_data != NULL, VISUAL_ERROR_GENERAL);

    VisParamContainer *paramcontainer = visual_plugin_get_params(input->plugin);

    VisBuffer buf;

    visual_buffer_init( &buf, pcm_ref.pcm_data, pcm_ref.size, NULL );
    visual_audio_samplepool_input( audio->samplepool, &buf, pcm_ref.rate, pcm_ref.encoding, pcm_ref.channels);
*/

/*
    if(paramcontainer != NULL && pcm_ref.do_beat)
    {
        VisParamEntry *entry = visual_param_container_get(paramcontainer, "isBeat");
        if(entry == NULL)
        {
            entry = visual_param_entry_new("isBeat");
            visual_param_container_add(paramcontainer, entry);
        }

        unsigned char scaled[pcm_ref.size];
        visual_mem_set(scaled, 0, sizeof(scaled));

        int i, isBeat;

        for(i = 0; i < pcm_ref.size && i < MAX_PCM; i++)
        {
            scaled[i] = pcm_ref.pcm_data[i] / (float)FLT_MAX * UCHAR_MAX;
        }
        isBeat = visual_audio_is_beat_with_data(audio, VISUAL_BEAT_ALGORITHM_PEAK, scaled, MAX_PCM);
        if(visual_timer_elapsed_msecs(timer) > pcm_ref.min_beat && isBeat)
        {
            visual_param_entry_set_integer(entry, isBeat);
            pcm_ref.is_beat = TRUE;
            visual_time_get(then);
        }
        int nowint = visual_time_get_msecs(now);
        int thenint = visual_time_get_msecs(then);
        if(pcm_ref.is_beat && nowint - thenint < pcm_ref.beat_hold)
        {
            visual_param_entry_set_integer(entry, TRUE);
            pcm_ref.is_beat = FALSE;
        }
    }
    */
    return 0;
}


// ---------- INPUT ----------

// Get the VisInput at the requested index.
LV::PluginRef &get_input(int index)
{

    LV::PluginList list = LV::PluginRegistry::instance()->get_plugins_by_type(VISUAL_PLUGIN_TYPE_INPUT);

    int count = list.size();

    if(index >= count)
        return list[0];

    return list[index];
}

int get_input_index()
{
    LV::PluginList list = LV::PluginRegistry::instance()->get_plugins_by_type(VISUAL_PLUGIN_TYPE_INPUT);
    int count = list.size();
    for(int i = 0; i < count; i++)
    {
        LV::PluginRef ref = list[i];
        if(ref.info->plugname && !strcmp(v.input_name, ref.info->plugname))
            return i;
    }
    return -1;

}

void finalizeInput(const char *input)
{

/*
    VisInput *old = visual_bin_get_input(v.bin);

    if(old != NULL)
        visual_object_unref(VISUAL_OBJECT(old));

    VisInput *inp = visual_input_new(input);

    if(strstr(input, "mic"))
    {
    	if (visual_input_set_callback (inp, v_upload_callback, NULL) < 0) {
    	    visual_log(VISUAL_LOG_CRITICAL, "Unable to set mic input callback.");	
    	}
    }

    visual_input_realize(inp);
    visual_bin_set_input(v.bin, inp);
*/
}


JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_cycleInput(JNIEnv *env, jobject obj, jint prev)
{
    v_cycleInput(prev);
    finalizeInput(v.input_name);
    return get_input_index();
}


// Get the count of available input plugins.
JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputCount(JNIEnv *env, jobject obj)
{

    LV::PluginList list = LV::PluginRegistry::instance()->get_plugins_by_type(VISUAL_PLUGIN_TYPE_INPUT);
    return list.size();
}

// Get the index of the current plugin. 
// Note that this index may change as new plugins are added.
JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputGetCurrent(JNIEnv *env)
{
    return get_input_index();
}


// Set the current input plugin to that at the provided index.
// Note that this does not immediately cause the plugin to change.
// It only sets the name for when the plugin does change.
// This name could change between calling this function and an actual plugin change!
JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputSetCurrent(JNIEnv *env, jobject obj, jint index, jboolean now)
{
    LV::PluginList list = LV::PluginRegistry::instance()->get_plugins_by_type(VISUAL_PLUGIN_TYPE_INPUT);
    int count = list.size();

    if(index >= count)
        return FALSE;
    
    LV::PluginRef ref = list[index];

    v.input_name = ref.info->plugname;

    if(now)
        finalizeInput(v.input_name);

    return TRUE;
}

// Set the current input plugin by its name. Do nothing and return false if the plugin doesn't exist.
JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputSetCurrentByName(JNIEnv *env, jobject obj, jstring name, jboolean now)
{
    jboolean iscopy;
    const char *input = env->GetStringUTFChars(name, &iscopy);
    v.input_name = input;
    if(now)
        finalizeInput(input);
    return TRUE;
}


// Get the input's plugin name.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputGetName(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_input(index);

    return env->NewStringUTF(ref.info->plugname);
}

// Get the input's plugin longname.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputGetLongName(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_input(index);

    return env->NewStringUTF(ref.info->name);
}

// Get the input's plugin author.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputGetAuthor(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_input(index);

    return env->NewStringUTF(ref.info->author);
}

// Get the input's plugin version.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputGetVersion(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_input(index);

    return env->NewStringUTF(ref.info->version);
}

// Get the input's plugin about string.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputGetAbout(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_input(index);

    return env->NewStringUTF(ref.info->about);
}

// Get the input's plugin help string.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputGetHelp(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_input(index);

    return env->NewStringUTF(ref.info->help);
}

// Get the input's plugin license string.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputGetLicense(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_input(index);

    char text[256];

    const char *license = ref.info->license;

    visual_mem_set(text, 0, sizeof(text));

    if(strcmp(license, "GPLv2"))
        visual_mem_copy(text, URL_GPLv2, strlen(URL_GPLv2));
    else if(strcmp(license, "GPLv3"))
        visual_mem_copy(text, URL_GPLv3, strlen(URL_GPLv3));
    else if(strcmp(license, "LGPL"))
        visual_mem_copy(text, URL_LGPL, strlen(URL_LGPL));
    else if(strcmp(license, "BSD"))
        visual_mem_copy(text, URL_BSD, strlen(URL_BSD));

    return env->NewStringUTF(text);

}

/*
VisParamEntry *get_input_param_entry(int index)
{
    VisParamContainer *cont = visual_plugin_get_params(visual_input_get_plugin(v.bin->get_input()));

    visual_return_val_if_fail(cont != NULL, NULL);

    int count = visual_list_count(cont->entries);

    visual_return_val_if_fail(index < count, NULL);

    VisParamEntry *entry = (VisParamEntry *)visual_list_get(cont->entries, index);

    visual_return_val_if_fail(entry != NULL, NULL);

    return entry;
}

JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputParamGetCount(JNIEnv *env, jobject obj)
{
    VisParamContainer *cont = visual_plugin_get_params(visual_input_get_plugin(v.bin->get_input()));

    visual_return_val_if_fail(cont != NULL, 0);

    int count = visual_list_count(cont->entries);

    return count;
}

JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputParamGetType(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *chars = env->GetStringUTFChars(name, &iscopy);

    jstring string;

    VisParamContainer *cont = visual_plugin_get_params(visual_input_get_plugin(v.bin->get_input()));

    VisParamEntry *entry = visual_param_container_get(cont, chars);

    visual_return_val_if_fail(entry != NULL, NULL);

    switch(entry->type)
    {
        case VISUAL_PARAM_ENTRY_TYPE_STRING:
            string = env->NewStringUTF("string");
            break;
        case VISUAL_PARAM_ENTRY_TYPE_INTEGER:
            string = env->NewStringUTF("integer");
            break;
        case VISUAL_PARAM_ENTRY_TYPE_FLOAT:
            string = env->NewStringUTF("float");
            break;
        case VISUAL_PARAM_ENTRY_TYPE_DOUBLE:
            string = env->NewStringUTF("double");
            break;
        default:
            return NULL;
            break;
    }

    return string;
}

JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputParamGetName(JNIEnv *env, jobject obj, jint index)
{
    VisParamEntry *entry = get_input_param_entry(index);
    
    jstring string = env->NewStringUTF(visual_param_entry_get_name(entry));
    
    return string;
}

JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputParamGetString(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *chars = env->GetStringUTFChars(name, &iscopy);
    visual_return_val_if_fail(chars != NULL, NULL);

    VisParamContainer *cont = visual_plugin_get_params(visual_input_get_plugin(v.bin->get_input()));

    VisParamEntry *entry = visual_param_container_get(cont, chars);

    jstring string = env->NewStringUTF(visual_param_entry_get_string(entry));

    return string;
}

JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputParamSetString(JNIEnv *env, jobject obj, jstring name, jstring newstring)
{
    jboolean iscopy;
    const char *param_name = env->GetStringUTFChars(name, &iscopy);
    const char *new_string = env->GetStringUTFChars(newstring, &iscopy);

    visual_return_val_if_fail(param_name != NULL, FALSE);
    visual_return_val_if_fail(new_string != NULL, FALSE);

    VisParamContainer *cont = visual_plugin_get_params(visual_input_get_plugin(v.bin->get_input()));

    VisParamEntry *entry = visual_param_container_get(cont, param_name);

    int ret = visual_param_entry_set_string(entry, (char *)new_string);

    return !ret;
}

JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputParamGetInteger(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *string = env->GetStringUTFChars(name, &iscopy);
    visual_return_val_if_fail(string != NULL, 0);

    VisParamContainer *cont = visual_plugin_get_params(visual_input_get_plugin(v.bin->get_input()));

    VisParamEntry *entry = visual_param_container_get(cont, string);

    int val = visual_param_entry_get_integer(entry);

    return val;
}

JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputParamSetInteger(JNIEnv *env, jobject obj, jstring name, jint newint)
{
    jboolean iscopy;
    const char *param_name = env->GetStringUTFChars(name, &iscopy);
    int new_int = newint;

    visual_return_val_if_fail(param_name != NULL, FALSE);

    VisParamContainer *cont = visual_plugin_get_params(visual_input_get_plugin(v.bin->get_input()));

    VisParamEntry *entry = visual_param_container_get(cont, param_name);

    int ret = visual_param_entry_set_integer(entry, new_int);

    return !ret;
}

JNIEXPORT jfloat JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputParamGetFloat(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *string = env->GetStringUTFChars(name, &iscopy);
    visual_return_val_if_fail(string != NULL, 0.0f);

    VisParamContainer *cont = visual_plugin_get_params(visual_input_get_plugin(v.bin->get_input()));

    VisParamEntry *entry = visual_param_container_get(cont, string);

    float val = visual_param_entry_get_float(entry);

    return val;
}

JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputParamSetFloat(JNIEnv *env, jobject obj, jstring name, jfloat newfloat)
{
    jboolean iscopy;
    const char *param_name = env->GetStringUTFChars(name, &iscopy);
    float new_float = newfloat;

    visual_return_val_if_fail(param_name != NULL, FALSE);

    VisParamContainer *cont = visual_plugin_get_params(visual_input_get_plugin(v.bin->get_input()));

    VisParamEntry *entry = visual_param_container_get(cont, param_name);

    int ret = visual_param_entry_set_float(entry, new_float);

    return !ret;
}

JNIEXPORT jdouble JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputParamGetDouble(JNIEnv *env, jobject obj, jstring name)
{    
    jboolean iscopy;
    const char *string = env->GetStringUTFChars(name, &iscopy);

    visual_return_val_if_fail(string != NULL, 0.0);

    VisParamContainer *cont = visual_plugin_get_params(visual_input_get_plugin(v.bin->get_input()));

    VisParamEntry *entry = visual_param_container_get(cont, string);

    double val = visual_param_entry_get_double(entry);

    return val;
}

JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputParamSetDouble(JNIEnv *env, jobject obj, jstring name, jdouble newdouble)
{
    jboolean iscopy;
    const char *param_name = env->GetStringUTFChars(name, &iscopy);
    double new_double = newdouble;

    visual_return_val_if_fail(param_name != NULL, FALSE);

    VisParamContainer *cont = visual_plugin_get_params(visual_input_get_plugin(v.bin->get_input()));

    VisParamEntry *entry = visual_param_container_get(cont, param_name);

    int ret = visual_param_entry_set_double(entry, new_double);

    return !ret;
}
*/

// ------ MORPH ------

// Get the VisMorph at the requested index.
LV::PluginRef &get_morph(int index)
{
    LV::PluginList list = LV::PluginRegistry::instance()->get_plugins_by_type(VISUAL_PLUGIN_TYPE_MORPH);

    return list[index];
}

int get_morph_index()
{
    LV::PluginList list = LV::PluginRegistry::instance()->get_plugins_by_type(VISUAL_PLUGIN_TYPE_MORPH);
    int count = list.size();
    for(int i = 0; i < count; i++)
    {
        LV::PluginRef ref = list[i];
        if(ref.info->plugname && !strcmp(v.morph_name, ref.info->plugname))
            return i;
    }
    return -1;
}

static void finalizeMorph(const char *morph)
{

}

#if 0
JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_cycleMorph(JNIEnv *env, jobject obj, jint prev)
{
    pthread_mutex_lock(&v.mutex);

    v_cycleMorph(prev);
    finalizeMorph(v.morph_name);

    pthread_mutex_unlock(&v.mutex);

    return get_morph_index();
}


// Get the count of available morph plugins.
JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphCount(JNIEnv *env, jobject obj)
{
    LV::PluginList list = LV::PluginRegistry::instance()->get_plugins_by_type(VISUAL_PLUGIN_TYPE_MORPH);
    return list.size();
}

// Get the count of available morph plugins.
JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_setMorphSteps(JNIEnv *env, jobject obj, jint steps)
{
    return visual_bin_switch_set_steps(v.bin, (int)steps);
}

// Get the index of the current plugin. 
// Note that this index may change as new plugins are added.
JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphGetCurrent(JNIEnv *env)
{
    VisMorph *morph = visual_bin_get_morph(v.bin);
    if(morph)
        return morph->plugin->ref->index;
    return -1;
}


// Set the current morph plugin to that at the provided index.
// Note that this does not immediately cause the plugin to change.
// It only sets the name for when the plugin does change.
// This name could change between calling this function and an actual plugin change!
JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphSetCurrent(JNIEnv *env, jobject obj, jint index, jboolean now)
{
    VisList *list = visual_morph_get_list();

    int count = visual_list_count(list);

    if(index >= count)
        return FALSE;
    
    LV::PluginRef ref = visual_list_get(list, index);

    v.morph_name = ref->info->plugname;

    if(now)
        finalizeMorph(v.morph_name);

    return TRUE;
}

// Set the current morph by name. use finalizeSwitch() to apply this change.
JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphSetCurrentByName(JNIEnv *env, jobject obj, jstring name, jboolean now)
{
    jboolean iscopy;
    const char *morph = env->GetStringUTFChars(name, &iscopy);
    if(visual_morph_valid_by_name(morph))
    {
        v.morph_name = morph;
        if(now)
        {
            finalizeMorph(morph);
        }
        return TRUE;
    }
    return FALSE;
}

// Get the morph plugin's name string.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphGetName(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_morph(index);

    return env->NewStringUTF(ref->info->plugname);
}

// Get the morph plugin's long name string.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphGetLongName(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_morph(index);

    return env->NewStringUTF(ref->info->name);
}

// Get the morph plugin's author string.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphGetAuthor(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_morph(index);

    return env->NewStringUTF(ref->info->author);
}

// Get the morph plugin's version string.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphGetVersion(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_morph(index);

    return env->NewStringUTF(ref->info->version);

}

// Get the morph plugin's about string.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphGetAbout(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_morph(index);

    return env->NewStringUTF(ref->info->about);
}

// Get the morph plugin's help string.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphGetHelp(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_morph(index);

    return env->NewStringUTF(ref->info->help);
}

// Get the morph plugin's license string.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphGetLicense(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_morph(index);

    char text[256];

    const char *license = ref->info->license;

    visual_mem_set(text, 0, sizeof(text));

    if(strcmp(license, "GPLv2"))
        visual_mem_copy(text, URL_GPLv2, strlen(URL_GPLv2));
    else if(strcmp(license, "GPLv3"))
        visual_mem_copy(text, URL_GPLv3, strlen(URL_GPLv3));
    else if(strcmp(license, "LGPL"))
        visual_mem_copy(text, URL_LGPL, strlen(URL_LGPL));
    else if(strcmp(license, "BSD"))
        visual_mem_copy(text, URL_BSD, strlen(URL_BSD));

    return env->NewStringUTF(text);

}

VisParamEntry *get_morph_param_entry(int index)
{
    VisParamContainer *cont = visual_plugin_get_params(visual_morph_get_plugin(v.bin->morph));

    visual_return_val_if_fail(cont != NULL, NULL);

    int count = visual_list_count(cont->entries);

    visual_return_val_if_fail(index < count, NULL);

    VisParamEntry *entry = (VisParamEntry *)visual_list_get(cont->entries, index);

    visual_return_val_if_fail(entry != NULL, NULL);

    return entry;
}

JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphParamGetCount(JNIEnv *env, jobject obj)
{
    VisParamContainer *cont = visual_plugin_get_params(visual_morph_get_plugin(v.bin->morph));

    visual_return_val_if_fail(cont != NULL, 0);

    int count = visual_list_count(cont->entries);

    return count;
}

JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphParamGetName(JNIEnv *env, jobject obj, jint index)
{
    VisParamEntry *entry = get_morph_param_entry(index);
    
    jstring string = env->NewStringUTF(visual_param_entry_get_name(entry));
    
    return string;
}

JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphParamGetType(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *chars = env->GetStringUTFChars(name, &iscopy);

    jstring string;

    VisParamContainer *cont = visual_plugin_get_params(visual_morph_get_plugin(v.bin->morph));

    VisParamEntry *entry = visual_param_container_get(cont, chars);

    visual_return_val_if_fail(entry != NULL, NULL);

    switch(entry->type)
    {
        case VISUAL_PARAM_ENTRY_TYPE_STRING:
            string = env->NewStringUTF("string");
            break;
        case VISUAL_PARAM_ENTRY_TYPE_INTEGER:
            string = env->NewStringUTF("integer");
            break;
        case VISUAL_PARAM_ENTRY_TYPE_FLOAT:
            string = env->NewStringUTF("float");
            break;
        case VISUAL_PARAM_ENTRY_TYPE_DOUBLE:
            string = env->NewStringUTF("double");
            break;
        default:
            return NULL;
            break;
    }

    return string;
}


JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphParamGetString(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *chars = env->GetStringUTFChars(name, &iscopy);
    visual_return_val_if_fail(chars != NULL, NULL);

    VisParamContainer *cont = visual_plugin_get_params(visual_morph_get_plugin(v.bin->morph));

    VisParamEntry *entry = visual_param_container_get(cont, chars);

    jstring string = env->NewStringUTF(visual_param_entry_get_string(entry));

    return string;
}

JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphParamSetString(JNIEnv *env, jobject obj, jstring name, jstring newstring)
{
    jboolean iscopy;
    const char *param_name = env->GetStringUTFChars(name, &iscopy);
    const char *new_string = env->GetStringUTFChars(newstring, &iscopy);

    visual_return_val_if_fail(param_name != NULL, FALSE);
    visual_return_val_if_fail(new_string != NULL, FALSE);

    VisParamContainer *cont = visual_plugin_get_params(visual_morph_get_plugin(v.bin->morph));

    VisParamEntry *entry = visual_param_container_get(cont, param_name);

    int ret = visual_param_entry_set_string(entry, (char *)new_string);

    return !ret;
}

JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphParamGetInteger(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *string = env->GetStringUTFChars(name, &iscopy);
    visual_return_val_if_fail(string != NULL, 0);

    VisParamContainer *cont = visual_plugin_get_params(visual_morph_get_plugin(v.bin->morph));

    VisParamEntry *entry = visual_param_container_get(cont, string);

    int val = visual_param_entry_get_integer(entry);

    return val;
}

JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphParamSetInteger(JNIEnv *env, jobject obj, jstring name, jint newint)
{
    jboolean iscopy;
    const char *param_name = env->GetStringUTFChars(name, &iscopy);
    int new_int = newint;

    visual_return_val_if_fail(param_name != NULL, FALSE);

    VisParamContainer *cont = visual_plugin_get_params(visual_morph_get_plugin(v.bin->morph));

    VisParamEntry *entry = visual_param_container_get(cont, param_name);

    int ret = visual_param_entry_set_integer(entry, new_int);

    return !ret;
}

JNIEXPORT jfloat JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphParamGetFloat(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *string = env->GetStringUTFChars(name, &iscopy);
    visual_return_val_if_fail(string != NULL, 0.0f);

    VisParamContainer *cont = visual_plugin_get_params(visual_morph_get_plugin(v.bin->morph));

    VisParamEntry *entry = visual_param_container_get(cont, string);

    float val = visual_param_entry_get_float(entry);

    return val;
}

JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphParamSetFloat(JNIEnv *env, jobject obj, jstring name, jfloat newfloat)
{
    jboolean iscopy;
    const char *param_name = env->GetStringUTFChars(name, &iscopy);
    float new_float = newfloat;

    visual_return_val_if_fail(param_name != NULL, FALSE);

    VisParamContainer *cont = visual_plugin_get_params(visual_morph_get_plugin(v.bin->morph));

    VisParamEntry *entry = visual_param_container_get(cont, param_name);

    int ret = visual_param_entry_set_float(entry, new_float);

    return !ret;
}

JNIEXPORT jdouble JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphParamGetDouble(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *string = env->GetStringUTFChars(name, &iscopy);
    visual_return_val_if_fail(string != NULL, 0.0);

    VisParamContainer *cont = visual_plugin_get_params(visual_morph_get_plugin(v.bin->morph));

    VisParamEntry *entry = visual_param_container_get(cont, string);

    double val = visual_param_entry_get_double(entry);

    return val;
}

JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_morphParamSetDouble(JNIEnv *env, jobject obj, jstring name, jdouble newdouble)
{
    jboolean iscopy;
    const char *param_name = env->GetStringUTFChars(name, &iscopy);
    double new_double = newdouble;

    visual_return_val_if_fail(param_name != NULL, FALSE);

    VisParamContainer *cont = visual_plugin_get_params(visual_morph_get_plugin(v.bin->morph));

    VisParamEntry *entry = visual_param_container_get(cont, param_name);

    int ret = visual_param_entry_set_double(entry, new_double);

    return !ret;
}
#endif

// ------ ACTORS ------

// Get the VisActor at the requested index.
LV::PluginRef &get_actor(int index)
{
    LV::PluginList list = LV::PluginRegistry::instance()->get_plugins_by_type(VISUAL_PLUGIN_TYPE_ACTOR);

    int count = list.size();

    if(index > 0 && index >= count)
        return list[0];

    return list[index];
}

int get_actor_index()
{
    LV::PluginList list = LV::PluginRegistry::instance()->get_plugins_by_type(VISUAL_PLUGIN_TYPE_ACTOR);
    for(unsigned int i = 0; i < list.size(); i++)
    {
        LV::PluginRef ref = list[i];
        if(ref.info->plugname && !strcmp(v.actor_name, ref.info->plugname))
            return i;
    }
    return -1;

}

void finalizeActor(const char *actor)
{
    pthread_mutex_lock(&v.mutex);
    v.bin->switch_actor((char *)actor);
    pthread_mutex_unlock(&v.mutex);
}

JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_cycleActor(JNIEnv *env, jobject obj, jint prev)
{
    v_cycleActor(prev);
    finalizeActor(v.actor_name);
    return get_actor_index();
}

// Get the count of available actor plugins.
JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorCount(JNIEnv *env, jobject obj)
{
    LV::PluginList list = LV::PluginRegistry::instance()->get_plugins_by_type(VISUAL_PLUGIN_TYPE_ACTOR);
    return list.size();
}

// Get the index of the current plugin. 
// Note that this index may change as new plugins are added.
JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorGetCurrent(JNIEnv *env)
{
    return get_actor_index();
    /*
    VisList *list = visual_actor_get_list();
    int count = visual_list_count(list), i;
    for(i = 0; i < count; i++)
    {
        LV::PluginRef ref = visual_list_get(list, i);
        if(ref->info->plugname && !strcmp(v.actor_name, ref->info->plugname))
            return i;
    }
    return -1;
    */
}


// Set the current actor plugin to that at the provided index.
// Note that this does not immediately cause the plugin to change.
// It only sets the name for when the plugin does change.
// This name could change between calling this function and an actual plugin change!
JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorSetCurrent(JNIEnv *env, jobject obj, jint index, jboolean now)
{
    LV::PluginList list = LV::PluginRegistry::instance()->get_plugins_by_type(VISUAL_PLUGIN_TYPE_ACTOR);
    int count = list.size();

    if(index >= count)
        return FALSE;
    
    LV::PluginRef ref = list[index];

    v.actor_name = ref.info->plugname;

    if(now)
        finalizeActor(v.actor_name);

    return TRUE;
}

// Set the current actor by its name.
JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorSetCurrentByName(JNIEnv *env, jobject obj, jstring name, jboolean now)
{
    jboolean iscopy;
    const char *actor = env->GetStringUTFChars(name, &iscopy);
    v.actor_name = actor;
    if(now)
        finalizeActor(actor);
    return TRUE;
}


// Get the actor's plugin name.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorGetName(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_actor(index);

    return env->NewStringUTF(ref.info->plugname);
}

#if 0
// Get the actor's long name.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorGetLongName(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_actor(index);
    const char *name;

    visual_return_val_if_fail(ref != NULL, NULL);
    name = ref.info->name;

    return env->NewStringUTF((char *)name);
}

// Get the actor's author.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorGetAuthor(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_actor(index);

    visual_return_val_if_fail(ref != NULL, NULL);

    return env->NewStringUTF(ref.info->author);
}

// Get the actor's version string.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorGetVersion(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_actor(index);

    visual_return_val_if_fail(ref != NULL, NULL);

    return env->NewStringUTF(ref->get_info()->get_version());
}

// Get the actor's about string.
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorGetAbout(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_actor(index);

    visual_return_val_if_fail(ref != NULL, NULL);

    return env->NewStringUTF(ref.info->about);
}

// Get the actor's help string.
/*
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorGetHelp(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_actor(index);

    visual_return_val_if_fail(ref != NULL, NULL);

    return env->NewStringUTF(ref.info->help);
}
*/

// Get the actor's license string.
/*
JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorGetLicense(JNIEnv *env, jobject obj, jint index)
{
    LV::PluginRef ref = get_actor(index);
    char text[256];

    visual_return_val_if_fail(ref != NULL, NULL);

    const char *license = ref.info->license;

    visual_mem_set(text, 0, sizeof(text));

    if(strcmp(license, "GPLv2"))
        visual_mem_copy(text, URL_GPLv2, strlen(URL_GPLv2));
    else if(strcmp(license, "GPLv3"))
        visual_mem_copy(text, URL_GPLv3, strlen(URL_GPLv3));
    else if(strcmp(license, "LGPL"))
        visual_mem_copy(text, URL_LGPL, strlen(URL_LGPL));
    else if(strcmp(license, "BSD"))
        visual_mem_copy(text, URL_BSD, strlen(URL_BSD));

    return env->NewStringUTF(text);
}
*/

JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorGetParamsCount(JNIEnv *env, jobject obj)
{
    VisParamContainer *params = visual_plugin_get_params(visual_actor_get_plugin(v.bin->get_actor()));

    int count = visual_list_count(params->entries);
    
    return count;
}

VisParamEntry *get_actor_param_entry(int index)
{
    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v.bin->get_actor()));

    visual_return_val_if_fail(cont != NULL, NULL);

    int count = visual_list_count(cont->entries);

    visual_return_val_if_fail(index < count, NULL);

    VisParamEntry *entry = (VisParamEntry *)visual_list_get(cont->entries, index);

    visual_return_val_if_fail(entry != NULL, NULL);

    return entry;
}

JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorParamGetCount(JNIEnv *env, jobject obj)
{
    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v.bin->get_actor()));

    visual_return_val_if_fail(cont != NULL, 0);

    int count = visual_list_count(cont->entries);

    return count;
}

JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorParamGetType(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *chars = env->GetStringUTFChars(name, &iscopy);

    jstring string;

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v.bin->get_actor()));

    VisParamEntry *entry = visual_param_container_get(cont, chars);

    visual_return_val_if_fail(entry != NULL, NULL);

    switch(entry->type)
    {
        case VISUAL_PARAM_ENTRY_TYPE_STRING:
            string = env->NewStringUTF("string");
            break;
        case VISUAL_PARAM_ENTRY_TYPE_INTEGER:
            string = env->NewStringUTF("integer");
            break;
        case VISUAL_PARAM_ENTRY_TYPE_FLOAT:
            string = env->NewStringUTF("float");
            break;
        case VISUAL_PARAM_ENTRY_TYPE_DOUBLE:
            string = env->NewStringUTF("double");
            break;
        default:
            return NULL;
            break;
    }

    return string;
}

JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorParamGetName(JNIEnv *env, jobject obj, jint index)
{
    VisParamEntry *entry = get_actor_param_entry(index);
    
    jstring string = env->NewStringUTF(visual_param_entry_get_name(entry));
    
    return string;
}

JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorParamGetString(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *chars = env->GetStringUTFChars(name, &iscopy);
    visual_return_val_if_fail(chars != NULL, NULL);

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v.bin->get_actor()));

    VisParamEntry *entry = visual_param_container_get(cont, chars);

    jstring string = env->NewStringUTF(visual_param_entry_get_string(entry));

    return string;
}

JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorParamSetString(JNIEnv *env, jobject obj, jstring name, jstring newstring)
{
    jboolean iscopy;
    const char *param_name = env->GetStringUTFChars(name, &iscopy);
    const char *new_string = env->GetStringUTFChars(newstring, &iscopy);

    visual_return_val_if_fail(param_name != NULL, FALSE);
    visual_return_val_if_fail(new_string != NULL, FALSE);

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v.bin->get_actor()));

    VisParamEntry *entry = visual_param_container_get(cont, param_name);

    int ret = visual_param_entry_set_string(entry, (char *)new_string);

    return !ret;
}


JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorParamGetInteger(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *string = env->GetStringUTFChars(name, &iscopy);
    visual_return_val_if_fail(string != NULL, 0);

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v.bin->get_actor()));

    VisParamEntry *entry = visual_param_container_get(cont, string);

    int val = visual_param_entry_get_integer(entry);

    return val;
}

JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorParamSetInteger(JNIEnv *env, jobject obj, jstring name, jint newint)
{
    jboolean iscopy;
    const char *param_name = env->GetStringUTFChars(name, &iscopy);
    int new_int = newint;

    visual_return_val_if_fail(param_name != NULL, FALSE);

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v.bin->get_actor()));

    VisParamEntry *entry = visual_param_container_get(cont, param_name);

    int ret = visual_param_entry_set_integer(entry, new_int);

    return !ret;
}

JNIEXPORT jfloat JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorParamGetFloat(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *string = env->GetStringUTFChars(name, &iscopy);
    visual_return_val_if_fail(string != NULL, 0.0f);

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v.bin->get_actor()));

    VisParamEntry *entry = visual_param_container_get(cont, string);

    float val = visual_param_entry_get_float(entry);

    return val;
}

JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorParamSetFloat(JNIEnv *env, jobject obj, jstring name, jfloat newfloat)
{
    jboolean iscopy;
    const char *param_name = env->GetStringUTFChars(name, &iscopy);
    float new_float = newfloat;

    visual_return_val_if_fail(param_name != NULL, FALSE);

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v.bin->get_actor()));

    VisParamEntry *entry = visual_param_container_get(cont, param_name);

    float ret = visual_param_entry_set_float(entry, new_float);

    return !ret;
}

JNIEXPORT jdouble JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorParamGetDouble(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *string = env->GetStringUTFChars(name, &iscopy);
    visual_return_val_if_fail(string != NULL, 0.0);

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v.bin->get_actor()));

    VisParamEntry *entry = visual_param_container_get(cont, string);

    double val = visual_param_entry_get_double(entry);

    return val;
}

JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorParamSetDouble(JNIEnv *env, jobject obj, jstring name, jdouble newdouble)
{
    jboolean iscopy;
    const char *param_name = env->GetStringUTFChars(name, &iscopy);
    double new_double = newdouble;

    visual_return_val_if_fail(param_name != NULL, FALSE);

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v.bin->get_actor()));

    VisParamEntry *entry = visual_param_container_get(cont, param_name);

    double ret = visual_param_entry_set_double(entry, new_double);

    return !ret;
}
#endif


/* End of plugin and parameter getters and setters. */



// For fallback audio source.
JNIEXPORT void JNICALL Java_net_starlon_droidvisuals_NativeHelper_uploadAudio(JNIEnv * env, jobject  obj, jshortArray data)
{
    jshort *pcm;
    jsize len = env->GetArrayLength(data);
    pcm = env->GetShortArrayElements(data, NULL);
    for(unsigned int i = 0; i < (unsigned int)len && i < (int)pcm_ref.size / sizeof(int16_t); i++)
    {
        pcm_ref.pcm_data[i] = pcm[i];
    }
    env->ReleaseShortArrayElements(data, pcm, 0);
}

// Reinitialize audio fields.
JNIEXPORT void JNICALL Java_net_starlon_droidvisuals_NativeHelper_resizePCM(jint size, jint samplerate, jint channels, jint encoding)
{
    //if(pcm_ref.pcm_data != NULL)
    //    visual_mem_free(pcm_ref.pcm_data);
    pcm_ref.size = MAX_PCM;//size;
    //pcm_ref.pcm_data = visual_mem_malloc(pcm_ref.size * sizeof(int16_t));

    //D/StarVisuals/StarVisualsActivity( 1102): Opened mic: 44100Hz, bits: 2, channel: 12, buffersize:8192
    switch(samplerate)
    {
        case 8000:
            pcm_ref.rate = VISUAL_AUDIO_SAMPLE_RATE_8000;
        break;
        case 11250:
            pcm_ref.rate = VISUAL_AUDIO_SAMPLE_RATE_11250;
        break;
        case 22500:
            pcm_ref.rate = VISUAL_AUDIO_SAMPLE_RATE_22500;
        break;
        case 32000:
            pcm_ref.rate = VISUAL_AUDIO_SAMPLE_RATE_32000;
        break;
        case 44100:
            pcm_ref.rate = VISUAL_AUDIO_SAMPLE_RATE_44100;
        break;
        case 48000:
            pcm_ref.rate = VISUAL_AUDIO_SAMPLE_RATE_48000;
        break;
        case 96000:
            pcm_ref.rate = VISUAL_AUDIO_SAMPLE_RATE_96000;
        break;
        default:
            pcm_ref.rate = VISUAL_AUDIO_SAMPLE_RATE_44100;
        break;
    }

    // It seems lv only supports stereo? hmm
    pcm_ref.channels = VISUAL_AUDIO_SAMPLE_CHANNEL_STEREO;

    // According to documentation 16BIT sample size is guaranteed to be supported.
    pcm_ref.encoding = VISUAL_AUDIO_SAMPLE_FORMAT_S16;
}

// Increment or decrement actor and morph
// Variable 'prev' is used to shift morph plugin around. 
// 0=left, 1=right, 2=up, 3=down, 4=cycle.. Any other and the current value is used.
JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_finalizeSwitch(JNIEnv * env, jobject  obj, jint prev)
{

    pthread_mutex_lock(&v.mutex);

    VisMorph *bin_morph = visual_bin_get_morph(v.bin);
    const char *morph = v.morph_name;

    
    if(bin_morph && !visual_morph_is_done(bin_morph))
        return FALSE;

    switch(prev)
    {
        case 1: v.morph_name = "slide_left"; break;
        case -1: v.morph_name = "slide_right"; break;
        case 2: v.morph_name = "slide_top"; break;
        case -2: v.morph_name = "slide_bottom"; break;
        case 0: prev = 1; break;
        default: v.morph_name = MORPH; break;
    }

    visual_log(VISUAL_LOG_INFO, "Switching actors %s -> %s", morph, v.morph_name);

    v.morph_name = "slide_up";
    v_cycleActor(prev);
    //v.bin->set_morph(v.morph_name);

    v.bin->switch_actor(v.actor_name);
    pthread_mutex_unlock(&v.mutex);
    return TRUE;
}

// Set the VisBin's morph style -- to morph or not to morph.
JNIEXPORT void JNICALL Java_net_starlon_droidvisuals_NativeHelper_setMorphStyle(JNIEnv * env, jobject  obj, jboolean morph)
{
    if(morph)
        visual_bin_switch_set_style(v.bin, VISUAL_SWITCH_STYLE_MORPH);
    else
        visual_bin_switch_set_style(v.bin, VISUAL_SWITCH_STYLE_DIRECT);
}

// Pump mouse motion events to the current actor.
JNIEXPORT void JNICALL Java_net_starlon_droidvisuals_NativeHelper_mouseMotion(JNIEnv * env, jobject  obj, jfloat x, jfloat y)
{
    visual_log(VISUAL_LOG_INFO, "Mouse motion: x %f, y %f", x, y);
    VisPluginData *plugin = visual_actor_get_plugin(visual_bin_get_actor(v.bin));
    VisEventQueue *eventqueue = visual_plugin_get_eventqueue(plugin);
    VisEvent *event = visual_event_new_mousemotion(x, y);
    eventqueue->add(*event);
}

// Pump mouse button events to the current actor.
JNIEXPORT void JNICALL Java_net_starlon_droidvisuals_NativeHelper_mouseButton(JNIEnv * env, jobject  obj, jint button, jfloat x, jfloat y)
{
    visual_log(VISUAL_LOG_INFO, "Mouse button: button %d, x %f, y %f", button, x, y);
    VisPluginData *plugin = visual_actor_get_plugin(visual_bin_get_actor(v.bin));
    VisEventQueue *eventqueue = visual_plugin_get_eventqueue(plugin);
    VisMouseState state = VISUAL_MOUSE_DOWN;
    VisEvent *event = visual_event_new_mousebutton(button, state, x, y);
    eventqueue->add(*event);
}


// Pump resize events to the current actor.
JNIEXPORT void JNICALL Java_net_starlon_droidvisuals_NativeHelper_screenResize(JNIEnv * env, jobject  obj, jint w, jint h)
{
    visual_log(VISUAL_LOG_INFO, "Screen resize w %d h %d", w, h);

    VisPluginData *plugin = visual_actor_get_plugin(visual_bin_get_actor(v.bin));
    VisEventQueue *eventqueue = visual_plugin_get_eventqueue(plugin);
    if(v.video->has_allocated_buffer())
        v.video->free_buffer();
    v.video->unref();
    v.video = LV::Video::create(w, h, v.bin->get_depth());
    VisEvent *event = visual_event_new_resize(w, h);
    eventqueue->add(*event);
}

// Pump keyboard events to the current actor.
JNIEXPORT void JNICALL Java_net_starlon_droidvisuals_NativeHelper_keyboardEvent(JNIEnv * env, jobject  obj, jint x, jint y)
{
    VisEventQueue *eventqueue = visual_plugin_get_eventqueue(visual_actor_get_plugin(visual_bin_get_actor(v.bin)));
    VisKey keysym = (VisKey)0;
    int keymod = 0;
    VisKeyState state = (VisKeyState)0;
    VisEvent *event = visual_event_new_keyboard(keysym, keymod, state);
    eventqueue->add(*event);
}

JNIEXPORT void JNICALL Java_net_starlon_droidvisuals_NativeHelper_visualsQuit(JNIEnv * env, jobject  obj, jboolean toExit)
{

    if(visual_is_initialized())
        visual_quit();
}

void app_main(int w, int h, const char *actor_, const char *input_, const char *morph_)
{

    usleep(333333); 
    int depthflag;
    VisVideoDepth depth;

    if(!visual_is_initialized())
    {
        visual_log_set_verbosity (VISUAL_LOG_DEBUG);
        visual_error_set_handler(my_error_handler, NULL);
        visual_log_set_handler(VISUAL_LOG_DEBUG, my_log_handler, NULL);
        visual_log_set_handler(VISUAL_LOG_WARNING, my_log_handler, NULL );
        visual_log_set_handler(VISUAL_LOG_ERROR, my_log_handler, NULL );
        visual_log_set_handler(VISUAL_LOG_CRITICAL, my_log_handler, NULL );
        visual_init (0, NULL);
        memset(&v, 0, sizeof(v));
        memset(&pcm_ref, 0, sizeof(pcm_ref));
        LV::PluginRegistry::instance()->add_path("/data/data/net.starlon.droidvisuals/lib");

    } else {
/*
        visual_video_free_buffer(v.video);
        visual_object_unref(VISUAL_OBJECT(v.video));
        v.video = NULL; // Will thread activity blowup in native_render() if we do this? Speifically free_buffer(). hmm This whole block of code is wrong.
*/
    }

    v.morph_name = morph_;
    v.actor_name = actor_;
    v.input_name = input_;

    v.bin    = new LV::Bin();

    v.bin->set_supported_depth(VISUAL_VIDEO_DEPTH_ALL);
    v.bin->set_preferred_depth(VISUAL_BIN_DEPTH_HIGHEST);

    VisActor *actor = visual_actor_new((char*)v.actor_name);
    VisInput *input = visual_input_new((char*)v.input_name);

    if(strstr(v.input_name, "mic"))
    {
/*
    	if (visual_input_set_callback (input, v_upload_callback, NULL) < 0) {

    	    visual_log(VISUAL_LOG_CRITICAL, "Unable to set mic input callback.");	

            visual_object_unref(VISUAL_OBJECT(input));

            v.input_name = "dummy";

            input = visual_input_new((char *)v.input_name);

    	} else {
        }
*/
    }

    depthflag = visual_actor_get_supported_depth(actor);
    depth = visual_video_depth_get_highest(depthflag);


    v.bin->set_depth(depth);

    v.bin->switch_set_style(VISUAL_SWITCH_STYLE_DIRECT);
    v.bin->switch_set_automatic (true);
    v.bin->switch_set_steps (12);

    v.video = LV::Video::create(w, h, depth);

    v.bin->set_video(v.video);

    v.bin->connect(actor, input);

    v.bin->realize();

    v.bin->sync(false);

    v.pluginIsGL = (depth == VISUAL_VIDEO_DEPTH_GL);

    pthread_mutex_init(&v.mutex, NULL);

    printf ("Libvisual version %s; bpp: %d %s\n", visual_get_version(), v.video->get_bpp(), (v.pluginIsGL ? "(GL)\n" : ""));
}

// Initialize the application's view and libvisual.
JNIEXPORT void JNICALL Java_net_starlon_droidvisuals_NativeHelper_initApp(JNIEnv * env, jobject  obj, jint w, jint h, jstring actor, jstring input, jstring morph)
{
    app_main(w, h, ACTOR, INPUT, MORPH);
}


/*
LV::VideoPtr new_video(int w, int h, VisVideoDepth depth)
{
    LV::VideoPtr video = LV::Video::create(w, h, depth);
    return video;
}
*/

/*
void swap_video_BGR(VisVideo *vid1, VisVideo *vid2)
{
    uint8_t *d = visual_video_get_pixels(vid1);
    uint8_t *s = visual_video_get_pixels(vid2);
    int i;

    for(i = 0; i < vid1->width * vid1->get_height() * sizeof(int32_t); i+=4)
    {

        d[i] = s[i+2];
        //d[i+1] = s[i+1];
        d[i+2] = s[i];
        d[i+3] = 0xff;
    }
}
*/

// Render the view's bitmap image.
JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_renderBitmap(JNIEnv * env, jobject  obj, jobject bitmap, jboolean do_swap)
{

    AndroidBitmapInfo  info;
    void*              pixels;
    int                ret;
    int depthflag;
    static VisVideoDepth depth;
    LV::VideoPtr vid;

    pthread_mutex_lock(&v.mutex);

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return FALSE;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return FALSE;
    }

    vid = LV::Video::wrap(pixels, false, info.width, info.height, DEVICE_DEPTH);

    if(v.bin->depth_changed()  || 
        ((int)info.width != v.video->get_width() || 
        (int)info.height != v.video->get_height()) ) 
    {

        depthflag = v.bin->get_depth();

        depth = visual_video_depth_get_highest(depthflag);

        
        if(v.video->has_allocated_buffer())
            v.video->free_buffer();

        v.video->set_dimension(info.width, info.height);
        v.video->set_depth(depth);

        v.pluginIsGL = (depth == VISUAL_VIDEO_DEPTH_GL);

        if(not v.pluginIsGL)
        {
            v.video->set_pitch(visual_video_bpp_from_depth(depth) * info.width);
            v.video->allocate_buffer();
        }
        v.bin->sync(true);
    }

    v.bin->run();

    if (not v.pluginIsGL ) {
        vid->convert_depth(v.video);
    }

    vid->unref();

    AndroidBitmap_unlockPixels(env, bitmap);

    pthread_mutex_unlock(&v.mutex);

    return v.pluginIsGL;
}

} // extern "C"
