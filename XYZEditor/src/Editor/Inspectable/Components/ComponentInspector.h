#pragma once
#include "Editor/Inspectable/Components/ComponentInspectors.h"

namespace XYZ {
	namespace Editor {

		template <typename ComponentType, typename InspectorType>
		struct ComponentInspector
		{
			ComponentInspector()
			{
				static_assert(std::is_base_of_v<Inspectable, InspectorType>, "InspectorType must be of base Inspectable");
			}

			template <typename T>
			static constexpr bool IsComponentType()
			{
				return std::is_same_v<ComponentType, T>;
			}

			InspectorType Inspector;
		};
	}
}