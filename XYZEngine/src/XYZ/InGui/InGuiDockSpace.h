#pragma once
#include "InGuiWindow.h"
#include "XYZ/Utils/DataStructures/MemoryPool.h"

#include <glm/glm.hpp>

namespace XYZ {
	
	enum class SplitType
	{
		Vertical, Horizontal, None
	};
	struct InGuiDockNode
	{
		InGuiDockNode(InGuiID id);

		std::pair<InGuiDockNode*, InGuiDockNode*> SplitNode(SplitType split);
		void									  UnSplit();
		glm::vec2								  GetCenter() const;
		glm::vec2								  GetAbsCenter() const;
		glm::vec2								  GetAbsPosition() const;

		InGuiRect								  LeftRect() const;
		InGuiRect								  RightRect() const;
		InGuiRect								  TopRect() const;
		InGuiRect								  BottomRect() const;
		InGuiRect								  MiddleRect() const;

		InGuiID					  ID;
		std::vector<InGuiWindow*> DockedWindows;

		InGuiDockNode* Parent;
		InGuiDockNode* Sibling;
		InGuiDockNode* Children[2];
		SplitType      Split;

		glm::vec2 Position;
		glm::vec2 Size;
	};

	struct InGuiDockSpace
	{
		InGuiDockSpace();

		void Init(const glm::vec2& position, const glm::vec2& size);
		void Destroy();

		bool IsInitialized() const { return m_Root; }

		

		InGuiDockNode* m_Root;
		MemoryPool m_Pool;
	};
}