#pragma once

#include "XYZ/Utils/VariableTypes.h"

#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace XYZ {


	struct ImGuiLink
	{
		ed::LinkId ID;
		ed::PinId  InputID;
		ed::PinId  OutputID;
	};

	
	class ImGuiNodeEditor;
	class ImGuiNode;

	struct ImGuiNodeValue
	{
		ImGuiNodeValue(ImGuiNode* parent);

		void OnImGuiRender(ImGuiNodeEditor* editor, ed::NodeId nodeID, float padding);

		ed::PinId InputPinID;
		ed::PinId OutputPinID;

		void SetName(std::string name);
		void SetType(VariableType type);

		const std::string& GetName() const { return m_Name; }
		VariableType GetType() const { return m_Type; }

	private:
		std::string  m_Name;
		VariableType m_Type = VariableType::None;

		ImGuiNode* m_Parent;
	};

	struct ImGuiNodeArgument
	{
		ImGuiNodeArgument(ImGuiNode* parent);

		void OnImGuiRender(ImGuiNodeEditor* editor, ed::NodeId nodeID);

		ed::PinId InputPinID;

		void SetName(std::string name);
		void SetType(VariableType type);

		const std::string& GetName() const { return m_Name; }
		VariableType GetType() const { return m_Type; }


	private:
		std::string  m_Name;
		VariableType m_Type = VariableType::None;
		
		ImGuiNode* m_Parent;
	};

	struct ImGuiNodeOutput
	{
		ImGuiNodeOutput(ImGuiNode* parent);

		void OnImGuiRender(ImGuiNodeEditor* editor, ed::NodeId nodeID, float padding);

		ed::PinId OutputPinID;

		void SetType(VariableType type);

		VariableType GetType() const { return m_Type; }

	private:
		VariableType m_Type = VariableType::None;
		float		 m_TypeTextSize = 0.0f;

		ImGuiNode* m_Parent;
	};

	class ImGuiNode
	{
	public:
		ImGuiNode(std::string name, ImGuiNodeEditor* editor);

		virtual ~ImGuiNode() = default;

		virtual void OnImGuiRender() = 0;

		virtual bool AcceptLink(ed::PinId inputPinID, ed::PinId outputPinID) = 0;

		virtual void DeleteLinks() = 0;

		virtual VariableType FindPinType(ed::PinId id) const = 0;
		
		const std::string& GetName() const { return m_Name; }
		const ed::NodeId   GetID() const { return m_ID; }

	private:
		std::string	m_Name;
		ed::NodeId	m_ID;	

	protected:
		uint32_t getNextID();

	protected:
		ImGuiNodeEditor* m_Editor;

		float m_OutputPadding = 0.0f;
		bool  m_RecalcPadding = true;

		friend ImGuiNodeEditor;
		friend ImGuiNodeValue;
		friend ImGuiNodeArgument;
		friend ImGuiNodeOutput;
	};


	class ImGuiValueNode : public ImGuiNode
	{
	public:
		ImGuiValueNode(std::string name, ImGuiNodeEditor* editor);
		
		virtual ~ImGuiValueNode() override;

		virtual void OnImGuiRender() override;

		virtual bool AcceptLink(ed::PinId inputPinID, ed::PinId outputPinID) override;

		virtual void DeleteLinks() override;

		virtual VariableType FindPinType(ed::PinId id) const override;

		void AddValue(std::string name, VariableType type);

		ImGuiNodeValue* FindValue(ed::PinId pinID);
		const ImGuiNodeValue* FindValue(ed::PinId pinID) const;

		const std::vector<ImGuiNodeValue>& GetValues() const { return m_Values; }

	private:
		void renderHeader();

		void calculatePadding();

	private:
		std::vector<ImGuiNodeValue> m_Values;
		
	};


	class ImGuiFunctionNode : public ImGuiNode
	{
	public:
		ImGuiFunctionNode(std::string name, ImGuiNodeEditor* editor);

		virtual ~ImGuiFunctionNode() override;

		virtual void OnImGuiRender() override;

		virtual bool AcceptLink(ed::PinId inputPinID, ed::PinId outputPinID) override;

		virtual void DeleteLinks() override;

		virtual VariableType FindPinType(ed::PinId id) const override;

		void AddInputArgument(std::string name, VariableType type);

		void AddOutput(VariableType type);

		ed::PinId GetInputPinID() const { return m_InputPinID;  }
		ed::PinId GetOutputPinID() const { return m_OutputPinID; }

		ImGuiNodeArgument* FindInputArgument(ed::PinId pinID);
		const ImGuiNodeArgument* FindInputArgument(ed::PinId pinID) const;
		

		const std::vector<ImGuiNodeArgument>& GetInputArguments() const { return m_InputArguments;}
		const std::vector<ImGuiNodeOutput>  & GetOutputArguments() const { return m_Outputs; }
	private:
		void renderHeader();

		void calculatePadding();

		void deleteInputArgumentLinks();
		void deleteOutputArgumentLinks();
	private:
		std::vector<ImGuiNodeArgument> m_InputArguments;
		std::vector<ImGuiNodeOutput>   m_Outputs;

		ed::PinId	 m_InputPinID;
		ed::PinId	 m_OutputPinID;		
	};
}