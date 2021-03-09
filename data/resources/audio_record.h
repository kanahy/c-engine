PjAudio pj_audio_record(const PjAudioMain* audio) { // TODO: recording list (not buffer)
    PjAudio result = {
        .buffer = 0
    };

    ALuint buffer = 0;
    ALCenum format = AL_FORMAT_MONO16;
    ALsizei size = 0;
    ALsizei frequency = 44100;
    ALCbyte capture_buffer[1048576];
    ALCbyte* capture_buffer_pointer = NULL;
    ALCdevice* capture_device = NULL;
    ALCint samples_available = 0;

    capture_buffer_pointer = capture_buffer;
    capture_device = alcCaptureOpenDevice(NULL, (ALCuint)frequency, format, 1024);

    alcProcessContext(audio->context);
    alcCaptureStart(capture_device);

    for (unsigned int i = 0; i < 5000000; ++i) {
        alcGetIntegerv(capture_device, ALC_CAPTURE_SAMPLES, (ALCsizei)sizeof(ALCint), &samples_available);

        if (samples_available) {
            alcCaptureSamples(capture_device, capture_buffer_pointer, samples_available);

            size += samples_available;
            capture_buffer_pointer += samples_available * 2;
        }
    }

    size *= 2;

    alcCaptureStop(capture_device);
    while (!alcCaptureCloseDevice(capture_device));

    alGenBuffers(1, &buffer);
    alGenSources(1, &result.buffer);
    alBufferData(buffer, format, capture_buffer, size, frequency);
    alSourcei(result.buffer, AL_BUFFER, (ALint)buffer);

    return result;
}
