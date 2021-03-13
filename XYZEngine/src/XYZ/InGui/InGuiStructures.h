#pragma once

#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Renderer/Framebuffer.h"
#include "XYZ/Renderer/Buffer.h"

#include <glm/glm.hpp>

namespace XYZ {
	namespace InGuiReturnType {
		enum ReturnType
		{
			Hoovered = BIT(0),
			Clicked = BIT(1),
			Modified = BIT(2)
		};
	}
	namespace InGuiInputFlags {
		enum InputFlags
		{
			LeftClicked = BIT(0),
			RightClicked = BIT(1)
		};
	}
	namespace InGuiResizeFlags {
		enum ResizeFlags
		{
			Left = BIT(0),
			Right = BIT(1),
			Top = BIT(2),
			Bottom = BIT(3)
		};
	}

	namespace InGuiWindowFlags {
		enum WindowFlags
		{
			Initialized = BIT(0),
			Hoovered = BIT(1),
			Collapsed = BIT(2),
			Docked = BIT(3)
		};
	}

	struct InGuiQuad
	{
		glm::vec4 Color;
		glm::vec4 TexCoord;
		glm::vec3 Position;
		glm::vec2 Size;
		uint32_t  TextureID;
		uint32_t  ScissorIndex = 0;
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
		InGuiMesh   OverlayMesh;
		InGuiMesh   ScrollableMesh;
		InGuiMesh   ScrollableOverlayMesh;
		InGuiMesh	TriangleMesh;

		InGuiLayout Layout;
		glm::vec2   Position;
		glm::vec2   Size;
		uint32_t    ID;
		uint16_t    Flags;

		static constexpr float PanelHeight = 25.0f;
	};

	struct InGuiScissor
	{
		float X;
		float Y;
		float Width;
		float Height;
	};

	struct InGuiRenderData
	{
		InGuiRenderData();

		Ref<Font>				 Font;
		Ref<Texture2D>			 Texture;
		Ref<Material>			 DefaultMaterial;
		Ref<Material>			 ScissorMaterial;
		Ref<ShaderStorageBuffer> ScissorBuffer;

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
			PAUSE,
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
		static constexpr uint32_t MaxNumberOfScissors = 32;
	};

	enum class InGuiTextCenter
	{
		Left,
		Right,
		Middle,
	};

	struct InGuiFrameData
	{
		void StartFrame();
		void EndFrame();

		static constexpr uint32_t NullID = 65536;
	
		glm::mat4				  ViewProjectionMatrix	= glm::mat4(1.0f); 
		glm::vec2				  MousePosition			= glm::vec2(0.0f); // Current mouse position
		glm::vec2				  MouseOffset			= glm::vec2(0.0f); // Previous mouse position
		uint16_t				  Flags					= 0; // Input flags
		uint8_t					  ResizeFlags			= 0; // Resize flags
		uint32_t				  ActiveWindowID		= NullID;
		uint32_t				  MovedWindowID			= NullID; 
		uint32_t				  ResizedWindowID		= NullID;
		uint32_t				  DropdownItemCount		= 0; // Number of items in current drop down
		float					  ScrollableHeight		= 0.0f; // Height of current scrollable
		glm::vec2			      DropdownSize			= glm::vec2(0.0f);
		glm::vec2				  LayoutOffset			= glm::vec2(0.0f);
		InGuiMesh*				  CurrentMesh			= nullptr;
		InGuiMesh*				  CurrentOverlayMesh	= nullptr;
		InGuiTextCenter			  TextCenter			= InGuiTextCenter::Left;
		bool					  ScrollableActive		= false;
		bool					  ActiveWidgets			= true; // Are widgets active
		bool					  EraseOutOfLine		= true;
		bool					  BlockEvents			= false;
		float					  ScrollOffset			= 0.0f;
		float					  HighestInRow			= 0.0f;

		uint32_t				  ModifyTextIndex		= 0;
		char					  TextBuffer[_MAX_PATH];
		char					  ModifyTextBuffer[_MAX_PATH];
		
		std::vector<InGuiScissor> Scissors;
		std::vector<Ref<Texture>> CustomTextures;
		
		std::vector<bool>		  HandleInput;
		size_t					  InputIndex			= 0;
	};




	enum class InGuiSplitType
	{
		None,
		Horizontal,
		Vertical
	};

	struct InGuiDockNode
	{
		InGuiDockNode();
		InGuiDockNode(uint32_t id);
		~InGuiDockNode();

		InGuiDockNode* Parent = nullptr;
		InGuiDockNode* FirstChild = nullptr;
		InGuiDockNode* SecondChild = nullptr;
		InGuiSplitType Type = InGuiSplitType::None;
		uint32_t ID;

		struct NodeData
		{
			glm::vec2 Size;
			glm::vec2 Position;
			std::vector<uint32_t> Windows;
		};
		NodeData Data;
	};

	struct InGuiContext
	{
		InGuiFrameData			 FrameData;
		InGuiRenderData			 RenderData;

		std::vector<InGuiWindow> Windows;
		std::vector<uint32_t>    EventListeners;
	};

}