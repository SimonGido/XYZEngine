#pragma once
#include "XYZ/Gui/Font.h"

#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture2D.h"
#include "XYZ/Renderer/Mesh.h"

#include <glm/glm.hpp>
#include <unordered_set>

namespace XYZ {
	namespace InGui {
		enum Node
		{
			LeftNode,
			RightNode,
			NumNodes
		};

		enum InGuiAxis
		{
			NoneAxis = 0,
			VerticalAxis = 1,
			HorizontalAxis = 2
		};

		enum InGuiWindowFlags
		{
			Collapsed    = 1 << 0,
			Moved	     = 1 << 1,
			Resized	     = 1 << 2,
			Hoovered     = 1 << 3,
			Docked		 = 1 << 4
		};

		enum InGuiFrameDataFlags
		{
			WindowRightResize    = 1 << 0,
			WindowLeftResize     = 1 << 1,
			WindowBottomResize   = 1 << 2,
			WindowTopResize      = 1 << 3,
			LeftMouseButtonDown  = 1 << 4,
			RightMouseButtonDown = 1 << 5,
			ClickHandled         = 1 << 6,
			DockingEnabled		 = 1 << 7
		};

		enum InGuiDocked
		{
			DockedLeft   = 0,
			DockedRight  = 1,
			DockedBottom = 2,
			DockedTop	 = 3,
			DockedMiddle = 4
		};

		struct InGuiWindow
		{
			glm::vec2 Position;
			glm::vec2 Size;
			uint8_t Flags;
		
			static constexpr float PanelSize = 25.0f;
		};

		struct InGuiText
		{
			std::vector<Vertex> Vertices;
			int32_t Width = 0;
			int32_t Height = 0;
		};

		// Render data
		struct InGuiRenderData
		{
			Ref<Material> Material;
			Ref<Texture2D> GuiTexture;
			Ref<Texture2D> ColorPickerTexture;
			Ref<Font> Font;
			Ref<SubTexture2D> ButtonSubTexture;
			Ref<SubTexture2D> CheckboxSubTextureChecked;
			Ref<SubTexture2D> CheckboxSubTextureUnChecked;
			Ref<SubTexture2D> SliderSubTexture;
			Ref<SubTexture2D> SliderHandleSubTexture;
			Ref<SubTexture2D> WindowSubTexture;
			Ref<SubTexture2D> MinimizeButtonSubTexture;
			Ref<SubTexture2D> DockSpaceSubTexture;

			uint32_t TextureID = 0;
			uint32_t FontTextureID = 0;
			uint32_t ColorPickerTextureID = 0;
		};

		

		struct InGuiDockNode
		{
			InGuiDockNode(const glm::vec2& position, const glm::vec2& size)
				: Position(position), Size(size)
			{
				Parent = nullptr;
				Children[LeftNode] = nullptr;
				Children[RightNode] = nullptr;
			}

			void InsertWindow(InGuiWindow* window, uint8_t docked);

			InGuiDockNode* Parent;
			InGuiDockNode* Children[2];
			std::unordered_set<InGuiWindow*> Windows;

			glm::vec2 Position;
			glm::vec2 Size;

			uint8_t SplitAxis = NoneAxis;
		};


		struct InGuiDockSpace
		{
			InGuiDockSpace();
			~InGuiDockSpace();

			InGuiDockNode* Root[NumNodes];

		private:
			void destroy(InGuiDockNode* node);

		};

		// Frame data used in functions
		struct InGuiFrameData
		{
			InGuiWindow* CurrentWindow = nullptr;
		
			glm::vec2 ModifiedWindowMouseOffset = { 0,0 };
			glm::vec2 WindowSpaceOffset = { 0,0 };
			glm::vec2 WindowSize = { 0,0 };
			glm::vec2 MousePosition = { 0,0 };
			glm::vec2 SelectedPoint = { 0,0 };

			float MaxHeightInRow = 0.0f;

			uint8_t Flags = 0;
		};


		// Config values
		struct InGuiConfig
		{
			glm::vec4 DefaultColor;
			glm::vec4 HooverColor;
			glm::vec4 SelectColor;
			glm::vec2 NameScale;
			float MaxTextLength;
		};

		struct InGuiContext
		{
			InGuiContext(const InGuiRenderData& renderData, const InGuiConfig& config);
			~InGuiContext();

			InGuiWindow* GetWindow(const std::string& name);
			InGuiWindow* CreateWindow(const std::string& name, const glm::vec2& position, const glm::vec2& size);
			

			InGuiFrameData FrameData;
			InGuiRenderData RenderData;
			InGuiConfig ConfigData;
			InGuiDockSpace DockSpace;

		private:
			std::unordered_map<std::string, InGuiWindow*> InGuiWindows;
		};



		void Init(const InGuiRenderData& renderData,const InGuiConfig& config);
		void EnableDockSpace();

		void Shutdown();

		void BeginFrame();
		void EndFrame();


		void OnLeftMouseButtonRelease();
		void OnRightMouseButtonRelease();
		void OnLeftMouseButtonPress();
		void OnRightMouseButtonPress();
		void OnWindowResize(const glm::vec2& size);
		void OnMouseMove(const glm::vec2& position);
	}
}