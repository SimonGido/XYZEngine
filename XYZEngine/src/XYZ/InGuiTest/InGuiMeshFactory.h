#pragma once
#include "InGuiRenderer.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Font.h"

namespace XYZ {


	class IGRenderData
	{
	public:
		enum 
		{
			Button = 0,
			CheckboxChecked,
			CheckboxUnChecked,
			Slider,
			SliderHandle,
			Window,
			MinimizeButton,
			CloseButton,
			DownArrow,
			RightArrow,
			LeftArrow,
			Pause,
			DockSpace,
			NumSubTextures = 32
		};
		Ref<SubTexture> SubTextures[NumSubTextures];
		Ref<Font> Font;

		static constexpr uint32_t TextureID = 0;
		static constexpr uint32_t FontTextureID = 1;
		static constexpr uint32_t ColorPickerTextureID = 2;
		static constexpr uint32_t DefaultTextureCount = 3;
		static constexpr uint32_t MaxNumberOfScissors = 32;
	};

	struct IGMeshFactoryData
	{
		uint32_t   SubTextureIndex;

		IGElement*    Element;
		IGMesh*		  Mesh;
		IGRenderData* RenderData;
	};

	class IGMeshFactory
	{
	public:
		template <IGElementType, typename ...Args>
		static glm::vec2 GenerateUI(const char* label, const glm::vec4& labelColor, const IGMeshFactoryData& data);

	};

}
