#pragma once


#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/Buffer.h"


namespace XYZ {

	struct IGQuad
	{
		glm::vec4 Color;
		glm::vec4 TexCoord;
		glm::vec3 Position;
		glm::vec2 Size;
		uint32_t  TextureID;
		uint32_t  ScissorIndex = 0;
	};

	struct IGLine
	{
		glm::vec4 Color;
		glm::vec3 P0;
		glm::vec3 P1;
	};

	struct IGMesh
	{
		std::vector<IGQuad>	Quads;
		std::vector<IGLine>	Lines;
	};

	struct IGScissor
	{
		float X;
		float Y;
		float Width;
		float Height;
	};

	class IGAllocator;
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
			White,
			NumSubTextures = 32
		};
		enum
		{
			DefaultColor,
			HooverColor,
			NumColors
		};

		void RebuildMesh(IGAllocator& allocator);

		IGMesh						 Mesh;
		IGMesh						 ScrollableMesh;
		Ref<Font>					 Font;
		Ref<Texture2D>				 Texture;
		Ref<Material>				 DefaultMaterial;
		Ref<Material>				 ScissorMaterial;
		Ref<ShaderStorageBuffer>	 ScissorBuffer;
		Ref<SubTexture>				 SubTextures[NumSubTextures];
		std::vector<Ref<SubTexture>> CustomSubTextures;
		std::vector<IGScissor>		 Scissors;
		bool						 Rebuild = true;
		bool						 RebuildTwice = false;

		static glm::vec4			 Colors[NumColors];
		static constexpr uint32_t	 TextureID = 0;
		static constexpr uint32_t	 FontTextureID = 1;
		static constexpr uint32_t	 DefaultTextureCount = 2;
		static constexpr uint32_t	 MaxNumberOfScissors = 32;

	private:
		void rebuildMesh(IGAllocator& allocator);
	};
}