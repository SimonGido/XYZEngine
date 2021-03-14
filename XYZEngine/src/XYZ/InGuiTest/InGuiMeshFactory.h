#pragma once
#include "InGuiRenderer.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/Buffer.h"

namespace XYZ {

	struct IGScissor
	{
		float X;
		float Y;
		float Width;
		float Height;
	};

	class IGRenderData
	{
	public:
		IGRenderData();
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
		Ref<Font>				 Font;
		Ref<Texture2D>			 Texture;
		Ref<Material>			 Material;
		Ref<ShaderStorageBuffer> ScissorBuffer;
		Ref<SubTexture>			 SubTextures[NumSubTextures];
		bool					 Rebuild = true;

		static constexpr uint32_t TextureID = 0;
		static constexpr uint32_t FontTextureID = 1;
		static constexpr uint32_t DefaultTextureCount = 2;
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
