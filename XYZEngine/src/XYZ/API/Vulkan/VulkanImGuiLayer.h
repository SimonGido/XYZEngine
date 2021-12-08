#pragma once
#include <vulkan/vulkan_core.h>

#include "XYZ/ImGui/ImGuiLayer.h"
#include <queue>

namespace  XYZ
{
    class VulkanImGuiLayer : public ImGuiLayer
    {
    public:
        VulkanImGuiLayer();
        virtual ~VulkanImGuiLayer() override;

        virtual void Begin() override;
        virtual void End() override;
        virtual void AddFont(const ImGuiFontConfig& config) override;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnImGuiRender() override;

        const std::vector<ImGuiFontConfig>& GetLoadedFonts() const override { return m_FontsLoaded; }
    private:
        void addWaitingFonts();
        void RT_uploadFonts();

    private:
        VkDescriptorPool m_DescriptorPool;
        std::vector<VkCommandBuffer> m_ImGuiCommandBuffers;

        std::queue<ImGuiFontConfig> m_AddFonts;
        std::vector<ImGuiFontConfig> m_FontsLoaded;
    };
}

