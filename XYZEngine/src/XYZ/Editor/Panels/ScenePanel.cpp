#include "stdafx.h"
#include "ScenePanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Renderer/SceneRenderer.h"


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
		mx -= m_Window->Position.x;
		my -= m_Window->Position.y;

		auto viewportWidth = m_Window->Size.x;
		auto viewportHeight = m_Window->Size.y;

		return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
	}

	ScenePanel::ScenePanel()
	{
		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
		uint32_t windowHeight = Application::Get().GetWindow().GetHeight();
		m_EditorCamera.SetViewportSize((float)windowWidth, (float)windowHeight);

		std::initializer_list<IGHierarchyElement> types{
			{
				IGElementType::ImageWindow,
				{	
					{IGElementType::Image, {}},
					{IGElementType::Image, {}}
				}
			}
		};
		size_t* handles = nullptr;
		auto [poolHandle, handleCount] = IG::AllocateUI(types);
		m_PoolHandle = poolHandle;
		m_HandleCount = handleCount;
		m_Window = &IG::GetUI<IGImageWindow>(m_PoolHandle, 0);	
		m_Window->Label = "Scene";
		m_Window->ResizeCallback = [&](const glm::vec2& size) {

			m_ViewportSize = size;
			SceneRenderer::SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			if (m_Context.Raw())
				m_Context->SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		};
		m_Window->ResizeCallback(m_Window->Size);

		m_PlayButton = &IG::GetUI<IGImage>(m_PoolHandle, 1);
		m_PauseButton = &IG::GetUI<IGImage>(m_PoolHandle, 2);
		m_PlayButton->Size = glm::vec2(25.0f);
		m_PauseButton->Size = glm::vec2(25.0f);
		m_PlayButton->SubTexture = IG::GetContext().RenderData.SubTextures[IGRenderData::RightArrow];
		m_PauseButton->SubTexture = IG::GetContext().RenderData.SubTextures[IGRenderData::Pause];
	}
	void ScenePanel::SetContext(Ref<Scene> context)
	{
		m_Context = context;
		m_Context->SetViewportSize((uint32_t)m_Window->Size.x, (uint32_t)m_Window->Size.y);
	}
	void ScenePanel::SetSubTexture(Ref<SubTexture> subTexture)
	{
		m_Window->SubTexture = subTexture;
	}

	void ScenePanel::OnUpdate(Timestep ts)
	{
		if (IS_SET(m_Window->Flags, IGWindow::Hoovered))
		{
			m_EditorCamera.OnUpdate(ts);	
			if (m_Context.Raw())
			{
				if ((uint32_t)m_SelectedEntity != (uint32_t)m_Context->GetSelectedEntity())
				{
					m_SelectedEntity = m_Context->GetSelectedEntity();
					m_ModifyFlags = 0;
				}
				if (m_ModifyFlags && m_Context->GetSelectedEntity().IsValid())
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
	}
	
	void ScenePanel::OnEvent(Event& event)
	{
		if (m_Context.Raw())
		{
			EventDispatcher dispatcher(event);
			dispatcher.Dispatch<WindowResizeEvent>(Hook(&ScenePanel::onWindowResize, this));
			dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&ScenePanel::onMouseButtonPress, this));
			if (IS_SET(m_Window->Flags, IGWindow::Hoovered))
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
			if (IS_SET(m_Window->Flags, IGWindow::Hoovered))
			{
				m_Context->SetSelectedEntity(Entity());
				auto [mouseX, mouseY] = getMouseViewportSpace();
				auto [origin, direction] = castRay(mouseX, mouseY);
				Ray ray = { origin,direction };
		
				for (Entity entityID : m_Context->GetEntities())
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
		if (m_Context->GetSelectedEntity().IsValid())
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