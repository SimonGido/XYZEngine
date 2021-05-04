#pragma once
#include "XYZ/Utils/DataStructures/MemoryPool.h"
#include "XYZ/Utils/DataStructures/Tree.h"
#include "XYZ/Renderer/SubTexture.h"
#include "BasicUITypes.h"


#include <glm/glm.hpp>

namespace XYZ {

	class bUITimeline : public bUIElement
	{
	public:
		bUITimeline(const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);
		bUITimeline(const bUITimeline& other);
		bUITimeline(bUITimeline&& other) noexcept;

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;

		virtual glm::vec2 GetSize() const override;

		struct Layout
		{
			float XOffset, YOffset;
			float XPadding, YPadding;
		};

		struct Row
		{
			std::string Name;
		};

		struct TimePoint
		{
			uint32_t Row;
			float	 Time;
		};

		std::vector<TimePoint> TimePoints;
		std::vector<Row>       Rows;
		Layout				   Layout;
		float				   CurrentTime;
		float				   Length;
		float				   SplitTime;
		float				   Zoom;
	};
}