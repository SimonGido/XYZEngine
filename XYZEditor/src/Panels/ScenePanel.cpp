#include "stdafx.h"
#include "ScenePanel.h"



namespace XYZ {

	std::pair<glm::vec3, glm::vec3> ScenePanel::castRay(float mx, float my) const
	{
		glm::vec4 mouseClipPos = { mx, my, -1.0f, 1.0f };

		auto inverseProj = glm::inverse(m_EditorCamera.GetProjectionMatrix());
		auto inverseView = glm::inverse(glm::mat3(m_EditorCamera.GetViewMatrix()));

		glm::vec4 ray = inverseProj * mouseClipPos;
		glm::vec3 rayPos = m_EditorCamera.GetPosition();
		glm::vec3 rayDir = inverseView * glm::vec3(ray);

		return { rayPos, rayDir };
	}

	std::pair<float, float> ScenePanel::getMouseViewportSpace() const
	{
		auto [mx, my] = Input::GetMousePosition();
		auto& window = InGui::GetWindow(m_PanelID);
		mx -= window.Position.x;
		my -= window.Position.y;

		auto viewportWidth = window.Size.x;
		auto viewportHeight = window.Size.y;

		return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
	}

	ScenePanel::ScenePanel(uint32_t panelID)
		:
		m_PanelID(panelID)
	{
		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
		uint32_t windowHeight = Application::Get().GetWindow().GetHeight();
		m_EditorCamera.SetViewportSize((float)windowWidth, (float)windowHeight);

		InGui::ImageWindow(m_PanelID, "Scene", glm::vec2(0.0f), glm::vec2(200.0f), m_SubTexture);
		InGui::End();

		m_ViewportSize = InGui::GetWindow(m_PanelID).Size;
	}
	void ScenePanel::SetContext(Ref<Scene> context)
	{
		m_Context = context;
	}
	void ScenePanel::SetSubTexture(Ref<SubTexture> subTexture)
	{
		m_SubTexture = subTexture;
	}

