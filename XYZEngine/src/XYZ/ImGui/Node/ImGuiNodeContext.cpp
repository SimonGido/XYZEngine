#include "stdafx.h"
#include "ImGuiNodeContext.h"

#include "XYZ/Debug/Profiler.h"


namespace XYZ {
	namespace UI {
		ImGuiNodeContext::ImGuiNodeContext(const std::string& name)
			:
			m_Name(name)
		{
			m_SaveFile = m_Name + ".json";
		}

		ImGuiNodeContext::~ImGuiNodeContext()
		{
			for (auto node : m_Nodes)
				delete node;
		}

		void ImGuiNodeContext::Clear()
		{
			for (auto node : m_Nodes)
				delete node;
			m_Nodes.clear();
			m_Links.clear();
			m_UniqueID = 1;
		}


		void ImGuiNodeContext::OnStart()
		{
			ed::Config config;
			config.SettingsFile = m_SaveFile.c_str();
			m_Context = ed::CreateEditor(&config);
		}
		void ImGuiNodeContext::OnStop()
		{
			ed::DestroyEditor(m_Context);
		}
		void ImGuiNodeContext::OnUpdate(Timestep ts)
		{
			XYZ_PROFILE_FUNC("ImGuiNodeEditor::OnUpdate");
			auto& io = ImGui::GetIO();

			ed::SetCurrentEditor(m_Context);
			ed::Begin(m_Name.c_str());

			for (auto node : m_Nodes)
			{
				node->Render();
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
		ImGuiNode* ImGuiNodeContext::AddNode(uint32_t flags)
		{
			ImGuiNode* node = new ImGuiNode(shared_from_this(), flags);
			m_Nodes.push_back(node);
			return node;
		}
		void ImGuiNodeContext::RemoveNode(ed::NodeId id)
		{
			size_t i = 0;
			for (auto node : m_Nodes)
			{
				if (node->GetID() == id)
				{
					m_Nodes.erase(m_Nodes.begin() + i);
					return;
				}
				i++;
			}
		}
		const ImGuiNode* ImGuiNodeContext::FindNodeByOutputID(ed::PinId id) const
		{
			for (auto node : m_Nodes)
			{
				if (node->GetOutputID() == id)
					return node;
			}
			return nullptr;
		}
		const ImGuiNode* ImGuiNodeContext::FindNodeByInputID(ed::PinId id) const
		{
			for (auto node : m_Nodes)
			{
				if (node->GetInputID() == id)
					return node;
			}
			return nullptr;
		}
		const ImGuiNodeValue* ImGuiNodeContext::FindValueByOutputID(ed::PinId id) const
		{
			for (auto node : m_Nodes)
			{
				for (auto& value : node->GetValues())
				{
					if (value.GetOutputID() == id)
						return &value;
				}
			}
			return nullptr;
		}
		const ImGuiNodeValue* ImGuiNodeContext::FindValueByInputID(ed::PinId id) const
		{
			for (auto node : m_Nodes)
			{
				for (auto& value : node->GetValues())
				{
					if (value.GetInputID() == id)
						return &value;
				}
			}
			return nullptr;
		}

		std::vector<const ImGuiNode*> ImGuiNodeContext::FindNodeSequence(const std::string& entryName) const
		{
			std::vector<const ImGuiNode*> result;
			// Find first node in sequence
			for (auto node : m_Nodes)
			{
				if (node->GetName() == entryName)
				{
					result.push_back(node);
					break;
				}
			}
			if (!result.empty())
			{
				addToNodeSequence(result.back(), result);
			}
			return result;
		}

		const ImGuiLink* ImGuiNodeContext::findLink(ed::LinkId id) const
		{
			for (auto& link : m_Links)
			{
				if (link.ID == id)
					return &link;
			}
			return nullptr;
		}

		const ImGuiLink* ImGuiNodeContext::findLink(ed::PinId inputID, ed::PinId outputID) const
		{
			for (auto& link : m_Links)
			{
				if (link.InputID == inputID 
				 && link.OutputID == outputID)
					return &link;
			}
			return nullptr;
		}

		

		void ImGuiNodeContext::addToNodeSequence(const ImGuiNode* last, std::vector<const ImGuiNode*>& result) const
		{
			for (auto node : m_Nodes)
			{
				auto link = findLink(last->GetOutputID(), node->GetInputID());
				if (link)
				{
					result.push_back(node);
					addToNodeSequence(result.back(), result);
					return;
				}
			}
		}

		void ImGuiNodeContext::createLinks()
		{
			XYZ_PROFILE_FUNC("ImGuiNodeEditor::createLinks");
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
								ImGuiLink link(shared_from_this());
								link.InputID = inputPinID;
								link.OutputID = outputPinID;
								// Since we accepted new link, lets add one to our list of links.
								m_Links.push_back(link);

								// Draw new link.
								ed::Link(m_Links.back().ID, m_Links.back().InputID, m_Links.back().OutputID);
							}
						}

					}
				}
			}
			ed::EndCreate(); // Wraps up object creation action handling.
		}
		void ImGuiNodeContext::deleteLinks()
		{
			XYZ_PROFILE_FUNC("ImGuiNodeEditor::deleteLinks");
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
		void ImGuiNodeContext::handleBackgroundMenu()
		{
			if (ImGui::BeginPopup("Background Menu"))
			{
				if (OnBackgroundMenu)
					OnBackgroundMenu();

				ImGui::EndPopup();
			}
		}
		void ImGuiNodeContext::handleNodeMenu(ed::NodeId id)
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
		void ImGuiNodeContext::handleLinkMenu(ed::LinkId id)
		{
			if (ImGui::BeginPopup("Link Context Menu"))
			{
				if (ImGui::MenuItem("Delete"))
				{
					auto link = findLink(id);
					if (link)
					{
						ed::DeleteLink(link->ID);
					}
				}
				ImGui::EndPopup();
			}
		}
		bool ImGuiNodeContext::acceptLink(ed::PinId inputPinID, ed::PinId outputPinID) const
		{
			for (auto node : m_Nodes)
			{
				if (node->AcceptLink(inputPinID, outputPinID))
					return true;
			}
			return false;
		}
		uint32_t ImGuiNodeContext::getNextID()
		{
			if (m_FreeIDs.empty())
				return m_UniqueID++;

			uint32_t result = m_FreeIDs.back();
			m_FreeIDs.pop();
			return result;
		}
	}
}