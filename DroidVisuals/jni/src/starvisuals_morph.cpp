// ------ MORPH ------

#include "starvisuals.h"

extern "C" {

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

void finalizeMorph(const char *morph)
{

}



void v_cycleMorph ()
{
    return;
    const char *str = visual_morph_get_plugin(v.bin->get_morph())->info->name;
    auto name = visual_morph_get_next_by_name(str);


    if(not name)
    {
        name = visual_morph_get_prev_by_name(str);
    }

    v.morph_name = name;
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

} // extern "C"
