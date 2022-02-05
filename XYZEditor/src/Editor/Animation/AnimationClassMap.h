#pragma once
#include "XYZ/Scene/SceneEntity.h"

#include "XYZ/Animation/Animator.h"

namespace XYZ {
	namespace Editor {
		class AnimationClassMap
		{
		public:
			struct ClassData
			{
				std::string				 ClassName;
				std::vector<std::string> VariableNames;
			};

			using iterator		 = std::unordered_map<std::string, std::vector<ClassData>>::iterator;
			using const_iterator = std::unordered_map<std::string, std::vector<ClassData>>::const_iterator;

			

			void BuildMap(SceneEntity& entity);
			void Erase(const_iterator iter);
			bool OnImGuiRender(std::string& selectedEntity, std::string& selectedClass, std::string& selectedVariable);
			
			template <typename F>
			void Execute(std::string_view selectedClass, std::string_view selectedVariable, F func);

			iterator	    begin()		   { return m_ClassData.begin(); }
			iterator	    end()          { return m_ClassData.end(); }
			const_iterator  cbegin() const { return m_ClassData.cbegin(); }
			const_iterator  cend()	 const { return m_ClassData.cend(); }

		private:
			template <typename T>
			void addToClassData(Reflection<T> refl, const SceneEntity& entity, const Ref<Animation>& animation);

		private:
			std::unordered_map<std::string, std::vector<ClassData>> m_ClassData;
		};

		template<typename F>
		inline void AnimationClassMap::Execute(std::string_view selectedClass, std::string_view selectedVariable, F func)
		{
			Reflect::For<ReflectedClasses::sc_NumClasses>([&](auto j) 
			{
				if (selectedClass == ReflectedClasses::sc_ClassNames[j.value])
				{
					auto reflClass = ReflectedClasses::Get<j.value>();
					Reflect::For<reflClass.sc_NumVariables>([&](auto i)
					{
						if (selectedVariable == reflClass.sc_VariableNames[i.value])
							func(j, i);
					});
				}
			});
		}

		template<typename T>
		inline void AnimationClassMap::addToClassData(Reflection<T> refl, const SceneEntity& entity, const Ref<Animation>& animation)
		{
			const std::string& entityName = entity.GetComponent<SceneTagComponent>().Name;
			std::vector<std::string> variables;
			if (entity.HasComponent<T>())
			{
				const char* className = refl.sc_ClassName;
				for (const auto variable : refl.sc_VariableNames)
				{
					if (!animation->HasProperty(className, variable, entityName))
						variables.push_back(std::string(variable));
				}
			}
			if (!variables.empty())
			{
				auto& data = m_ClassData[entityName];
				data.push_back({ refl.sc_ClassName, std::move(variables) });
			}
		}
	}
}