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
		else if (ed::ShowNodeContextMenu(&m_ContextNodeID))
			ImGui::OpenPopup("Node Context Menu");
		else if (ed::ShowLinkContextMenu(&m_ContextLinkID))
			ImGui::OpenPopup("Link Context Menu");

		handleBackgroundMenu();
		handleNodeMenu(m_ContextNodeID);
		handleLinkMenu(m_ContextLinkID);
		ed::Resume();

		ed::End();
		ed::SetCurrentEditor(nullptr);
	}

	void ImGuiNodeEditor::Clear()
	{
		for (auto node : m_Nodes)
			delete node;

		m_Nodes.clear();

		m_Links.clear();
		m_FreeIDs = std::queue<uint32_t>();
		m_UniqueID = 1;
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

	void ImGuiNodeEditor::RemoveNode(ed::NodeId id)
	{
		for (auto it = m_Nodes.begin(); it != m_Nodes.end(); ++it)
		{
			if ((*it)->GetID() == id)
			{
				delete (*it);
				m_Nodes.erase(it);
				return;
			}
		}
	}

	ImGuiNode* ImGuiNodeEditor::FindNode(ed::NodeId id)
	{
		for (auto node : m_Nodes)
		{
			if (node->GetID() == id)
				return node;
		}
		return nullptr;
	}

	const ImGuiLink* ImGuiNodeEditor::FindLink(ed::PinId pinID) const
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

	const ImGuiLink* ImGuiNodeEditor::FindLink(ed::PinId inputID, ed::PinId outputID) const
	{
		for (auto& link : m_Links)
		{
			if (link.InputID == inputID && link.OutputID == outputID)
				return &link;
		}
		return nullptr;
	}

	const ImGuiLink* ImGuiNodeEditor::FindLink(ed::LinkId linkID) const
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
	std::vector<ImGuiValueNode*> ImGuiNodeEditor::FindValueNodes() const
	{
		std::vector<ImGuiValueNode*> result;
		for (auto node : m_Nodes)
		{
			if (auto valueNode = dynamic_cast<ImGuiValueNode*>(node))
			{
				result.push_back(valueNode);
			}
		}
		return result;
	}

	std::vector<ImGuiFunctionNode*> ImGuiNodeEditor::FindFunctionSequence() const
	{
		std::vector<ImGuiFunctionNode*> result;
		// Find first node in sequence
		for (auto node : m_Nodes)
		{
			if (auto funcNode = dynamic_cast<ImGuiFunctionNode*>(node))
			{
				// Nothing is connected to the input => it is root
				if (!FindLink(funcNode->GetInputPinID()))
				{
					result.push_back(funcNode);
					break;
				}
			}
		}
		if (!result.empty())
		{
			addOutputFunctionSequence(result.back(), result);
		}
		return result;
	}

	void ImGuiNodeEditor::addOutputFunctionSequence(ImGuiFunctionNode* last, std::vector<ImGuiFunctionNode*>& result) const
	{
		for (auto node : m_Nodes)
		{
			if (auto funcNode = dynamic_cast<ImGuiFunctionNode*>(node))
			{
				auto link = FindLink(last->GetOutputPinID(), funcNode->GetInputPinID());
				if (link)
				{
					result.push_back(funcNode);
					addOutputFunctionSequence(result.back(), result);
					return;
				}
			}
		}
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
					// ed::AcceptNewItem() return true when user release mouse button.
					if (ed::AcceptNewItem())
					{
						if (acceptLink(inputPinID, outputPinID))
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

	void ImGuiNodeEditor::handleNodeMenu(ed::NodeId id)
	{
		if (ImGui::BeginPopup("Node Context Menu"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				RemoveNode(id);
			}
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