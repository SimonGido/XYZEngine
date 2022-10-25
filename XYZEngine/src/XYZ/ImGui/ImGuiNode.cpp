#include "stdafx.h"
#include "ImGuiNode.h"

#include "ImGuiNodeEditor.h"

namespace XYZ {

    static constexpr ImVec4 sc_StructHeaderColor   = ImVec4(0.5f, 0.5f, 1.0f, 1.0f);
    static constexpr ImVec4 sc_FunctionHeaderColor = ImVec4(0.8f, 0.5f, 1.0f, 1.0f);
    static constexpr ImVec4 sc_VariableColor       = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);


    static void SetPinPadding(ed::NodeId id, float padding)
    {
        static ImVec2 arrowSize = ImGui::CalcTextSize("->");

        auto nodePosition = ed::GetNodePosition(id);
        ImGui::SetCursorPosX(nodePosition.x + padding);
        ed::PinRect(ImVec2(ImGui::GetCursorPos()), ImVec2(ImGui::GetCursorPosX() + arrowSize.x, ImGui::GetCursorPosY() + arrowSize.y));
    }

    static void CalculateOffset(const std::string& name, float& value)
    {
        value += ImGui::CalcTextSize(name.c_str()).x;
    }

    template <typename ...Args>
    static float CalculateOffsets(Args&& ...args)
    {
        float result = 0.0f;
        (CalculateOffset(args, result), ...);
        result += (sizeof ...(Args)) * ImGui::GetStyle().ItemSpacing.x;

        result += ImGui::GetStyle().FramePadding.x;
        return result;
    }

    //static float CalculateLeftOffset(const std::string& name, VariableType type)
    //{
    //    float result = ImGui::CalcTextSize("->").x
    //        + ImGui::CalcTextSize(name.c_str()).x
    //        + 4 * ImGui::GetStyle().ItemSpacing.x
    //        + ImGui::GetStyle().FramePadding.x;
    //
    //    if (type != VariableType::None)
    //    {
    //        result += ImGui::CalcTextSize(VariableTypeToImGui(type).c_str()).x;
    //    }
    //
    //    return result;
    //}

  
    ImGuiNodeValue::ImGuiNodeValue(ImGuiNode* parent)
        :
        m_Parent(parent)
    {
    }

    void ImGuiNodeValue::OnImGuiRender(ImGuiNodeEditor* editor, ed::NodeId nodeID, float& padding)
    {
        static ImVec2 arrowSize = ImGui::CalcTextSize("->");

        if (m_RecalcPadding)
        {
            padding = std::max(CalculateOffsets( "->", m_Name, VariableTypeToImGui(m_Type)), padding);
            m_RecalcPadding = false;
        }

        // Input pin
        ed::BeginPin(InputPinID, ed::PinKind::Input);
        ImGui::Text("->");
        ed::EndPin();
        // Display value
        ImGui::SameLine();
        ImGui::TextColored(sc_VariableColor, VariableTypeToImGui(m_Type).c_str());
        ImGui::SameLine();
        ImGui::Text(m_Name.c_str());
        ImGui::SameLine();

        // Output pin
        ed::BeginPin(OutputPinID, ed::PinKind::Output);
          
        SetPinPadding(nodeID, padding);
        ImGui::Text("->");
       
        ed::EndPin();
    }

    void ImGuiNodeValue::SetName(std::string name)
    {
        m_Name = std::move(name);
        m_RecalcPadding = true;
        m_Parent->m_OutputPadding = 0.0f;
    }

    void ImGuiNodeValue::SetType(VariableType type)
    {
        m_Type = type;
        m_RecalcPadding = true;
        m_Parent->m_OutputPadding = 0.0f;
    }

    ImGuiNodeArgument::ImGuiNodeArgument(ImGuiNode* parent)
        :
        m_Parent(parent)
    {
    }

    void ImGuiNodeArgument::OnImGuiRender(ImGuiNodeEditor* editor, ed::NodeId nodeID, float& padding)
    { 
        if (m_RecalcPadding)
        {
            padding = std::max(CalculateOffsets( "->", m_Name, VariableTypeToImGui(m_Type)), padding);
            m_RecalcPadding = false;
        }

        // Input pin
        ed::BeginPin(InputPinID, ed::PinKind::Input);
        ImGui::Text("->");
        ed::EndPin();
        // Display value
        ImGui::SameLine();
        ImGui::TextColored(sc_VariableColor, VariableTypeToImGui(m_Type).c_str());
        ImGui::SameLine();
        ImGui::Text(m_Name.c_str());
    }

    void ImGuiNodeArgument::SetName(std::string name)
    {
        m_Name = std::move(name);
        m_RecalcPadding = true;
        m_Parent->m_OutputPadding = 0.0f;
    }

    void ImGuiNodeArgument::SetType(VariableType type)
    {
        m_Type = type;
        m_RecalcPadding = true;
        m_Parent->m_OutputPadding = 0.0f;
    }

    ImGuiNodeOutput::ImGuiNodeOutput(ImGuiNode* parent)
        :
        m_Parent(parent)
    {
    }

    void ImGuiNodeOutput::OnImGuiRender(ImGuiNodeEditor* editor, ed::NodeId nodeID, float& padding)
    {
    }

    void ImGuiNodeOutput::SetType(VariableType type)
    {
        m_Type = type;
        m_RecalcPadding = true;
        m_Parent->m_OutputPadding = 0.0f;
    }


    ImGuiNode::ImGuiNode(std::string name, ImGuiNodeEditor* editor)
        :
        m_Name(std::move(name)),
        m_Editor(editor)
    {
        m_ID = editor->getNextID();
    }

    uint32_t ImGuiNode::getNextID()
    {
        return m_Editor->getNextID();
    }
	

    ImGuiValueNode::ImGuiValueNode(std::string name, ImGuiNodeEditor* editor)
        : ImGuiNode(std::move(name), editor)
    {}

    ImGuiValueNode::~ImGuiValueNode()
    {
        for (auto& val : m_Values)
        {
            m_Editor->m_FreeIDs.push(val.InputPinID.Get());
            m_Editor->m_FreeIDs.push(val.OutputPinID.Get());
        }
        m_Editor->m_FreeIDs.push(GetID().Get());
    }



    void ImGuiValueNode::OnImGuiRender()
    {
        ed::BeginNode(GetID());

        renderHeader();

        for (auto& value : m_Values)
        {
            value.OnImGuiRender(m_Editor, GetID(), m_OutputPadding);
        }

        ed::EndNode();
    }

    bool ImGuiValueNode::AcceptLink(ed::PinId inputPinID, ed::PinId outputPinID)
    {
        auto inputType = m_Editor->FindPinType(inputPinID);
        auto outputValue = FindValue(outputPinID);

        if (outputValue)
        {
            if (inputType == outputValue->GetType())
                return true;
        }
        return false;
    }

    VariableType ImGuiValueNode::FindPinType(ed::PinId id) const
    {
        for (auto& var : m_Values)
        {
            if (var.InputPinID == id)
                return var.GetType();
            if (var.OutputPinID == id)
                return var.GetType();
        }
        return VariableType::None;
    }

    void ImGuiValueNode::AddValue(std::string name, VariableType type)
    {
        auto& val = m_Values.emplace_back(this);
        val.SetName(std::move(name));
        val.SetType(type);
        val.InputPinID = m_Editor->getNextID();
        val.OutputPinID = m_Editor->getNextID();
    }

    ImGuiNodeValue* ImGuiValueNode::FindValue(ed::PinId pinID)
    {
        for (auto& val : m_Values)
        {
            if (val.InputPinID == pinID)
                return &val;
            if (val.OutputPinID == pinID)
                return &val;
        }
        return nullptr;
    }
    const ImGuiNodeValue* ImGuiValueNode::FindValue(ed::PinId pinID) const
    {
        for (auto& val : m_Values)
        {
            if (val.InputPinID == pinID)
                return &val;
            if (val.OutputPinID == pinID)
                return &val;
        }
        return nullptr;
    }
    void ImGuiValueNode::renderHeader()
    {
        ImGui::TextColored(sc_StructHeaderColor, GetName().c_str());
    }
    ImGuiFunctionNode::ImGuiFunctionNode(std::string name, ImGuiNodeEditor* editor)
        :
        ImGuiNode(std::move(name), editor)
    {
        m_InputPinID = editor->getNextID();
        m_OutputPinID = editor->getNextID();
        m_OutputValuePinID = editor->getNextID();
    }
    ImGuiFunctionNode::~ImGuiFunctionNode()
    {
        for (auto& val : m_InputArguments)
        {
            m_Editor->m_FreeIDs.push(val.InputPinID.Get());
        }
        m_Editor->m_FreeIDs.push(GetID().Get());
        m_Editor->m_FreeIDs.push(m_OutputValuePinID.Get());
        m_Editor->m_FreeIDs.push(m_InputPinID.Get());
        m_Editor->m_FreeIDs.push(m_OutputPinID.Get());
    }
    void ImGuiFunctionNode::OnImGuiRender()
    {
        ed::BeginNode(GetID());

        renderHeader();

        for (auto& value : m_InputArguments)
        {
            value.OnImGuiRender(m_Editor, GetID(), m_OutputPadding);
        }      
   
        renderOutputValuePin();

        ed::EndNode();
        m_RecalcPadding = false;
    }
    bool ImGuiFunctionNode::AcceptLink(ed::PinId inputPinID, ed::PinId outputPinID)
    {
        auto inputType = m_Editor->FindPinType(inputPinID);
        auto output = FindInputArgument(outputPinID);

        if (output)
        {
            if (inputType == output->GetType())
                return true;
        }
        return false;
    }
    VariableType ImGuiFunctionNode::FindPinType(ed::PinId id) const
    {
        if (m_OutputValuePinID == id)
            return m_OutputType;

        for (auto& var : m_InputArguments)
        {
            if (var.InputPinID == id)
                return var.GetType();
        }
        return VariableType::None;
    }
    void ImGuiFunctionNode::AddInputArgument(std::string name, VariableType type)
    {
        auto& val = m_InputArguments.emplace_back(this);
        val.SetName(std::move(name));
        val.SetType(type);
        val.InputPinID = m_Editor->getNextID();
    }
    void ImGuiFunctionNode::AddOutput(VariableType type)
    {
        auto& val = m_Outputs.emplace_back(this);
        val.SetType(type);
        val.OutputPinID = m_Editor->getNextID();
    }
    ImGuiNodeArgument* ImGuiFunctionNode::FindInputArgument(ed::PinId pinID)
    {
        for (auto& val : m_InputArguments)
        {
            if (val.InputPinID == pinID)
                return &val;
        }
        return nullptr;
    }
    const ImGuiNodeArgument* ImGuiFunctionNode::FindInputArgument(ed::PinId pinID) const
    {
        for (auto& val : m_InputArguments)
        {
            if (val.InputPinID == pinID)
                return &val;
        }
        return nullptr;
    }

    void ImGuiFunctionNode::SetOutputType(VariableType type)
    {
        m_OutputType = type;
        m_RecalcPadding = true;
    }

    void ImGuiFunctionNode::renderHeader()
    {
        if (m_RecalcPadding)
        {
            m_OutputPadding = std::max(CalculateOffsets("->", "->", GetName()), m_OutputPadding);       
        }
        ed::BeginPin(m_InputPinID, ed::PinKind::Input);
        ImGui::Text("->");
        ed::EndPin();

        ImGui::SameLine();
        ImGui::TextColored(sc_FunctionHeaderColor, GetName().c_str());
        ImGui::SameLine();

        ed::BeginPin(m_OutputPinID, ed::PinKind::Output);
        SetPinPadding(GetID(), m_OutputPadding);
        ImGui::Text("->");
        ed::EndPin();
    }


    void ImGuiFunctionNode::renderOutputValuePin()
    {
        if (m_RecalcPadding)
        {
            m_OutputPadding = std::max(CalculateOffsets("->", VariableTypeToImGui(m_OutputType)), m_OutputPadding);
        }
        ed::BeginPin(m_OutputValuePinID, ed::PinKind::Output);
        ImGui::TextColored(sc_VariableColor, VariableTypeToImGui(m_OutputType).c_str());
        ImGui::SameLine();
        SetPinPadding(GetID(), m_OutputPadding);
        ImGui::Text("->");
        ed::EndPin();
    }
    
   

}