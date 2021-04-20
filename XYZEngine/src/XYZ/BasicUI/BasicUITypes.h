#pragma once

#include <glm/glm.hpp>

namespace XYZ {
	
	enum class bUIElementType
	{
		None,
		Button,
		Checkbox,
		Slider,
		Window,
		Scrollbox
	};

	enum class bUICallbackType
	{
		Hoover,
		Active
	};

	struct bUIElement;
	using bUICallback = std::function<void(bUICallbackType, bUIElement&)>;

	class bUIRenderer;
	struct bUIElement
	{
		bUIElement(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);
		virtual ~bUIElement() = default;

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) {};
		virtual void OnUpdate();
		virtual bool OnMouseMoved(const glm::vec2& mousePosition);
		virtual bool OnLeftMousePressed(const glm::vec2& mousePosition);
		virtual bool OnRightMousePressed(const glm::vec2& mousePosition);
		virtual bool OnLeftMouseReleased() { return false; };
		virtual bool OnMouseScrolled(const glm::vec2& mousePosition, const glm::vec2& offset) { return false; }
		virtual glm::vec2 GetAbsolutePosition() const;
		void HandleVisibility(uint32_t scissorID);

		glm::vec2			 Coords;
		glm::vec2			 Size;
		glm::vec4			 Color;
		glm::vec4			 ActiveColor;
		std::string			 Label;
		std::string			 Name;
		bUIElement*			 Parent;
		bool				 Visible;
		bool				 ChildrenVisible;
		const bUIElementType Type;

		std::vector<bUICallback> Callbacks;

	private:
		uint32_t depth();

		int32_t ID = -1;

		friend class bUILoader;
		friend class bUIData;
		friend class bUI;
		friend class bUIAllocator;
	};

	struct bUIButton : public bUIElement
	{
		bUIButton(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;
	};

	struct bUICheckbox : public bUIElement
	{
		bUICheckbox(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;
		virtual void OnUpdate() override;
		virtual bool OnLeftMousePressed(const glm::vec2& mousePosition) override;

		bool Checked;
	};

	struct bUISlider : public bUIElement
	{
		bUISlider(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;
		virtual bool OnMouseMoved(const glm::vec2& mousePosition) override;
		virtual bool OnLeftMousePressed(const glm::vec2& mousePosition) override;
		virtual bool OnLeftMouseReleased() override;

		float Value;
	private:
		bool Modified;
	};

	struct bUIWindow : public bUIElement
	{
		bUIWindow(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;
		virtual bool OnMouseMoved(const glm::vec2& mousePosition) override;
		virtual bool OnLeftMousePressed(const glm::vec2& mousePosition) override;
		virtual bool OnRightMousePressed(const glm::vec2& mousePosition) override;
		virtual bool OnLeftMouseReleased() override { ResizeFlags = 0; return false; };

		glm::vec2 ButtonSize = { 25.0f, 25.0f };

	private:
		enum ResizeFlags
		{
			Right  = BIT(0),
			Left   = BIT(1),
			Bottom = BIT(2)
		};
		uint8_t ResizeFlags = 0;
		static constexpr glm::vec2 sc_ResizeOffset = glm::vec2(5.0f);
	};

	struct bUIScrollbox : public bUIElement
	{
		bUIScrollbox(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;
		virtual void OnUpdate();
		virtual bool OnMouseMoved(const glm::vec2& mousePosition) override { return false; };
		virtual bool OnLeftMousePressed(const glm::vec2& mousePosition) override { return false; };
		virtual bool OnRightMousePressed(const glm::vec2& mousePosition) override { return false; };
		virtual bool OnLeftMouseReleased() override { return false; };
		virtual bool OnMouseScrolled(const glm::vec2& mousePosition, const glm::vec2& offset) override;
		virtual glm::vec2 GetAbsolutePosition() const override;
		glm::vec2 GetAbsoluteScrollPosition() const;

		glm::vec2 Offset;

		float Speed = 5.0f;

		bool FitParent = true;
	};

}

