

#include "starvisuals.h"

extern V *v;


// ---------- INPUT ----------

int v_upload_callback (VisInput* input, VisAudio *audio, void* unused)
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
    visual_return_val_if_fail(pcm_ref->pcm_data != NULL, VISUAL_ERROR_GENERAL);

    VisParamContainer *paramcontainer = visual_plugin_get_params(input->plugin);

    VisBuffer buf;

    visual_buffer_init( &buf, pcm_ref->pcm_data, pcm_ref->size, NULL );
    visual_audio_samplepool_input( audio->samplepool, &buf, pcm_ref->rate, pcm_ref->encoding, pcm_ref->channels);
*/
    return 0;
}



extern "C" {


/* End of plugin and parameter getters and setters. */



// For fallback audio source.
JNIEXPORT void JNICALL Java_net_starlon_droidvisuals_NativeHelper_uploadAudio(JNIEnv * env, jobject  obj, jshortArray data)
{
    jshort *pcm;
    jsize len = env->GetArrayLength(data);
    pcm = env->GetShortArrayElements(data, NULL);
    for(unsigned int i = 0; i < (unsigned int)len && i < (int)v->pcm_ref.size / sizeof(int16_t); i++)
    {
        v->pcm_ref.pcm_data[i] = pcm[i];
    }
    env->ReleaseShortArrayElements(data, pcm, 0);
}

// Reinitialize audio fields.
JNIEXPORT void JNICALL Java_net_starlon_droidvisuals_NativeHelper_resizePCM(jint size, jint samplerate, jint channels, jint encoding)
{
    //if(v->pcm_ref.pcm_data != NULL)
    //    visual_mem_free(v->pcm_ref.pcm_data);
    v->pcm_ref.size = MAX_PCM;//size;
    //v->pcm_ref.pcm_data = visual_mem_malloc(v->pcm_ref.size * sizeof(int16_t));

    //D/StarVisuals/StarVisualsActivity( 1102): Opened mic: 44100Hz, bits: 2, channel: 12, buffersize:8192
    switch(samplerate)
    {
        case 8000:
            v->pcm_ref.rate = VISUAL_AUDIO_SAMPLE_RATE_8000;
        break;
        case 11250:
            v->pcm_ref.rate = VISUAL_AUDIO_SAMPLE_RATE_11250;
        break;
        case 22500:
            v->pcm_ref.rate = VISUAL_AUDIO_SAMPLE_RATE_22500;
        break;
        case 32000:
            v->pcm_ref.rate = VISUAL_AUDIO_SAMPLE_RATE_32000;
        break;
        case 44100:
            v->pcm_ref.rate = VISUAL_AUDIO_SAMPLE_RATE_44100;
        break;
        case 48000:
            v->pcm_ref.rate = VISUAL_AUDIO_SAMPLE_RATE_48000;
        break;
        case 96000:
            v->pcm_ref.rate = VISUAL_AUDIO_SAMPLE_RATE_96000;
        break;
        default:
            v->pcm_ref.rate = VISUAL_AUDIO_SAMPLE_RATE_44100;
        break;
    }

    // It seems lv only supports stereo? hmm
    v->pcm_ref.channels = VISUAL_AUDIO_SAMPLE_CHANNEL_STEREO;

    // According to documentation 16BIT sample size is guaranteed to be supported.
    v->pcm_ref.encoding = VISUAL_AUDIO_SAMPLE_FORMAT_S16;
}



} // extern "C"
