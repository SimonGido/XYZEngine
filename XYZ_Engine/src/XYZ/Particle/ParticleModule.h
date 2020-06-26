#pragma once
#include "XYZ/Renderer/Shader.h"


namespace XYZ {

	enum class ParticleModuleType
	{
		Renderer,
		Emission,
		SizeOverLifeTime,
		ColorOverLifeTime
	};


	class ParticleModule
	{
	public:
		ParticleModule(const std::string& filepath) {};
		virtual void OnAttach() {};
		virtual void OnDetach() {};

	private:

	};
}