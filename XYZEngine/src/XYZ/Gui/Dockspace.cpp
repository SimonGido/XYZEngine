#include "stdafx.h"
#include "Dockspace.h"

#include "XYZ/Core/Input.h"

#include "XYZ/Scene/Serializer.h"

static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
{
	return (pos.x + size.x / 2.0f > point.x &&
		pos.x - size.x / 2.0f < point.x &&
		pos.y + size.y / 2.0f > point.y &&
		pos.y - size.y / 2.0f < point.y);
}

static bool CollideNotCentered(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
{
	return (pos.x + size.x > point.x &&
		pos.x		   < point.x&&
		pos.y + size.y > point.y &&
		pos.y < point.y);
}

static glm::vec2 MouseToWorld(const glm::vec2& point, const glm::vec2& windowSize)
{
	glm::vec2 offset = { windowSize.x / 2,windowSize.y / 2 };
	return { point.x - offset.x, offset.y - point.y };
}

namespace XYZ {
	Dockspace::Dockspace(ECSManager* ecs, GuiContext* context)
		:
		m_ECS(ecs),
		m_Context(context)
	{
		for (uint32_t i = 0; i < ecs->GetNumberOfEntities(); ++i)
		{
			if (ecs->Contains<Dockable>(i))
			{
				auto& button = m_ECS->GetStorageComponent<Button>(i);
				button.RegisterCallback<ClickEvent>(Hook(&Dockspace::onButtonPress, this));
				button.RegisterCallback<ReleaseEvent>(Hook(&Dockspace::onButtonRelease, this));
			}
		}
	}

	void Dockspace::OnUpdate(Timestep ts)
	{
		if (m_ModifiedEntity != NULL_ENTITY)
		{
			auto& transform = m_ECS->GetStorageComponent<RectTransform>(m_ModifiedEntity);

			auto [mx, my] = Input::GetMousePosition();
			auto [width, height] = Input::GetWindowSize();
			glm::vec2 mousePosition = MouseToWorld({ mx,my }, { width, height });
			transform.Position = glm::vec3(mousePosition - m_MouseOffset, 0.0f);
		}
		else if (m_ResizeData.Entity != NULL_ENTITY)
		{
			handleResize();
		}
	}
	void Dockspace::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		if (dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&Dockspace::onMouseButtonPress, this)))
		{
		}
		else if (dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&Dockspace::onMouseButtonRelease, this)))
		{
		}
		else if (dispatcher.Dispatch<WindowResizeEvent>(Hook(&Dockspace::onWindowResize, this)))
		{
		}
	}
	void Dockspace::SetRoot(uint32_t entity)
	{
		m_RootEntity = entity;
		if (m_RootEntity == NULL_ENTITY)
		{
			m_RootEntity = m_ECS->CreateEntity();
			m_ECS->AddComponent<IDComponent>(m_RootEntity, IDComponent());
		}
		if (!m_ECS->Contains<DockNodeComponent>(m_RootEntity))
		{
			auto [width, height] = Input::GetWindowSize();
			glm::vec2 size = { width, height };
			m_ECS->AddComponent<DockNodeComponent>(m_RootEntity, DockNodeComponent(glm::vec3(-size / 2.0f , 0.0f), size));
		}
		else
		{
			auto [width, height] = Input::GetWindowSize();
			glm::vec2 size = { width, height };
			auto& dockNode = m_ECS->GetStorageComponent<DockNodeComponent>(m_RootEntity);
			dockNode.Position = glm::vec3(-size / 2.0f, 0.0f);
			dockNode.Size = size;
		}
		if (!m_ECS->Contains<Relationship>(m_RootEntity))
		{
			m_ECS->AddComponent<Relationship>(m_RootEntity, Relationship());
		}
	}

	Entity Dockspace::CreatePanel(uint32_t canvas, const std::string& name, const PanelSpecification& specs)
	{
		XYZ_ASSERT(m_RootEntity != NULL_ENTITY, "No root found");
		glm::vec2 buttonSize = glm::vec2(50.0f, 30.0f);
		glm::vec3 buttonPosition = glm::vec3(
			specs.Position.x,
			specs.Position.y,
			0.0f
		);

		PanelSpecification modifiedSpecs(
			glm::vec3((specs.Size.x / 2.0f) - (buttonSize.x / 2.0f), -(specs.Size.y / 2.0f) + (buttonSize.y / 2.0f), 0.0f),
			glm::vec2(specs.Size),
			specs.Color
		);

		ButtonSpecification panelButtonSpecs(
			name,
			buttonPosition,
			buttonSize,
			glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
			glm::vec4(0.4f, 1.0f, 0.8f, 1.0f),
			glm::vec4(1.0f, 0.5f, 0.8f, 0.0f),
			1
		);
		uint32_t buttonEntity = m_Context->CreateButton(canvas, panelButtonSpecs);
		m_ECS->AddComponent<Dockable>(buttonEntity, Dockable());

		uint32_t panelEntity = m_Context->CreatePanel(canvas, modifiedSpecs);
		Relationship::RemoveRelation(panelEntity, *m_ECS);
		Relationship::SetupRelation(buttonEntity, panelEntity, *m_ECS);

		auto& button = m_ECS->GetStorageComponent<Button>(buttonEntity);

		button.RegisterCallback<ClickEvent>(Hook(&Dockspace::onButtonPress, this));
		button.RegisterCallback<ReleaseEvent>(Hook(&Dockspace::onButtonRelease, this));
		return { panelEntity, m_ECS };
	}
	Entity Dockspace::CreateRenderWindow(uint32_t canvas, const std::string& name, const ImageSpecification& specs)
	{
		XYZ_ASSERT(m_RootEntity != NULL_ENTITY, "No root found");
		glm::vec2 buttonSize = glm::vec2(50.0f, 30.0f);
		glm::vec3 buttonPosition = glm::vec3(
			specs.Position.x,
			specs.Position.y,
			0.0f
		);

		ImageSpecification modifiedSpecs(
			specs.SubTexture,
			glm::vec3((specs.Size.x / 2.0f) - (buttonSize.x / 2.0f), -(specs.Size.y / 2.0f) + (buttonSize.y / 2.0f), 0.0f),
			glm::vec2(specs.Size),
			specs.Color
		);

		ButtonSpecification panelButtonSpecs(
			name,
			buttonPosition,
			buttonSize,
			glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
			glm::vec4(0.4f, 1.0f, 0.8f, 1.0f),
			glm::vec4(1.0f, 0.5f, 0.8f, 1.0f),
			1
		);
		uint32_t buttonEntity = m_Context->CreateButton(canvas, panelButtonSpecs);
		m_ECS->AddComponent<Dockable>(buttonEntity, Dockable());

		uint32_t panelEntity = m_Context->CreateImage(canvas, modifiedSpecs);

		Relationship::RemoveRelation(panelEntity, *m_ECS);
		Relationship::SetupRelation(buttonEntity, panelEntity, *m_ECS);

		auto& button = m_ECS->GetStorageComponent<Button>(buttonEntity);
		button.RegisterCallback<ClickEvent>(Hook(&Dockspace::onButtonPress, this));
		button.RegisterCallback<ReleaseEvent>(Hook(&Dockspace::onButtonRelease, this));

		return { panelEntity, m_ECS };
	}
	bool Dockspace::onWindowResize(WindowResizeEvent& event)
	{
		glm::vec2 newWindowSize = { event.GetWidth(), event.GetHeight() };
		glm::vec2 oldSize = m_ECS->GetStorageComponent<DockNodeComponent>(m_RootEntity).Size;
		glm::vec2 scale = (newWindowSize / oldSize);
		rescaleNode(m_RootEntity, scale);
		updateEntities(m_RootEntity);

		return false;
	}
	bool Dockspace::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		auto [mx, my] = Input::GetMousePosition();
		auto [width, height] = Input::GetWindowSize();
		glm::vec2 mousePosition = MouseToWorld({ mx,my }, { width, height });

		findResize(m_RootEntity, mousePosition);
		return false;
	}
	bool Dockspace::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		m_ModifiedEntity = NULL_ENTITY;
		m_ResizeData.Entity = NULL_ENTITY;
		return false;
	}
	bool Dockspace::onButtonPress(ClickEvent& event)
	{
		auto [mx, my] = Input::GetMousePosition();
		auto [width, height] = Input::GetWindowSize();
		glm::vec2 mousePosition = MouseToWorld({ mx,my }, { width, height });

		uint32_t buttonEntity = event.GetEntity();
		auto& transform = m_ECS->GetStorageComponent<RectTransform>(buttonEntity);

		if (Collide(transform.WorldPosition, transform.Size, mousePosition))
		{
			m_ModifiedEntity = buttonEntity;
			m_MouseOffset = mousePosition - glm::vec2(transform.Position);
			removeFromNode(m_RootEntity, buttonEntity);
			return true;
		}
		return false;
	}
	bool Dockspace::onButtonRelease(ReleaseEvent& event)
	{
		auto [mx, my] = Input::GetMousePosition();
		auto [width, height] = Input::GetWindowSize();
		glm::vec2 mousePosition = MouseToWorld({ mx,my }, { width, height });

		uint32_t buttonEntity = event.GetEntity();
		uint32_t nodeEntity = findNode(m_RootEntity, mousePosition);
		if (nodeEntity != NULL_ENTITY)
		{
			uint32_t node = splitNode(nodeEntity, resolveSplit(nodeEntity, mousePosition), mousePosition);
			insertToNode(node, buttonEntity);
		}
		return false;
	}
	uint32_t Dockspace::findResize(uint32_t nodeEntity, const glm::vec2& mousePosition)
	{
		static constexpr float offset = 5.0f;

		auto& currentRel = m_ECS->GetStorageComponent<Relationship>(nodeEntity);
		auto& dockNode = m_ECS->GetStorageComponent<DockNodeComponent>(nodeEntity);

		if (CollideNotCentered(dockNode.Position, dockNode.Size, mousePosition) && currentRel.FirstChild != NULL_ENTITY)
		{
			auto& firstNodeRel = m_ECS->GetStorageComponent<Relationship>(currentRel.FirstChild);
			auto& firstDockNode = m_ECS->GetStorageComponent<DockNodeComponent>(currentRel.FirstChild);
			auto& secondDockNode = m_ECS->GetStorageComponent<DockNodeComponent>(firstNodeRel.NextSibling);

			if (dockNode.Split == SplitType::Horizontal)
			{
				if (mousePosition.y > firstDockNode.Position.y + firstDockNode.Size.y - offset
				 && mousePosition.y < firstDockNode.Position.y + firstDockNode.Size.y + offset)
				{
					m_ResizeData.Entity = nodeEntity;
					m_ResizeData.Type = dockNode.Split;
					return nodeEntity;
				}
			}
			else
			{
				if (mousePosition.x > firstDockNode.Position.x + firstDockNode.Size.x  - offset
				 && mousePosition.x < firstDockNode.Position.x + firstDockNode.Size.x + offset)
				{
					m_ResizeData.Entity = nodeEntity;
					m_ResizeData.Type = dockNode.Split;
					return nodeEntity;
				}
			}
		}
		uint32_t result = NULL_ENTITY;
		uint32_t currentEntity = currentRel.FirstChild;
		while (currentEntity != NULL_ENTITY)
		{
			result = findResize(currentEntity, mousePosition);
			if (result != NULL_ENTITY)
				return result;

			currentEntity = m_ECS->GetStorageComponent<Relationship>(currentEntity).NextSibling;
		}
		return result;
	}
	void Dockspace::handleResize()
	{
		auto [mx, my] = Input::GetMousePosition();
		auto [width, height] = Input::GetWindowSize();
		glm::vec2 mousePosition = MouseToWorld({ mx,my }, { width, height });

		auto& dockNode = m_ECS->GetComponent<DockNodeComponent>(m_ResizeData.Entity);
		auto& rel = m_ECS->GetStorageComponent<Relationship>(m_ResizeData.Entity);
		if (rel.FirstChild != NULL_ENTITY)
		{
			auto& childRel = m_ECS->GetStorageComponent<Relationship>(rel.FirstChild);
			auto& firstDockNode = m_ECS->GetStorageComponent<DockNodeComponent>(rel.FirstChild);
			auto& secondDockNode = m_ECS->GetStorageComponent<DockNodeComponent>(childRel.NextSibling);

			if (m_ResizeData.Type == SplitType::Horizontal)
			{
				firstDockNode.Size.y = mousePosition.y - dockNode.Position.y;
				secondDockNode.Position.y = dockNode.Position.y + firstDockNode.Size.y;
				secondDockNode.Size.y = (dockNode.Position.y + dockNode.Size.y) - secondDockNode.Position.y;
			}
			else if (m_ResizeData.Type == SplitType::Vertical)
			{
				glm::vec2 leftOld = firstDockNode.Size;
				glm::vec2 rightOld = secondDockNode.Size;
				glm::vec2 leftNew = { mousePosition.x - dockNode.Position.x, firstDockNode.Size.y };
				glm::vec2 rightNew = { ( dockNode.Position.x + dockNode.Size.x) - secondDockNode.Position.x , secondDockNode.Size.y };
				
				firstDockNode.Size.x = leftNew.x;
				secondDockNode.Position.x = dockNode.Position.x + firstDockNode.Size.x;
				secondDockNode.Size.x = rightNew.x;
			}
			firstDockNode.Execute<ComponentResizedEvent>(ComponentResizedEvent({ rel.FirstChild, m_ECS }));
			secondDockNode.Execute<ComponentResizedEvent>(ComponentResizedEvent({ childRel.NextSibling, m_ECS }));

			adjustNodeChildren(rel.FirstChild);
			adjustNodeChildren(childRel.NextSibling);
		}
		updateEntities(m_ResizeData.Entity);
	}
	uint32_t Dockspace::splitNode(uint32_t nodeEntity, SplitType type, const glm::vec2& mousePosition)
	{
		auto& dockNode = m_ECS->GetComponent<DockNodeComponent>(nodeEntity);
		dockNode.Split = type;
		uint32_t result = nodeEntity;
		if (type == SplitType::Horizontal)
		{
			glm::vec3 topPosition = { dockNode.Position.x, dockNode.Position.y + dockNode.Size.y / 2.0f, 0.0f };
			glm::vec3 bottomPosition = { dockNode.Position.x, dockNode.Position.y, 0.0f };
			glm::vec2 size = { dockNode.Size.x, dockNode.Size.y / 2.0f };

			uint32_t top = createNode(nodeEntity, topPosition, size);
			uint32_t bottom = createNode(nodeEntity, bottomPosition, size);

			// Old dock node no longer valid
			auto& updatedDockNode = m_ECS->GetStorageComponent<DockNodeComponent>(nodeEntity);
			if (mousePosition.y < topPosition.y)
			{
				result = bottom;
				for (auto entity : updatedDockNode.Entities)
					insertToNode(top, entity);
			}
			else
			{
				result = top;
				for (auto entity : updatedDockNode.Entities)
					insertToNode(bottom, entity);
			}
			updatedDockNode.Entities.clear();
		}
		else if (type == SplitType::Vertical)
		{
			glm::vec3 leftPosition = { dockNode.Position.x, dockNode.Position.y, 0.0f };
			glm::vec3 rightPosition = { dockNode.Position.x + dockNode.Size.x / 2.0f, dockNode.Position.y, 0.0f };
			glm::vec2 size = { dockNode.Size.x / 2.0f, dockNode.Size.y };

			uint32_t right = createNode(nodeEntity, rightPosition, size);
			uint32_t left = createNode(nodeEntity, leftPosition, size);

			// Old dock node no longer valid
			auto& updatedDockNode = m_ECS->GetComponent<DockNodeComponent>(nodeEntity);

			if (mousePosition.x > leftPosition.x + size.x)
			{
				result = right;
				for (auto entity : updatedDockNode.Entities)
					insertToNode(left, entity);
			}
			else
			{
				result = left;
				for (auto entity : updatedDockNode.Entities)
					insertToNode(right, entity);
			}
			updatedDockNode.Entities.clear();
		}
		return result;
	}

	void Dockspace::updateEntities(uint32_t nodeEntity)
	{
		auto& dockNode = m_ECS->GetStorageComponent<DockNodeComponent>(nodeEntity);
		for (auto entity : dockNode.Entities)
			adjustEntityTransform(nodeEntity, entity);

		auto& rel = m_ECS->GetStorageComponent<Relationship>(nodeEntity);
		uint32_t current = rel.FirstChild;
		while (current != NULL_ENTITY)
		{
			updateEntities(current);
			current = m_ECS->GetStorageComponent<Relationship>(current).NextSibling;
		}
	}

	void Dockspace::destroyNode(uint32_t nodeEntity)
	{
		auto& rel = m_ECS->GetStorageComponent<Relationship>(nodeEntity);
		auto& dockNode = m_ECS->GetStorageComponent<DockNodeComponent>(rel.Parent);

		uint32_t sibling = rel.NextSibling;
		if (rel.NextSibling == NULL_ENTITY)
			sibling = rel.PreviousSibling;
		
		auto& dockNodeSibling = m_ECS->GetStorageComponent<DockNodeComponent>(sibling);
		if (dockNodeSibling.Split != SplitType::None)
			return;

		for (auto entity : dockNodeSibling.Entities)
			insertToNode(rel.Parent, entity);

		uint32_t tmpSibling = sibling;
		Relationship::RemoveRelation(sibling, *m_ECS);
		m_ECS->DestroyEntity(tmpSibling);
		

		Relationship::RemoveRelation(nodeEntity, *m_ECS);
		m_ECS->DestroyEntity(nodeEntity);
		dockNode.Split = SplitType::None;
	}
	void Dockspace::adjustNodeChildren(uint32_t nodeEntity)
	{
		auto& rel = m_ECS->GetStorageComponent<Relationship>(nodeEntity);
		auto& dockNode = m_ECS->GetStorageComponent<DockNodeComponent>(nodeEntity);
		uint32_t current = rel.FirstChild;
		if (current != NULL_ENTITY)
		{
			uint32_t sibling = m_ECS->GetStorageComponent<Relationship>(current).NextSibling;
			auto& firstDockNode = m_ECS->GetStorageComponent<DockNodeComponent>(current);
			auto& secondDockNode = m_ECS->GetStorageComponent<DockNodeComponent>(sibling);

			if (dockNode.Split == SplitType::Horizontal)
			{
				glm::vec2 halfSize = { dockNode.Size.x , dockNode.Size.y / 2 };
				glm::vec3 bottomPos = { dockNode.Position.x ,dockNode.Position.y,0.0f };
				glm::vec3 topPos = { dockNode.Position.x , dockNode.Position.y + firstDockNode.Size.y ,0.0f };

				firstDockNode.Position = bottomPos;
				secondDockNode.Position = topPos;
				firstDockNode.Size.x = dockNode.Size.x;
				secondDockNode.Size.x = dockNode.Size.x;	
				secondDockNode.Size.y = dockNode.Size.y - firstDockNode.Size.y;
			}
			else if (dockNode.Split == SplitType::Vertical)
			{
				firstDockNode.Position = dockNode.Position;
				firstDockNode.Size.x = secondDockNode.Position.x - firstDockNode.Position.x;
				firstDockNode.Size.y = dockNode.Size.y;
				secondDockNode.Size.x = dockNode.Size.x - firstDockNode.Size.x;
				secondDockNode.Size.y = dockNode.Size.y;
			}
			firstDockNode.Execute<ComponentResizedEvent>(ComponentResizedEvent({ current, m_ECS }));
			secondDockNode.Execute<ComponentResizedEvent>(ComponentResizedEvent({ sibling, m_ECS }));
			
			adjustNodeChildren(current);
			adjustNodeChildren(sibling);
		}
	}
	void Dockspace::adjustEntityTransform(uint32_t nodeEntity, uint32_t entity)
	{
		auto& dockNode = m_ECS->GetStorageComponent<DockNodeComponent>(nodeEntity);
		auto& transform = m_ECS->GetStorageComponent<RectTransform>(entity);
		auto& entityRel = m_ECS->GetStorageComponent<Relationship>(entity);
		auto& firstChildTransform = m_ECS->GetStorageComponent<RectTransform>(entityRel.FirstChild);


		transform.Position = dockNode.Position + glm::vec3(
			transform.Size.x / 2.0f, dockNode.Size.y - transform.Size.y / 2.0f,
			0.0f
		);
		
		firstChildTransform.Size = dockNode.Size;
		firstChildTransform.Position = glm::vec3(
			(dockNode.Size.x / 2.0f) - (transform.Size.x / 2.0f),
		   -(dockNode.Size.y / 2.0f) + (transform.Size.y / 2.0f),
			0.0f
		);
		firstChildTransform.Execute<ComponentResizedEvent>(ComponentResizedEvent({ entityRel.FirstChild, m_ECS }));
	}
	bool Dockspace::removeFromNode(uint32_t nodeEntity, uint32_t entity)
	{
		auto& rel = m_ECS->GetStorageComponent<Relationship>(nodeEntity);
		auto& dockNode = m_ECS->GetStorageComponent<DockNodeComponent>(nodeEntity);
		auto it = std::find(dockNode.Entities.begin(), dockNode.Entities.end(), entity);
		if (it != dockNode.Entities.end())
		{
			dockNode.Entities.erase(it);
			if (dockNode.Entities.empty())
			{
				if (nodeEntity != m_RootEntity)
				{
					destroyNode(nodeEntity);
				}
			}
			return true;
		}
		
		uint32_t currentEntity = rel.FirstChild;
		while (currentEntity != NULL_ENTITY)
		{
			if (removeFromNode(currentEntity, entity))
				return true;

			currentEntity = m_ECS->GetStorageComponent<Relationship>(currentEntity).NextSibling;
		}
		return false;
	}
	void Dockspace::resizeNode(uint32_t nodeEntity, const glm::vec3& positionDiff, const glm::vec2& sizeDiff)
	{
		auto& dockNode = m_ECS->GetStorageComponent<DockNodeComponent>(nodeEntity);
		dockNode.Position += positionDiff;
		dockNode.Size += sizeDiff;


		auto& rel = m_ECS->GetStorageComponent<Relationship>(nodeEntity);
		uint32_t current = rel.FirstChild;
		while (current != NULL_ENTITY)
		{
			resizeNode(current, positionDiff, sizeDiff);
			current = m_ECS->GetStorageComponent<Relationship>(current).NextSibling;
		}
	}
	void Dockspace::rescaleNode(uint32_t nodeEntity, const glm::vec2 scale)
	{
		auto& dockNode = m_ECS->GetStorageComponent<DockNodeComponent>(nodeEntity);
		dockNode.Size *= scale;
		dockNode.Position *= glm::vec3(scale, 1.0f);


		auto& rel = m_ECS->GetStorageComponent<Relationship>(nodeEntity);
		uint32_t current = rel.FirstChild;
		while (current != NULL_ENTITY)
		{
			rescaleNode(current, scale);
			current = m_ECS->GetStorageComponent<Relationship>(current).NextSibling;
		}
	}
	void Dockspace::insertToNode(uint32_t nodeEntity, uint32_t entity)
	{
		adjustEntityTransform(nodeEntity, entity);
		m_ECS->GetStorageComponent<DockNodeComponent>(nodeEntity).Entities.push_back(entity);
	}
	uint32_t Dockspace::createNode(uint32_t parent, const glm::vec3& position, const glm::vec2& size)
	{
		uint32_t entity = m_ECS->CreateEntity();
		m_ECS->AddComponent<IDComponent>(entity, IDComponent());

		auto& dock = m_ECS->AddComponent<DockNodeComponent>(entity, DockNodeComponent(position, size));
		m_ECS->AddComponent<Relationship>(entity, Relationship());

		m_ECS->AddComponent<LineRenderer>(entity, LineRenderer(glm::vec4(0.1f, 0.1f, 0.0f, 1.0f), LineMesh()));

		Relationship::SetupRelation(parent, entity, *m_ECS);
		return entity;
	}

	uint32_t Dockspace::findNode(uint32_t entity, const glm::vec2& mousePosition)
	{
		auto& currentRel = m_ECS->GetStorageComponent<Relationship>(entity);
		auto& dockNode = m_ECS->GetStorageComponent<DockNodeComponent>(entity);

		if (currentRel.FirstChild == NULL_ENTITY && CollideNotCentered(dockNode.Position, dockNode.Size, mousePosition))
			return entity;

		uint32_t result = NULL_ENTITY;
		uint32_t currentEntity = currentRel.FirstChild;
		while (currentEntity != NULL_ENTITY)
		{
			result = findNode(currentEntity, mousePosition);
			if (result != NULL_ENTITY)
				return result;

			currentEntity = m_ECS->GetStorageComponent<Relationship>(currentEntity).NextSibling;
		}
		return result;
	}
	SplitType Dockspace::resolveSplit(uint32_t nodeEntity, const glm::vec2& mousePosition)
	{
		auto& dockNode = m_ECS->GetStorageComponent<DockNodeComponent>(nodeEntity);
		if (mousePosition.x < dockNode.Position.x + dockNode.Size.x / 4.0f
		 || mousePosition.x > dockNode.Position.x + dockNode.Size.x - (dockNode.Size.x / 4.0f))
		{
			return SplitType::Vertical;
		}
		else if (mousePosition.y < dockNode.Position.y + dockNode.Size.y / 4.0f
			  || mousePosition.y > dockNode.Position.y + dockNode.Size.y - (dockNode.Size.y / 4.0f))
		{
			return SplitType::Horizontal;
		}
		return SplitType::None;
	}

}