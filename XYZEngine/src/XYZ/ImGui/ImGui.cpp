#include "stdafx.h"
#include "ImGui.h"

#include "XYZ/API/Vulkan/VulkanImage.h"

#include "XYZ/Renderer/RendererAPI.h"

#include <imgui_internal.h>
#include <backends/imgui_impl_vulkan_with_textures.h>

namespace XYZ {
	namespace UI {
		namespace Utils {

			static uint32_t s_Counter = 0;
			static char		s_IDBuffer[16];

			const char* GenerateID()
			{
				s_IDBuffer[0] = '#';
				s_IDBuffer[1] = '#';
				memset(s_IDBuffer + 2, 0, 14);
				sprintf_s(s_IDBuffer + 2, 14, "%o", s_Counter++);

				return &s_IDBuffer[0];
			}

			static ImTextureID GetTextureID(const Ref<Image2D>& image)
			{
				if (RendererAPI::GetType() == RendererAPI::Type::Vulkan)
				{
					Ref<VulkanImage2D> vulkanImage = image.As<VulkanImage2D>();
					const auto& imageInfo = vulkanImage->GetImageInfo();
					if (!imageInfo.ImageView)
						return nullptr;

					return ImGui_ImplVulkan_AddTexture(imageInfo.Sampler, imageInfo.ImageView, vulkanImage->GetDescriptor().imageLayout);
				}
			}
			static ImGuiID GetImageID(const Ref<Image2D>& image)
			{
				if (RendererAPI::GetType() == RendererAPI::Type::Vulkan)
				{
					Ref<VulkanImage2D> vulkanImage = image.As<VulkanImage2D>();
					const auto& imageInfo = vulkanImage->GetImageInfo();
					if (!imageInfo.ImageView)
						return 0;

					return (ImGuiID)((((uint64_t)imageInfo.ImageView) >> 32) ^ (uint32_t)imageInfo.ImageView);
				}
			}
		}

		namespace Vulkan {
			static void VulkanImage(const Ref<Image2D>& image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
			{
				const auto textureID = Utils::GetTextureID(image);
				if (textureID == nullptr)
					return;
				ImGui::Image(textureID, size, uv0, uv1);
			}
			static bool VulkanImageButton(const char* stringID, const Ref<Image2D>& image, const ImVec2& size, int framePadding, const ImVec4& tintColor, const ImVec2& uv0, const ImVec2& uv1)
			{
				const ImTextureID textureID = Utils::GetTextureID(image);
				if (textureID == nullptr)
					return false;

				ImGuiID id = Utils::GetImageID(image);
				if (stringID)
				{
					const ImGuiID strID = ImGui::GetID(stringID);
					id = id ^ strID;
				}
				return ImGui::ImageButtonEx(id, textureID, size, uv0, uv1, ImVec2{ (float)framePadding, (float)framePadding }, tintColor, tintColor);

			}
			static bool VulkanImageButtonTransparent(const char* stringID, const Ref<Image2D>& image, const ImVec2& size, const ImVec4& hoverColor, const ImVec4& clickColor, const ImVec4& tintColor, const ImVec2& uv0, const ImVec2& uv1)
			{
				const ImTextureID textureID = Utils::GetTextureID(image);
				if (textureID == nullptr)
					return false;


				ImVec2 cursorPos = ImGui::GetCursorPos();
				ImGui::InvisibleButton(Utils::GenerateID(), size);
				const bool clicked = ImGui::IsItemClicked();
				const bool hoovered = ImGui::IsItemHovered();
				ImGui::SetCursorPos(cursorPos);
				if (clicked)
					ImGui::Image(textureID, size, uv0, uv1, clickColor);
				else if (hoovered)
					ImGui::Image(textureID, size, uv0, uv1, hoverColor);
				else
					ImGui::Image(textureID, size, uv0, uv1, tintColor);

				return clicked;
			}
		}

		void Image(const Ref<Image2D>& image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
		{
			if (RendererAPI::GetType() == RendererAPI::Type::Vulkan)
			{
				Vulkan::VulkanImage(image, size, uv0, uv1);
			}
		}

		bool ImageButton(const char* stringID, const Ref<Image2D>& image, const ImVec2& size, int framePadding, const ImVec4& tintColor, const ImVec2& uv0, const ImVec2& uv1)
		{
			if (RendererAPI::GetType() == RendererAPI::Type::Vulkan)
			{
				return Vulkan::VulkanImageButton(stringID, image, size, framePadding, tintColor, uv0, uv1);
			}
			return false;
		}

		bool ImageButtonTransparent(const char* stringID, const Ref<Image2D>& image, const ImVec2& size, const ImVec4& hoverColor, const ImVec4& clickColor, const ImVec4& tintColor, const ImVec2& uv0, const ImVec2& uv1)
		{
			if (RendererAPI::GetType() == RendererAPI::Type::Vulkan)
			{
				return Vulkan::VulkanImageButtonTransparent(stringID, image, size, hoverColor, clickColor, tintColor, uv0, uv1);
			}
			return false;
		}
	}
}