#include "stdafx.h"
#include "VoxelPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"

#include "XYZ/Renderer/SceneRenderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer2D.h"


#include "XYZ/Utils/Math/AABB.h"
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

			static AABB VoxelModelToAABB(const glm::mat4& transform, uint32_t width, uint32_t height, uint32_t depth, float voxelSize)
			{
				AABB result;
				glm::vec3 min = glm::vec3(0.0f);
				glm::vec3 max = glm::vec3(width, height, depth) * voxelSize;

				result = result.TransformAABB(transform);
				return result;
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
			m_ProceduralMesh = Ref<VoxelProceduralMesh>::Create();
			VoxelSubmesh submesh;
			submesh.Width = 400;
			submesh.Height = 100;
			submesh.Depth = 400;
			submesh.ColorIndices.resize(submesh.Width * submesh.Height * submesh.Depth);

			VoxelInstance instance;
			instance.SubmeshIndex = 0;
			instance.Transform = glm::mat4(1.0f);

			for (int i = 0; i < submesh.Width; ++i)
			{
				for (int j = 0; j < submesh.Height; ++j)
				{
					for (int k = 0; k < submesh.Depth; ++k)
					{
						submesh.ColorIndices[Utils::Index3D(i, j, k, submesh.Width, submesh.Height)] = Utils::RandomColorIndex();
					}
				}
			}

			m_ProceduralMesh->SetSubmeshes({ submesh });
			m_ProceduralMesh->SetInstances({ instance });
			
			m_DeerMesh   = Ref<VoxelSourceMesh>::Create(Ref<VoxelMeshSource>::Create("Assets/Voxel/anim/deer.vox"));
			m_CastleMesh = Ref<VoxelSourceMesh>::Create(Ref<VoxelMeshSource>::Create("Assets/Voxel/castle.vox"));
			m_KnightMesh = Ref<VoxelSourceMesh>::Create(Ref<VoxelMeshSource>::Create("Assets/Voxel/chr_knight.vox"));
				

			uint32_t count = 50;
			m_CastleTransforms.resize(count);
			m_KnightTransforms.resize(count);
			m_DeerTransforms.resize(count);

			float xOffset = 0.0f;
			for (uint32_t i = 0; i < count; ++i)
			{
				m_CastleTransforms[i].GetTransform().Translation.x = xOffset;
				m_CastleTransforms[i].GetTransform().Rotation.x = glm::radians(-90.0f);

				m_KnightTransforms[i].GetTransform().Translation.x = xOffset;
				m_KnightTransforms[i].GetTransform().Translation.y = 30.0f;
				m_KnightTransforms[i].GetTransform().Rotation.x = glm::radians(-90.0f);

				m_DeerTransforms[i].GetTransform().Translation.x = xOffset;
				m_DeerTransforms[i].GetTransform().Rotation.x = glm::radians(-90.0f);
				m_DeerTransforms[i].GetTransform().Translation.y = 50.0f;

				xOffset += 30.0f;
			}
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
			if (ImGui::Begin("Voxels Transform"))
			{
				int id = 0;
				for (auto& transform : m_CastleTransforms)
				{
					drawTransform(transform, id++);
					ImGui::NewLine();
				}
				for (auto& transform : m_KnightTransforms)
				{
					drawTransform(transform, id++);
					ImGui::NewLine();
				}
				for (auto& transform : m_DeerTransforms)
				{
					drawTransform(transform, id++);
					ImGui::NewLine();
				}
			}
			ImGui::End();
		}

		void VoxelPanel::OnUpdate(Timestep ts)
		{
			if (m_VoxelRenderer.Raw())
			{
				m_EditorCamera.OnUpdate(ts);

				const glm::mat4 mvp = m_EditorCamera.GetViewProjection();
				

				m_VoxelRenderer->BeginScene({
					m_EditorCamera.GetViewProjection(),
					m_EditorCamera.GetViewMatrix(),
					m_EditorCamera.GetProjectionMatrix(),
					m_EditorCamera.GetPosition(),
					m_EditorCamera.CreateFrustum()
				});
				
				m_VoxelRenderer->SetColors(m_CastleMesh->GetMeshSource()->GetColorPallete());
	
				for (size_t i = 0; i < m_CastleTransforms.size(); ++i)
				{
					const glm::mat4 castleTransform = m_CastleTransforms[i].GetLocalTransform();
					const glm::mat4 knightTransform = m_KnightTransforms[i].GetLocalTransform();
					const glm::mat4 deerTransform = m_DeerTransforms[i].GetLocalTransform();

					m_VoxelRenderer->SubmitMesh(m_CastleMesh, castleTransform, 1.0f);
					m_VoxelRenderer->SubmitMesh(m_KnightMesh, knightTransform, 1.0f);
					m_VoxelRenderer->SubmitMesh(m_DeerMesh, deerTransform, &m_DeerKeyFrame, 1.0f);
				}

				m_VoxelRenderer->SubmitMesh(m_ProceduralMesh, glm::mat4(1.0f), 3.0f);

				if (m_CurrentTime > m_KeyLength)
				{
					const uint32_t numKeyframes = m_DeerMesh->GetMeshSource()->GetInstances()[0].ModelAnimation.SubmeshIndices.size();
					m_DeerKeyFrame = (m_DeerKeyFrame + 1) % numKeyframes;
					m_CurrentTime = 0.0f;
				}
				m_CurrentTime += ts;
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


		void VoxelPanel::handlePanelResize(const glm::vec2& newSize)
		{
			if (m_ViewportSize.x != newSize.x || m_ViewportSize.y != newSize.y)
			{
				m_ViewportSize = newSize;
				m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
				m_VoxelRenderer->SetViewportSize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
			}
		}

		void VoxelPanel::drawTransform(TransformComponent& transform, int id) const
		{
			ImGui::PushID(id);
			glm::vec3 rotation = glm::degrees(transform->Rotation);
			ImGui::DragFloat3("Translation", glm::value_ptr(transform.GetTransform().Translation), 0.1f);
			if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 0.1f))
			{
				transform.GetTransform().Rotation = glm::radians(rotation);
			}
			ImGui::PopID();
		}
	}
}
