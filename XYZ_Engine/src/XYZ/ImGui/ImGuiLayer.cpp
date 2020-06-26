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
    EventManager::Get().AddHandler(EventType::KeyPressed, std::bind(&ImGuiLayer::OnKeyPressedEvent, this, std::placeholders::_1));
    EventManager::Get().AddHandler(EventType::KeyReleased, std::bind(&ImGuiLayer::OnKeyReleasedEvent, this, std::placeholders::_1));
    EventManager::Get().AddHandler(EventType::KeyTyped, std::bind(&ImGuiLayer::OnKeyTypedEvent, this, std::placeholders::_1));
    EventManager::Get().AddHandler(EventType::MouseButtonPressed, std::bind(&ImGuiLayer::OnMouseButtonPressedEvent, this, std::placeholders::_1));
    EventManager::Get().AddHandler(EventType::MouseButtonReleased, std::bind(&ImGuiLayer::OnMouseButtonReleasedEvent, this, std::placeholders::_1));
    EventManager::Get().AddHandler(EventType::MouseMoved, std::bind(&ImGuiLayer::OnMouseMovedEvent, this, std::placeholders::_1));
    EventManager::Get().AddHandler(EventType::MouseScroll, std::bind(&ImGuiLayer::OnMouseScrolledEvent, this, std::placeholders::_1));
    EventManager::Get().AddHandler(EventType::WindowResized, std::bind(&ImGuiLayer::OnWindowResizedEvent, this, std::placeholders::_1));
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

bool XYZ::ImGuiLayer::OnMouseButtonPressedEvent(event_ptr e)
{
    auto ecast = std::static_pointer_cast<MouseButtonPressEvent>(e);
    auto& io = ImGui::GetIO();
    io.MouseDown[ecast->GetButton()] = true;
    return false;
}

bool XYZ::ImGuiLayer::OnMouseButtonReleasedEvent(event_ptr e)
{
    auto ecast = std::static_pointer_cast<MouseButtonReleaseEvent>(e);
    auto& io = ImGui::GetIO();
    io.MouseDown[ecast->GetButton()] = false;
    return false;
}

bool XYZ::ImGuiLayer::OnMouseMovedEvent(event_ptr e)
{
    auto ecast = std::static_pointer_cast<MouseMovedEvent>(e);
    auto& io = ImGui::GetIO();
    io.MousePos = ImVec2(ecast->GetX(), ecast->GetY());
    return false;
}

bool XYZ::ImGuiLayer::OnMouseScrolledEvent(event_ptr e)
{
    auto ecast = std::static_pointer_cast<MouseScrollEvent>(e);
    auto& io = ImGui::GetIO();
    io.MouseWheel += (float)ecast->GetOffsetX();
    io.MouseWheelH += (float)ecast->GetOffsetY();
    return false;
}

bool XYZ::ImGuiLayer::OnWindowResizedEvent(event_ptr e)
{
    auto ecast = std::static_pointer_cast<WindowResizeEvent>(e);
    auto& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(ecast->GetWidth(), ecast->GetWidth());
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    RenderCommand::SetViewPort(0, 0, ecast->GetWidth(), ecast->GetHeight());
    return false;
}

bool XYZ::ImGuiLayer::OnKeyPressedEvent(event_ptr e)
{
    auto ecast = std::static_pointer_cast<KeyPressedEvent>(e);
    auto& io = ImGui::GetIO();
    io.KeysDown[ecast->GetKey()] = true;

    io.KeyCtrl = io.KeysDown[XYZ::KeyCode::XYZ_KEY_LEFT_CONTROL] || io.KeysDown[XYZ::KeyCode::XYZ_KEY_RIGHT_CONTROL];
    io.KeyAlt = io.KeysDown[XYZ::KeyCode::XYZ_KEY_LEFT_ALT] || io.KeysDown[XYZ::KeyCode::XYZ_KEY_RIGHT_ALT];
    io.KeyShift = io.KeysDown[XYZ::KeyCode::XYZ_KEY_LEFT_SHIFT] || io.KeysDown[XYZ::KeyCode::XYZ_KEY_RIGHT_SHIFT];
    io.KeySuper = io.KeysDown[XYZ::KeyCode::XYZ_KEY_LEFT_SUPER] || io.KeysDown[XYZ::KeyCode::XYZ_KEY_RIGHT_SUPER];
    return false;
}

bool XYZ::ImGuiLayer::OnKeyReleasedEvent(event_ptr e)
{
    auto ecast = std::static_pointer_cast<KeyReleasedEvent>(e);
    auto& io = ImGui::GetIO();
    io.KeysDown[ecast->GetKey()] = false;

    io.KeyCtrl = io.KeysDown[XYZ::KeyCode::XYZ_KEY_LEFT_CONTROL] || io.KeysDown[XYZ::KeyCode::XYZ_KEY_RIGHT_CONTROL];
    io.KeyAlt = io.KeysDown[XYZ::KeyCode::XYZ_KEY_LEFT_ALT] || io.KeysDown[XYZ::KeyCode::XYZ_KEY_RIGHT_ALT];
    io.KeyShift = io.KeysDown[XYZ::KeyCode::XYZ_KEY_LEFT_SHIFT] || io.KeysDown[XYZ::KeyCode::XYZ_KEY_RIGHT_SHIFT];
    io.KeySuper = io.KeysDown[XYZ::KeyCode::XYZ_KEY_LEFT_SUPER] || io.KeysDown[XYZ::KeyCode::XYZ_KEY_RIGHT_SUPER];
    return false;
}

bool XYZ::ImGuiLayer::OnKeyTypedEvent(event_ptr e)
{
    auto ecast = std::static_pointer_cast<KeyTypedEvent>(e);
    auto& io = ImGui::GetIO();
    int kc = ecast->GetKey();
    if (kc > 0 && kc < 0x10000)
        io.AddInputCharacter((unsigned short)kc);
    return false;
}
