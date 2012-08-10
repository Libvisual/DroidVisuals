
#include "starvisuals.h"

extern V *v;


void v_cycleActor (int prev, bool nogl)
{
    const char * name;

    if(nogl)
        name = prev ? visual_actor_get_prev_by_name_nogl(v->actor_name.c_str())
                     : visual_actor_get_next_by_name_nogl(v->actor_name.c_str());
    else
        name = prev ? visual_actor_get_prev_by_name(v->actor_name.c_str())
                     : visual_actor_get_next_by_name(v->actor_name.c_str());

    if (!name) {
        if(nogl)
            name = prev ? visual_actor_get_prev_by_name_nogl(0)
                    : visual_actor_get_next_by_name_nogl(0);
        else      
            name = prev ? visual_actor_get_prev_by_name(0)
                    : visual_actor_get_next_by_name(0);
    }

    v->actor_name = (std::string)name;

/*
    if (strstr (v->exclude_actors.c_str(), name) != 0)
        v_cycleActor(prev);
*/
}

void finalizeActor(std::string actor)
{
}

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
    const char *name = visual_actor_get_plugin(v->bin->get_actor())->info->name;
    if(!name)
    {
        visual_log(VISUAL_LOG_DEBUG, "get_actor_index() failed");
        return -1;
    }
    for(unsigned int i = 0; i < list.size(); i++)
    {
        LV::PluginRef ref = list[i];
        if(ref.info->plugname && !strcmp(name, ref.info->plugname))
            return i;
    }
    return -1;

}


extern "C" {


JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_cycleActor(JNIEnv *env, jobject obj, jint prev, jboolean nogl)
{
    v->lock();
    v_cycleActor(prev, (bool)nogl);
    v->set_actor(v->actor_name);
    v->unlock();
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
        if(ref->info->plugname && !strcmp(v->actor_name, ref->info->plugname))
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

    v->actor_name = (std::string)ref.info->plugname;

    if(now)
        finalizeActor(v->actor_name);

    return TRUE;
}

// Set the current actor by its name.
JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorSetCurrentByName(JNIEnv *env, jobject obj, jstring name, jboolean now)
{
    jboolean iscopy;
    const char *actor = env->GetStringUTFChars(name, &iscopy);
    v->actor_name = (std::string)actor;
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
    VisParamContainer *params = visual_plugin_get_params(visual_actor_get_plugin(v->bin->get_actor()));

    int count = visual_list_count(params->entries);
    
    return count;
}

VisParamEntry *get_actor_param_entry(int index)
{
    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v->bin->get_actor()));

    visual_return_val_if_fail(cont != NULL, NULL);

    int count = visual_list_count(cont->entries);

    visual_return_val_if_fail(index < count, NULL);

    VisParamEntry *entry = (VisParamEntry *)visual_list_get(cont->entries, index);

    visual_return_val_if_fail(entry != NULL, NULL);

    return entry;
}

JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorParamGetCount(JNIEnv *env, jobject obj)
{
    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v->bin->get_actor()));

    visual_return_val_if_fail(cont != NULL, 0);

    int count = visual_list_count(cont->entries);

    return count;
}

JNIEXPORT jstring JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorParamGetType(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *chars = env->GetStringUTFChars(name, &iscopy);

    jstring string;

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v->bin->get_actor()));

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

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v->bin->get_actor()));

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

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v->bin->get_actor()));

    VisParamEntry *entry = visual_param_container_get(cont, param_name);

    int ret = visual_param_entry_set_string(entry, (char *)new_string);

    return !ret;
}


JNIEXPORT jint JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorParamGetInteger(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *string = env->GetStringUTFChars(name, &iscopy);
    visual_return_val_if_fail(string != NULL, 0);

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v->bin->get_actor()));

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

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v->bin->get_actor()));

    VisParamEntry *entry = visual_param_container_get(cont, param_name);

    int ret = visual_param_entry_set_integer(entry, new_int);

    return !ret;
}

JNIEXPORT jfloat JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorParamGetFloat(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *string = env->GetStringUTFChars(name, &iscopy);
    visual_return_val_if_fail(string != NULL, 0.0f);

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v->bin->get_actor()));

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

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v->bin->get_actor()));

    VisParamEntry *entry = visual_param_container_get(cont, param_name);

    float ret = visual_param_entry_set_float(entry, new_float);

    return !ret;
}

JNIEXPORT jdouble JNICALL Java_net_starlon_droidvisuals_NativeHelper_actorParamGetDouble(JNIEnv *env, jobject obj, jstring name)
{
    jboolean iscopy;
    const char *string = env->GetStringUTFChars(name, &iscopy);
    visual_return_val_if_fail(string != NULL, 0.0);

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v->bin->get_actor()));

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

    VisParamContainer *cont = visual_plugin_get_params(visual_actor_get_plugin(v->bin->get_actor()));

    VisParamEntry *entry = visual_param_container_get(cont, param_name);

    double ret = visual_param_entry_set_double(entry, new_double);

    return !ret;
}
#endif


} // extern "C"
