#include "stdafx.h"	
#include "EntityPropertyMap.h"



namespace XYZ {
	namespace Editor {
		void EntityPropertyMap::BuildMap(Ref<Animation>& anim)
		{
			m_PropertyData.clear();
			
			processPropertyType(anim->GetProperties<glm::vec4>());
			processPropertyType(anim->GetProperties<glm::vec3>());
			processPropertyType(anim->GetProperties<glm::vec2>());
			processPropertyType(anim->GetProperties<float>());
			processPropertyType(anim->GetProperties<void*>());
		}
	}
}