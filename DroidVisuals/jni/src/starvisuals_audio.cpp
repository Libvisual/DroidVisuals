

#include "starvisuals.h"

extern "C" {

// ---------- INPUT ----------






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



} // extern "C"
