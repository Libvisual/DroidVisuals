
#include "starvisuals.h"

V *v;

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


// Set the VisBin's morph style -- to morph or not to morph.
JNIEXPORT void JNICALL Java_net_starlon_droidvisuals_NativeHelper_setMorphStyle(JNIEnv * env, jobject  obj, jboolean morph)
{
    if(morph)
        visual_bin_switch_set_style(v->bin, VISUAL_SWITCH_STYLE_MORPH);
    else
        visual_bin_switch_set_style(v->bin, VISUAL_SWITCH_STYLE_DIRECT);
}


// Increment or decrement actor and morph
// Variable 'prev' is used to shift morph plugin around.
// 0=left, 1=right, 2=up, 3=down, 4=cycle.. Any other and the current value is used.
JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_finalizeSwitch(JNIEnv * env, jobject obj, jint prev, jboolean nogl)
{

    int depthflag;
    VisVideoDepth depth;


    v->lock();

    VisMorph *bin_morph = visual_bin_get_morph(v->bin);
    std::string morph = v->morph_name;

    std::string actor = v->actor_name;
    
    if(bin_morph && !visual_morph_is_done(bin_morph))
    {
        v->unlock();
        return FALSE;
    }


    switch(prev)
    {
        case 1: v->morph_name = ("slide_left"); break;
        case -1: v->morph_name = ("slide_right"); break;
        case 2: v->morph_name = ("slide_top"); break;
        case -2: v->morph_name = ("slide_bottom"); break;
        case 0: prev = 0; break;
        default: v->morph_name = (MORPH); break;
    }

    //Update v->actor_name.
    v_cycleActor((int)prev, (bool)nogl); 

    v->set_morph(v->morph_name);

    v->set_actor(v->actor_name);

    v->unlock();

    visual_log(VISUAL_LOG_DEBUG, "finalizeSwitch() - old %s, new %s", actor.c_str(), v->actor_name.c_str());
    return TRUE;
}

JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_setIsActive(JNIEnv *env, jobject obj, jboolean is_active)
{
    v->is_active = (int)is_active;

    return 0;
}

JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_getIsActive(JNIEnv *env, jobject obj)
{
    return v->is_active;
}

JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorIsGL(JNIEnv *env, jobject obj)
{
    if(v->bin->get_depth() == VISUAL_VIDEO_DEPTH_GL)
        return true;
    return false;
}



JNIEXPORT void JNICALL Java_net_starlon_droidvisuals_NativeHelper_visualsQuit(JNIEnv * env, jobject  obj, jboolean toExit)
{
    if(visual_is_initialized())
        visual_quit();
}

void app_main(int w, int h, const std::string &actor_, const std::string &input_, const std::string &morph_);
// Initialize the application's view and libvisual.
JNIEXPORT void JNICALL Java_net_starlon_droidvisuals_NativeHelper_initApp(JNIEnv * env, jobject  obj, jint w, jint h, jstring actor, jstring input, jstring morph, jboolean defaults)
{
    app_main(w, h, ACTOR, INPUT, MORPH);
}

void app_main(int w, int h, const std::string &actor_, const std::string &input_, const std::string &morph_)
{
    if(!visual_is_initialized())
    {
        visual_log_set_verbosity (VISUAL_LOG_DEBUG);
        visual_error_set_handler(my_error_handler, NULL);
        visual_log_set_handler(VISUAL_LOG_DEBUG, my_log_handler, NULL);
        visual_log_set_handler(VISUAL_LOG_WARNING, my_log_handler, NULL );
        visual_log_set_handler(VISUAL_LOG_ERROR, my_log_handler, NULL );
        visual_log_set_handler(VISUAL_LOG_CRITICAL, my_log_handler, NULL );
        visual_init (0, NULL);
        LV::PluginRegistry::instance()->add_path("/data/data/net.starlon.droidvisuals/lib");

    }

    if(v){
        delete v;
    }

    v = new V(w, h, actor_, input_, morph_, VISUAL_SWITCH_STYLE_DIRECT);
}



/*
LV::VideoPtr new_video(int w, int h, VisVideoDepth depth)
{
    LV::VideoPtr video = LV::Video::create(w, h, depth);
    return video;
}
*/

void swap_RGBxBGR(VisVideo *vid1, VisVideo *vid2)
{
    int r,i;
    int8_t *pixels_out =  (int8_t *)visual_video_get_pixels(vid1);
    int8_t *pixels_in = (int8_t *)visual_video_get_pixels(vid2);
    for(i = 0; i < (vid1->get_bpp() * vid1->get_width()) - 4; i+=4)
    {
        pixels_out[i] = pixels_in[i+2];
        pixels_out[i+1] = pixels_in[i+1];
        pixels_out[i+2] = pixels_in[i];
        pixels_out[i+3] = pixels_in[i+3];
    }
    
    
}

// Render the view's bitmap image.
JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_renderBitmap(JNIEnv * env, jobject  obj, jobject bitmap)
{
    AndroidBitmapInfo  info;
    void*              pixels;
    int                ret;
    int depthflag;
    static VisVideoDepth depth;
    LV::VideoPtr vid;

    v->lock();

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        v->unlock();
        return FALSE;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        v->unlock();
        return FALSE;
    }

    vid = LV::Video::wrap(pixels, false, info.width, info.height, DEVICE_DEPTH);
    

    if(v->bin->depth_changed()  || 
        ((int)info.width != v->video->get_width() || 
        (int)info.height != v->video->get_height()) ) 
    {

        depthflag = v->bin->get_depth();

        depth = visual_video_depth_get_highest(depthflag);
        
        if(v->video->has_allocated_buffer())
            v->video->free_buffer();

        v->video->set_dimension(info.width, info.height);
        v->video->set_depth(depth);

        v->video->set_pitch(visual_video_bpp_from_depth(depth) * info.width);
        v->video->allocate_buffer();

        if(v->video_flip->has_allocated_buffer())
            v->video_flip->free_buffer();

        v->video_flip->set_dimension(info.width, info.height);
        v->video_flip->set_depth(DEVICE_DEPTH);

        v->video_flip->set_pitch(visual_video_bpp_from_depth(DEVICE_DEPTH) * info.width);
        v->video_flip->allocate_buffer();
        
        VisPluginData *plugin = visual_actor_get_plugin(visual_bin_get_actor(v->bin));
        VisEventQueue *eventqueue = visual_plugin_get_eventqueue(plugin);
        VisEvent *event = visual_event_new_resize(info.width, info.height);
        eventqueue->add(*event);

        v->bin->sync(true);
    }

    v->bin->run();


    if (not v->pluginIsGL ) {
#if 0
        // This should work, but there's only black.
        v->video_flip->convert_depth(v->video);
        swap_RGBxBGR(vid.get(), v->video_flip.get());
#else
        vid->convert_depth(v->video);
#endif
    }

    vid->unref();

    AndroidBitmap_unlockPixels(env, bitmap);

    v->unlock();

    return v->pluginIsGL;
}

} // extern "C"
