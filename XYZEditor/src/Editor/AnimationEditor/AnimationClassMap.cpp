#include "stdafx.h"
#include "AnimationClassMap.h"

#include "XYZ/ImGui/ImGui.h"

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

		void AnimationClassMap::BuildMap(const SceneEntity& entity)
		{
			m_ClassData.clear();
			if (entity)
			{
				const std::vector<Entity> tree = entity.GetComponent<Relationship>().GetTree(*entity.GetECS());
				Reflect::For([&](auto j) {
					auto reflClass = ReflectedClasses::Get<j.value>();
					Helper::AddToClassMap(reflClass, entity, m_Animation, m_ClassMap);
					for (const Entity node : tree)
					{
						addToClassData(reflClass, { node, entity.GetScene() });
					}

				}, std::make_index_sequence<ReflectedClasses::sc_NumClasses>());
			}
		}
		void AnimationClassMap::Erase(const_iterator iter)
		{
			m_ClassData.erase(iter);
		}
	}
}