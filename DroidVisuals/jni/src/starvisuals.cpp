
#include "starvisuals.h"



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

extern "C" {


// Increment or decrement actor and morph
// Variable 'prev' is used to shift morph plugin around.
// 0=left, 1=right, 2=up, 3=down, 4=cycle.. Any other and the current value is used.
JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_finalizeSwitch(JNIEnv * env, jobject obj, jint prev)
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

    v_cycleActor((int)prev);
    //visual_bin_set_morph(v.bin, (char *)v.morph_name);
    visual_bin_switch_actor_by_name(v.bin, (char *)v.actor_name);
    pthread_mutex_unlock(&v.mutex);

    return TRUE;
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

JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorIsGL(JNIEnv *env, jobject obj)
{
    //pthread_mutex_lock(&v.mutex);
    VisVideoDepth depth = v.bin->get_depth();
    //pthread_mutex_lock(&v.mutex);
    if(depth == VISUAL_VIDEO_DEPTH_GL)
        return true;
    return false;
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


JNIEXPORT void JNICALL Java_net_starlon_droidvisuals_NativeHelper_visualsQuit(JNIEnv * env, jobject  obj, jboolean toExit)
{

    pthread_mutex_lock(&v.mutex);
    if(visual_is_initialized())
        visual_quit();
    pthread_mutex_unlock(&v.mutex);
}

void app_main(int w, int h, const char *actor_, const char *input_, const char *morph_)
{

    int depthflag;
    VisVideoDepth depth;

    if(!visual_is_initialized())
    {
        pthread_mutex_init(&v.mutex, NULL);
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


    }

    pthread_mutex_lock(&v.mutex);

    v.morph_name = morph_;
    v.actor_name = actor_;
    v.input_name = input_;

    if(v.bin == NULL)
    {
        v.bin    = new LV::Bin();
        v.bin->set_supported_depth(VISUAL_VIDEO_DEPTH_ALL);
    }

    VisActor *actor = visual_actor_new((char*)v.actor_name);
    VisInput *input = visual_input_new((char*)v.input_name);

visual_log(VISUAL_LOG_CRITICAL, "WTF aAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA %p", actor);
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

visual_log(VISUAL_LOG_CRITICAL, "depthflaggggggggggggggggggggggggggggg %d", depthflag);

    if((depthflag == VISUAL_VIDEO_DEPTH_GL))
    {
        
        depth = visual_video_depth_get_highest(depthflag);
    }
    else
    {
        depth = visual_video_depth_get_highest_nogl(depthflag);
    }

    v.bin->set_depth(depth);

    v.bin->switch_set_style(VISUAL_SWITCH_STYLE_DIRECT);
    v.bin->switch_set_steps (12);

    if(v.video.get() != NULL)
    {
        v.video->free_buffer();
        v.video->unref();
    }

    v.video = LV::Video::create(w, h, depth);

    v.bin->set_video(v.video);

    v.bin->connect(actor, input);

    v.bin->realize();

    v.bin->sync(false);

    v.pluginIsGL = (depth == VISUAL_VIDEO_DEPTH_GL);

    pthread_mutex_unlock(&v.mutex);

    visual_log (VISUAL_LOG_CRITICAL, "Libvisual version %s; bpp: %d %s\n", visual_get_version(), v.video->get_bpp(), (v.pluginIsGL ? "(GL)\n" : ""));

}

// Initialize the application's view and libvisual.
JNIEXPORT void JNICALL Java_net_starlon_droidvisuals_NativeHelper_initApp(JNIEnv * env, jobject  obj, jint w, jint h, jstring actor, jstring input, jstring morph, jboolean defaults)
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
JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_renderBitmap(JNIEnv * env, jobject  obj, jobject bitmap)
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

        v.video->set_pitch(visual_video_bpp_from_depth(depth) * info.width);
        v.video->allocate_buffer();

        VisPluginData *plugin = visual_actor_get_plugin(visual_bin_get_actor(v.bin));
        VisEventQueue *eventqueue = visual_plugin_get_eventqueue(plugin);
        VisEvent *event = visual_event_new_resize(info.width, info.height);
        eventqueue->add(*event);

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
