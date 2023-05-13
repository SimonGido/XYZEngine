#include "stdafx.h"
#include "VoxelPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"

#include "XYZ/Renderer/SceneRenderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer2D.h"


#include "XYZ/Utils/Math/AABB.h"
#include "XYZ/Utils/Math/Math.h"
#include "XYZ/Utils/Math/Perlin.h"
#include "XYZ/Utils/Random.h"
#include "XYZ/Utils/Algorithms/Raymarch.h"


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
			
			static uint8_t RandomColorIndex(uint32_t x = 0, uint32_t y = 255)
			{
				return static_cast<uint8_t>(RandomNumber(x, y));
			}


			static uint32_t Index3D(int x, int y, int z, int width, int height)
			{
				return x + width * (y + height * z);
			}

			static uint32_t Index3D(const glm::ivec3& index, int width, int height)
			{
				return index.x + width * (index.y + height * index.z);
			}


			static uint32_t Index2D(int x, int z, int depth)
			{
				return x * depth + z;
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

		VoxelSubmesh GenerateSnow(uint32_t width, uint32_t height, uint32_t depth, float voxelSize)
		{
			VoxelSubmesh submesh;
			submesh.Width = width;
			submesh.Height = height;
			submesh.Depth = depth;
			submesh.VoxelSize = voxelSize;

			submesh.ColorIndices.resize(submesh.Width * submesh.Height * submesh.Depth);
			memset(submesh.ColorIndices.data(), 0, submesh.ColorIndices.size());

			for (uint32_t x = 0; x < width; x++)
			{
				for (uint32_t z = 0; z < depth; z++)
				{
					if (Utils::RandomColorIndex(0, 10) == 0) // % chance to generate snow voxel
						submesh.ColorIndices[Utils::Index3D(x, height - 1, z, width, height)] = Utils::RandomColorIndex(0, 1);
				}
			}
			return submesh;
		}

		VoxelPanel::VoxelPanel(std::string name)
			:
			EditorPanel(std::forward<std::string>(name)),
			m_ViewportSize(0.0f),
			m_EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f)
		{

			m_DeerMesh = Ref<VoxelSourceMesh>::Create(Ref<VoxelMeshSource>::Create("Assets/Voxel/anim/deer.vox"));
			m_CastleMesh = Ref<VoxelSourceMesh>::Create(Ref<VoxelMeshSource>::Create("Assets/Voxel/castle.vox"));
			m_KnightMesh = Ref<VoxelSourceMesh>::Create(Ref<VoxelMeshSource>::Create("Assets/Voxel/chr_knight.vox"));

			m_ProceduralMesh = Ref<VoxelProceduralMesh>::Create();

			VoxelSubmesh submesh;
			submesh.Width = 256;
			submesh.Height = 256;
			submesh.Depth = 256;
			submesh.VoxelSize = 6.0f;
			{
				const glm::vec3 rayDir = glm::normalize(glm::vec3(1, 1, 1));
				const glm::vec3 delta = glm::abs(glm::vec3(submesh.Width, submesh.Height, submesh.Depth) / rayDir);
				const float maxDistance = std::max(delta.x, std::max(delta.y, delta.z));
			}
		
			submesh.ColorIndices.resize(submesh.Width * submesh.Height * submesh.Depth);
			memset(submesh.ColorIndices.data(), 0, submesh.ColorIndices.size());

			
			VoxelInstance instance;
			instance.SubmeshIndex = 0;
			instance.Transform = glm::mat4(1.0f);
	
			auto colorPallete = m_KnightMesh->GetColorPallete();
			
			colorPallete[Water] = { 0, 100, 220, 255 };
			colorPallete[Snow] = { 255, 255, 255, 255 }; // Snow
			colorPallete[Grass] = { 1, 60, 32, 255 }; // Grass
			colorPallete[Wood] = { 150, 75, 0, 255 };
			colorPallete[Leaves] = { 1, 100, 40, 255 };

			m_ProceduralMesh->SetColorPallete(colorPallete);
			m_ProceduralMesh->SetSubmeshes({ submesh});
			m_ProceduralMesh->SetInstances({ instance });

	
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
			pushGenerateVoxelMeshJob();

			Ref<Shader> waterShader = Shader::Create("Resources/Shaders/Voxel/WaterTest.glsl");
			Ref<ShaderAsset> waterShaderAsset = Ref<ShaderAsset>::Create(waterShader);
			m_WaterMaterial = Ref<MaterialAsset>::Create(waterShaderAsset);
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
			if (ImGui::Begin("Voxels Generator"))
			{
				ImGui::DragInt("Frequency", (int*)&m_Frequency, 1.0f, 0, INT_MAX);
				ImGui::DragInt("Octaves", (int*)&m_Octaves, 1.0f, 0, INT_MAX);
				ImGui::DragInt("Seed", (int*)&m_Seed, 1.0f, 0, INT_MAX);
				
				if (ImGui::Button("Apply") && !m_Generating)
				{
					pushGenerateVoxelMeshJob();
				}
				ImGui::DragFloat("Top Grid Size", &m_TopGridSize, 1.0f, 0, 1024);
				
				if (ImGui::Button("Compress"))
				{
					m_ProceduralMesh->Compress(64);
				}
				ImGui::Checkbox("Update Water", &m_UpdateWater);
			}
			ImGui::End();

			if (ImGui::Begin("Transforms"))
			{
				int id = 0;
				ImGui::Text("%d", id);
				for (auto& transform : m_CastleTransforms)
					drawTransform(transform, id++);
				ImGui::NewLine();
			}
			ImGui::End();
		}

		void VoxelPanel::OnUpdate(Timestep ts)
		{
			if (m_VoxelRenderer.Raw())
			{			
				m_EditorCamera.OnUpdate(ts);
				
				const glm::mat4 mvp = m_EditorCamera.GetViewProjection();
				
				if (!m_Generating && m_GenerateVoxelsFuture.valid())
				{
					VoxelSubmesh terrainSubmesh = m_ProceduralMesh->GetSubmeshes()[0];
					
					m_Terrain = std::move(m_GenerateVoxelsFuture.get());
									
					if (m_SpaceColonization != nullptr)
						delete m_SpaceColonization;

					const float voxelSize = 3.0f;
					auto height = m_Terrain.TerrainHeightmap[Utils::Index2D(terrainSubmesh.Width / 2, terrainSubmesh.Depth / 2, terrainSubmesh.Depth)];
					const glm::vec3 voxelPosition = glm::vec3(256, height, 256) * voxelSize;
					
					SCInitializer initializer;
					initializer.AttractorsCount = 200;
					initializer.AttractorsCenter = voxelPosition + glm::vec3(0, 100 * voxelSize, 0);
					initializer.AttractorsRadius = 50.0f * voxelSize;
					initializer.AttractionRange = 20.0f * voxelSize;
					initializer.KillRange = 10.0f * voxelSize;
					initializer.RootPosition = voxelPosition;
					initializer.BranchLength = 7 * voxelSize;

					m_SpaceColonization = new SpaceColonization(initializer);

					m_SpaceColonization->VoxelizeAttractors(m_Terrain.Terrain, terrainSubmesh.Width, terrainSubmesh.Height, terrainSubmesh.Depth, voxelSize);

					terrainSubmesh.ColorIndices = m_Terrain.Terrain;
					
					m_ProceduralMesh->SetSubmeshes({ terrainSubmesh });
					m_VoxelRenderer->SubmitComputeData(m_Terrain.WaterMap.data(), m_Terrain.WaterMap.size(), 0, m_WaterDensityAllocation, true);
				}

				m_VoxelRenderer->BeginScene({
					m_EditorCamera.GetViewProjection(),
					m_EditorCamera.GetViewMatrix(),
					m_EditorCamera.GetProjectionMatrix(),
					m_EditorCamera.GetPosition(),
					m_EditorCamera.CreateFrustum()
				});
				
				if (Input::IsKeyPressed(KeyCode::KEY_SPACE))
				{
					auto& submesh = m_ProceduralMesh->GetSubmeshes()[0];
					for (uint32_t y = 0; y < submesh.Height; ++y)
					{
						m_ProceduralMesh->SetVoxelColor(0, 256, y, 256, RandomNumber(5u, 255u));
					}
				}
				//for (uint32_t i = 0; i < 9; i++)
					m_VoxelRenderer->SubmitMesh(m_ProceduralMesh, glm::mat4(1.0f), false);

				for (size_t i = 0; i < m_CastleTransforms.size(); ++i)
				{
					const glm::mat4 castleTransform = m_CastleTransforms[i].GetLocalTransform();
					const glm::mat4 knightTransform = m_KnightTransforms[i].GetLocalTransform();
					const glm::mat4 deerTransform = m_DeerTransforms[i].GetLocalTransform();
				
					m_VoxelRenderer->SubmitMesh(m_CastleMesh, castleTransform);
					m_VoxelRenderer->SubmitMesh(m_KnightMesh, knightTransform);
					m_VoxelRenderer->SubmitMesh(m_DeerMesh, deerTransform, &m_DeerKeyFrame);
				}
				
				submitWater();
				
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

		static int32_t radius = 7;
		bool VoxelPanel::OnEvent(Event& event)
		{
			if (m_ViewportHovered && m_ViewportFocused)
				m_EditorCamera.OnEvent(event);

			if (event.GetEventType() == EventType::KeyPressed)
			{
				KeyPressedEvent& keyPressedE = (KeyPressedEvent&)event;
				if (keyPressedE.IsKeyPressed(KeyCode::KEY_N))
				{
					if (m_SpaceColonization != nullptr)
					{
						VoxelSubmesh terrainSubmesh = m_ProceduralMesh->GetSubmeshes()[0];
						m_SpaceColonization->Grow(
							terrainSubmesh.ColorIndices, 
							terrainSubmesh.Width, 
							terrainSubmesh.Height, 
							terrainSubmesh.Depth, 
							terrainSubmesh.VoxelSize,
							radius
						);		
						if (radius > 1 && RandomNumber(0u, 1u) != 0)
							radius--;

						m_ProceduralMesh->SetSubmeshes({ terrainSubmesh });
					}
				}
			}

			return false;
		}

		void VoxelPanel::SetSceneContext(const Ref<Scene>& context)
		{
		}

		void VoxelPanel::SetVoxelRenderer(const Ref<VoxelRenderer>& voxelRenderer)
		{
			m_VoxelRenderer = voxelRenderer;
			m_VoxelRenderer->CreateComputeAllocation(512 * 400 * 512, m_WaterDensityAllocation);
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

		void VoxelPanel::pushGenerateVoxelMeshJob()
		{
			m_Generating = true;

			uint32_t seed = m_Seed;
			uint32_t frequency = m_Frequency;
			uint32_t octaves = m_Octaves;

			auto& terrainSubmesh = m_ProceduralMesh->GetSubmeshes()[0];

			uint32_t width = terrainSubmesh.Width;
			uint32_t height = terrainSubmesh.Height;
			uint32_t depth = terrainSubmesh.Depth;
	
			auto& pool = Application::Get().GetThreadPool();
			m_GenerateVoxelsFuture = pool.SubmitJob([this, seed, frequency, octaves, width, height, depth]() {

				auto terrain = generateVoxelTerrainMesh(
					seed, frequency, octaves,
					width, height, depth
				);
				
				m_Generating = false;
				return terrain;
			});
		}

		static uint32_t reqDispatchCounter = 0;
		static int randSeeds[9];

		void VoxelPanel::submitWater()
		{
			if (reqDispatchCounter == 0 && m_UpdateWater)
			{
				reqDispatchCounter = 3;
				for (int i = 0; i < 9; i++)
					randSeeds[i] = RandomNumber(0u, 500u);
			}

			if (reqDispatchCounter > 0)
			{
				const glm::ivec3 workGroups = {
					std::ceil(512.0f / 3.0f / 16),
					std::ceil(512.0f / 3.0f / 16),
					400 / 2 / 4 // submesh height / 2 / local_size_z
				};

				for (uint32_t j = 0; j < 2; ++j)
				{
					for (uint32_t i = 0; i < 9; ++i)
					{
						m_VoxelRenderer->SubmitEffect(
							m_WaterMaterial,
							workGroups,
							PushConstBuffer
							{
								randSeeds[i],
								0u, // Model index
								(uint32_t)Empty, 
								(uint32_t)Water, 
								255u, // Max density
								i, 
								j
							}
						);
					}
				}
				reqDispatchCounter--;
			}
		}

		
		
		VoxelTerrain VoxelPanel::generateVoxelTerrainMesh(
			uint32_t seed, uint32_t frequency, uint32_t octaves,
			uint32_t width, uint32_t height, uint32_t depth
		)
		{
			const double fx = ((double)frequency / (double)width);
			const double fy = ((double)frequency / (double)height);

			VoxelTerrain result;
			result.Terrain.resize(width * height * depth);
			result.WaterMap.resize(width * height * depth);
			memset(result.WaterMap.data(), 0, result.WaterMap.size());

			result.TerrainHeightmap.resize(width * depth);
			for (uint32_t x = 0; x < width; ++x)
			{
				for (uint32_t z = 0; z < depth; ++z)
				{
					const double xDouble = x;
					const double zDouble = z;
					const double val = Perlin::Octave2D(xDouble * fx, zDouble * fy, octaves);
					const uint32_t genHeight = val * height;

					for (uint32_t y = 0; y < genHeight; y++)
					{
						result.Terrain[Utils::Index3D(x, y, z, width, height)] = Grass;
					}
		
					result.TerrainHeightmap[Utils::Index2D(x, z, depth)] = genHeight;
					
					if (genHeight < 50)
					{
						const uint32_t waterIndex = Utils::Index3D(x, 50, z, width, height);
						result.Terrain[waterIndex] = Water; // Water
						result.WaterMap[waterIndex] = 125; // Half max density
					}

					//for (int32_t y = genHeight; y < 150; ++y)
					//{
					//	const uint32_t waterIndex = Utils::Index3D(x, y, z, width, height);
					//	result.Terrain[waterIndex] = Water; // Water
					//	result.WaterMap[waterIndex] = 125; // Half max density
					//}
					
					//const uint32_t waterIndex = Utils::Index3D(x, 150, z, width, height);
					//result.Terrain[waterIndex] = Water; // Water
					//result.WaterMap[waterIndex] = 125; // Half max density
				}
			}
			return result;
		}
	}
}
