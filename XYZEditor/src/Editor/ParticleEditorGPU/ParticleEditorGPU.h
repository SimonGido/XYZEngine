#pragma once
#include "XYZ/ImGui/ImGuiNodeEditor.h"
#include "Editor/EditorPanel.h"
#include "ParticleEditorFunctions.h"

namespace XYZ {
	namespace Editor {
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
			void createDefaultFunctions();

		private:
			ImGuiNodeEditor	m_NodeEditor;
			Ref<Scene>		m_Scene;
			float			m_Timestep = 0.0f;

			std::unordered_map<std::string, ParticleEditorFunction> m_Functions;
		};
	}
}