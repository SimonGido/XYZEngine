#include "stdafx.h"
#include "ImGui.h"

#include "XYZ/API/Vulkan/VulkanImage.h"

#include "XYZ/Renderer/RendererAPI.h"

#include <imgui_internal.h>
#include <backends/imgui_impl_vulkan_with_textures.h>

namespace XYZ {
	namespace UI {
		namespace Utils {
			static int		s_UIContextID = 0;
			static uint32_t s_Counter = 0;
			static char		s_IDBuffer[16];
			static char		s_PathBuffer[_MAX_PATH];

			const char* GenerateID()
			{
				s_IDBuffer[0] = '#';
				s_IDBuffer[1] = '#';
				memset(s_IDBuffer + 2, 0, 14);
				sprintf_s(s_IDBuffer + 2, 14, "%o", s_Counter++);

				return &s_IDBuffer[0];
			}

			void SetPathBuffer(const char* value, size_t size)
			{
				memcpy(s_PathBuffer, value, size);
				s_PathBuffer[size] = '\0';
			}

			void SetPathBuffer(const std::string& value)
			{
				memcpy(s_PathBuffer, value.c_str(), value.size());
				s_PathBuffer[value.size()] = '\0';
			}

			char* GetPathBuffer()
			{
				return s_PathBuffer;
			}

			void PushID()
			{
				ImGui::PushID(s_UIContextID++);
				s_Counter = 0;
			}

			void PopID()
			{
				ImGui::PopID();
				s_UIContextID--;
			}

			float Convert_Linear_ToSRGB(float theLinearValue)
			{
				return theLinearValue <= 0.0031308f
					? theLinearValue * 12.92f
					: powf(theLinearValue, 1.0f / 2.2f) * 1.055f - 0.055f;
			}

			float Convert_SRGB_ToLinear(float thesRGBValue)
			{
				return thesRGBValue <= 0.04045f
					? thesRGBValue / 12.92f
					: powf((thesRGBValue + 0.055f) / 1.055f, 2.2f);
			}

			ImVec4 ConvertToSRGB(const ImVec4& colour)
			{
				return ImVec4(
					Convert_Linear_ToSRGB(colour.x),
					Convert_Linear_ToSRGB(colour.y),
					Convert_Linear_ToSRGB(colour.z),
					colour.w
				);
			}

			ImVec4 ConvertToLinear(const ImVec4& colour)
			{
				return ImVec4(
					Convert_SRGB_ToLinear(colour.x),
					Convert_SRGB_ToLinear(colour.y),
					Convert_SRGB_ToLinear(colour.z),
					colour.w
				);
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

		bool BeginTreeNode(const char* name, bool defaultOpen)
		{
			ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
			if (defaultOpen)
				treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;

			return ImGui::TreeNodeEx(name, treeNodeFlags);
		}

		void EndTreeNode()
		{
			ImGui::TreePop();
		}

		bool Vec2Control(const std::array<const char*, 2>& names, glm::vec2& values, float resetValue, float speed)
		{
			return UI::FloatControls(
				names[0], "##X", values.x, resetValue, speed,
				names[1], "##Y", values.y, resetValue, speed
			);
		}
		bool Vec3Control(const std::array<const char*, 3>& names, glm::vec3& values, float resetValue, float speed)
		{
			return UI::FloatControls(
				names[0], "##X", values.x, resetValue, speed,
				names[1], "##Y", values.y, resetValue, speed,
				names[2], "##Z", values.z, resetValue, speed
			);
		}
		bool Vec4Control(const std::array<const char*, 4>& names, glm::vec4& values, float resetValue, float speed)
		{
			return UI::FloatControls(
				names[0], "##X", values.x, resetValue, speed,
				names[1], "##Y", values.y, resetValue, speed,
				names[2], "##Z", values.z, resetValue, speed,
				names[3], "##W", values.w, resetValue, speed
			);
		}
		bool IVec2Control(const std::array<const char*, 2>& names, glm::ivec2& values, float resetValue, float speed)
		{
			return UI::IntControls(
				names[0], "##X", values.x, resetValue, speed,
				names[1], "##Y", values.y, resetValue, speed
			);
		}
		bool IVec3Control(const std::array<const char*, 3>& names, glm::ivec3& values, float resetValue, float speed)
		{
			return UI::IntControls(
				names[0], "##X", values.x, resetValue, speed,
				names[1], "##Y", values.y, resetValue, speed,
				names[2], "##Z", values.z, resetValue, speed
			);
		}
		bool IVec4Control(const std::array<const char*, 4>& names, glm::ivec4& values, float resetValue, float speed)
		{
			return UI::IntControls(
				names[0], "##X", values.x, resetValue, speed,
				names[1], "##Y", values.y, resetValue, speed,
				names[2], "##Z", values.z, resetValue, speed,
				names[3], "##W", values.w, resetValue, speed
			);
		}
	}
}