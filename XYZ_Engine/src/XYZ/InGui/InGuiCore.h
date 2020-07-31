#pragma once
#include "XYZ/Gui/Font.h"

#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture2D.h"
#include "XYZ/Renderer/Mesh.h"


#include <glm/glm.hpp>
#include <unordered_set>

namespace XYZ {
	namespace InGui {
		
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
			DockingHandled		 = 1 << 7,
			DockingEnabled		 = 1 << 8,
			DockspaceResized     = 1 << 9
		};

		struct InGuiDockNode;
		struct InGuiWindow
		{
			glm::vec2 Position;
			glm::vec2 Size;
			uint8_t Flags;
			
			InGuiDockNode* DockNode = nullptr;
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

			uint16_t Flags = 0;
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

		enum class SplitAxis
		{
			None,
			Vertical,
			Horizontal
		};

		enum class DockPosition
		{
			None,
			Left,
			Right,
			Bottom,
			Top,
			Middle
		};

		struct InGuiDockNode
		{
			InGuiDockNode(const glm::vec2& pos, const glm::vec2& size, InGuiDockNode* parent = nullptr)
				:
				Position(pos), Size(size), Parent(parent)
			{
				Children[0] = nullptr;
				Children[1] = nullptr;
			}

			glm::vec2 Position;
			glm::vec2 Size;

			InGuiDockNode* Parent;
			InGuiDockNode* Children[2];
			std::vector<InGuiWindow*> Windows;

			SplitAxis Split = SplitAxis::None;
		};


		class InGuiDockSpace
		{
		public:
			InGuiDockSpace(const glm::vec2& pos, const glm::vec2& size);
			~InGuiDockSpace();

			void InsertWindow(InGuiWindow* window, const glm::vec2& mousePos);
			void RemoveWindow(InGuiWindow* window);

			void FitToWindow(const glm::vec2& size);
			void ShowDockSpace();

			void Update();

		private:
			void resize();
			void adjustChildrenProps(InGuiDockNode* node);
			void detectResize(InGuiDockNode* node);
			void insertWindow(InGuiWindow* window, const glm::vec2& mousePos, InGuiDockNode* node);
			void destroy(InGuiDockNode** node);
			void rescale(const glm::vec2& scale, InGuiDockNode* node);
			void splitNode(InGuiDockNode* node,SplitAxis axis);
			void unsplitNode(InGuiDockNode* node);
			void update(InGuiDockNode* node);

			void showNode(InGuiDockNode* node, const glm::vec2& mousePos);
			DockPosition collideWithMarker(InGuiDockNode* node, const glm::vec2& mousePos);

		private:
			InGuiDockNode* m_Root;

			InGuiDockNode* m_ResizedNode = nullptr;

			static constexpr glm::vec2 sc_QuadSize = { 50,50 };
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
			InGuiDockSpace *DockSpace;

		private:
			std::unordered_map<std::string, InGuiWindow*> InGuiWindows;
		};



		void Init(const InGuiRenderData& renderData,const InGuiConfig& config);
	

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