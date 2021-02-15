#ifndef audio_h
#define audio_h


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#include <stb/stb_vorbis.c>

#define DR_FLAC_IMPLEMENTATION
#include <dr/dr_flac.h>

#define DR_MP3_IMPLEMENTATION
#include <dr/dr_mp3.h>

#define DR_WAV_IMPLEMENTATION
#include <dr/dr_wav.h>

#include <AL/al.h>
#include <AL/alc.h>


ALCenum alc_debug(ALCdevice* device) {
    ALCenum result = alcGetError(device);

    if (result != ALC_NO_ERROR) {
        printf("ALC error: %s\n", alcGetString(device, result));
    }

    return result;
}

ALenum al_debug() {
    ALenum result = alGetError();

    if (result != AL_NO_ERROR) {
        printf("AL error: %s\n", alGetString(result));
    }

    return result;
}


ALCdevice* audio_device_create(ALCcontext** context) {
    ALCdevice* result = NULL;
    context[0] = NULL;

    result = alcOpenDevice(NULL);

    if (result) {
        context[0] = alcCreateContext(result, NULL);

        if (context[0]) {
            if (alcMakeContextCurrent(context[0])) {
                return result;
            }
            else {
                puts("Failed to alcMakeContextCurrent(context)");
            }

            alcDestroyContext(context[0]);
            context[0] = NULL;
        }
        else {
            puts("Failed to alcCreateContext(context, NULL)");
        }

        if (alcCloseDevice(result)) {
            result = NULL;
        }
        else {
            puts("Failed to alcCloseDevice(device)");
        }
    }
    else {
        puts("Failed to alcOpenDevice(NULL)");
    }

    return result;
}

void audio_device_destroy(ALCdevice** device, ALCcontext** context) {
    if (alcMakeContextCurrent(NULL)) {
        if (context[0]) {
            alcDestroyContext(context[0]);
            context[0] = NULL;
        }

        if (device[0]) {
            if (alcCloseDevice(device[0])) {
                device[0] = NULL;
            }
            else {
                puts("Failed to alcCloseDevice(device)");
            }
        }
    }
    else {
        puts("Failed to alcMakeContextCurrent(NULL)");
    }
}


ALenum audio_get_format_from_channel_count(unsigned int channel_count) {
    ALenum format = 0;

    switch (channel_count) {
        case 1: format = AL_FORMAT_MONO16; break;
        case 2: format = AL_FORMAT_STEREO16; break;
        case 4: format = alGetEnumValue("AL_FORMAT_QUAD16"); break;
        case 6: format = alGetEnumValue("AL_FORMAT_51CHN16"); break;
        case 7: format = alGetEnumValue("AL_FORMAT_61CHN16"); break;
        case 8: format = alGetEnumValue("AL_FORMAT_71CHN16"); break;
        default: format = 0; break;
    }

    // Fixes a bug on OS X
    if (format == -1) {
        format = 0;
    }

    return format;
}


void audio_buffer_destroy(ALuint* buffer) {
    if (buffer && *buffer) {
        alDeleteBuffers(1, buffer);

        if (al_debug() == AL_NO_ERROR) {
            *buffer = 0;
        }
    }
}

ALuint audio_buffer_create(const char* file_name) {
    ALuint result = 0;

    if (file_check_extension(file_name, "ogg")) {
        int size = 0;
        int channels = 0;
        int sample_rate = 0;
        short* data = NULL;

        size = stb_vorbis_decode_filename(file_name, &channels, &sample_rate, &data);

        if (data) {
            alGenBuffers(1, &result);

            if (al_debug() == AL_NO_ERROR) {
                alBufferData(result, audio_get_format_from_channel_count((unsigned int)channels), data, (ALsizei)((unsigned int)size * (unsigned int)channels * sizeof(int16_t)), (ALsizei)sample_rate);

                if (al_debug() != AL_NO_ERROR) {
                    audio_buffer_destroy(&result);
                }
            }

            free(data);
            data = NULL;
        }
    }
    else if (file_check_extension(file_name, "flac")) {
        drflac* flac_data = drflac_open_file(file_name, NULL);

        if (flac_data) {
            int16_t* data = (int16_t*)calloc((size_t)flac_data->totalPCMFrameCount * flac_data->channels, sizeof(int16_t));

            if (data) {
                drflac_read_pcm_frames_s16(flac_data, flac_data->totalPCMFrameCount, data);

                alGenBuffers(1, &result);

                if (al_debug() == AL_NO_ERROR) {
                    alBufferData(result, audio_get_format_from_channel_count(flac_data->channels), data, (ALsizei)(flac_data->totalPCMFrameCount * flac_data->channels * sizeof(int16_t)), (ALsizei)flac_data->sampleRate);

                    if (al_debug() != AL_NO_ERROR) {
                        audio_buffer_destroy(&result);
                    }
                }

                free(data);
            }

            drflac_close(flac_data);
        }
    }
    else if (file_check_extension(file_name, "mp3")) {
        drmp3_config config = {
            .channels = 0,
            .sampleRate = 0
        };

        drmp3_uint64 total_pcm_frame_count = 0;
        drmp3_int16* data = NULL;

        data = drmp3_open_file_and_read_pcm_frames_s16(file_name, &config, &total_pcm_frame_count, NULL);

        if (data) {
            alGenBuffers(1, &result);

            if (al_debug() == AL_NO_ERROR) {
                alBufferData(result, audio_get_format_from_channel_count(config.channels), data, (ALsizei)(total_pcm_frame_count * config.channels * sizeof(int16_t)), (ALsizei)config.sampleRate);

                if (al_debug() != AL_NO_ERROR) {
                    audio_buffer_destroy(&result);
                }
            }

            free(data);
            data = NULL;
        }
    }
    else if (file_check_extension(file_name, "wav")) {
        drwav wav_data;

        if (drwav_init_file(&wav_data, file_name, NULL)) {
            int16_t* data = (int16_t*)calloc((size_t)wav_data.totalPCMFrameCount * wav_data.channels, sizeof(int16_t));
            drwav_read_pcm_frames_s16(&wav_data, wav_data.totalPCMFrameCount, data);

            if (data) {
                alGenBuffers(1, &result);

                if (al_debug() == AL_NO_ERROR) {
                    alBufferData(result, audio_get_format_from_channel_count(wav_data.channels), data, (ALsizei)(wav_data.totalPCMFrameCount * wav_data.channels * sizeof(int16_t)), (ALsizei)wav_data.sampleRate);

                    if (al_debug() != AL_NO_ERROR) {
                        audio_buffer_destroy(&result);
                    }
                }
            }

            drwav_uninit(&wav_data);
        }
    }

    return result;
}

