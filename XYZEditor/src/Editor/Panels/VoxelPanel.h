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

namespace XYZ {
	namespace Editor {


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

			static std::vector<uint8_t> generateVoxelMesh(
				uint32_t seed, uint32_t frequency, uint32_t octaves,
				uint32_t width, uint32_t height, uint32_t depth
			);


			static void generateVoxelTree(
				std::vector<uint8_t>& voxels,
				uint32_t maxHeight, 
				uint32_t minBranches,
				uint32_t maxBranches,
				uint32_t x, uint32_t y, uint32_t z,
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


			std::future<std::vector<uint8_t>> m_GenerateVoxelsFuture;
			bool m_Generating = false;
		};
	}
}