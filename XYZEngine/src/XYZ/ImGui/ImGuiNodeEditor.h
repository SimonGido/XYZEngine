#pragma once

#include "XYZ/Core/Timestep.h"

#include "ImGuiNode.h"
#include "ImGuiVariableTypeExtension.h"

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

		void Clear();

		template <typename T>
		T* AddNode(std::string name);

		void RemoveNode(const std::string_view name);
		
		void RemoveNode(ed::NodeId id);

		ImGuiNode*			FindNode(ed::NodeId id);
		ImGuiNodeValue*		FindNodeValue(ed::PinId id);
		ImGuiFunctionNode*  FindFunctionNode(ed::PinId id);
		ImGuiFunctionNode*  FindFunctionNodeByInput(ed::PinId id);

		const ImGuiLink*	FindLink(ed::PinId pinID) const;
		const ImGuiLink*	FindLink(ed::PinId inputID, ed::PinId outputID) const;
		const ImGuiLink*	FindLink(ed::LinkId linkID) const;
		const VariableType*	FindPinType(ed::PinId pinID) const;

		std::vector<ImGuiValueNode*>	FindValueNodes() const;
		std::vector<ImGuiFunctionNode*> FindFunctionSequence() const;

		std::function<void()> OnBackgroundMenu;

		ImGuiVariableTypeExtension VariableExtension;
	private:
		void addOutputFunctionSequence(ImGuiFunctionNode* last, std::vector<ImGuiFunctionNode*>& result) const;
		void createLinks();
		void deleteLinks();

		void handleBackgroundMenu();
		void handleNodeMenu(ed::NodeId id);
		void handleLinkMenu(ed::LinkId id);

		bool acceptLink(ed::PinId inputPinID, ed::PinId outputPinID) const;

		uint32_t getNextID();
	
	private:
		std::string m_Name;
		std::string m_SaveFile;
		ax::NodeEditor::EditorContext* m_Context;

		std::vector<ImGuiNode*> m_Nodes;
		std::vector<ImGuiLink>  m_Links;

		uint32_t m_UniqueID = 1;

		ed::LinkId m_ContextLinkID;
		ed::NodeId m_ContextNodeID;

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