ALint audio_buffer_get_frequency(const ALuint* buffer) {
    if (buffer && *buffer) {
        ALint result = 0;
        alGetBufferi(*buffer, AL_FREQUENCY, &result);
        return al_debug() == AL_NO_ERROR ? result : 0;
    }

    return 0;
}

ALint audio_buffer_get_bits(const ALuint* buffer) {
    if (buffer && *buffer) {
        ALint result = 0;
        alGetBufferi(*buffer, AL_BITS, &result);
        return al_debug() == AL_NO_ERROR ? result : 0;
    }

    return 0;
}

ALint audio_buffer_get_channel_count(const ALuint* buffer) {
    if (buffer && *buffer) {
        ALint result = 0;
        alGetBufferi(*buffer, AL_CHANNELS, &result);
        return al_debug() == AL_NO_ERROR ? result : 0;
    }

    return 0;
}

ALint audio_buffer_get_size(const ALuint* buffer) {
    if (buffer && *buffer) {
        ALint result = 0;
        alGetBufferi(*buffer, AL_SIZE, &result);
        return al_debug() == AL_NO_ERROR ? result : 0;
    }

    return 0;
}

ALfloat audio_buffer_get_time(const ALuint* buffer) {
    if (buffer && *buffer) {
        ALint frequency = audio_buffer_get_frequency(buffer);
        ALint bits = audio_buffer_get_bits(buffer);
        ALint channel_count = audio_buffer_get_channel_count(buffer);
        ALint size = audio_buffer_get_size(buffer);

        if (!frequency || !bits || !channel_count || !size) {
            return FLT_MIN;
        }

        return (1.0f * (float)size) / (float)(frequency * channel_count * (bits / 8));
    }

    return FLT_MIN;
}


void audio_source_destroy(ALuint* source) {
    if (source && *source) {
        alSourceStop(*source);

        if (al_debug() == AL_NO_ERROR) {
            alSourcei(*source, AL_BUFFER, 0);

            if (al_debug() == AL_NO_ERROR) {
                alDeleteSources(1, source);

                if (al_debug() == AL_NO_ERROR) {
                    *source = 0;
                }
            }
        }
    }
}

ALuint audio_source_create(const ALuint* buffer) {
    ALuint result = 0;

    if (buffer && *buffer) {
        alGenSources(1, &result);

        if (al_debug() == AL_NO_ERROR) {
            alSourcei(result, AL_BUFFER, (ALint)*buffer);

            if (al_debug() != AL_NO_ERROR) {
                audio_source_destroy(&result);
            }
        }
    }

    return result;
}

void audio_source_set_position(const ALuint* source, ALfloat x, ALfloat y, ALfloat z) {
    if (source && *source) {
        alSource3f(*source, AL_POSITION, x, y, z);
        al_debug();
    }
}

void audio_source_set_time(const ALuint* source, ALfloat time) {
    if (source && *source) {
        alSourcef(*source, AL_SEC_OFFSET, time < FLT_MIN ? FLT_MIN : time > 100.0f ? 100.0f : time);
        al_debug();
    }
}

void audio_source_play(const ALuint* source) {
    if (source && *source) {
        alSourcePlay(*source);
        al_debug();
    }
}

void audio_source_stop(const ALuint* source) {
    if (source && *source) {
        alSourceStop(*source);
        al_debug();
    }
}

void audio_source_rewind(const ALuint* source) {
    if (source && *source) {
        alSourceRewind(*source);
        al_debug();
    }
}

void audio_source_pause(const ALuint* source) {
    if (source && *source) {
        alSourcePause(*source);
        al_debug();
    }
}


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus


#endif // audio_h
