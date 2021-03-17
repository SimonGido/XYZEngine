#pragma once
#include "InGuiElement.h"
#include "InGuiMeshFactory.h"

namespace XYZ {
	class IGWindow : public IGElement
	{
	public:
		IGWindow(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual bool OnLeftClick(const glm::vec2& mousePosition) override;
		virtual bool OnLeftRelease(const glm::vec2& mousePosition) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData) override;

		enum Flags
		{
			Initialized = BIT(0),
			Hoovered	= BIT(1),
			Moved       = BIT(2),
			Collapsed   = BIT(3),
			Docked	    = BIT(4)
		};

		uint8_t Flags = 0;

		static constexpr float PanelHeight = 25.0f;
	};
	class IGButton : public IGElement
	{
	public:
		IGButton(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual bool OnLeftClick(const glm::vec2& mousePosition) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData) override;
	};
	class IGCheckbox : public IGElement
	{
	public:
		IGCheckbox(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		
		virtual bool OnLeftClick(const glm::vec2& mousePosition) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData) override;

		bool Checked = false;
	};
	class IGSlider : public IGElement
	{
	public:
		IGSlider(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

	};
}