
#include <XYZ.h>

#include "XYZ/Project/Project.h"
#include "XYZ/Plugin/PluginInterface.h"

#define EXPORT extern "C" __declspec(dllexport)

using namespace XYZ;


class MyPlugin : public PluginInterface
{
public:
	virtual void OnCreate() override
	{
		m_ActiveScene = AssetManager::GetAsset<Scene>("Assets/Scenes/Scene.scene");
		m_ActiveScene->CreateEntity("Entity From Plugin");
	}

	virtual void OnDestroy() override
	{

	}

	virtual void OnUpdate(Timestep ts) override
	{

	}

private:
	Ref<Scene> m_ActiveScene;
};


EXPORT PluginInterface* CreatePlugin()
{
	PluginInterface* plugin = new MyPlugin();
	return plugin;
}

EXPORT void DestroyPlugin(PluginInterface* plugin)
{
	delete plugin;
}