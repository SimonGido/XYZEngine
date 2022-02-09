#include "stdafx.h"
#include "AnimationClassMap.h"

#include "XYZ/ImGui/ImGui.h"

#include "XYZ/Scene/Components.h"


namespace XYZ {
	namespace Editor {
		namespace Helper {
			template <typename ...Args>
			static bool IntControl(const char* text, const char* id, int32_t& value, Args&& ...args)
			{
				ImGui::Text(text, std::forward<Args>(args)...);
				ImGui::SameLine();
				return ImGui::InputInt(id, &value);
			}
		}

		void AnimationClassMap::BuildMap(SceneEntity& entity, const Ref<Animation>& animation)
		{
			m_ClassData.clear();
			if (entity)
			{
				const std::vector<Entity> tree = entity.GetComponent<Relationship>().GetTree(*entity.GetECS());
				Reflect::For([&](auto j) {
					auto reflClass = ReflectedClasses::Get<j.value>();
					addToClassData(reflClass, entity, animation);
					for (const Entity node : tree)
					{
						addToClassData(reflClass, { node, entity.GetScene() }, animation);
					}

				}, std::make_index_sequence<ReflectedClasses::sc_NumClasses>());
			}
		}
		void AnimationClassMap::Erase(const_iterator iter)
		{
			m_ClassData.erase(iter);
		}
		bool AnimationClassMap::OnImGuiRender(std::string& selectedEntity, std::string& selectedClass, std::string& selectedVariable)
		{
			for (auto& [entityName, classDataVec] : m_ClassData)
			{
				if (ImGui::BeginMenu(entityName.c_str()))
				{
					for (auto classIt = classDataVec.begin(); classIt != classDataVec.end(); ++classIt)
					{
						if (ImGui::BeginMenu(classIt->ClassName.c_str()))
						{
							auto& variables = classIt->VariableNames;
							for (auto it = variables.begin(); it != variables.end(); ++it)
							{
								if (ImGui::MenuItem(it->c_str()))
								{
									selectedEntity = entityName;
									selectedClass = classIt->ClassName;
									selectedVariable = *it;
								
									
									variables.erase(it);
									if (variables.empty())
										classDataVec.erase(classIt);
									if (classDataVec.empty())
										m_ClassData.erase(entityName);

									ImGui::EndMenu();
									ImGui::EndMenu();
									return true;
								}
							}
							ImGui::EndMenu();
						}
					}
					ImGui::EndMenu();
				}
			}
			return false;
		}
	}
}