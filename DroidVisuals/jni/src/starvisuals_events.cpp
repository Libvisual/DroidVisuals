

#include "starvisuals.h"

extern "C" {

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

} // extern "C"
