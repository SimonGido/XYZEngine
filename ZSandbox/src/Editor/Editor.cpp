#include "Editor.h"
#include "stdafx.h"
#include "imgui/imgui.h"


Editor::Editor() : XYZ::Layer()
{

}

Editor::~Editor()
{
}

void Editor::OnImGuiRender()
{
	ImGui::ShowDemoWindow();
}