	void ScenePanel::OnUpdate(Timestep ts)
	{
		if (IS_SET(InGui::GetWindow(m_PanelID).Flags, InGuiWindowFlags::Hoovered))
		{
			m_EditorCamera.OnUpdate(ts);	
			if (m_Context.Raw())
			{
				if (m_SelectedEntity != (uint32_t)m_Context->GetSelectedEntity())
				{
					m_SelectedEntity = m_Context->GetSelectedEntity();
					m_ModifyFlags = 0;
				}
				if (m_ModifyFlags && (uint32_t)m_Context->GetSelectedEntity() != NULL_ENTITY)
				{
					auto [mx, my] = Input::GetMousePosition();
					float distX = mx - m_OldMousePosition.x;
					float distY = m_OldMousePosition.y - my;
					auto& transform = m_Context->GetSelectedEntity().GetComponent<TransformComponent>();
					glm::vec3* modifiedVal = nullptr;
					
					if (IS_SET(m_ModifyFlags, ModifyFlags::Move)) modifiedVal = &transform.Translation;
					if (IS_SET(m_ModifyFlags, ModifyFlags::Rotate)) modifiedVal = &transform.Rotation;
					if (IS_SET(m_ModifyFlags, ModifyFlags::Scale)) modifiedVal = &transform.Scale;

					if (modifiedVal)
					{
						if (IS_SET(m_ModifyFlags, ModifyFlags::X))
						{
							modifiedVal->x += (distX + distY) * m_MoveSpeed * ts;
						}
						else if (IS_SET(m_ModifyFlags, ModifyFlags::Y))
						{
							modifiedVal->y += (distX + distY) * m_MoveSpeed * ts;
						}
						else if (IS_SET(m_ModifyFlags, ModifyFlags::Z))
						{
							modifiedVal->z += (distX + distY) * m_MoveSpeed * ts;
						}
						else
						{
						}
					}
					m_OldMousePosition = { Input::GetMouseX(), Input::GetMouseY() };
				}
			}
		}

		glm::vec2 newViewportSize = InGui::GetWindow(m_PanelID).Size;
		if (m_Context.Raw() && m_ViewportSize != newViewportSize)
		{
			m_ViewportSize = newViewportSize;
			SceneRenderer::SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			m_Context->SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}
	}
	void ScenePanel::OnInGuiRender()
	{
		if (InGui::ImageWindow(m_PanelID, "Scene", glm::vec2(0.0f), glm::vec2(200.0f), m_SubTexture))
		{
			
		}
		InGui::End();
	}
	void ScenePanel::OnEvent(Event& event)
	{
		if (m_Context.Raw())
		{
			EventDispatcher dispatcher(event);
			dispatcher.Dispatch<WindowResizeEvent>(Hook(&ScenePanel::onWindowResize, this));
			dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&ScenePanel::onMouseButtonPress, this));
			if (IS_SET(InGui::GetWindow(m_PanelID).Flags, InGuiWindowFlags::Hoovered))
			{
				dispatcher.Dispatch<KeyPressedEvent>(Hook(&ScenePanel::onKeyPress, this));
				m_EditorCamera.OnEvent(event);
			}
		}
	}
	bool ScenePanel::onWindowResize(WindowResizeEvent& event)
	{
		return false;
	}
	bool ScenePanel::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_LEFT) && !Input::IsKeyPressed(KeyCode::KEY_LEFT_ALT))
		{
			if (IS_SET(InGui::GetWindow(m_PanelID).Flags, InGuiWindowFlags::Hoovered))
			{
				m_Context->SetSelectedEntity(NULL_ENTITY);
				auto [mouseX, mouseY] = getMouseViewportSpace();
				auto [origin, direction] = castRay(mouseX, mouseY);
				Ray ray = { origin,direction };

				uint32_t res = NULL_ENTITY;
		
				for (uint32_t entityID : m_Context->GetEntities())
				{
					SceneEntity entity(entityID, m_Context.Raw());
					TransformComponent& transformComponent = entity.GetComponent<TransformComponent>();
					glm::mat4 entityTransform = transformComponent.GetTransform();
					
					
					AABB aabb(
						transformComponent.Translation - (transformComponent.Scale / 2.0f),
						transformComponent.Translation + (transformComponent.Scale / 2.0f)
					);
					
					if (ray.IntersectsAABB(aabb))
					{
						m_Context->SetSelectedEntity(entityID);
						return true;
					}				
				}			
			}
		}
		return false;
	}
	bool ScenePanel::onKeyPress(KeyPressedEvent& event)
	{
		if ((uint32_t)m_Context->GetSelectedEntity() != NULL_ENTITY)
		{
			if (m_ModifyFlags)
			{
				if (event.IsKeyPressed(KeyCode::KEY_X))
				{
					m_ModifyFlags |= ModifyFlags::X;
					m_ModifyFlags &= ~(ModifyFlags::Y | ModifyFlags::Z);
				}
				else if (event.IsKeyPressed(KeyCode::KEY_Y))
				{
					m_ModifyFlags |= ModifyFlags::Y;
					m_ModifyFlags &= ~(ModifyFlags::X | ModifyFlags::Z);
				}
				else if (event.IsKeyPressed(KeyCode::KEY_Z))
				{
					m_ModifyFlags |= ModifyFlags::Z;
					m_ModifyFlags &= ~(ModifyFlags::Y | ModifyFlags::X);
				}
			}
			else
			{
				m_ModifyFlags = 0;
			}

			if (event.IsKeyPressed(KeyCode::KEY_G))
			{
				m_ModifyFlags = ModifyFlags::Move;
			}
			else if (event.IsKeyPressed(KeyCode::KEY_R))
			{
				m_ModifyFlags = ModifyFlags::Rotate;
			}
			else if (event.IsKeyPressed(KeyCode::KEY_S))
			{
				m_ModifyFlags = ModifyFlags::Scale;
			}
			
			if (m_ModifyFlags) m_OldMousePosition = { Input::GetMouseX(), Input::GetMouseY() };
			return m_ModifyFlags;
		}
		m_ModifyFlags = 0;
		return false;
	}
	bool ScenePanel::onKeyRelease(KeyReleasedEvent& event)
	{
		return false;
	}
}