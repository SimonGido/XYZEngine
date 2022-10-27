#include "stdafx.h"
#include "ImGuiNode.h"

#include "ImGuiNodeEditor.h"

#include "XYZ/ImGui/ImGui.h"

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
  
    ImGuiNodeValue::ImGuiNodeValue(ImGuiValueNode* parent)
        :
        m_Parent(parent)
    {
        memset(Data.data(), 0, Data.size());
    }

    void ImGuiNodeValue::OnImGuiRender(ImGuiNodeEditor* editor, ed::NodeId nodeID)
    {
        static std::string editID = "##EditValue";

        if (!m_AllowEditData)
        {
            ed::BeginPin(m_InputPinID, ed::PinKind::Input);
            ImGui::Text("->");
            ed::EndPin();
            ImGui::SameLine();
        }

        UI::CursrorMoverX cursorMover;
        {
            ImGui::TextColored(sc_VariableColor, m_Type.Name.c_str());
            ImGui::SameLine();

            cursorMover.Move(m_Parent->m_ValueTypeMaxWidth + ImGui::GetStyle().ItemSpacing.x);
            ImGui::Text(m_Name.c_str());
           
            ImGui::SameLine();
            cursorMover.Move(m_Parent->m_ValueNameMaxWidth + ImGui::GetStyle().ItemSpacing.x);
        }

        if (m_AllowEditData)
        {
            UI::ScopedWidth scopedWidth(m_EditValueWidth);
            editor->VariableExtension.EditValue(m_Type.Name, editID.c_str(), Data.data());
            ImGui::SameLine();
        }
       
        // Output pin
        ed::BeginPin(m_OutputPinID, ed::PinKind::Output);
        ImGui::Text("->");
        ed::EndPin();
    }

   

    void ImGuiNodeValue::SetName(std::string name)
    {
        m_Name = std::move(name);
        m_Parent->m_RecalcPadding = true;
    }
    void ImGuiNodeValue::SetType(const VariableType& type)
    {
        m_Type = type;
        m_Parent->m_RecalcPadding = true;
    }

    void ImGuiNodeValue::SetEditValueWidth(float width)
    {
        m_EditValueWidth = width;
        m_Parent->m_RecalcPadding = true;
    }
    void ImGuiNodeValue::SetAllowEdit(bool val)
    {
        m_AllowEditData = val;
        m_Parent->m_RecalcPadding = true;
    }
    ImGuiNodeArgument::ImGuiNodeArgument(ImGuiFunctionNode* parent)
        :
        m_Parent(parent)
    {
    }

    void ImGuiNodeArgument::OnImGuiRender(ImGuiNodeEditor* editor, ed::NodeId nodeID)
    { 
        // Input pin
        ed::BeginPin(InputPinID, ed::PinKind::Input);
        ImGui::Text("->");
        ed::EndPin();

        // Display value
        ImGui::SameLine();
        ImGui::TextColored(sc_VariableColor, m_Type.Name.c_str());
        ImGui::SameLine();
        ImGui::Text(m_Name.c_str());
    }

    void ImGuiNodeArgument::SetName(std::string name)
    {
        m_Name = std::move(name);
        m_Parent->m_RecalcPadding = true;
    }

    void ImGuiNodeArgument::SetType(const VariableType& type)
    {
        m_Type = type;
        m_Parent->m_RecalcPadding = true;
    }

    ImGuiNodeOutput::ImGuiNodeOutput(ImGuiFunctionNode* parent)
        :
        m_Parent(parent)
    {
    }

    void ImGuiNodeOutput::OnImGuiRender(ImGuiNodeEditor* editor, ed::NodeId nodeID)
    {   
        ImGui::TextColored(sc_VariableColor, m_Type.Name.c_str());
        ImGui::SameLine();

        ed::BeginPin(OutputPinID, ed::PinKind::Output);
        ImGui::Text("->");
        ed::EndPin();
    }

    void ImGuiNodeOutput::SetType(const VariableType& type)
    {
        m_Type = type;
        m_Parent->m_RecalcPadding = true;
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
        DeleteLinks();
        for (auto& val : m_Values)
        {
            m_Editor->m_FreeIDs.push(val.m_OutputPinID.Get());
            m_Editor->m_FreeIDs.push(val.m_InputPinID.Get());
        }
        m_Editor->m_FreeIDs.push(GetID().Get());
    }



    void ImGuiValueNode::OnImGuiRender()
    {
        ed::BeginNode(GetID());
        if (m_RecalcPadding)
            calculatePadding();

        ImGui::TextColored(sc_StructHeaderColor, GetName().c_str());

        for (auto& value : m_Values)
        {
            value.OnImGuiRender(m_Editor, GetID());
        }

        ed::EndNode();
    }

    bool ImGuiValueNode::AcceptLink(ed::PinId inputPinID, ed::PinId outputPinID)
    {
        auto inputType = m_Editor->FindPinType(inputPinID);
        auto outputValue = FindValue(outputPinID);

        if (outputValue && inputType)
        {
            if (*inputType == outputValue->GetType())
                return true;
        }
        return false;
    }

    void ImGuiValueNode::DeleteLinks()
    {
        for (auto it = m_Editor->m_Links.begin(); it != m_Editor->m_Links.end();)
        {
            for (auto& val : m_Values)
            {
                if (it->InputID == val.m_OutputPinID
                 || it->OutputID == val.m_OutputPinID
                 || it->InputID == val.m_InputPinID
                 || it->OutputID == val.m_OutputPinID)
                {
                    it = m_Editor->m_Links.erase(it);
                }
                else
                {
                    it++;
                }
            }
        }
    }

    const VariableType* ImGuiValueNode::FindPinType(ed::PinId id) const
    {
        for (auto& var : m_Values)
        {
            if (var.m_OutputPinID == id)
                return &var.m_Type;
            if (var.m_InputPinID == id)
                return &var.m_Type;
        }
        return nullptr;
    }

    void ImGuiValueNode::AddValue(std::string name, const VariableType& type, bool allowEdit)
    {
        auto& val = m_Values.emplace_back(this);
        val.SetType(type);
        val.SetName(std::move(name));
        val.SetAllowEdit(allowEdit);
        val.m_OutputPinID = m_Editor->getNextID();
        val.m_InputPinID = m_Editor->getNextID();
    }

    void ImGuiValueNode::AllowEditValue(bool val)
    {
        for (auto& value : m_Values)
            value.SetAllowEdit(val);
    }

    ImGuiNodeValue* ImGuiValueNode::FindValue(ed::PinId pinID)
    {
        for (auto& val : m_Values)
        {
            if (val.m_OutputPinID == pinID || val.m_InputPinID == pinID)
                return &val;
        }
        return nullptr;
    }
    const ImGuiNodeValue* ImGuiValueNode::FindValue(ed::PinId pinID) const
    {
        for (auto& val : m_Values)
        {
            if (val.m_OutputPinID == pinID)
                return &val;
        }
        return nullptr;
    }

    void ImGuiValueNode::calculatePadding()
    {
        m_ValueNameMaxWidth = 0.0f;
        m_ValueTypeMaxWidth = 0.0f;
        m_ValueEditMaxWidth = 0.0f;

        m_RecalcPadding = false;
        for (auto& value : m_Values)
        {
            const float nameWidth = ImGui::CalcTextSize(value.GetName().c_str()).x;
            m_ValueNameMaxWidth = std::max(nameWidth, m_ValueNameMaxWidth);

            const float typeWidth = ImGui::CalcTextSize(value.GetType().Name.c_str()).x;
            m_ValueTypeMaxWidth = std::max(typeWidth, m_ValueTypeMaxWidth);

            m_ValueEditMaxWidth = std::max(value.m_EditValueWidth, m_ValueEditMaxWidth);
        }
    }

    ImGuiFunctionNode::ImGuiFunctionNode(std::string name, ImGuiNodeEditor* editor)
        :
        ImGuiNode(std::move(name), editor)
    {
        m_InputPinID = editor->getNextID();
        m_OutputPinID = editor->getNextID();
    }
    ImGuiFunctionNode::~ImGuiFunctionNode()
    {
        DeleteLinks();
        for (auto& val : m_InputArguments)
        {
            m_Editor->m_FreeIDs.push(val.InputPinID.Get());
        }
        for (auto& val : m_Outputs)
        {
            m_Editor->m_FreeIDs.push(val.OutputPinID.Get());
        }
        m_Editor->m_FreeIDs.push(GetID().Get());
        m_Editor->m_FreeIDs.push(m_InputPinID.Get());
        m_Editor->m_FreeIDs.push(m_OutputPinID.Get());
    }
    void ImGuiFunctionNode::OnImGuiRender()
    {
        if (m_RecalcPadding)
            calculatePadding();

        ed::BeginNode(GetID());

        renderHeader();
        const float offset = m_ValueTypeMaxWidth + m_ValueNameMaxWidth + 3 * ImGui::GetStyle().ItemSpacing.x;
        const float cursorPos = ImGui::GetCursorPosX();
        size_t outputIndex = 0;
        for (size_t i = 0; i < m_InputArguments.size(); ++i)
        {
            auto& inputValue = m_InputArguments[i];
            inputValue.OnImGuiRender(m_Editor, GetID());
            if (outputIndex < m_Outputs.size())
            {
                auto& outputValue = m_Outputs[outputIndex++];
                ImGui::SameLine();
                ImGui::SetCursorPosX(cursorPos + offset);
                outputValue.OnImGuiRender(m_Editor, GetID());
            }
        }
        for (size_t i = outputIndex; i < m_Outputs.size(); ++i)
        {
            auto& outputValue = m_Outputs[i];
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
            outputValue.OnImGuiRender(m_Editor, GetID());
        }
        

        ed::EndNode();
        m_RecalcPadding = false;
    }
    bool ImGuiFunctionNode::AcceptLink(ed::PinId inputPinID, ed::PinId outputPinID)
    {
        auto inputType = m_Editor->FindPinType(inputPinID);
        auto output = FindInputArgument(outputPinID);

        if (output && inputType)
        {
            if (*inputType == output->GetType())
                return true;
        }

        auto inputNode = m_Editor->FindFunctionNodeByInput(outputPinID);
        if (inputNode)
        {
            // Connecting functions, pin is owned by function node
            if (outputPinID == m_InputPinID
              && inputPinID != m_OutputPinID) // Do not connect with itself
                return true;
        }
        return false;
    }
    void ImGuiFunctionNode::DeleteLinks()
    {
        deleteInputArgumentLinks();
        deleteOutputArgumentLinks();
        for (auto it = m_Editor->m_Links.begin(); it != m_Editor->m_Links.end();)
        {
            if (it->InputID == m_InputPinID
             || it->InputID == m_OutputPinID)
            {
                it = m_Editor->m_Links.erase(it);
            }
            else
            {
                it++;
            }
        }
    }
    const VariableType* ImGuiFunctionNode::FindPinType(ed::PinId id) const
    {
        for (auto& var : m_Outputs)
        {
            if (var.OutputPinID == id)
                return &var.GetType();
        }

        for (auto& var : m_InputArguments)
        {
            if (var.InputPinID == id)
                return &var.GetType();
        }
   
        return nullptr;
    }
    void ImGuiFunctionNode::AddInputArgument(std::string name, const VariableType& type)
    {
        auto& val = m_InputArguments.emplace_back(this);
        val.SetName(std::move(name));
        val.SetType(type);
        val.InputPinID = m_Editor->getNextID();
    }
    void ImGuiFunctionNode::AddOutput(const VariableType& type)
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


    void ImGuiFunctionNode::renderHeader()
    {
        ed::BeginPin(m_InputPinID, ed::PinKind::Input);
        ImGui::Text("->");
        ed::EndPin();

        UI::CursrorMoverX cursorMover;
        ImGui::SameLine();
        ImGui::TextColored(sc_FunctionHeaderColor, GetName().c_str());
        ImGui::SameLine();

        float offset = (2 * ImGui::GetStyle().ItemSpacing.x) + (2 * m_ValueTypeMaxWidth) + m_ValueNameMaxWidth;
        cursorMover.Move(offset);

        ed::BeginPin(m_OutputPinID, ed::PinKind::Output);
        ImGui::Text("->");
        ed::EndPin();
    }


 
    void ImGuiFunctionNode::calculatePadding()
    {
        m_RecalcPadding = false;
        
        m_ValueNameMaxWidth = 0.0f;
        m_ValueTypeMaxWidth = 0.0f;

        for (auto& value : m_Outputs)
        {
            const float typeWidth = ImGui::CalcTextSize(value.GetType().Name.c_str()).x;
            m_ValueTypeMaxWidth = std::max(typeWidth, m_ValueTypeMaxWidth);
        }
        for (auto& value : m_InputArguments)
        {
            const float nameWidth = ImGui::CalcTextSize(value.GetName().c_str()).x;
            m_ValueNameMaxWidth = std::max(nameWidth, m_ValueNameMaxWidth);

            const float typeWidth = ImGui::CalcTextSize(value.GetType().Name.c_str()).x + ImGui::CalcTextSize("->").x;
            m_ValueTypeMaxWidth = std::max(typeWidth, m_ValueTypeMaxWidth);
        }
    }
    void ImGuiFunctionNode::deleteInputArgumentLinks()
    {
        for (auto it = m_Editor->m_Links.begin(); it != m_Editor->m_Links.end();)
        {
            for (auto& arg : m_InputArguments)
            {
                if (it->InputID == arg.InputPinID
                    || it->OutputID == arg.InputPinID)
                {
                    it = m_Editor->m_Links.erase(it);
                }
                else
                {
                    it++;
                }
            }
        }
    }
    void ImGuiFunctionNode::deleteOutputArgumentLinks()
    {
        for (auto it = m_Editor->m_Links.begin(); it != m_Editor->m_Links.end();)
        {
            for (auto& arg : m_Outputs)
            {
                if (it->InputID == arg.OutputPinID
                 || it->OutputID == arg.OutputPinID)
                {
                    it = m_Editor->m_Links.erase(it);
                }
                else
                {
                    it++;
                }
            }
        }
    }
}
