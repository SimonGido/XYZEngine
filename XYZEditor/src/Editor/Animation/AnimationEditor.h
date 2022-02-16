#pragma once
#include "Editor/EditorPanel.h"
#include "AnimationClassMap.h"
#include "EntityPropertyMap.h"

#include "XYZ/Animation/Animation.h"
#include "XYZ/Animation/AnimatorController.h"

#include <imgui.h>

namespace XYZ {
	namespace Editor {

		class AnimationEditor : public EditorPanel
		{
		public:
			AnimationEditor(std::string name);

			virtual void OnImGuiRender(bool& open) override;
			virtual void OnUpdate(Timestep ts) override;
			virtual void SetSceneContext(const Ref<Scene>& scene) override;

			void SetContext(const Ref<Animation>& context);
			
		private:

			void setEntity(const SceneEntity& entity);
			void drawEntityTree(const SceneEntity& entity);
			void drawEntityNode(const SceneEntity& entity);
			void drawEntityTreeProperties(const SceneEntity& entity);



			void onEntityChanged(ECSManager& ecs, Entity entity);
			void propertySection();
			void timelineSection();
			
			void handleEditKeyValues();
			void handleAddKey(std::string_view path, std::string_view componentName, std::string_view valueName);
			void handleRemoveKeys(std::string_view path, std::string_view componentName, std::string_view valueName, 
				std::vector<ImNeoKeyFrame>& keyFrames, const ImVector<uint32_t>& selection
			);
			
			bool checkFramesValid(std::string_view path, std::string_view componentName, std::string_view valueName, const std::vector<ImNeoKeyFrame>& neoKeyFrames);

			template <typename T>
			bool editKeyValueSpecialized(uint32_t frame, T& value, const std::string& valName);

			template <uint16_t valIndex, typename ComponentType, typename T>
			void addReflectedProperty(Reflection<ComponentType> refl, const T& val, const std::string& path, const std::string& valName);

			template <typename T>
			void addKeyToProperty(std::string_view path, std::string_view componentName, std::string_view valName, uint32_t frame, const T& val);

			template <typename T>
			Property<T>* getProperty(const T& val, std::string_view path, std::string_view componentName, std::string_view valName);


			template <typename Func, typename ...Args>
			void execFor(std::string_view path, std::string_view componentName, std::string_view valName, Func func, Args&& ...args);

			template <typename Func, typename ...Args>
			void execFor(std::string_view path, std::string_view componentName, std::string_view valName, Func func, Args&& ...args) const;


			SceneEntity findEntity(std::string_view path) const;
		private:
			glm::vec2		   m_ButtonSize;
			SceneEntity		   m_Entity;
			Ref<Animation>	   m_Animation;
			Ref<Scene>		   m_Scene;

			AnimationPlayer		 m_Player;
			AnimationClassMap    m_ClassMap;
			EntityPropertyMap    m_EntityPropertyMap;

			bool		m_Playing;
			float		m_SplitterWidth;

			uint32_t	m_CurrentFrame = 0;
			uint32_t	m_FrameMin = 0;
			uint32_t	m_FrameMax = 30;
			uint32_t	m_OffsetFrame = 0;
			float		m_Zoom = 1.0f;

		};



		template<uint16_t valIndex, typename ComponentType, typename T>
		inline void AnimationEditor::addReflectedProperty(Reflection<ComponentType> refl, const T& val, const std::string& path, const std::string& valName)
		{
			m_Animation->AddProperty<ComponentType, T, valIndex>(path);
		}
		template<typename T>
		inline void AnimationEditor::addKeyToProperty(std::string_view path, std::string_view componentName, std::string_view valName, uint32_t frame, const T& val)
		{
			auto prop = m_Animation->GetProperty<T>(path, componentName, valName);
			if (prop)
			{
				prop->AddKeyFrame({ val, frame });
			}
		}

		template<typename T>
		inline Property<T>* AnimationEditor::getProperty(const T& val, std::string_view path, std::string_view componentName, std::string_view valName)
		{
			return m_Animation->GetProperty<T>(path, componentName, valName);
		}

		template<typename Func, typename ...Args>
		inline void AnimationEditor::execFor(std::string_view path, std::string_view componentName, std::string_view valName, Func func, Args && ...args)
		{
			Utils::For([&](auto j) {
				if (ReflectedComponents::sc_ClassNames[j.value] == componentName)
				{
					auto reflClass = ReflectedComponents::Get<j.value>();
					Utils::For([&](auto i) {
						if (reflClass.sc_VariableNames[i.value] == valName)
						{
							func(j, i, std::forward<Args>(args)...);
						}
					}, std::make_index_sequence<reflClass.sc_NumVariables>());
				}
			}, std::make_index_sequence<ReflectedComponents::sc_NumClasses>());
		}

		template<typename Func, typename ...Args>
		inline void AnimationEditor::execFor(std::string_view path, std::string_view componentName, std::string_view valName, Func func, Args && ...args) const
		{
			Utils::For([&](auto j) {
				if (ReflectedComponents::sc_ClassNames[j.value] == componentName)
				{
					auto reflClass = ReflectedComponents::Get<j.value>();
					Utils::For([&](auto i) {
						if (reflClass.sc_VariableNames[i.value] == valName)
						{
							func(j, i, std::forward<Args>(args)... );
						}
					}, std::make_index_sequence<reflClass.sc_NumVariables>());
				}
			}, std::make_index_sequence<ReflectedComponents::sc_NumClasses>());
		}
	}
}
