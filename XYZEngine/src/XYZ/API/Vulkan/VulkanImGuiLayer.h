#pragma once
#include <vulkan/vulkan_core.h>

#include "XYZ/ImGui/ImGuiLayer.h"

namespace  XYZ
{
    class VulkanImGuiLayer : public ImGuiLayer
    {
    public:
        VulkanImGuiLayer();
        virtual ~VulkanImGuiLayer() override;

        virtual void Begin() override;
        virtual void End() override;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnImGuiRender() override;

    private:
        VkDescriptorPool m_DescriptorPool;
    };
}

