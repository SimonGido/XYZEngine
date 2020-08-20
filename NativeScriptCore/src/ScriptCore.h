#pragma once
#include <XYZ.h>
#include "XYZ/NativeScript/Haha.h"

namespace XYZ {
	namespace Script {
		void NativeSetColor(const glm::vec4& color, uint32_t entity) {
			PerModuleInterface::g_pSystemTable->SetColor(color, entity);
		}
	}
}