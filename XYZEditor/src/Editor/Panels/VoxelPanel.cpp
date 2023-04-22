#include "stdafx.h"
#include "VoxelPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"

#include "XYZ/Renderer/SceneRenderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer2D.h"


#include "XYZ/Utils/Math/Math.h"
#include "XYZ/ImGui/ImGui.h"

#include "Editor/Event/EditorEvents.h"
#include "Editor/EditorHelper.h"
#include "EditorLayer.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace XYZ {
	namespace Editor {

		namespace Utils {
			static std::random_device s_RandomDev; // obtain a random number from hardware
			static std::mt19937 s_RandomGen(s_RandomDev());

			static uint8_t RandomColorIndex()
			{
				std::uniform_int_distribution<> distr(0, 255); // define the range
				return static_cast<uint8_t>(distr(s_RandomGen));
			}


			static uint32_t Index3D(int x, int y, int z, int width, int height)
			{
				return x + width * (y + height * z);
			}



			static std::array<glm::vec2, 2> ImGuiViewportBounds()
			{
				const auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
				const auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
				const auto viewportOffset = ImGui::GetWindowPos();

				std::array<glm::vec2, 2> result;
				result[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
				result[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
				return result;
			}
		}

		VoxelPanel::VoxelPanel(std::string name)
			:
			EditorPanel(std::forward<std::string>(name)),
			m_ViewportSize(0.0f),
			m_EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f)
		{
			for (int i = 0; i < VOXEL_GRID_SIZE; ++i)
			{
				for (int j = 0; j < VOXEL_GRID_SIZE; ++j)
				{
					for (int k = 0; k < VOXEL_GRID_SIZE; ++k)
					{
						m_Voxels[Utils::Index3D(i, j, k, VOXEL_GRID_SIZE, VOXEL_GRID_SIZE)] = Utils::RandomColorIndex();
					}
				}
			}

			m_VoxelMeshSource = Ref<VoxelMeshSource>::Create("Assets/Voxel/castle.vox");
		}

		VoxelPanel::~VoxelPanel()
		{
		}

		void VoxelPanel::OnImGuiRender(bool& open)
		{
			{
				UI::ScopedStyleStack styleStack(true, ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
				if (ImGui::Begin("Scene", &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
				{
					if (m_VoxelRenderer.Raw())
					{
						const ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
						m_ViewportBounds = Utils::ImGuiViewportBounds();
						m_ViewportFocused = ImGui::IsWindowFocused();
						m_ViewportHovered = ImGui::IsWindowHovered();

						ImGuiLayer* imguiLayer = Application::Get().GetImGuiLayer();
						const bool blocked = imguiLayer->GetBlockedEvents();
						// Only unlock possible here
						imguiLayer->BlockEvents(blocked && !m_ViewportFocused && !m_ViewportHovered);


						UI::Image(m_VoxelRenderer->GetFinalPassImage(), viewportPanelSize);
			
						handlePanelResize({ viewportPanelSize.x, viewportPanelSize.y });
					}
				}
				if (m_VoxelRenderer.Raw())
				{
					m_VoxelRenderer->OnImGuiRender();
				}
				ImGui::End();
			}
			drawVoxelsSpecification();
		}

		void VoxelPanel::OnUpdate(Timestep ts)
		{
			if (m_VoxelRenderer.Raw())
			{
				m_EditorCamera.OnUpdate(ts);

				m_VoxelRenderer->BeginScene(
					m_EditorCamera.GetViewProjection(), 
					m_EditorCamera.GetViewMatrix(), 
					m_EditorCamera.GetProjectionMatrix(), 
					m_EditorCamera.GetPosition()
				);
				
				m_VoxelRenderer->SetColors(m_VoxelMeshSource->GetColorPallete());
	
				m_VoxelRenderer->SubmitMesh(m_VoxelMeshSource, m_Transform.GetLocalTransform(), 1.0f);
				m_VoxelRenderer->EndScene();
			}
		}

		bool VoxelPanel::OnEvent(Event& event)
		{
			if (m_ViewportHovered && m_ViewportFocused)
				m_EditorCamera.OnEvent(event);

			return false;
		}

		void VoxelPanel::SetSceneContext(const Ref<Scene>& context)
		{
		}

		void VoxelPanel::SetVoxelRenderer(const Ref<VoxelRenderer>& voxelRenderer)
		{
			m_VoxelRenderer = voxelRenderer;
		}

	
		std::pair<glm::vec3, glm::vec3> VoxelPanel::castRay(float mx, float my) const
		{
			const glm::vec4 mouseClipPos = { mx, my, -1.0f, 1.0f };

			const auto inverseProj = glm::inverse(m_EditorCamera.GetProjectionMatrix());
			const auto inverseView = glm::inverse(glm::mat3(m_EditorCamera.GetViewMatrix()));

			const glm::vec4 ray = inverseProj * mouseClipPos;
			glm::vec3 rayPos = m_EditorCamera.GetPosition();
			glm::vec3 rayDir = inverseView * glm::vec3(ray);

			return { rayPos, rayDir };
		}

		std::pair<float, float> VoxelPanel::getMouseViewportSpace() const
		{
			auto [mx, my] = Input::GetMousePosition();
			auto [winPosX, winPosY] = Input::GetWindowPosition();
			mx -= ImGui::GetWindowPos().x;
			my -= ImGui::GetWindowPos().y;
			mx += winPosX;
			my += winPosY;

			const auto viewportWidth = ImGui::GetWindowSize().x;
			const auto viewportHeight = ImGui::GetWindowSize().y;

			return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };

		}

		void VoxelPanel::handlePanelResize(const glm::vec2& newSize)
		{
			if (m_ViewportSize.x != newSize.x || m_ViewportSize.y != newSize.y)
			{
				m_ViewportSize = newSize;
				m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
				m_VoxelRenderer->SetViewportSize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
			}
		}

		void VoxelPanel::drawVoxelsSpecification()
		{
			if (ImGui::Begin("Voxels Specification"))
			{
				ImGui::DragFloat3("Translation", glm::value_ptr(m_Transform.GetTransform().Translation), 0.1f);
				ImGui::DragFloat3("Rotation", glm::value_ptr(m_Transform.GetTransform().Rotation), 0.1f);
			}
			ImGui::End();
		}
	}
}
