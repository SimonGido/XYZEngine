#pragma once

#include "XYZ/Core/Timestep.h"

#include "ImGuiNode.h"

#include <imgui.h>
#include <imgui_node_editor.h>

namespace XYZ {

	class ImGuiNodeEditor
	{
	public:
		ImGuiNodeEditor(const std::string& name);

		void OnStart();
		void OnStop();
		
		void OnUpdate(Timestep ts);

		ImGuiNodeValue* FindNodeValue(ed::PinId pinID);
		ImGuiLink*		FindLink(ed::PinId pinID);
		ImGuiLink*		FindLink(ed::LinkId linkID);
	private:
		void createLinks();
		void deleteLinks();

		void handleDeleteLink(ed::LinkId id);
	private:
		std::string m_Name;
		std::string m_SaveFile;
		ax::NodeEditor::EditorContext* m_Context;

		std::vector<ImGuiNode> m_Nodes;
		std::vector<ImGuiLink> m_Links;

		uint32_t m_UniqueID = 0;

		ed::LinkId m_ContextLinkID;
	};

}