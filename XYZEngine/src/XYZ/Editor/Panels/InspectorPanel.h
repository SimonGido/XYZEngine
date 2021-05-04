#pragma once

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Editor/EditorUI.h"

namespace XYZ {
	namespace Editor {
		class InspectorPanel : public EditorUI
		{
		public:
			InspectorPanel(const std::string& filepath);
			virtual ~InspectorPanel() override;

			virtual void OnUpdate(Timestep ts) override;
			virtual void OnReload() override;
			virtual void SetupUI() override;

			void SetContext(SceneEntity context, bool forceRebuildUI = false);

		private:
			void updateLayout(bUIAllocator& allocator);
			void addComponent(uint16_t id);

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