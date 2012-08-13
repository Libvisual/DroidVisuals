#include "starvisuals.h"

V::V(int w, int h, std::string actor_, std::string input_, std::string morph_, VisBinSwitchStyle switch_style)
{
        int depthflag;
        VisVideoDepth depth;

        pthread_mutex_init(&mutex, NULL);

        actor_name = actor_;
        input_name = input_;
        morph_name = morph_;

        bin = new LV::Bin();
        bin->set_supported_depth(VISUAL_VIDEO_DEPTH_ALL);
        bin->set_preferred_depth(VISUAL_BIN_DEPTH_LOWEST);

        VisActor *actor = visual_actor_new(actor_name.c_str());

        VisInput *input = NULL;
        if(strstr(input_name.c_str(), "mic") == 0)
        {
        	if ((input = visual_input_new("dummy")) && 
                visual_input_set_callback (input, v_upload_callback, NULL) < 0) {
                visual_log(VISUAL_LOG_DEBUG, "Established dummy input for microphone callback."); 
        	} else {
                input = NULL;
                visual_log(VISUAL_LOG_DEBUG, "Unable to apply callback to dummy input.");
            }
        }

        if(input == NULL)
            input = visual_input_new(input_name.c_str());

        if(input == NULL)
            visual_log(VISUAL_LOG_DEBUG, "Unable to create input plugin <%s>", input_name.c_str());

        depthflag = visual_actor_get_supported_depth(actor);
    
        if((depthflag == VISUAL_VIDEO_DEPTH_GL))
        {
            
            depth = VISUAL_VIDEO_DEPTH_GL;
        }
        else
        {
            depth = visual_video_depth_get_highest_nogl(depthflag);
        }
    
        bin->switch_set_style(switch_style);
        bin->switch_set_steps (12);
        bin->switch_set_automatic(true);
        bin->switch_set_rate(1.0f);
        bin->switch_set_mode(VISUAL_MORPH_MODE_TIME);

        LV::Time time(2, 0);
        bin->switch_set_time(time);
    
        video = LV::Video::create(w, h, depth);
        video_flip = LV::Video::create(w, h, DEVICE_DEPTH);
    
        bin->set_video(video);
    
        bin->set_depth(depth);

        bin->connect(actor, input);
    
        set_morph(morph_);

        bin->realize();

        bin->sync(false);

        bin->depth_changed(); 

        pluginIsGL = (depth == VISUAL_VIDEO_DEPTH_GL);

        visual_log (VISUAL_LOG_INFO, "Libvisual version %s; bpp: %d %s\n", visual_get_version(), video->get_bpp(), (pluginIsGL ? "(GL)\n" : ""));

    
}

    
V::~V() 
{
        visual_object_unref(VISUAL_OBJECT(bin->get_actor()));
        visual_object_unref(VISUAL_OBJECT(bin->get_input()));
        visual_object_unref(VISUAL_OBJECT(bin->get_morph()));
        video->free_buffer();
        video->unref();
        delete bin;
}

void V::realize()
{
    bin->realize();
}

void V::set_actor(std::string act)
{
    int depthflag;
    VisVideoDepth depth;
    bin->switch_actor(act);
    pluginIsGL = (bin->get_depth() == VISUAL_VIDEO_DEPTH_GL);
    // handle depth of new actor
    depthflag = visual_actor_get_supported_depth(bin->get_actor());
    if (depthflag == VISUAL_VIDEO_DEPTH_GL)
    {
        bin->set_depth(VISUAL_VIDEO_DEPTH_GL);
    }
    else
    {
        depth = visual_video_depth_get_highest_nogl(depthflag);
        if ((bin->get_supported_depth() & depth) > 0)
            bin->set_depth(depth);
        else
            bin->set_depth(visual_video_depth_get_highest_nogl(bin->get_supported_depth()));
    }
    bin->force_actor_depth (bin->get_depth ());
}

void V::set_input(std::string input_)
{
    VisInput *input = visual_input_new(input_.c_str());
    bin->set_input(input);
}

void V::set_morph(std::string morph_)
{
    //bin->set_morph(morph_);
}

void V::lock()
{
    pthread_mutex_lock(&mutex);
}

void V::unlock()
{
    pthread_mutex_unlock(&mutex);
}
