#pragma once

#include "XYZ/Event/Event.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Font.h"

#include <glm/glm.hpp>

namespace XYZ {

	namespace InGuiReturnType {
		enum ReturnType
		{
			Hoovered = BIT(0),
			Clicked  = BIT(1)
		};
	}
	namespace InGuiInputFlags {
		enum InputFlags
		{
			LeftClicked  = BIT(0),
			RightClicked = BIT(1)
		};
	}

	struct InGuiQuad
	{
		glm::vec4 Color;
		glm::vec4 TexCoord;
		glm::vec3 Position;
		glm::vec2 Size;
		uint32_t  TextureID;
	};

	struct InGuiLine
	{
		glm::vec4 Color;
		glm::vec3 P0;
		glm::vec3 P1;
	};

	struct InGuiMesh
	{
		std::vector<InGuiQuad>	Quads;
		std::vector<InGuiLine>	Lines;
	};

	struct InGuiWindow
	{
		InGuiMesh Mesh;
		glm::vec2 Position;
		glm::vec2 Size;
		uint32_t  ID;
		bool	  Initialized = false;
	};

	struct InGuiRenderData
	{
		InGuiRenderData();

		Ref<Font>		   Font;
		Ref<Texture2D>	   Texture;
		Ref<Material>	   Material;

		enum
		{
			BUTTON = 0,
			CHECKBOX_CHECKED,
			CHECKBOX_UNCHECKED,
			SLIDER,
			SLIDER_HANDLE,
			WINDOW,
			MIN_BUTTON,
			CLOSE_BUTTON,
			DOWN_ARROW,
			RIGHT_ARROW,
			LEFT_ARROW,
			DOCKSPACE,
			NUM_SUBTEXTURES = 32
		};
		enum
		{
			DEFAULT_COLOR,
			HOOVER_COLOR,
			SELECT_COLOR,
			LINE_COLOR,
			SELECTOR_COLOR,
			NUM_COLORS
		};

		Ref<SubTexture> SubTexture[NUM_SUBTEXTURES];
		glm::vec4		Color[NUM_COLORS];

		static constexpr uint32_t TextureID = 0;
		static constexpr uint32_t FontTextureID = 1;
		static constexpr uint32_t ColorPickerTextureID = 2;
		static constexpr uint32_t DefaultTextureCount = 3;
	};

	struct InGuiFrameData
	{
		glm::mat4  ViewProjectionMatrix;
		glm::vec2  MousePosition;
		uint32_t   ActiveWindowID;
		uint16_t   Flags;
	};

	struct InGuiContext
	{
		InGuiFrameData			FrameData;

		std::vector<InGuiWindow> Windows;
		std::vector<InGuiMesh>   Meshes;
	};

	class InGui
	{
	public:
		static void Init();
		static void Destroy();

		static void BeginFrame(const glm::mat4& viewProjectionMatrix);
		static void EndFrame();
		static void OnEvent(Event& event);

		static uint8_t Begin(uint32_t id, const char* name, const glm::vec2& position, const glm::vec2& size);
		static uint8_t End();

	private:
		static InGuiContext* s_Context;
	};

}