#pragma once
#include "VulkanImage.h"

#include "XYZ/ImGui/ImGui.h"

#include "XYZ/Renderer/RendererAPI.h"

#include <imgui_internal.h>
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

		bool ImageButton(const char* stringID, const Ref<Image2D>& image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
		{
			if (RendererAPI::GetAPI() == RendererAPI::API::Vulkan)
			{
				Ref<VulkanImage2D> vulkanImage = image.As<VulkanImage2D>();
				const auto& imageInfo = vulkanImage->GetImageInfo();
				if (!imageInfo.ImageView)
					return false;
				const auto textureID = ImGui_ImplVulkan_AddTexture(imageInfo.Sampler, imageInfo.ImageView, vulkanImage->GetDescriptor().imageLayout);
				ImGuiID id = (ImGuiID)((((uint64_t)imageInfo.ImageView) >> 32) ^ (uint32_t)imageInfo.ImageView);
				if (stringID)
				{
					const ImGuiID strID = ImGui::GetID(stringID);
					id = id ^ strID;
				}
	
				return ImGui::ImageButtonEx(id, textureID, size, uv0, uv1, ImVec2{ (float)frame_padding, (float)frame_padding }, bg_col, tint_col);
			}
		}
	}
}