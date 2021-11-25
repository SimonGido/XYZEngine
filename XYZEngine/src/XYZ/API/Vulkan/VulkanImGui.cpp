#pragma once
#include "VulkanImage.h"

#include "XYZ/ImGui/ImGui.h"

#include "XYZ/Renderer/RendererAPI.h"


#include <backends/imgui_impl_vulkan_with_textures.h>

namespace XYZ {
	namespace UI {
		void Image(const Ref<Image2D>& image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
		{
			if (RendererAPI::GetAPI() == RendererAPI::API::Vulkan)
			{
				Ref<VulkanImage2D> vulkanImage = image.As<VulkanImage2D>();
				const auto& imageInfo = vulkanImage->GetImageInfo();
				if (!imageInfo.ImageView)
					return;

				const auto textureID = ImGui_ImplVulkan_AddTexture(imageInfo.Sampler, imageInfo.ImageView, vulkanImage->GetDescriptor().imageLayout);
				ImGui::Image(textureID, size, uv0, uv1);
			}
		}
	}
}