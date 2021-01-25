#pragma once

#include "XYZ/Event/Event.h"
#include "XYZ/Event/InputEvent.h"
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
	namespace InGuiResizeFlags {
		enum ResizeFlags
		{
			Left   = BIT(0),
			Right  = BIT(1),
			Top    = BIT(2),
			Bottom = BIT(3)
		};
	}

	namespace InGuiWindowFlags {
		enum WindowFlags
		{
			Initialized   = BIT(0),
			EventBlocking = BIT(1),
			Hoovered	  = BIT(2),
			Collapsed	  = BIT(3)
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

	struct InGuiLayout
	{
		float LeftPadding = 10.0f, RightPadding = 10.0f, TopPadding = 10.0f, BottomPadding = 10.0f;	
		float SpacingX = 5.0f;
		float SpacingY = 5.0f;
	};

	struct InGuiWindow
	{
		InGuiMesh   Mesh;
		InGuiLayout Layout;
		glm::vec2   Position;
		glm::vec2   Size;
		uint32_t    ID;
		uint16_t    Flags;

		static constexpr float PanelHeight = 25.0f;
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

		static constexpr uint32_t TextureID				= 0;
		static constexpr uint32_t FontTextureID			= 1;
		static constexpr uint32_t ColorPickerTextureID  = 2;
		static constexpr uint32_t DefaultTextureCount   = 3;
	};

	struct InGuiFrameData
	{
		static constexpr uint32_t NullID = 65536;

		glm::mat4  ViewProjectionMatrix;
		glm::vec2  MousePosition;
		glm::vec2  MouseOffset;
		uint16_t   Flags;
		uint8_t	   ResizeFlags;
		uint32_t   ActiveWindowID  = NullID;
		uint32_t   MovedWindowID   = NullID;
		uint32_t   ResizedWindowID = NullID;


		std::vector<bool> HandleInput;
		size_t			  InputIndex = 0;
	};



	struct InGuiContext
	{
		InGuiFrameData			FrameData;
		InGuiRenderData			RenderData;

		std::vector<InGuiWindow> Windows;
	};

	class InGui
	{
	public:
		static void Init();
		static void Destroy();

		static void BeginFrame(const glm::mat4& viewProjectionMatrix);
		static void EndFrame();
		static void OnEvent(Event& event);
		static void SetLayout(uint32_t id, const InGuiLayout& layout);
		static void Separator();

		static bool Begin(uint32_t id, const char* name, const glm::vec2& position, const glm::vec2& size);
		static bool ImageWindow(uint32_t id, const char* name, const glm::vec2& position, const glm::vec2& size, Ref<SubTexture> subTexture);
		static void End();
		static bool BeginGroup(const char* name, bool& open);

		static uint8_t PushNode(const char* name, const glm::vec2& size, bool & open);
		static void BeginChildren();
		static void EndChildren();

		static uint8_t Button(const char* name, const glm::vec2& size);
		static uint8_t Checkbox(const char* name, const glm::vec2& size, bool& val);
		static uint8_t Slider(const char* name, const glm::vec2& size, float& val);
		static uint8_t Text(const char* text);
		static uint8_t Float(const char* name, const glm::vec2& size, float& val);
		static uint8_t Image(const glm::vec2& size, Ref<SubTexture> subTexture);

	private:
		static void saveLayout();
		static void loadLayout();
		static bool eraseOutOfBorders(size_t oldQuadCount, const glm::vec2& genSize, InGuiWindow& window);
		static InGuiWindow& getInitializedWindow(uint32_t id, const glm::vec2& position, const glm::vec2& size);
		static void handleWindowMove();
		static void handleWindowResize();

		static bool onMouseButtonPress(MouseButtonPressEvent& event);
		static bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		static bool onMouseMove(MouseMovedEvent& event);
		static bool onKeyTyped(KeyTypedEvent& event);
		static bool onKeyPressed(KeyPressedEvent& event);

	private:
		static InGuiContext* s_Context;
	};

}