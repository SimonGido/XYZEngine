#include "stdafx.h"
#include "Haha.h"


#include "XYZ/ECS/Entity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Scene/SceneManager.h"

#include "XYZ/NativeScript/NativeScriptCore.h"

void XYZ::SetColor(uint32_t entity)
{
	Scene* scene = PerModuleInterface::g_pSystemTable->CurrentScene;
	scene->SetFilepath("Opica kufawas");
	Entity ent({ entity, scene });
	ent.GetComponent<SpriteRenderer>()->Color = { 0,1,0,1 };
}
