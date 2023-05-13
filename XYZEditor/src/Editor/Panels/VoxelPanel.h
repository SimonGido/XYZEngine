#pragma once

#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/SceneIntersection.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Event/InputEvent.h"

#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/VoxelRenderer.h"
#include "XYZ/Utils/Math/Ray.h"

#include "XYZ/Asset/Renderer/VoxelMeshSource.h"

#include "Editor/EditorPanel.h"
#include "Editor/EditorCamera.h"

#include "XYZ/Utils/Algorithms/SpaceColonization.h"

namespace XYZ {
	namespace Editor {

		class ChunkSystem
		{
		public:



		private:
			static constexpr uint32_t sc_ChunkWidth = 512;
			static constexpr uint32_t sc_ChunkHeight = 400;
			static constexpr uint32_t sc_ChunkDepth = 512;



			std::array<Ref<VoxelProceduralMesh>, 9> m_Chunks;
		};


		struct VoxelTerrain
		{
			std::vector<uint8_t> Terrain;
			std::vector<uint8_t> WaterMap;
			std::vector<uint16_t> TerrainHeightmap;
		};

		class VoxelPanel : public EditorPanel
		{
		public:
			VoxelPanel(std::string name);
			~VoxelPanel();

			virtual void OnImGuiRender(bool& open) override;
			virtual void OnUpdate(Timestep ts) override;
			virtual bool OnEvent(Event& event) override;

			virtual void SetSceneContext(const Ref<Scene>& context);

			void SetVoxelRenderer(const Ref<VoxelRenderer>& voxelRenderer);

			EditorCamera& GetEditorCamera() { return m_EditorCamera; }

		private:
			void handlePanelResize(const glm::vec2& newSize);
			void drawTransform(TransformComponent& transform, int id) const;

			void pushGenerateVoxelMeshJob();
			void submitWater();
			
			static VoxelTerrain generateVoxelTerrainMesh(
				uint32_t seed, uint32_t frequency, uint32_t octaves,
				uint32_t width, uint32_t height, uint32_t depth
			);

		private:
			Ref<VoxelRenderer>			m_VoxelRenderer;
			glm::vec2				    m_ViewportSize;
			std::array<glm::vec2, 2>	m_ViewportBounds{};

			bool m_ViewportFocused;
			bool m_ViewportHovered;

			EditorCamera m_EditorCamera;
			
			std::vector<TransformComponent> m_CastleTransforms;
			std::vector<TransformComponent> m_KnightTransforms;
			std::vector<TransformComponent> m_DeerTransforms;


			Ref<VoxelSourceMesh> m_CastleMesh;
			Ref<VoxelSourceMesh> m_KnightMesh;
			Ref<VoxelSourceMesh> m_DeerMesh;
			Ref<VoxelProceduralMesh> m_ProceduralMesh;

			Ref<MaterialAsset> m_WaterMaterial;
			
			static constexpr uint8_t Empty = 0;
			static constexpr uint8_t Water = 1;
			static constexpr uint8_t Snow = 2;
			static constexpr uint8_t Grass = 3;
			static constexpr uint8_t Wood = 4;
			static constexpr uint8_t Leaves = 5;

			uint32_t m_DeerKeyFrame = 0;
			float	 m_KeyLength = 0.3f;
			float	 m_CurrentTime = 0.0f;

			uint32_t m_Frequency = 2;
			uint32_t m_Octaves = 3;
			uint32_t m_Seed = 60;

			VoxelTerrain m_Terrain;
			std::future<VoxelTerrain> m_GenerateVoxelsFuture;
	
			bool m_Generating = false;

			StorageBufferAllocation m_WaterDensityAllocation;
			bool m_UpdateWater = false;

			float m_TopGridSize = 32.0f;

			SpaceColonization* m_SpaceColonization = nullptr;
		};
	}
}