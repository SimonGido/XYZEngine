#pragma once
#include "XYZ/Scene/Scene.h"

#include <glm/glm.hpp>


namespace XYZ {
	struct UBCameraData
	{
		glm::mat4 ViewProjectionMatrix;
		glm::mat4 ProjectionMatrix;
		glm::mat4 ViewMatrix;
		glm::vec3 CameraPosition;

		uint32_t Padding;

		static constexpr uint32_t Binding = 0;
		static constexpr uint32_t Set = 0;
	};

	struct UBRendererData
	{
		uint32_t TilesCountX{ 0 };
		bool ShowLightComplexity = false;
		char Padding1[3] = { 0, 0, 0 };  // Bools are 4-bytes in GLSL

		static constexpr uint32_t Binding = 1;
		static constexpr uint32_t Set = 0;
	};



	struct SSBOPointLights3D
	{
		static constexpr uint32_t MaxLights = 1024;

		uint32_t	 Count{ 0 };
		glm::vec3	 Padding{};
		PointLight3D PointLights[MaxLights];

		static constexpr uint32_t Binding = 2;
		static constexpr uint32_t Set = 0;
	};

	struct SSBOBoneTransformData
	{
		static constexpr uint32_t MaxBoneTransforms = 10 * 1024;

		// NOTE: always heap allocate
		glm::mat4 Data[MaxBoneTransforms];

		static constexpr uint32_t Binding = 3;
		static constexpr uint32_t Set = 0;
	};

	struct SSBOLightCulling
	{
		static constexpr uint32_t Binding = 4;
		static constexpr uint32_t Set = 0;
	};

	struct SSBOIndirectData
	{
		static constexpr uint32_t MaxCommands = 1024;
		static constexpr uint32_t MaxSize = MaxCommands * sizeof(IndirectIndexedDrawCommand);

		static constexpr uint32_t Binding = 5;
		static constexpr uint32_t Set = 0;
	};

	struct SSBOComputeData
	{
		static constexpr uint32_t MaxSize = 100 * 1024 * 1024;
		static constexpr uint32_t Count = 3;


		static constexpr uint32_t Binding[Count] = { 6, 7, 8 };
		static constexpr uint32_t Set = 0;
	};

	struct UBSceneData
	{
		static constexpr uint32_t MaxDirectionalLights = 4;

		float EnvironmentMapIntensity = 1.0f;
		uint32_t NumberDirectionalLights;
		glm::vec2 Padding{0.0f};

		DirectionalLight DirectionalLights[MaxDirectionalLights];

		static constexpr uint32_t Binding = 9;
		static constexpr uint32_t Set = 0;
	};
}