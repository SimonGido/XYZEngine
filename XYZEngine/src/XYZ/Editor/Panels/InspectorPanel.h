#pragma once

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/BasicUI/BasicUI.h"

namespace XYZ {
	namespace Editor {
		class InspectorPanel
		{
		public:
			InspectorPanel();
			~InspectorPanel();

			void SetContext(SceneEntity context, bool forceRebuildUI = false);

			void OnUpdate();
		private:
			void updateLayout(bUIAllocator& allocator);
			void addComponent(uint16_t id);
			void setContextUI();

			void setSceneTagComponent();
			void setTransformComponent();
			void setSpriteRenderer();
			void setScriptComponent();
			void buildScriptComponent();

		private:
			bUILayout m_SceneTagLayout;
			bUILayout m_TransformLayout;
			bUILayout m_SpriteRendererLayout;
			bUILayout m_ScriptLayout;

			SceneEntity m_Context;
			bool		m_ReloadContext = false;
		};
	}
}