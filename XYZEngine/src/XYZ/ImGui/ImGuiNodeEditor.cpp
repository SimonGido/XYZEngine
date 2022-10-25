#include "stdafx.h"
#include "ImGuiNodeEditor.h"

namespace ed = ax::NodeEditor;

namespace XYZ {
	ImGuiNodeEditor::ImGuiNodeEditor(const std::string& name)
		:
		m_Name(name)
	{
		m_SaveFile = m_Name + ".json";
	}
	ImGuiNodeEditor::~ImGuiNodeEditor()
	{
		for (auto node : m_Nodes)
			delete node;
	}
	void ImGuiNodeEditor::OnStart()
	{
		ed::Config config;
		config.SettingsFile = m_SaveFile.c_str();
		m_Context = ed::CreateEditor(&config);
	}
	void ImGuiNodeEditor::OnStop()
	{
		ed::DestroyEditor(m_Context);
	}

	void ImGuiNodeEditor::OnUpdate(Timestep ts)
	{
		auto& io = ImGui::GetIO();

		if (ImGui::Begin(m_Name.c_str()))
		{
			ed::SetCurrentEditor(m_Context);
			ed::Begin(m_Name.c_str());
			
			for (auto node : m_Nodes)
			{
				node->OnImGuiRender();
			}
			// Submit Links
			for (auto& linkInfo : m_Links)
				ed::Link(linkInfo.ID, linkInfo.InputID, linkInfo.OutputID);
		
			createLinks();
			deleteLinks();
	
			ed::Suspend();
			if (ed::ShowBackgroundContextMenu())
				ImGui::OpenPopup("Background Menu");
			else if (ed::ShowLinkContextMenu(&m_ContextLinkID))
				ImGui::OpenPopup("Link Context Menu");

			handleBackgroundMenu();
			handleLinkMenu(m_ContextLinkID);
			ed::Resume();

			ed::End();
			ed::SetCurrentEditor(nullptr);
		}
		ImGui::End();
	}

	

	void ImGuiNodeEditor::RemoveNode(const std::string_view name)
	{
		for (auto it = m_Nodes.begin(); it != m_Nodes.end(); ++it)
		{
			if ((*it)->GetName() == name)
			{
				delete (*it);
				m_Nodes.erase(it);
				return;
			}
		}
	}

	ImGuiLink* ImGuiNodeEditor::FindLink(ed::PinId pinID)
	{
		for (auto& link : m_Links)
		{
			if (link.InputID == pinID)
				return &link;
			if (link.OutputID == pinID)
				return &link;
		}
		return nullptr;
	}

	ImGuiLink* ImGuiNodeEditor::FindLink(ed::LinkId linkID)
	{
		for (auto& link : m_Links)
		{
			if (link.ID == linkID)
				return &link;
		}
		return nullptr;
	}

	VariableType ImGuiNodeEditor::FindPinType(ed::PinId pinID)
	{
		for (auto node : m_Nodes)
		{
			auto type = node->FindPinType(pinID);
			if (type != VariableType::None)
				return type;
		}
		return VariableType::None;
	}

	void ImGuiNodeEditor::createLinks()
	{
		// Handle creation action, returns true if editor want to create new object (node or link)
		if (ed::BeginCreate())
		{
			ed::PinId inputPinID, outputPinID;
			if (ed::QueryNewLink(&inputPinID, &outputPinID))
			{
				if (inputPinID && outputPinID) // both are valid, let's accept link
				{
					if (acceptLink(inputPinID, outputPinID))
					{
						// ed::AcceptNewItem() return true when user release mouse button.
						if (ed::AcceptNewItem())
						{
							// Since we accepted new link, lets add one to our list of links.
							m_Links.push_back({ ed::LinkId(m_UniqueID++), inputPinID, outputPinID });

							// Draw new link.
							ed::Link(m_Links.back().ID, m_Links.back().InputID, m_Links.back().OutputID);
						}
					}
				}
			}
		}
		ed::EndCreate(); // Wraps up object creation action handling.
	}
	void ImGuiNodeEditor::deleteLinks()
	{
		if (ed::BeginDelete())
		{
			// There may be many links marked for deletion, let's loop over them.
			ed::LinkId deletedLinkId;
			while (ed::QueryDeletedLink(&deletedLinkId))
			{
				// If you agree that link can be deleted, accept deletion.
				if (ed::AcceptDeletedItem())
				{
					// Then remove link from your data.
					for (auto it = m_Links.begin(); it != m_Links.end(); ++it)
					{
						if (it->ID == deletedLinkId)
						{
							m_Links.erase(it);
							break;
						}
					}
				}
				// You may reject link deletion by calling:
				// ed::RejectDeletedItem();
			}
		}
		ed::EndDelete();
	}



	void ImGuiNodeEditor::handleBackgroundMenu()
	{
		if (ImGui::BeginPopup("Background Menu"))
		{
			if (OnBackgroundMenu)
				OnBackgroundMenu();

			ImGui::EndPopup();
		}
	}

	void ImGuiNodeEditor::handleLinkMenu(ed::LinkId id)
	{
		if (ImGui::BeginPopup("Link Context Menu"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				auto link = FindLink(id);
				if (link)
				{
					ed::DeleteLink(link->ID);
				}
			}
			ImGui::EndPopup();
		}
	}

	bool ImGuiNodeEditor::acceptLink(ed::PinId inputPinID, ed::PinId outputPinID) const
	{
		for (auto node : m_Nodes)
		{
			if (node->AcceptLink(inputPinID, outputPinID))
				return true;
		}
		return false;
	}

	uint32_t ImGuiNodeEditor::getNextID()
	{
		if (m_FreeIDs.empty())
			return m_UniqueID++;

		uint32_t result = m_FreeIDs.back();
		m_FreeIDs.pop();
		return result;
	}
}