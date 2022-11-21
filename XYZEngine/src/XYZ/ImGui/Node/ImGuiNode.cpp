#include "stdafx.h"
#include "ImGuiNode.h"

#include "ImGuiNodeContext.h"

#include "XYZ/ImGui/ImGui.h"

namespace XYZ {
	namespace UI {

        static constexpr ImVec4 sc_StructHeaderColor = ImVec4(0.5f, 0.5f, 1.0f, 1.0f);
        static constexpr ImVec4 sc_FunctionHeaderColor = ImVec4(0.8f, 0.5f, 1.0f, 1.0f);
        static constexpr ImVec4 sc_VariableColor = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
        static constexpr float  sc_EditValueWidth = 150.0f;


        ImGuiNodeID::ImGuiNodeID(std::shared_ptr<ImGuiNodeContext> context)
            :
            m_Context(std::move(context))
        {
            m_ID = m_Context->getNextID();
        }

        void ImGuiNodeID::Destroy()
        {
            if (m_ID != 0)
                m_Context->m_FreeIDs.push(m_ID);
        
            m_ID = 0;
        }

        ImGuiLink::ImGuiLink(const std::shared_ptr<ImGuiNodeContext>& context)
            :
            ID(context),
            InputID(0),
            OutputID(0)
        {
        }

		ImGuiNodeValue::ImGuiNodeValue(std::shared_ptr<ImGuiNodeContext> context, ImGuiNode* parent, uint32_t flags)
			:
			m_Context(std::move(context)),
            m_Parent(parent),
            m_InputPinID(m_Context),
            m_OutputPinID(m_Context),
			m_Flags(flags)
		{
		}

        ImGuiNodeValue::~ImGuiNodeValue()
        {
        }

		void ImGuiNodeValue::Render()
        {
            static std::string editID = "##EditValue" + std::to_string(m_InputPinID);
            
            const float cursorPosX = ImGui::GetCursorPosX();
            const float inputOffset = ImGui::CalcTextSize("->").x + ImGui::GetStyle().ItemSpacing.x;
            const float typeOffset = m_Parent->m_ValueTypeMaxWidth + ImGui::GetStyle().ItemSpacing.x;
            const float nameOffset = m_Parent->m_ValueNameMaxWidth + ImGui::GetStyle().ItemSpacing.x;
            const float editOffset = m_Parent->m_ValueEditMaxWidth + ImGui::GetStyle().ItemSpacing.x;

            if (IS_SET(m_Flags, ImGuiNodeValueFlags_AllowInput))
            {
                ed::BeginPin(m_InputPinID, ed::PinKind::Input);
                ImGui::Text("->");
                ed::EndPin();
                ImGui::SameLine();
            }

            ImGui::SetCursorPosX(cursorPosX + inputOffset);
            if (m_IsArray)
            {
                const std::string arrayType = m_Type.Name + "[ ]";
                ImGui::TextColored(sc_VariableColor, arrayType.c_str());
            }
            else
            {
                ImGui::TextColored(sc_VariableColor, m_Type.Name.c_str());
            }
            if (IS_SET(m_Flags, ImGuiNodeValueFlags_AllowName))
            {
                ImGui::SameLine();
                ImGui::SetCursorPosX(cursorPosX + inputOffset + typeOffset);
               
                ImGui::Text(m_Name.c_str());
            }

            if (IS_SET(m_Flags, ImGuiNodeValueFlags_AllowEdit))
            {
                if (m_Context->VariableExtension.HasEdit(m_Type.Name))
                {
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(cursorPosX + inputOffset + typeOffset + nameOffset);

                    UI::ScopedWidth scopedWidth(sc_EditValueWidth);
                    m_Context->VariableExtension.EditValue(m_Type.Name, editID.c_str(), (std::byte*)m_Data.Data);
                }
            }

            if (IS_SET(m_Flags, ImGuiNodeValueFlags_AllowOutput))
            {
                ImGui::SameLine();
                ImGui::SetCursorPosX(cursorPosX + inputOffset + typeOffset + nameOffset + editOffset);

                // Output pin
                ed::BeginPin(m_OutputPinID, ed::PinKind::Output);
                ImGui::Text("->");
                ed::EndPin();
            }

            for (auto& val : m_Values)
                val.Render();
        }

        void ImGuiNodeValue::SetValueName(std::string name)
        {
            m_Name = std::move(name);
            m_Parent->m_RecalcPadding = true;
        }

        void ImGuiNodeValue::SetValueType(VariableType type)
        {
            m_Type = std::move(type);
            m_Parent->m_RecalcPadding = true;
            m_Data.Allocate(m_Type.Size);
            m_Data.ZeroInitialize();
        }

        void ImGuiNodeValue::SetFlags(uint32_t flags)
        {
            m_Flags = flags;
            m_Parent->m_RecalcPadding = true;
        }

        void ImGuiNodeValue::SetArray(bool val)
        {
            m_IsArray = val;
            m_Parent->m_RecalcPadding = true;
        }

        ImGuiNodeValue& ImGuiNodeValue::AddValue(std::string name, const VariableType& type, uint32_t flags)
        {
            auto& val = m_Values.emplace_back(m_Context, m_Parent, flags);
            val.SetValueName(std::move(name));
            val.SetValueType(type);
            m_Parent->m_RecalcPadding = true;
            return val;
        }

        ImGuiNode::ImGuiNode(std::shared_ptr<ImGuiNodeContext> context, uint32_t flags)
            :
            m_Context(std::move(context)),
            m_ID(m_Context),
            m_InputID(m_Context),
            m_OutputID(m_Context),
            m_Flags(flags)
        {
        }

