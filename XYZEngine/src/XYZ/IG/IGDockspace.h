#pragma once
#include "IGUIElements.h"

#include <glm/glm.hpp>


namespace XYZ {

	enum class IGSplitType
	{
		None,
		Horizontal,
		Vertical
	};

	struct IGDockNode
	{
		IGDockNode();
		IGDockNode(uint32_t id);
		~IGDockNode();

		IGDockNode* Parent = nullptr;
		IGDockNode* FirstChild = nullptr;
		IGDockNode* SecondChild = nullptr;
		
		IGSplitType Type = IGSplitType::None;
		uint32_t ID;

		struct NodeData
		{
			glm::vec2 Size = glm::vec2(0.0f);
			glm::vec2 Position = glm::vec2(0.0f);
			std::vector<IGWindow*> Windows;
		};
		NodeData Data;
	};

	class IGDockspace
	{
	public:
		IGDockspace();
		IGDockspace(const glm::vec2& pos, const glm::vec2& size);
		~IGDockspace();

		void SetRootSize(const glm::vec2& size);
		void SubmitToRenderer(IGRenderData* renderData);
		void SetVisibility(bool visibility) { m_Visible = visibility; }

		bool InsertWindow(IGWindow* window, const glm::vec2& mousePos);
		bool RemoveWindow(IGWindow* window);

		bool OnMouseLeftPress(const glm::vec2& mousePos, bool& handled);
		bool OnMouseLeftRelease();
		bool OnMouseMove(const glm::vec2& mousePos, bool& handled);

	private:
		IGMesh      m_Mesh;
		MemoryPool  m_Pool;
		IGDockNode* m_Root;
		IGDockNode* m_ResizedNode = nullptr;
		bool		m_Visible = false;
	};

}