#include "stdafx.h"
#include "Haha.h"




namespace XYZ {
	void SetColor(const glm::vec4& color, uint32_t entity)
	{
		Scene* scene = PerModuleInterface::g_pSystemTable->CurrentScene;
		Entity ent({ entity, scene });
		ent.GetComponent<SpriteRenderer>()->Color = color;
	}
}