#include "stdafx.h"
#include "OpenGLImGuiLayer.h"


#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/API/Vulkan/VulkanImGuiLayer.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>


namespace XYZ
{
    void OpenGLImGuiLayer::OnAttach()
    {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows


        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        SetDarkThemeColors();

        Application& app = Application::Get();
        GLFWwindow*  window = static_cast<GLFWwindow*>(app.GetWindow().GetWindow());


        // Setup Platform/Renderer bindings
#ifdef RENDER_THREAD_ENABLED
    		io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
    		auto result = Renderer::GetPool().SubmitJob([window]()->bool {
    			ImGui_ImplGlfw_InitForOpenGL(window, true);
    			ImGui_ImplOpenGL3_Init("#version 410");
    			ImGui_ImplOpenGL3_NewFrame();
    
    			return true;
    		});
    		result.wait();
#else
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 410");
        ImGui_ImplOpenGL3_NewFrame();
#endif
    }

    void OpenGLImGuiLayer::OnDetach()
    {
#ifdef RENDER_THREAD_ENABLED
    		{
    			auto result = Renderer::GetPool().SubmitJob([this]()->bool {
    				ImGui_ImplOpenGL3_Shutdown();
    				ImGui_ImplGlfw_Shutdown();
    				ImGui::DestroyContext();
    				return true;
    			});
    			result.wait();
    		}
#else
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
#endif
    }

    void OpenGLImGuiLayer::OnEvent(Event& e)
    {
        if (m_BlockEvents)
        {
            ImGuiIO& io = ImGui::GetIO();
            e.Handled = (
                e.GetEventType() == EventType::KeyPressed
                || e.GetEventType() == EventType::KeyReleased
                || e.GetEventType() == EventType::KeyTyped
                || e.GetEventType() == EventType::MouseButtonPressed
                || e.GetEventType() == EventType::MouseButtonReleased
                || e.GetEventType() == EventType::MouseScroll
                || e.GetEventType() == EventType::MouseMoved
            );
        }
    }

    void OpenGLImGuiLayer::Begin()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (m_EnableDockspace)
            beginDockspace();
    }

    static void CopyImDrawData(ImDrawData& copy, const ImDrawData* drawData)
    {
        copy = *drawData;
        copy.CmdLists = new ImDrawList*[drawData->CmdListsCount];
        for (int i = 0; i < drawData->CmdListsCount; ++i)
            copy.CmdLists[i] = drawData->CmdLists[i]->CloneOutput();
    }

    void OpenGLImGuiLayer::End()
    {
        if (m_EnableDockspace)
            endDockspace();

        ImGui::Render();
#ifdef RENDER_THREAD_ENABLED
    		{
    			ImDrawData copy;
    			CopyImDrawData(copy, ImGui::GetDrawData());
    			
    			Renderer::Submit([copy]() mutable {		
    				ImGui_ImplOpenGL3_RenderDrawData(&copy);
    				for (int i = 0; i < copy.CmdListsCount; ++i)
    					IM_DELETE(copy.CmdLists[i]);
    				delete[]copy.CmdLists;
    				copy.Clear();
    			});
    
    			ImGuiIO& io = ImGui::GetIO();
    			Application& app = Application::Get();
    			io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());
    			
    			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    			{	
    				ImGui::UpdatePlatformWindows();
    				ImGui::RenderPlatformWindowsDefault();
    			}
    		}
#else
        ImGuiIO&     io = ImGui::GetIO();
        Application& app = Application::Get();
        io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
#endif
    }
}
