#pragma once
#include <XYZ.h>

namespace XYZ {

	class Inspectable
	{
	public:
		virtual void OnInGuiRender() {};
		virtual void OnUpdate(Timestep ts) {};
		virtual void OnEvent(Event& event) {};

		static void SetupMaterialValuesLengths(const std::vector<Uniform>& uniforms, std::vector<int32_t>& lengths, std::vector<int32_t>& selected);
		static void ShowUniforms(const uint8_t* buffer, const std::vector<Uniform>& uniforms, std::vector<int32_t>& lengths, std::vector<int32_t>& selected);
		
	};

}