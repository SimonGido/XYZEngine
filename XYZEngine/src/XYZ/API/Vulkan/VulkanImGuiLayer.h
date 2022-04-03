#pragma once
#include "XYZ/ImGui/ImGuiLayer.h"
#include "VulkanRendererAPI.h"
#include "VulkanImage.h"
#include "VulkanDescriptorAllocator.h"

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

        ImTextureID AddImage(const Ref<VulkanImage2D>& image);
        Ref<VulkanDescriptorAllocator> GetDescriptorAllocator() const { return m_DescriptorAllocator; }
        const std::vector<ImGuiFontConfig>& GetLoadedFonts() const override { return m_FontsLoaded; }
    private:
        void addWaitingFonts();
        static void RT_uploadFonts();
        static void RT_beginRender();
        static void RT_endRender();

    private:
        Ref<VulkanDescriptorAllocator> m_DescriptorAllocator;
        VkDescriptorPool               m_DescriptorPool;


        std::queue<ImGuiFontConfig>  m_AddFonts;
        std::vector<ImGuiFontConfig> m_FontsLoaded;

        struct ImageDescriptorSet
        {
            Ref<VulkanImage2D>  Image;
            VkDescriptorSet		Descriptor;
        };

        std::map<ImGuiID, ImageDescriptorSet> m_ImGuiImageDescriptors;
    };
}

