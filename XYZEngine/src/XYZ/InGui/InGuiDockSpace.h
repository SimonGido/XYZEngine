#pragma once
#include "InGuiStructures.h"

namespace XYZ {
	class InGuiDockSpace
	{
		friend class InGuiContext;
	public:
		InGuiDockSpace(InGuiDockNode* root);
		~InGuiDockSpace();

		void InsertWindow(InGuiWindow* window, const glm::vec2& mousePos);
		void RemoveWindow(InGuiWindow* window);

		void Begin();
		void Update(const glm::vec2& mousePos);
		void Render(const glm::vec2& mousePos, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);

		bool OnRightMouseButtonPress(const glm::vec2& mousePos);
		bool OnLeftMouseButtonPress();
		bool OnRightMouseButtonRelease(InGuiWindow* window, const glm::vec2& mousePos);
		bool OnWindowResize(const glm::vec2& winSize);

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
		void updateAll(InGuiDockNode* node);
		void showNodeWindows(InGuiDockNode* node, const glm::vec2& mousePos, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
		void showNode(InGuiDockNode* node, const glm::vec2& mousePos, const InGuiRenderConfiguration& renderConfig);
		DockPosition collideWithMarker(InGuiDockNode* node, const glm::vec2& mousePos);

	private:
		InGuiDockNode* m_Root;

		InGuiDockNode* m_ResizedNode = nullptr;

		uint32_t m_NextNodeID = 0;
		std::queue<uint32_t> m_FreeIDs;

		bool m_DockSpaceVisible = false;
		static constexpr glm::vec2 sc_QuadSize = { 50,50 };

		friend class InGuiContext;
		friend class InGui;
	};
}