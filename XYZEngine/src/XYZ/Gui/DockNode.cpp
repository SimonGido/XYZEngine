#include "stdafx.h"
#include "DockNode.h"

#include "LineRenderer.h"

namespace XYZ {

	static void OnResize(ComponentResizedEvent& event)
	{		
		Entity entity = event.GetEntity();
		auto& dockNode = entity.GetStorageComponent<DockNodeComponent>();
		auto& mesh = entity.GetStorageComponent<LineRenderer>().LineMesh;
		mesh.Points.clear();
		mesh.Points.push_back(dockNode.Position);
		mesh.Points.push_back(dockNode.Position + glm::vec3(dockNode.Size.x, 0.0f, 0.0f));
		
		mesh.Points.push_back(dockNode.Position + glm::vec3(dockNode.Size.x, 0.0f, 0.0f));
		mesh.Points.push_back(dockNode.Position + glm::vec3(dockNode.Size, 0.0f));
		
		mesh.Points.push_back(dockNode.Position + glm::vec3(dockNode.Size, 0.0f));
		mesh.Points.push_back(dockNode.Position + glm::vec3(0.0f, dockNode.Size.y, 0.0f));
		
		mesh.Points.push_back(dockNode.Position + glm::vec3(0.0f, dockNode.Size.y, 0.0f));
		mesh.Points.push_back(dockNode.Position);
	}

	DockNodeComponent::DockNodeComponent(const glm::vec3& position, const glm::vec2& size)
		:
		Position(position), Size(size)
	{
		RegisterCallback<ComponentResizedEvent>(Hook(&DockNodeComponent::onResize, this));
	}

	bool DockNodeComponent::onResize(ComponentResizedEvent& event)
	{
		auto entity = event.GetEntity();
		if (entity.HasComponent<LineRenderer>())
		{
			OnResize(event);
			return true;
		}
		return false;
	}

}