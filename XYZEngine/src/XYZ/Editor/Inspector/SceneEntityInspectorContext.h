#pragma once
#include "InspectorContext.h"


namespace XYZ {
	namespace Editor {
		class SceneEntityInspectorContext : public InspectorContext
		{
		public:
			SceneEntityInspectorContext();

			virtual void OnImGuiRender() override;

			void SetContext(SceneEntity context);
		private:
			SceneEntity m_Context;

			Ref<Material> m_DefaultMaterial;
			Ref<SubTexture> m_DefaultSubTexture;


			static constexpr glm::vec4 sc_ColliderColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		};
	}
}