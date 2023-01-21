
#include <XYZ.h>

#include "XYZ/Project/Project.h"

#include <cr.h>

#define EXPORT extern "C" __declspec(dllexport)

using namespace XYZ;



Ref<Scene> ActiveScene;

EXPORT void OnLoad()
{
	ActiveScene = AssetManager::GetAsset<Scene>("Assets/Scenes/Scene.xyz");
	ActiveScene->CreateEntity("Entity From Plugin");
}

EXPORT void OnClose()
{

}

EXPORT void OnUpdate(Timestep ts)
{
	
}

