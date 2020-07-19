#pragma once
#include "Material.h"
#include "XYZ/Gui/Font.h"

namespace XYZ {

	struct UIRenderData
	{
		glm::vec2 ViewportSize;
	};

	

	class InGuiRenderer
	{
	public:
		enum TextCentered
		{
			None = 1 << 0,
			Middle = 1 << 1,
			Left = 1 << 2,
			Right = 1 << 3,
			Top = 1 << 4,
			Bottom = 1 << 4
		};
		/**
		* Initialize Renderer2D instance and register batch system
		*/
		static void Init();

		/**
		* Delete instance of the renderer
		*/
		static void Shutdown();

		/**
		* Initial setup before rendering
		* @param[in] camera		Reference to the camera
		*/
		static void BeginScene(const UIRenderData& data);

		static void SetMaterial(const Ref<Material>& material);
		static void SubmitUI(const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color = glm::vec4(1));
		static void SubmitText(const std::string& text,const Ref<Font>& font, const glm::vec2& position, const glm::vec2& scale, uint32_t textureID, const glm::vec4& color = glm::vec4(1));
		static void SubmitCenteredText(const std::string& text, const Ref<Font>& font, const glm::vec2& position, const glm::vec2& scale, uint32_t textureID, const glm::vec4& color = glm::vec4(1), int centered = TextCentered::Middle);
		/**
		* Execute the command queue
		*/
		static void Flush();

		/**
		* Clean up after rendering
		*/
		static void EndScene();

	};
}