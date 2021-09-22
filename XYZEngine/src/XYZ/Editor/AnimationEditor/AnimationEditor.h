#pragma once
#include "XYZ/Animation/Animation.h"
#include "AnimationSequencer.h"

namespace XYZ {
	namespace Editor {

		class AnimationEditor
		{
		public:
			struct ClassData
			{
				std::vector<std::string> VariableNames;
			};
			using ClassMap = std::unordered_map<std::string, ClassData>;


			AnimationEditor();

			void SetContext(const Ref<Animation>& context);
			void SetScene(const Ref<Scene>& scene);

			void OnUpdate(Timestep ts);
			void OnImGuiRender(bool& open);
		private:
			void handleSelected();
			void buildClassMap(const SceneEntity& entity);

			std::pair<int32_t, int32_t> getClassAndVariable();
	
		private:
			Ref<Animation>	   m_Context;
			Ref<Scene>		   m_Scene;
			SceneEntity		   m_SelectedEntity;

			
			AnimationSequencer m_Sequencer;
			ClassMap		   m_ClassMap;
			int				   m_SelectedEntry;
			int				   m_FirstFrame;
			int				   m_CurrentFrame;
			bool			   m_Expanded;
			bool			   m_Playing;
		};
	}
}