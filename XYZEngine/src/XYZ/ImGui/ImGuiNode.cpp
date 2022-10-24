#include "stdafx.h"
#include "ImGuiNode.h"

#include "ImGuiNodeEditor.h"

namespace XYZ {

    struct ImGuiNodeSettings
    {
        ImGuiNodeSettings()
        {
            ValueTypeColors[ImGuiNodeValue::Float]  = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
            ValueTypeColors[ImGuiNodeValue::Vec2]   = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
            ValueTypeColors[ImGuiNodeValue::Vec3]   = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
            ValueTypeColors[ImGuiNodeValue::Vec4]   = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
            ValueTypeColors[ImGuiNodeValue::Int]    = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
            ValueTypeColors[ImGuiNodeValue::IVec2]  = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
            ValueTypeColors[ImGuiNodeValue::IVec3]  = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
            ValueTypeColors[ImGuiNodeValue::IVec4]  = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
            ValueTypeColors[ImGuiNodeValue::Bool]   = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
            

            ValueTypeNames[ImGuiNodeValue::Float]   = "Float";
            ValueTypeNames[ImGuiNodeValue::Vec2]    = "Vec2";
            ValueTypeNames[ImGuiNodeValue::Vec3]    = "Vec3";
            ValueTypeNames[ImGuiNodeValue::Vec4]    = "Vec4";
            ValueTypeNames[ImGuiNodeValue::Int]     = "Int";
            ValueTypeNames[ImGuiNodeValue::IVec2]   = "IVec2";
            ValueTypeNames[ImGuiNodeValue::IVec3]   = "IVec3";
            ValueTypeNames[ImGuiNodeValue::IVec4]   = "IVec4";
            ValueTypeNames[ImGuiNodeValue::Bool]    = "Bool";
        }

        ImVec4 ValueTypeColors[ImGuiNodeValue::NumValues];
        
        std::string ValueTypeNames[ImGuiNodeValue::NumValues];
    };

    static ImGuiNodeSettings s_Settings;

    static void RenderValue()
    {

    }

    void ImGuiNodeValue::OnImGuiRender(ImGuiNodeEditor* editor)
    {
        // Input pin
        ed::BeginPin(InputPinID, ed::PinKind::Input);
        ImGui::Text("->");
        ed::EndPin();

        // Display value
        ImGui::SameLine();
        ImGui::TextColored(s_Settings.ValueTypeColors[Type], s_Settings.ValueTypeNames[Type].c_str());
        ImGui::SameLine();
        ImGui::Text(Name.c_str());
        ImGui::SameLine();
        
        // Output pin
        ed::BeginPin(OutputPinID, ed::PinKind::Output);
        ImGui::Text("->");
        ed::EndPin();
    }


	void ImGuiNode::OnImGuiRender(ImGuiNodeEditor* editor)
	{
        ed::BeginNode(ID);
        
        ImGui::Text(Name.c_str());
        for (auto& value : Values)
        {
            value.OnImGuiRender(editor);
        }

        ed::EndNode();
	}
}