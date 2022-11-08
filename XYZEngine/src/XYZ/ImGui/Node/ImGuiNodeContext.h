#pragma once
#include "XYZ/Core/Timestep.h"

#include "ImGuiNode.h"
#include "XYZ/ImGui/ImGuiVariableTypeExtension.h"

#include <imgui.h>
#include <imgui_node_editor.h>

#include <queue>

namespace XYZ {
	namespace UI {
		class ImGuiNodeContext : public std::enable_shared_from_this<ImGuiNodeContext>
		{
		public:
			ImGuiNodeContext(const std::string& name);
			~ImGuiNodeContext();

			void Clear();

			void OnStart();
			void OnStop();
			void OnUpdate(Timestep ts);

			ImGuiNode* AddNode(uint32_t flags);

			void RemoveNode(ed::NodeId id);
			
			const ImGuiNode* FindNodeByOutputID(ed::PinId id) const;
			const ImGuiNode* FindNodeByInputID(ed::PinId id) const;
			const ImGuiNodeValue* FindValueByOutputID(ed::PinId id) const;
			const ImGuiNodeValue* FindValueByInputID(ed::PinId id) const;

			std::vector<const ImGuiNode*> FindNodeSequence(const std::string& entryName) const;
			const std::vector<ImGuiNode*>& GetNodes() const { return m_Nodes; }

			std::function<void()> OnBackgroundMenu;

			ImGuiVariableTypeExtension VariableExtension;
		private:
			const ImGuiLink* findLink(ed::LinkId id) const;
			const ImGuiLink* findLink(ed::PinId inputID, ed::PinId outputID) const;

			void addToNodeSequence(const ImGuiNode* last, std::vector<const ImGuiNode*>& result) const;
			
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
			std::vector<ImGuiLink> m_Links;

			uint32_t m_UniqueID = 1;

			ed::LinkId m_ContextLinkID;
			ed::NodeId m_ContextNodeID;

			std::queue<uint32_t> m_FreeIDs;

			friend ImGuiNodeID;
		};
	}
}