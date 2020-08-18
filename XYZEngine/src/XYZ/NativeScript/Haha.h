#pragma once
#include "XYZ/ECS/Entity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Scene/SceneManager.h"
#include "XYZ/NativeScript/NativeScriptCore.h"

#include <RuntimeObjectSystem/ObjectInterfacePerModule.h>
#include <glm/glm.hpp>

namespace XYZ {

	void SetColor(const glm::vec4& color,uint32_t entity);

}