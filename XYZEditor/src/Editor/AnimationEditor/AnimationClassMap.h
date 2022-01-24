#pragma once
#include "XYZ/Scene/SceneEntity.h"


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

			

			void BuildMap(const SceneEntity& entity);
			void Erase(const_iterator iter);
			
			template <typename F>
			void Execute(F func);

			iterator	    begin()		   { return m_ClassData.begin(); }
			iterator	    end()          { return m_ClassData.end(); }
			const_iterator  cbegin() const { return m_ClassData.cbegin(); }
			const_iterator  cend()	 const { return m_ClassData.cend(); }

		private:
			template <typename T>
			void addToClassData(Reflection<T> refl, const SceneEntity& entity);

		private:
			std::unordered_map<std::string, std::vector<ClassData>> m_ClassData;
		};

		template<typename F>
		inline void AnimationClassMap::Execute(F func)
		{
			Reflect::For<ReflectedClasses::sc_NumClasses>([&](auto j) 
			{
				auto reflClass = ReflectedClasses::Get<j.value>();
				Reflect::For<reflClass.sc_NumVariables>([&](auto i) 
				{
					func(j, i);
				});
			});
		}

		template<typename T>
		inline void AnimationClassMap::addToClassData(Reflection<T> refl, const SceneEntity& entity)
		{
			const std::string& entityName = entity.GetComponent<SceneTagComponent>().Name;
			auto& data = m_ClassData[entityName];

			if (entity.HasComponent<T>())
			{
				data.push_back({ refl.sc_ClassName });
				auto& variables = data.back().VariableNames;

				const char* className = refl.sc_ClassName;
				for (const auto variable : refl.sc_VariableNames)
					variables.push_back(variable);
			}
		}
	}
}