#pragma once
#include "Editor/EditorPanel.h"
#include "AnimationSequencer.h"
#include "AnimationClassMap.h"

#include "XYZ/Animation/Animation.h"
#include "XYZ/Animation/Animator.h"

namespace XYZ {
	namespace Editor {

		class AnimationEditor : public EditorPanel
		{
		public:
			AnimationEditor(std::string name);

			virtual void OnImGuiRender(bool& open) override;
			virtual void OnUpdate(Timestep ts) override;
			virtual void SetSceneContext(const Ref<Scene>& scene) override;

			void SetContext(const Ref<Animator>& context);
		private:
			void onEntitySelected();
			void propertySection();
			void timelineSection();
			
			void handleEditKeyEndFrames();
			void handleEditKeyValues();
			void handleAddKey();
			void keySelectionActions();
	
			template <typename ComponentType, typename T>
			void editKeyValue(Reflection<ComponentType> refl, const std::string& path, uint32_t frame, const T& val, const std::string& valName);

			template <typename T>
			bool editKeyValueSpecialized(uint32_t frame, T& value, const std::string& valName);

			template <uint16_t valIndex, typename ComponentType, typename T>
			void addReflectedProperty(Reflection<ComponentType> refl, const T& val, const std::string& path, const std::string& valName);

			template <typename ComponentType, typename T>
			void addKeyToProperty(Reflection<ComponentType> refl, const std::string& path, uint32_t frame, const T& val, const std::string& valName);

			template <typename ComponentType, typename T>
			Property<T>* getProperty(Reflection<ComponentType> refl, const T& val, const std::string& path, const std::string& valName);

		private:
			glm::vec2 m_ButtonSize;

			Ref<Animator>	   m_Context;
			Ref<Animation>	   m_Animation;
			Ref<Scene>		   m_Scene;
			SceneEntity		   m_AnimatorEntity;

			AnimationClassMap		 m_ClassMap;
			AnimationSequencer		 m_Sequencer;

			int			m_SelectedEntry;
			int			m_FirstFrame;
			int			m_CurrentFrame;
			bool		m_Expanded;
			bool		m_Playing;
			float		m_SplitterWidth;
			int			m_FrameMin = 0;
			int			m_FrameMax = 30;
		};

		template<typename ComponentType, typename T>
		inline void AnimationEditor::editKeyValue(Reflection<ComponentType> refl, const std::string& path, uint32_t frame, const T& val, const std::string& valName)
		{
			//auto prop = m_Context->GetProperty<ComponentType, T>(path, valName);
			//if (prop && !prop->Empty())
			//{
			//	T value = prop->GetValue(frame);
			//	if (editKeyValueSpecialized(frame, value, valName))
			//	{
			//		if (prop->HasKeyAtFrame(frame))
			//		{
			//			size_t key = prop->FindKey(frame);
			//			prop->SetKeyValue(value, key);
			//		}
			//		else
			//		{
			//			prop->AddKeyFrame({ value, frame });
			//			m_Sequencer.AddKey(m_SelectedEntry, frame);
			//		}
			//	}
			//}
		}

		template<uint16_t valIndex, typename ComponentType, typename T>
		inline void AnimationEditor::addReflectedProperty(Reflection<ComponentType> refl, const T& val, const std::string& path, const std::string& valName)
		{
			m_Animation->AddProperty<ComponentType, T, valIndex>(path, valName);
			const int itemTypeIndex = m_Sequencer.GetItemTypeIndex(refl.sc_ClassName);
			if (!m_Sequencer.ItemExists(itemTypeIndex))
				 m_Sequencer.AddItem(itemTypeIndex);

			m_Sequencer.AddLine(itemTypeIndex, valName);
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
