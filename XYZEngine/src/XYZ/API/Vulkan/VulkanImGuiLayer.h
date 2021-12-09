#pragma once
#include "XYZ/ImGui/ImGuiLayer.h"
#include "VulkanRendererAPI.h"
#include "VulkanImage.h"

#include <vulkan/vulkan_core.h>

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

        VkDescriptorSet AddImage(const Ref<VulkanImage2D>& image);
        const std::vector<ImGuiFontConfig>& GetLoadedFonts() const override { return m_FontsLoaded; }
    private:
        void addWaitingFonts();
        static void RT_uploadFonts();

    private:
        VkDescriptorPool m_DescriptorPool;
        std::vector<VkCommandBuffer> m_ImGuiCommandBuffers;

        std::queue<ImGuiFontConfig>  m_AddFonts;
        std::vector<ImGuiFontConfig> m_FontsLoaded;

        struct ImageDescriptorSet
        {
            Ref<VulkanImage2D>                 Image;
            VkDescriptorSet					   Descriptor;
            VulkanDescriptorAllocator::Version Version;
        };

        std::unordered_map<ImGuiID, ImageDescriptorSet> m_ImGuiImageDescriptors[3];
        
        std::mutex m_UpdateQueueLock;
        uint32_t m_CurrentFrame;
    };
}

