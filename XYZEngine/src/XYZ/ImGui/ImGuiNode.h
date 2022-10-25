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

		void OnImGuiRender(ImGuiNodeEditor* editor, ed::NodeId nodeID, float& padding);

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

		bool m_RecalcPadding = true;
	};

	struct ImGuiNodeArgument
	{
		ImGuiNodeArgument(ImGuiNode* parent);

		void OnImGuiRender(ImGuiNodeEditor* editor, ed::NodeId nodeID, float& padding);

		ed::PinId InputPinID;

		void SetName(std::string name);
		void SetType(VariableType type);

		const std::string& GetName() const { return m_Name; }
		VariableType GetType() const { return m_Type; }


	private:
		std::string  m_Name;
		VariableType m_Type = VariableType::None;
		
		ImGuiNode* m_Parent;

		bool m_RecalcPadding = true;
	};

	struct ImGuiNodeOutput
	{
		ImGuiNodeOutput(ImGuiNode* parent);

		void OnImGuiRender(ImGuiNodeEditor* editor, ed::NodeId nodeID, float& padding);

		ed::PinId OutputPinID;

		void SetType(VariableType type);

		VariableType GetType() const { return m_Type; }

	private:
		VariableType m_Type = VariableType::None;

		ImGuiNode* m_Parent;

		bool m_RecalcPadding = true;
	};

	class ImGuiNode
	{
	public:
		ImGuiNode(std::string name, ImGuiNodeEditor* editor);

		virtual ~ImGuiNode() = default;

		virtual void OnImGuiRender() = 0;

		virtual bool AcceptLink(ed::PinId inputPinID, ed::PinId outputPinID) = 0;

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

		virtual VariableType FindPinType(ed::PinId id) const override;

		void AddValue(std::string name, VariableType type);

		ImGuiNodeValue* FindValue(ed::PinId pinID);
		const ImGuiNodeValue* FindValue(ed::PinId pinID) const;

	private:
		void renderHeader();

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

		virtual VariableType FindPinType(ed::PinId id) const override;

		void AddInputArgument(std::string name, VariableType type);

		void AddOutput(VariableType type);


		ImGuiNodeArgument* FindInputArgument(ed::PinId pinID);
		const ImGuiNodeArgument* FindInputArgument(ed::PinId pinID) const;


		void SetOutputType(VariableType type);

		VariableType GetOutputType() const { return m_OutputType; }

		
	private:
		void renderHeader();
		void renderOutputValuePin();

	private:
		std::vector<ImGuiNodeArgument> m_InputArguments;
		std::vector<ImGuiNodeOutput>   m_Outputs;

		VariableType m_OutputType = VariableType::None;
		ed::PinId	 m_OutputValuePinID;
		ed::PinId	 m_InputPinID;
		ed::PinId	 m_OutputPinID;

		bool m_RecalcPadding = true;
	};
}