#include "stdafx.h"
#include "ImGuiLayer.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/RenderCommand.h"
#include "API/OpenGL/OpenGLRendererAPI.h"

#include "XYZ/Core/KeyCodes.h"

#include "imgui.h"
#include "ImGuiBuild.h"

#include <GLFW/glfw3.h>
#include <GL/glew.h>


XYZ::ImGuiLayer::ImGuiLayer()
    : Layer()
{
}

XYZ::ImGuiLayer::~ImGuiLayer()
{
}

void XYZ::ImGuiLayer::OnAttach()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
   
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    
    io.KeyMap[ImGuiKey_Tab] = KeyCode::XYZ_KEY_TAB;
    //io.KeyMap[ImGuiKey_LeftArrow] = KeyCode::XYZ_ARR;
    //io.KeyMap[ImGuiKey_Tab] = KeyCode::XYZ_KEY_TAB;
    //io.KeyMap[ImGuiKey_Tab] = KeyCode::XYZ_KEY_TAB;
    //io.KeyMap[ImGuiKey_Tab] = KeyCode::XYZ_KEY_TAB;
    io.KeyMap[ImGuiKey_End] = KeyCode::XYZ_KEY_END;
    io.KeyMap[ImGuiKey_Home] = KeyCode::XYZ_KEY_HOME;
    io.KeyMap[ImGuiKey_PageDown] = KeyCode::XYZ_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageUp] = KeyCode::XYZ_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Space] = KeyCode::XYZ_KEY_SPACE;
    io.KeyMap[ImGuiKey_Backspace] = KeyCode::XYZ_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Delete] = KeyCode::XYZ_KEY_Delete;
    io.KeyMap[ImGuiKey_Insert] = KeyCode::XYZ_KEY_INSERT;
    io.KeyMap[ImGuiKey_Enter] = KeyCode::XYZ_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = KeyCode::XYZ_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = KeyCode::XYZ_KEY_A;
    io.KeyMap[ImGuiKey_C] = KeyCode::XYZ_KEY_C;
    io.KeyMap[ImGuiKey_V] = KeyCode::XYZ_KEY_V;
    io.KeyMap[ImGuiKey_X] = KeyCode::XYZ_KEY_X;
    io.KeyMap[ImGuiKey_Y] = KeyCode::XYZ_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = KeyCode::XYZ_KEY_Z;

    ImGui_ImplOpenGL3_Init("#version 410");
}

void XYZ::ImGuiLayer::OnDetach()
{
}

void XYZ::ImGuiLayer::OnImGuiRender()
{
}

void XYZ::ImGuiLayer::Begin(const float ts)
{
    ImGuiIO& io = ImGui::GetIO();
    Application& app = Application::Get();
    io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());
    io.DeltaTime = ts > 0.0f ? ts : 1.0f / 60.0f;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
}

void XYZ::ImGuiLayer::End()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


