#pragma once
#include "XYZ/Core/KeyCodes.h"
#include "XYZ/Core/MouseCodes.h"
#include "XYZ/Gui/Font.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture2D.h"

#include "XYZ/Renderer/InGuiRenderer.h"


namespace XYZ {
	enum InGuiWindowFlags
	{
		Moved		   = BIT(0),
		Collapsed	   = BIT(1),
		MenuEnabled	   = BIT(2),
		Modified	   = BIT(3),
		EventListener  = BIT(4),
		Hoovered	   = BIT(5),
		LeftResizing   = BIT(6),
		RightResizing  = BIT(7),
		TopResizing    = BIT(8),
		BottomResizing = BIT(9),
		Docked		   = BIT(10)
	};

	enum InGuiPerFrameFlags
	{
		LeftMouseButtonPressed  = BIT(0),
		RightMouseButtonPressed = BIT(1),
		ClickHandled			= BIT(2)
	};

	struct InGuiRenderConfiguration
	{
		InGuiRenderConfiguration();
		Ref<Font>		  Font;
		Ref<Material>	  Material;
		Ref<SubTexture2D> ButtonSubTexture;
		Ref<SubTexture2D> CheckboxSubTextureChecked;
		Ref<SubTexture2D> CheckboxSubTextureUnChecked;
		Ref<SubTexture2D> SliderSubTexture;
		Ref<SubTexture2D> SliderHandleSubTexture;
		Ref<SubTexture2D> WindowSubTexture;
		Ref<SubTexture2D> MinimizeButtonSubTexture;
		Ref<SubTexture2D> DownArrowButtonSubTexture;
		Ref<SubTexture2D> RightArrowButtonSubTexture;
		Ref<SubTexture2D> DockSpaceSubTexture;

		uint32_t TextureID = 0;
		uint32_t FontTextureID = 1;
		uint32_t ColorPickerTextureID = 2;

		glm::vec4 DefaultColor = { 1.0f,1.0f,1.0f,1.0f };
		glm::vec4 HooverColor = { 0.4f, 1.8f, 1.7f, 1.0f };
		glm::vec4 SelectColor = { 0.8f,0.0f,0.2f,0.6f };


		static constexpr uint32_t DefaultTextureCount = 3;
		uint32_t NumTexturesInUse = DefaultTextureCount;


		friend class InGui;
	};

	struct InGuiDockNode;
	struct InGuiWindow
	{
		InGuiMesh Mesh;
		std::string Name;

		glm::vec2 Position;
		glm::vec2 Size;

		float MinimalWidth;
		uint16_t Flags = 0;

		InGuiDockNode* DockNode = nullptr;
		static constexpr float PanelSize = 25.0f;
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
		InGuiDockNode(const glm::vec2& pos, const glm::vec2& size, uint32_t id, InGuiDockNode* parent = nullptr)
			:
			Position(pos), Size(size), ID(id), Parent(parent)
		{
			Children[0] = nullptr;
			Children[1] = nullptr;
		}

		glm::vec2 Position;
		glm::vec2 Size;

		InGuiDockNode* Parent;
		InGuiDockNode* Children[2];
		std::vector<InGuiWindow*> Windows;
		uint32_t ID;
		SplitAxis Split = SplitAxis::None;
		DockPosition Dock = DockPosition::None;
	};

	class InGuiDockSpace
	{
		friend class InGuiContext;
	public:
		InGuiDockSpace(InGuiDockNode* root);
		~InGuiDockSpace();

		void InsertWindow(InGuiWindow* window, const glm::vec2& mousePos);
		void RemoveWindow(InGuiWindow* window);

		void FitToViewPort(const glm::vec2& size);

		void Begin();
		void End(const glm::vec2& mousePos, const InGuiRenderConfiguration& renderConfig);

		bool OnRightMouseButtonPress(const glm::vec2& mousePos);
		bool OnLeftMouseButtonPress();
		bool OnRightMouseButtonRelease(InGuiWindow* window, const glm::vec2& mousePos);

	private:
		void resize(const glm::vec2& mousePos);
		void adjustChildrenProps(InGuiDockNode* node);
		void detectResize(InGuiDockNode* node, const glm::vec2& mousePos);
		void insertWindow(InGuiWindow* window, const glm::vec2& mousePos, InGuiDockNode* node);
		void destroy(InGuiDockNode** node);
		void rescale(const glm::vec2& scale, InGuiDockNode* node);
		void splitNodeProportional(InGuiDockNode* node, SplitAxis axis, const glm::vec2& firstSize);
		void unsplitNode(InGuiDockNode* node);
		void update(InGuiDockNode* node);

		void showNode(InGuiDockNode* node, const glm::vec2& mousePos, const InGuiRenderConfiguration& renderConfig);
		DockPosition collideWithMarker(InGuiDockNode* node, const glm::vec2& mousePos);

	private:
		InGuiDockNode* m_Root;

		InGuiDockNode* m_ResizedNode = nullptr;

		uint32_t m_NodeCount = 0;
		bool m_DockSpaceVisible = false;
		static constexpr glm::vec2 sc_QuadSize = { 50,50 };

		friend class InGuiContext;
		friend class InGui;
	};


	struct InGuiPerFrameData
	{
		InGuiPerFrameData();
		void ResetWindowData();

		InGuiWindow* EventReceivingWindow;
		InGuiWindow* ModifiedWindow;
		InGuiWindow* CurrentWindow;

		glm::vec2 WindowSize;
		glm::vec2 ModifiedWindowMouseOffset;
		glm::vec2 WindowSpaceOffset;
		glm::vec2 MenuBarOffset;
		glm::vec2 PopupOffset;

		glm::vec2 MousePosition;
		glm::vec2 SelectedPoint;

		float MaxHeightInRow;
		float LastMenuBarWidth;

		MouseCode Code;
		KeyCode Key;
		KeyMode Mode;

		bool CapslockEnabled;	

		uint16_t Flags = 0;
	};


	using InGuiWindowMap = std::unordered_map<std::string, InGuiWindow*>;
	using InGuiEventListeners = std::vector<InGuiWindow*>;
	using InGuiRenderQueue = std::vector<InGuiMesh*>;

	struct InGuiContext
	{
		InGuiRenderConfiguration RenderConfiguration;
		InGuiPerFrameData PerFrameData;
		InGuiWindowMap Windows;
		InGuiRenderQueue RenderQueue;
		InGuiDockSpace* DockSpace = nullptr;
	};
}