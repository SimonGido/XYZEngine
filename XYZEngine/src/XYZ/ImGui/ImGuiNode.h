#pragma once

#include "XYZ/Scene/BlueprintVariableType.h"

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
	class ImGuiValueNode;
	class ImGuiFunctionNode;

	struct ImGuiNodeValue
	{
		ImGuiNodeValue(ImGuiValueNode* parent);

		void OnImGuiRender(ImGuiNodeEditor* editor, ed::NodeId nodeID);

		void SetName(std::string name);
		void SetType(const VariableType& type);
		void SetEditValueWidth(float width);
		void SetAllowEdit(bool val);

		const std::string& GetName() const { return m_Name; }
		const VariableType& GetType() const { return m_Type; }


		std::array<std::byte, 128> Data;
	private:
		ImGuiValueNode*  m_Parent;
		std::string		 m_Name;
		VariableType	 m_Type;
		ed::PinId		 m_InputPinID;
		ed::PinId		 m_OutputPinID;
		float			 m_EditValueWidth = 150.0f;
		bool			 m_AllowEditData = true;

		friend ImGuiValueNode;
	};

	struct ImGuiNodeArgument
	{
		ImGuiNodeArgument(ImGuiFunctionNode* parent);

		void OnImGuiRender(ImGuiNodeEditor* editor, ed::NodeId nodeID);

		ed::PinId InputPinID;

		void SetName(std::string name);
		void SetType(const VariableType& type);

		const std::string& GetName() const { return m_Name; }
		const VariableType& GetType() const { return m_Type; }


	private:
		std::string  m_Name;
		VariableType m_Type;

		ImGuiFunctionNode* m_Parent;
	};



	struct ImGuiNodeOutput
	{
		ImGuiNodeOutput(ImGuiFunctionNode* parent);

		void OnImGuiRender(ImGuiNodeEditor* editor, ed::NodeId nodeID);

		ed::PinId OutputPinID;

		void SetType(const VariableType& type);

		const VariableType& GetType() const { return m_Type; }

	private:
		VariableType m_Type;

		ImGuiFunctionNode* m_Parent;
	};

	class ImGuiNode
	{
	public:
		ImGuiNode(std::string name, ImGuiNodeEditor* editor);

		virtual ~ImGuiNode() = default;

		virtual void OnImGuiRender() = 0;

		virtual bool AcceptLink(ed::PinId inputPinID, ed::PinId outputPinID) { return false; };

		virtual void DeleteLinks() = 0;

		virtual const VariableType* FindPinType(ed::PinId id) const = 0;

		const std::string& GetName() const { return m_Name; }
		const ed::NodeId   GetID() const { return m_ID; }

	private:
		std::string	m_Name;
		ed::NodeId	m_ID;

	protected:
		uint32_t getNextID();

	protected:
		ImGuiNodeEditor* m_Editor;

		friend ImGuiNodeEditor;
	};


	class ImGuiValueNode : public ImGuiNode
	{
	public:
		ImGuiValueNode(std::string name, ImGuiNodeEditor* editor);

		virtual ~ImGuiValueNode() override;

		virtual void OnImGuiRender() override;

		virtual bool AcceptLink(ed::PinId inputPinID, ed::PinId outputPinID) override;

		virtual void DeleteLinks() override;

		virtual const VariableType* FindPinType(ed::PinId id) const override;

		void AddValue(std::string name, const VariableType& type, bool allowEdit = true);

		void AllowEditValue(bool val);

		ImGuiNodeValue* FindValue(ed::PinId pinID);
		const ImGuiNodeValue* FindValue(ed::PinId pinID) const;

		const std::vector<ImGuiNodeValue>& GetValues() const { return m_Values; }

	private:
		void calculatePadding();

	private:
		std::vector<ImGuiNodeValue> m_Values;
		
		float m_ValueTypeMaxWidth = 0.0f;
		float m_ValueNameMaxWidth = 0.0f;
		float m_ValueEditMaxWidth = 0.0f;

		bool  m_RecalcPadding = true;

		friend ImGuiNodeValue;
	};


	class ImGuiFunctionNode : public ImGuiNode
	{
	public:
		ImGuiFunctionNode(std::string name, ImGuiNodeEditor* editor);

		virtual ~ImGuiFunctionNode() override;

		virtual void OnImGuiRender() override;

		virtual bool AcceptLink(ed::PinId inputPinID, ed::PinId outputPinID) override;

		virtual void DeleteLinks() override;

		virtual const VariableType* FindPinType(ed::PinId id) const override;

		void AddInputArgument(std::string name, const VariableType& type);

		void AddOutput(const VariableType& type);

		ed::PinId GetInputPinID() const { return m_InputPinID; }
		ed::PinId GetOutputPinID() const { return m_OutputPinID; }

		ImGuiNodeArgument* FindInputArgument(ed::PinId pinID);
		const ImGuiNodeArgument* FindInputArgument(ed::PinId pinID) const;


		const std::vector<ImGuiNodeArgument>& GetInputArguments() const { return m_InputArguments; }
		const std::vector<ImGuiNodeOutput>& GetOutputArguments() const { return m_Outputs; }
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

		float m_ValueTypeMaxWidth = 0.0f;
		float m_ValueNameMaxWidth = 0.0f;
		
		bool  m_RecalcPadding = true;

		friend ImGuiNodeOutput;
		friend ImGuiNodeArgument;
	};
}
