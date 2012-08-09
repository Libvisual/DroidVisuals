#include "starvisuals.h"

extern V *v;

extern "C" {

// Get the VisInput at the requested index.
LV::PluginRef &get_input(int index)
{

    LV::PluginList list = LV::PluginRegistry::instance()->get_plugins_by_type(VISUAL_PLUGIN_TYPE_INPUT);

    int count = list.size();

    if(index >= count)
        return list[0];

    return list[index];
}

void v_cycleInput(int prev)
{
    const char *name;

    name = visual_input_get_next_by_name(v->input_name.c_str());

    if(!name)
    {
        v->input_name = (std::string)visual_input_get_next_by_name(0);
        return;
    }

    v->input_name = (std::string)name;
}


int get_input_index()
{
    LV::PluginList list = LV::PluginRegistry::instance()->get_plugins_by_type(VISUAL_PLUGIN_TYPE_INPUT);
    int count = list.size();
    for(int i = 0; i < count; i++)
    {
        LV::PluginRef ref = list[i];
        if(ref.info->plugname && !strcmp(v->input_name.c_str(), ref.info->plugname))
            return i;
    }
    return -1;

}

void finalizeInput(std::string input)
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
    finalizeInput(v->input_name);
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

    v->input_name = ref.info->plugname;

    if(now)
        finalizeInput(v->input_name);

    return TRUE;
}

// Set the current input plugin by its name. Do nothing and return false if the plugin doesn't exist.
JNIEXPORT jboolean JNICALL Java_net_starlon_droidvisuals_NativeHelper_inputSetCurrentByName(JNIEnv *env, jobject obj, jstring name, jboolean now)
{
    jboolean iscopy;
    const char *input = env->GetStringUTFChars(name, &iscopy);
    v->input_name = (std::string)input;
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

} // extern "C"
