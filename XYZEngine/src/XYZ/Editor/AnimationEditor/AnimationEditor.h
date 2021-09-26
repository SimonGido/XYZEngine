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
			void handleAddKey();
			void buildClassMap(const SceneEntity& entity);

			template <typename ComponentType, typename T>
			void addReflectedProperty(Reflection<ComponentType> refl, const SceneEntity& entity, const T& val, const std::string& valName);

			template <typename ComponentType, typename T>
			void addKeyToProperty(Reflection<ComponentType> refl, SceneEntity entity, int frame, const T& val, const std::string& valName);

			bool getClassAndVariable(size_t& classIndex, size_t& variableIndex);
			
			static bool getClassAndVariableFromNames(std::string_view className, std::string_view variableName, size_t& classIndex, size_t& variableIndex);
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

		template<typename ComponentType, typename T>
		inline void AnimationEditor::addReflectedProperty(Reflection<ComponentType> refl, const SceneEntity& entity, const T& val, const std::string& valName)
		{
			m_Context->AddProperty<ComponentType, T>(entity, valName);
			if (!m_Sequencer.ItemTypeExists(refl.sc_ClassName))
				m_Sequencer.AddItemType(refl.sc_ClassName);	

			int itemTypeIndex = m_Sequencer.GetItemTypeIndex(refl.sc_ClassName);
			if (!m_Sequencer.ItemExists(itemTypeIndex, entity))
				m_Sequencer.AddItem(itemTypeIndex, entity);

			m_Sequencer.AddLine(itemTypeIndex, entity, valName);
		}
		template<typename ComponentType, typename T>
		inline void AnimationEditor::addKeyToProperty(Reflection<ComponentType> refl, SceneEntity entity, int frame, const T& val, const std::string& valName)
		{
			auto prop = m_Context->GetProperty<ComponentType, T>(entity, valName);
			if (prop)
			{
				prop->AddKeyFrame({ val, static_cast<uint32_t>(frame) });
			}
		}
	}
}