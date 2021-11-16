#pragma once
#include "XYZ/Animation/Animation.h"
#include "AnimationSequencer.h"
#include "XYZ/Animation/Animator.h"

namespace XYZ {
	namespace Editor {

		class AnimationEditor
		{
		public:
			struct ClassData
			{
				std::string				 ClassName;
				std::vector<std::string> VariableNames;
			};
			using ClassMap = std::unordered_map<std::string, std::vector<ClassData>>;

		
			AnimationEditor();

			void SetContext(const Ref<Animator>& context);
			void SetScene(const Ref<Scene>& scene);

			void OnUpdate(Timestep ts);
			void OnImGuiRender(bool& open);
		private:
			void propertySection();
			void timelineSection();
			
			void handleEditKeyEndFrames();
			void handleEditKeyValues();
			void handleAddKey();
			void keySelectionActions();
			void buildClassMap(SceneEntity entity);

			template <typename ComponentType, typename T>
			void editKeyValue(Reflection<ComponentType> refl, const std::string& path, uint32_t frame, const T& val, const std::string& valName);

			template <typename T>
			bool editKeyValueSpecialized(uint32_t frame, T& value, const std::string& valName);

			template <typename ComponentType, typename T>
			void addReflectedProperty(Reflection<ComponentType> refl, const std::string & path, const T& val, const std::string& valName, uint16_t valIndex);

			template <typename ComponentType, typename T>
			void addKeyToProperty(Reflection<ComponentType> refl, const std::string& path, uint32_t frame, const T& val, const std::string& valName);

			template <typename ComponentType, typename T>
			Property<T>* getProperty(Reflection<ComponentType> refl, const T& val, const std::string& path, const std::string& valName);

			bool getClassVariableAndPath(std::string& path, size_t& classIndex, size_t& variableIndex);
			
			static bool getClassAndVariableFromNames(std::string_view className, std::string_view variableName, size_t& classIndex, size_t& variableIndex);
		private:
			Ref<Animator>	   m_Context;
			Ref<Animation>	   m_Animation;
			Ref<Scene>		   m_Scene;
			SceneEntity		   m_SelectedEntity;

			
			AnimationSequencer m_Sequencer;
			ClassMap		   m_ClassMap;
			int				   m_SelectedEntry;
			int				   m_FirstFrame;
			uint32_t		   m_CurrentFrame;
			bool			   m_Expanded;
			bool			   m_Playing;
			bool			   m_OpenSelectionActions;
			float			   m_PropertySectionWidth;
			float			   m_TimelineSectionWidth;
		};

		template<typename ComponentType, typename T>
		inline void AnimationEditor::editKeyValue(Reflection<ComponentType> refl, const std::string& path, uint32_t frame, const T& val, const std::string& valName)
		{
			auto prop = m_Context->GetProperty<ComponentType, T>(path, valName);
			if (prop && !prop->Empty())
			{
				T value = prop->GetValue(frame);
				if (editKeyValueSpecialized(frame, value, valName))
				{
					if (prop->HasKeyAtFrame(frame))
					{
						size_t key = prop->FindKey(frame);
						prop->SetKeyValue(value, key);
					}
					else
					{
						prop->AddKeyFrame({ value, frame });
						m_Sequencer.AddKey(m_SelectedEntry, frame);
					}
				}
			}
		}

		template<typename ComponentType, typename T>
		inline void AnimationEditor::addReflectedProperty(Reflection<ComponentType> refl, const std::string& path, const T& val, const std::string& valName, uint16_t valIndex)
		{
			m_Animation->AddProperty<ComponentType, T>(path, valName, valIndex);
			if (!m_Sequencer.ItemTypeExists(refl.sc_ClassName))
				m_Sequencer.AddItemType(refl.sc_ClassName);

			const int itemTypeIndex = m_Sequencer.GetItemTypeIndex(refl.sc_ClassName);
			if (!m_Sequencer.ItemExists(itemTypeIndex, path))
				m_Sequencer.AddItem(itemTypeIndex, path);

			m_Sequencer.AddLine(itemTypeIndex, path, valName);
		}
		template<typename ComponentType, typename T>
		inline void AnimationEditor::addKeyToProperty(Reflection<ComponentType> refl, const std::string& path, uint32_t frame, const T& val, const std::string& valName)
		{
			auto prop = m_Animation->GetProperty<ComponentType, T>(path, valName);
			if (prop)
			{
				prop->AddKeyFrame({ val, frame });
			}
		}
		template<typename ComponentType, typename T>
		inline Property<T>* AnimationEditor::getProperty(Reflection<ComponentType> refl, const T& val, const std::string& path, const std::string& valName)
		{
			return m_Animation->GetProperty<ComponentType, T>(path, valName);
		}
	}
}
