#include "RectTransform.h"

#include "CanvasRenderer.h"

namespace XYZ {


	RectTransform::RectTransform(const glm::vec3& position, const glm::vec2& size)
		: WorldPosition(position), Position(position), Size(size)
	{
		RegisterCallback<CanvasRendererRebuildEvent>(Hook(&RectTransform::onCanvasRendererRebuild, this));
	}

	bool RectTransform::onCanvasRendererRebuild(CanvasRendererRebuildEvent& event)
	{
		if (event.GetEntity().HasComponent<CanvasRenderer>())
		{
			auto& renderer = event.GetEntity().GetComponent<CanvasRenderer>();
			renderer.Mesh.Vertices.clear();

			event.GetSpecification().Rebuild(event.GetEntity());
			return true;
		}
		return false;
	}
}