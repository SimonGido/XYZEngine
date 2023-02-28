#pragma once
#include "XYZ/Scene/Blueprint.h"

#include "Editor/EditorPanel.h"
#include "Editor/Blueprint/BlueprintManager.h"

namespace XYZ {
	namespace Editor {
		//TODO: rename to blueprint editor
		class ParticleEditorGPU : public EditorPanel
		{
		public:
			ParticleEditorGPU(std::string name);
			virtual ~ParticleEditorGPU() override;

			virtual void OnImGuiRender(bool& open) override;
			virtual void OnUpdate(Timestep ts) override;
			virtual bool OnEvent(Event& e) override;

			virtual void SetSceneContext(const Ref<Scene>& scene) override;

		private:
			void onBackgroundMenu();

			void onParticleSystemSelected();

			void editBlueprintTypes();

			Ref<Blueprint> createBlueprint() const;
		private:
			Ref<Scene>			   m_Scene;
			Ref<ParticleSystemGPU> m_ParticleSystem;
			Ref<Blueprint>		   m_Blueprint;

			float				   m_Timestep = 0.0f;
			float				   m_SplitterWidth = 200.0f;

			std::unordered_map<std::string, size_t> m_PerTypeSelection;

			VariableManager	 m_VariableManager;
			BlueprintManager m_BlueprintManager;
		};
	}
}