#pragma once
#include "XYZ/Core/KeyCodes.h"
#include "XYZ/Core/MouseCodes.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture2D.h"
#include "XYZ/Renderer/Framebuffer.h"
#include "XYZ/Renderer/InGuiRenderer.h"


namespace XYZ {
	namespace InGuiWindowFlag {
		enum InGuiWindowFlags
	{
			Moved			= BIT(0),
			Collapsed		= BIT(1),
			MenuEnabled		= BIT(2),
			Modified		= BIT(3),
			EventListener	= BIT(4),
			Hoovered		= BIT(5),
			LeftResizing	= BIT(6),
			RightResizing	= BIT(7),
			TopResizing		= BIT(8),
			BottomResizing	= BIT(9),
			Visible			= BIT(10),
			AutoPosition	= BIT(11),
			Docked			= BIT(12),
			Resized			= BIT(13),
			LeftClicked		= BIT(14),
			RightClicked	= BIT(15),
			Initialized		= BIT(16),
			ForceNewLine	= BIT(17),
			Dockable		= BIT(18)
		};
	}

	namespace InGuiPerFrameFlag {
		enum InGuiPerFrameFlags
		{
			LeftMouseButtonPressed	 = BIT(0),
			RightMouseButtonPressed	 = BIT(1),
			LeftMouseButtonReleased	 = BIT(2),
			RightMouseButtonReleased = BIT(3),
			ClickHandled			 = BIT(4),
			ReleaseHandled			 = BIT(5)
		};
	}

	namespace InGuiNodeFlag {
		enum InGuiNodeFlags
		{
			NodeMoved		= BIT(0),
			NodeModified	= BIT(1),
			NodeHoovered	= BIT(2)
		};
	}


	struct TextInfo
	{
		glm::vec2 Size = { 0.0f,0.0f };
		uint32_t Count = 0;
	};


	struct InGuiRenderConfiguration
	{
		InGuiRenderConfiguration();

		Ref<Font>		  Font;
		Ref<Texture2D>	  InTexture;
		Ref<Material>	  InMaterial;

		enum
		{
			BUTTON = 0,
			CHECKBOX_CHECKED,
			CHECKBOX_UNCHECKED,
			SLIDER,
			SLIDER_HANDLE,
			WINDOW,
			MIN_BUTTON,
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

		Ref<SubTexture2D> SubTexture[NUM_SUBTEXTURES];
		glm::vec4 Color[NUM_COLORS];

		static constexpr uint32_t DefaultTextureCount = 3;
		mutable uint32_t NumTexturesInUse = DefaultTextureCount;

		static constexpr uint32_t TextureID = 0;
		static constexpr uint32_t FontTextureID = 1;
		static constexpr uint32_t ColorPickerTextureID = 2;

		friend class InGui;
	};

	struct InGuiDockNode;
	struct InGuiWindow
	{
		InGuiMesh Mesh;
		InGuiLineMesh LineMesh;

		glm::vec2 Position = { 0.0f,0.0f };
		glm::vec2 Size = { 0.0f,0.0f };

		uint32_t Flags = 0;
		uint32_t ID = 0;
		float MinimalWidth = 0.0f;

		const char* Name = nullptr;

		InGuiDockNode* DockNode = nullptr;

		std::function<void(const glm::vec2&)> OnResizeCallback;
		static constexpr float PanelSize = 25.0f;
	};

	struct InGuiPerFrameData
	{
		InGuiPerFrameData();
		~InGuiPerFrameData();

		void ResetWindowData();

		InGuiWindow* EventReceivingWindow;
		InGuiWindow* ModifiedWindow;
		InGuiWindow* CurrentWindow;
		InGuiMesh* ActiveMesh;
		InGuiLineMesh* ActiveLineMesh;

		InGuiVertex* TempVertices;

		glm::mat4 ViewProjectionMatrix;
		glm::vec2 WindowSize;
		glm::vec2 PopupSize;
		glm::vec2 ModifiedWindowMouseOffset;
		glm::vec2 WindowSpaceOffset;
		glm::vec2 MenuBarOffset;
		glm::vec2 PopupOffset;
		
		float LeftNodePinOffset;
		float RightNodePinOffset;

		glm::vec2 MousePosition;
		glm::vec2 SelectedPoint;

		float MaxHeightInRow;
		float MenuItemOffset;

		float ItemOffset;

		int Code;
		int KeyCode;
		int Mode;
		bool CapslockEnabled;

		uint32_t PopupItemCount = 0;
		uint16_t Flags = 0;
		std::vector<TextureRendererIDPair> TexturePairs;
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
			VisibleWindow = nullptr;
		}

		glm::vec2 Position;
		glm::vec2 Size;

		InGuiDockNode* Parent;
		InGuiDockNode* Children[2];
		InGuiWindow* VisibleWindow;
		std::vector<InGuiWindow*> Windows;
		uint32_t ID;
		SplitAxis Split = SplitAxis::None;
		DockPosition Dock = DockPosition::None;
	};


	using InGuiWindowMap = std::unordered_map<uint32_t, InGuiWindow*>;
	using InGuiEventListeners = std::vector<InGuiWindow*>;

	class InGuiRenderQueue
	{
	public:
		void PushOverlay(InGuiMesh* mesh, InGuiLineMesh* lineMesh);
		void Push(InGuiMesh* mesh, InGuiLineMesh* lineMesh);
		void Reset();

		const std::vector<InGuiMesh*>& GetMeshes() const { return m_InGuiMeshes; }
		const std::vector<InGuiLineMesh*>& GetLineMeshes() const { return m_InGuiLineMeshes; }
	private:
		std::vector<InGuiMesh*> m_InGuiMeshes;
		std::vector<InGuiLineMesh*> m_InGuiLineMeshes;
		uint32_t m_NumOverLayers = 0;
	};

	struct InGuiFrameData
	{
		InGuiPerFrameData PerFrameData;
		InGuiRenderQueue RenderQueue;
	};
}