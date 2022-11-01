#pragma once

#include "XYZ/Scene/BlueprintVariableType.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"

#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace XYZ {
	namespace UI {

		class ImGuiNodeContext;
		class ImGuiNode;

		struct ImGuiNodeID
		{
			ImGuiNodeID(std::shared_ptr<ImGuiNodeContext> context);
		
			void Destroy();

			bool operator==(const ed::LinkId id) const
			{
				return m_ID == id.Get();
			}

			bool operator==(const ed::PinId id) const
			{
				return m_ID == id.Get();
			}
			bool operator ==(const ed::NodeId id) const
			{
				return m_ID == id.Get();
			}

			operator uint32_t () const { return m_ID; }
			operator ed::PinId() const { return m_ID; }
			operator ed::LinkId() const { return m_ID; }
			operator ed::NodeId() const { return m_ID; }
		private:
			std::shared_ptr<ImGuiNodeContext> m_Context;
			uint32_t m_ID;
		};

		struct ImGuiLink
		{
			ImGuiLink(const std::shared_ptr<ImGuiNodeContext>& context);

			ImGuiNodeID ID;
			ed::PinId InputID;
			ed::PinId OutputID;
		};
		

		enum ImGuiNodeValueFlags
		{
			ImGuiNodeValueFlags_AllowInput		= BIT(0),
			ImGuiNodeValueFlags_AllowOutput		= BIT(1),
			ImGuiNodeValueFlags_AllowEdit		= BIT(2),
			ImGuiNodeValueFlags_AllowName		= BIT(3)
		};

		class ImGuiNodeValue
		{
		public:
			ImGuiNodeValue(std::shared_ptr<ImGuiNodeContext> context, ImGuiNode& parent, uint32_t flags);
			~ImGuiNodeValue();

			void Render();
			
			void SetValueName(std::string name);
			void SetValueType(VariableType type);
			void SetFlags(uint32_t flags);
			
			bool HasFlag(uint32_t flag) const { return IS_SET(m_Flags, flag); }
			uint32_t GetFlags() const { return m_Flags; }
			const std::string& GetName() const { return m_Name; }
			const VariableType& GetType() const { return m_Type; }

			const ImGuiNodeID& GetInputID() const { return m_InputPinID; }
			const ImGuiNodeID& GetOutputID() const { return m_OutputPinID; }

		private:
			std::shared_ptr<ImGuiNodeContext> m_Context;
			ImGuiNode&		m_Parent;
			std::string		m_Name;
			VariableType	m_Type;
			
			ImGuiNodeID		m_InputPinID;
			ImGuiNodeID		m_OutputPinID;

			ByteBuffer		m_Data;
			uint32_t		m_Flags = 0;
		};


		enum ImGuiNodeFlags
		{
			ImGuiNodeFlags_AllowInput	 = BIT(0),
			ImGuiNodeFlags_AllowOutput   = BIT(1),
			ImGuiNodeFlags_AllowName	 = BIT(2)
		};

		class ImGuiNode
		{
		public:
			ImGuiNode(std::shared_ptr<ImGuiNodeContext> context, uint32_t flags);

			void Render();

			bool AcceptLink(ed::PinId inputID, ed::PinId outputID) const;
			void AddValue(std::string name, const VariableType& type, uint32_t flags);

			void SetName(std::string name);
			void SetType(const VariableType& type);
			void SetFlags(uint32_t flags);

			const VariableType& GetType() const { return m_Type; }

			const ImGuiNodeID& GetID() const { return m_ID; }
			const ImGuiNodeID& GetInputID() const { return m_InputID; }
			const ImGuiNodeID& GetOutputID() const { return m_OutputID; }
		
			const std::vector<ImGuiNodeValue>& GetValues() const { return m_Values; }
		private:
			void calculatePadding();

			bool acceptNodeLink(ed::PinId inputID, ed::PinId outputID) const;
			bool acceptValueLink(ed::PinId inputID, ed::PinId outputID) const;

		private:
			std::shared_ptr<ImGuiNodeContext> m_Context;
			ImGuiNodeID m_ID;
			ImGuiNodeID m_InputID;
			ImGuiNodeID m_OutputID;
			
			VariableType m_Type;
			std::string  m_Name;

			uint32_t m_Flags;

			std::vector<ImGuiNodeValue> m_Values;
			
			float m_ValueTypeMaxWidth = 0.0f;
			float m_ValueNameMaxWidth = 0.0f;
			float m_ValueEditMaxWidth = 0.0f;

			bool  m_RecalcPadding = true;

			friend ImGuiNodeValue;
		};

	}
}