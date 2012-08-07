

#include "starvisuals.h"

extern "C" {

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


// Set the VisBin's morph style -- to morph or not to morph.
JNIEXPORT void JNICALL Java_net_starlon_droidvisuals_NativeHelper_setMorphStyle(JNIEnv * env, jobject  obj, jboolean morph)
{
    if(morph)
        visual_bin_switch_set_style(v.bin, VISUAL_SWITCH_STYLE_MORPH);
    else
        visual_bin_switch_set_style(v.bin, VISUAL_SWITCH_STYLE_DIRECT);
}

} // extern "C"
