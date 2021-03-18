#pragma once
#include "IGElement.h"
#include "IGMeshFactory.h"

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
			Initialized	 = BIT(0),
			Hoovered	 = BIT(1),
			Moved		 = BIT(2),
			LeftResize	 = BIT(3),
			RightResize	 = BIT(4),
			BottomResize = BIT(5),
			Collapsed	 = BIT(6),
			Docked		 = BIT(7)
		};
	

		uint8_t Flags = 0;

		static constexpr float PanelHeight = 25.0f;
	};
	class IGButton : public IGElement
	{
	public:
		IGButton(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual bool OnLeftClick(const glm::vec2& mousePosition) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData) override;
	};
	class IGCheckbox : public IGElement
	{
	public:
		IGCheckbox(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual bool OnLeftClick(const glm::vec2& mousePosition) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData) override;

		bool Checked = false;
	};
	class IGSlider : public IGElement
	{
	public:
		IGSlider(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual bool OnLeftClick(const glm::vec2& mousePosition) override;
		virtual bool OnLeftRelease(const glm::vec2& mousePosition) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData) override;


		float Value = 0.0f;
	private:
		bool Modified = false;
	};

	class IGText : public IGElement
	{
	public:
		IGText(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
	
		virtual bool OnLeftClick(const glm::vec2& mousePosition) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData) override;

		std::string Text = "Text";
	};

	class IGFloat : public IGElement
	{
	public:
		IGFloat(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual bool OnLeftClick(const glm::vec2& mousePosition) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition) override;
		virtual bool OnKeyType(char character) override;
		virtual bool OnKeyPress(int32_t mode, int32_t key) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData) override;

		
		float GetValue() const;
		const char* GetBuffer() const { return Buffer; }


		bool  Listen = false;
		static constexpr size_t BufferSize = 60;

	private:
		float Value = 0.0f;
		uint32_t ModifiedIndex;
		char Buffer[BufferSize];
	};

}