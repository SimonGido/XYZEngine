#pragma once

#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/SceneIntersection.h"

#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Event/InputEvent.h"

#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/VoxelRenderer.h"
#include "XYZ/Utils/Math/Ray.h"


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
			std::pair<glm::vec3, glm::vec3> castRay(float mx, float my) const;
			std::pair<float, float>		    getMouseViewportSpace()		const;

			void handlePanelResize(const glm::vec2& newSize);
			void drawVoxelsSpecification();

		private:
			Ref<VoxelRenderer>			m_VoxelRenderer;
			glm::vec2				    m_ViewportSize;
			std::array<glm::vec2, 2>	m_ViewportBounds{};

			bool m_ViewportFocused;
			bool m_ViewportHovered;

			EditorCamera m_EditorCamera;

			static constexpr uint32_t VOXEL_GRID_SIZE = 32;

			uint32_t m_Voxels[VOXEL_GRID_SIZE * VOXEL_GRID_SIZE * VOXEL_GRID_SIZE];

			VoxelsSpecification m_VoxelsSpecification;
		};
	}
}