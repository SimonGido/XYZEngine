#pragma once

#include "XYZ/Core/Timestep.h"

#include "ImGuiNode.h"

#include <imgui.h>
#include <imgui_node_editor.h>

#include <queue>

namespace XYZ {

	class ImGuiNodeEditor
	{
	public:
		ImGuiNodeEditor(const std::string& name);
		~ImGuiNodeEditor();

		void OnStart();
		void OnStop();
		
		void OnUpdate(Timestep ts);

		template <typename T>
		T* AddNode(std::string name);

		void RemoveNode(const std::string_view name);
		
		ImGuiLink*	FindLink(ed::PinId pinID);
		ImGuiLink*	FindLink(ed::LinkId linkID);
		VariableType FindPinType(ed::PinId pinID);

		std::function<void()> OnBackgroundMenu;

	private:
		void createLinks();
		void deleteLinks();

		void handleBackgroundMenu();
		void handleLinkMenu(ed::LinkId id);

		bool acceptLink(ed::PinId inputPinID, ed::PinId outputPinID) const;

		uint32_t getNextID();
	
	private:
		std::string m_Name;
		std::string m_SaveFile;
		ax::NodeEditor::EditorContext* m_Context;

		std::vector<ImGuiNode*> m_Nodes;
		std::vector<ImGuiLink>  m_Links;

		uint32_t m_UniqueID = 0;

		ed::LinkId m_ContextLinkID;

		std::queue<uint32_t> m_FreeIDs;

		friend ImGuiNode;
		friend ImGuiValueNode;
		friend ImGuiFunctionNode;
	};

	template<typename T>
	inline T* ImGuiNodeEditor::AddNode(std::string name)
	{
		static_assert(std::is_base_of_v<ImGuiNode, T>, "Node must be derived from ImGuiNode");
		T* newNode = new T(std::move(name), this);
		m_Nodes.push_back(newNode);
		return newNode;
	}

}