#pragma once
#include "XYZ/Scene/SceneEntity.h"

#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture.h"

namespace XYZ {
	namespace Editor {
		class InspectorPanel
		{
		public:
			InspectorPanel();

			void OnImGuiRender();
	

			void SetContext(SceneEntity context);

		private:
			void drawComponents(SceneEntity entity);

		private:
			SceneEntity m_Context;

			Ref<Material> m_DefaultMaterial;
			Ref<SubTexture> m_DefaultSubTexture;


			static constexpr glm::vec4 sc_BoxColliderColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		};
	}
}