        void ImGuiNode::Render()
        {
            if (m_RecalcPadding)
                calculatePadding();

            ed::BeginNode(m_ID);
            
            const float cursorPosX = ImGui::GetCursorPosX();
            const float inputOffset = ImGui::CalcTextSize("->").x + ImGui::GetStyle().ItemSpacing.x;
            const float typeOffset = m_ValueTypeMaxWidth + ImGui::GetStyle().ItemSpacing.x;
            const float nameOffset = m_ValueNameMaxWidth + ImGui::GetStyle().ItemSpacing.x;
            const float editOffset = m_ValueEditMaxWidth + ImGui::GetStyle().ItemSpacing.x;

            if (IS_SET(m_Flags, ImGuiNodeFlags_AllowInput))
            {
                ed::BeginPin(m_InputID, ed::PinKind::Input);
                ImGui::Text("->");
                ed::EndPin();
                ImGui::SameLine();
            }
            
            ImGui::SetCursorPosX(cursorPosX + inputOffset);
            ImGui::TextColored(sc_VariableColor, m_Type.Name.c_str());
  
            if (IS_SET(m_Flags, ImGuiNodeFlags_AllowName))
            {
                ImGui::SameLine();
                ImGui::SetCursorPosX(cursorPosX + inputOffset + typeOffset);
                
                ImGui::TextColored(sc_FunctionHeaderColor, m_Name.c_str());
            }

            if (IS_SET(m_Flags, ImGuiNodeFlags_AllowOutput))
            {
                ImGui::SameLine();
                ImGui::SetCursorPosX(cursorPosX + inputOffset + typeOffset + nameOffset + editOffset);
                ed::BeginPin(m_OutputID, ed::PinKind::Output);
                ImGui::Text("->");
                ed::EndPin();
            }

            for (auto& value : m_Values)
                value.Render();

            ed::EndNode();
        }

        bool ImGuiNode::AcceptLink(ed::PinId inputID, ed::PinId outputID) const
        {
            if (acceptNodeLink(inputID, outputID))
                return true;
            if (acceptValueLink(inputID, outputID))
                return true;
            return false;
        }

        void ImGuiNode::SetName(std::string name)
        {
            m_Name = std::move(name);
            m_RecalcPadding = true;
        }

        void ImGuiNode::SetType(const VariableType& type)
        {
            m_Type = type;
            m_RecalcPadding = true;
        }
        void ImGuiNode::SetFlags(uint32_t flags)
        {
            m_Flags = flags;
            m_RecalcPadding = true;
        }

        ImGuiNodeValue& ImGuiNode::AddValue(std::string name, const VariableType& type, uint32_t flags)
        {
            auto& val = m_Values.emplace_back(m_Context, this, flags);
            val.SetValueName(std::move(name));
            val.SetValueType(type);
            m_RecalcPadding = true;
            return val;
        }

        void ImGuiNode::calculatePadding()
        {
            m_RecalcPadding = false;
            m_ValueNameMaxWidth = 0.0f;
            m_ValueTypeMaxWidth = 0.0f;
            m_ValueEditMaxWidth = 0.0f;
            
            if (IS_SET(m_Flags, ImGuiNodeFlags_AllowName))
            {
                const float nameWidth = ImGui::CalcTextSize(m_Name.c_str()).x;
                m_ValueNameMaxWidth = std::max(nameWidth, m_ValueNameMaxWidth);
            }

            const float typeWidth = ImGui::CalcTextSize(m_Type.Name.c_str()).x + ImGui::CalcTextSize("->").x;
            m_ValueTypeMaxWidth = std::max(typeWidth, m_ValueTypeMaxWidth);

            for (auto& value : m_Values)
            {
                if (value.HasFlag(ImGuiNodeValueFlags_AllowName))
                {
                    const float nameWidth = ImGui::CalcTextSize(value.GetName().c_str()).x;
                    m_ValueNameMaxWidth = std::max(nameWidth, m_ValueNameMaxWidth);
                }
                if (value.HasFlag(ImGuiNodeValueFlags_AllowEdit))
                {
                    m_ValueEditMaxWidth = sc_EditValueWidth;
                }
            
                float typeWidth = ImGui::CalcTextSize(value.GetType().Name.c_str()).x + ImGui::CalcTextSize("->").x;
                if (value.IsArray())
                    typeWidth += ImGui::CalcTextSize("[ ]").x;
                m_ValueTypeMaxWidth = std::max(typeWidth, m_ValueTypeMaxWidth);
            }
        }
        bool ImGuiNode::acceptNodeLink(ed::PinId inputID, ed::PinId outputID) const
        {
            if (m_InputID == outputID)
            {
                auto outputNode = m_Context->FindNodeByOutputID(inputID);
                if (outputNode)
                {
                    if (outputNode->GetType() == m_Type)
                        return true;
                }
                else
                {
                    auto outputValue = m_Context->FindValueByOutputID(inputID);
                    if (outputValue)
                    {
                        if (outputValue->GetType() == m_Type)
                            return true;
                    }
                }
            }

            return false;
        }
        bool ImGuiNode::acceptValueLink(ed::PinId inputID, ed::PinId outputID) const
        {
            auto outputValue = m_Context->FindValueByOutputID(inputID);
            if (outputValue)
            {
                for (auto& value : m_Values)
                {
                    if (value.GetInputID() == outputID)
                    {
                        if (outputValue->GetType() == value.GetType())
                        {
                            return true;
                        }
                    }
                }
            }
            return false;
        }
    }
}