#include "stdafx.h"
#include "BasicUIAdvancedTypes.h"

#include "BasicUIRenderer.h"
#include "BasicUIHelper.h"
#include "BasicUI.h"

namespace XYZ {
	namespace Helper {
		static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
		{
			return (pos.x + size.x > point.x &&
				pos.x		   < point.x&&
				pos.y + size.y >  point.y &&
				pos.y < point.y);
		}
	}


	bUITimeline::bUITimeline(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color, label, name, type)
	{
		Layout.XOffset = 10.0f;
		Layout.YOffset = 10.0f;
		Layout.XPadding = 15.0f;
		Layout.YPadding = 15.0f;

		CurrentTime = 0.4f;
		Length = 1.0f;
		SplitTime = 0.15f;
		Zoom = 400.0f;
	}

	bUITimeline::bUITimeline(const bUITimeline& other)
		:
		bUIElement(other),
		TimePoints(other.TimePoints),
		Rows(other.Rows),
		Layout(other.Layout),
		CurrentTime(other.CurrentTime),
		Length(other.Length),
		SplitTime(other.SplitTime),
		Zoom(other.Zoom)
	{
	}

	bUITimeline::bUITimeline(bUITimeline&& other) noexcept
		:
		bUIElement(std::move(other)),
		TimePoints(std::move(other.TimePoints)),
		Rows(std::move(other.Rows)),
		Layout(other.Layout),
		CurrentTime(other.CurrentTime),
		Length(other.Length),
		SplitTime(other.SplitTime),
		Zoom(other.Zoom)
	{
	}

	void bUITimeline::PushQuads(bUIRenderer& renderer, uint32_t& scissorID)
	{
		ScissorID = scissorID;
		if (!Visible || !HandleVisibility(ScissorID))
			return;
		
		renderer.Submit<bUITimeline>(*this, scissorID, bUI::GetContext().Config.GetSubTexture(bUIConfig::Button));
	}
	bool bUITimeline::OnLeftMousePressed(const glm::vec2& mousePosition)
	{
		if (!Visible || !HandleVisibility(ScissorID))
			return false;

		glm::vec2 absolutePosition = GetAbsolutePosition();
		float sizeY = bUI::GetContext().Config.GetFont()->GetLineHeight();
		if (Helper::Collide(absolutePosition, { Size.x, sizeY }, mousePosition))
		{
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Active, *this);
			return BlockEvents;
		}
		return false;
	}
	glm::vec2 bUITimeline::GetSize() const
	{
		Ref<Font> font = bUI::GetContext().Config.GetFont();
		return { Size.x, Rows.size() * (font->GetLineHeight() + Layout.YPadding) };
	}
}
