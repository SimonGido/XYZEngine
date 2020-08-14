#pragma once

#include "XYZ/ECS/ECSManager.h"
#include <glm/glm.hpp>


// TODO 
/*
Create AudioContext
Separate Buffer gen
Create API for parsing data
*/


namespace XYZ {

	/* !@class AudioSource
	* @brief source of audio
	*/
	class AudioSource
	{
	public:
		/**
		* Construct
		* @param[in] data,
		* @param[in] size,
		* @param[in] length,
		* @param[in] sampleRate,
		* @param[in] channels
		*/
		AudioSource(void* data, uint32_t size,float length, int sampleRate, int channels);
		~AudioSource();
		
		/**
		* Play audio until end
		*/
		void Play();


		void SetPosition(const glm::vec2& pos);
		void SetGain(float gain);
		void SetPitch(float pitch);
		void SetSpatial(bool spatial);
		void SetLoop(bool loop);

	private:
		uint32_t m_BufferHandle = 0;
		uint32_t m_SourceHandle = 0;
		bool m_Loaded = false;
		bool m_Spatial = false;

		float m_Duration = 0; // in seconds

		// Attributes
		glm::vec2 m_Position = glm::vec2(0);
		float m_Gain = 1.0f;
		float m_Pitch = 1.0f;
		bool m_Loop = false;
	};


	class Audio
	{
	public:
		static void Init();
		static void ShutDown();
		
		static std::shared_ptr<AudioSource> Create(const std::string& filename);
	};
}
