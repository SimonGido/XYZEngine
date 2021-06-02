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
		~InGuiDockNode();

		std::pair<InGuiDockNode*, InGuiDockNode*> SplitNode(SplitType split, bool moveWindowsToFirst);
		void									  UnSplit();
		void									  Update();
		void									  RemoveWindow(InGuiWindow* window);
		void									  HandleResize(const glm::vec2& mousePosition);
		

		glm::vec2								  GetCenter() const;
		glm::vec2								  GetAbsCenter() const;
		glm::vec2								  GetAbsPosition() const;

		InGuiRect								  Rect() const;
		InGuiRect								  LeftRect() const;
		InGuiRect								  RightRect() const;
		InGuiRect								  TopRect() const;
		InGuiRect								  BottomRect() const;
		InGuiRect								  MiddleRect() const;

		InGuiID					  ID;
		std::vector<InGuiWindow*> DockedWindows;

		InGuiDockNode* Parent;
		InGuiDockNode* Children[2];
		SplitType      Split;

		glm::vec2 Position;
		glm::vec2 Size;

	private:
		void fitChildren(const glm::vec2& originalSize);
	};

	struct InGuiDockSpace
	{
		InGuiDockSpace();

		void Init(const glm::vec2& position, const glm::vec2& size);
		void Destroy();
		void Resize(const glm::vec2& size);
		bool FindResizedNode(const glm::vec2& mousePosition);
		bool PushNodeRectangle(const InGuiRect& rect);
		bool InsertWindow(InGuiWindow* window);
		bool IsInitialized() const { return Root; }

		InGuiID GetNextID();

		InGuiDockNode*							Root;
		InGuiDrawList							Drawlist;
		MemoryPool<10 * sizeof(InGuiDockNode)>	Pool;

		InGuiDockNode* ResizedNode;

	private:
		std::queue<InGuiID> FreeIDs;
		InGuiID NextID;
		
		friend InGuiDockNode;
		friend class InGuiSerializer;
	};
}