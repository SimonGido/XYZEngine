#include "stdafx.h"
#include "Audio.h"


#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"
#include "alc/alcmain.h"


#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include "minimp3_ex.h"


namespace XYZ {

    static ALCdevice* s_AudioDevice = nullptr;
	static mp3dec_t s_Mp3d;
	
	static ALenum GetOpenALFormat(uint32_t channels)
	{
		switch (channels)
		{
		case 1:  return AL_FORMAT_MONO16;
		case 2:  return AL_FORMAT_STEREO16;
		}
		XYZ_ASSERT(false, "Not Supported format");
		return 0;
	}


	void Audio::Init()
	{
        ALCcontext* ctx;

        /* Open and initialize a device */
        s_AudioDevice = alcOpenDevice(NULL);
  
        XYZ_ASSERT(s_AudioDevice,"Could not open a device!");
       
        ctx = alcCreateContext(s_AudioDevice, NULL);
        if (ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE)
        {
            if (ctx != NULL)
                alcDestroyContext(ctx);
            alcCloseDevice(s_AudioDevice);
            XYZ_ASSERT(false, "Could not set a context!");
        }
       
        XYZ_LOG_WARN("Audio Device Info:");
        XYZ_LOG_WARN("Name: ", s_AudioDevice->DeviceName);
        XYZ_LOG_WARN("Sample Rate: ", s_AudioDevice->Frequency);
        XYZ_LOG_WARN("Max Sources: " , s_AudioDevice->SourcesMax);
        XYZ_LOG_WARN("Mono: " ,s_AudioDevice->NumMonoSources);
        XYZ_LOG_WARN("Stereo: ", s_AudioDevice->NumStereoSources);
        
        mp3dec_init(&s_Mp3d);


        // Init listener
        ALfloat listenerPos[] = { 0.0,0.0,0.0 };
        ALfloat listenerVel[] = { 0.0,0.0,0.0 };
        ALfloat listenerOri[] = { 0.0,0.0,-1.0, 0.0,1.0,0.0 };
        alListenerfv(AL_POSITION, listenerPos);
        alListenerfv(AL_VELOCITY, listenerVel);
        alListenerfv(AL_ORIENTATION, listenerOri);
    }
	
	void Audio::ShutDown()
	{
        ALCdevice* device;
        ALCcontext* ctx;

        ctx = alcGetCurrentContext();
        if (ctx == NULL)
            return;

        device = alcGetContextsDevice(ctx);

        alcMakeContextCurrent(NULL);
        alcDestroyContext(ctx);
        alcCloseDevice(device);
	}

	std::shared_ptr<AudioSource> Audio::Create(const std::string& filename)
	{
		mp3dec_file_info_t info;
		int loadResult = mp3dec_load(&s_Mp3d, filename.c_str(), &info, NULL, NULL);
		uint32_t size = info.samples * sizeof(mp3d_sample_t);

		auto sampleRate = info.hz;
		auto channels = info.channels;
		float lengthSeconds = size / (info.avg_bitrate_kbps * 1024.0f);

		return std::make_shared<AudioSource>(info.buffer,size,lengthSeconds,sampleRate,channels);
	}

	

	AudioSource::AudioSource(void* data, uint32_t size, float length, int sampleRate, int channels)
	{
		auto alFormat = GetOpenALFormat(channels);
		alGenBuffers(1, &m_BufferHandle);
		alBufferData(m_BufferHandle, alFormat, data, size, sampleRate);

		alGenSources(1, &m_SourceHandle);
		alSourcei(m_SourceHandle, AL_BUFFER, m_BufferHandle);

		XYZ_ASSERT(alGetError() == AL_NO_ERROR, "Failed to setup sound source");
	}

	AudioSource::~AudioSource()
	{
		alSourceStop(m_SourceHandle);
		alDeleteSources(1, &m_SourceHandle);
		alDeleteBuffers(1, &m_BufferHandle);
	}

	void AudioSource::Play()
	{
		alSourcePlay(m_SourceHandle);
	}

	void XYZ::AudioSource::SetPosition(const glm::vec2& pos)
	{
		m_Position = pos;
		glm::vec3 p = glm::vec3(m_Position, 0);
		alSourcefv(m_SourceHandle, AL_POSITION, (float*)&p);
	}

	void XYZ::AudioSource::SetGain(float gain)
	{
		m_Gain = gain;

		alSourcef(m_SourceHandle, AL_GAIN, gain);
	}

	void XYZ::AudioSource::SetPitch(float pitch)
	{
		m_Pitch = pitch;

		alSourcef(m_SourceHandle, AL_PITCH, pitch);
	}

	void XYZ::AudioSource::SetSpatial(bool spatial)
	{
		m_Spatial = spatial;

		alSourcei(m_SourceHandle, AL_SOURCE_SPATIALIZE_SOFT, spatial ? AL_TRUE : AL_FALSE);
		alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
	}

	void XYZ::AudioSource::SetLoop(bool loop)
	{
		m_Loop = loop;

		alSourcei(m_SourceHandle, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
	}

}