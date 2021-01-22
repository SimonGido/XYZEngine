#pragma once

#include "XYZ/ECS/Entity.h"
#include "XYZ/Scene/Scene.h"



extern "C"
{
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoClassField MonoClassField;
}


namespace XYZ {

	
	struct EntityScriptClass;
	struct EntityInstance
	{
		EntityScriptClass* ScriptClass = nullptr;
		Scene* SceneInstance = nullptr;

		uint32_t Handle = 0;

		_MonoObject* GetInstance();
	};

	//using EntityInstanceMap = std::unordered_map<GUID, std::unordered_map<GUID, uint32_t>>;

	class ScriptEngine
	{
	public:
		static void Init(const std::string& assemblyPath);
		static void Shutdown();

		static void LoadXYZRuntimeAssembly(const std::string& path);
		static void ReloadAssembly(const std::string& path);

		static void SetSceneContext(const Ref<Scene>& scene);
		static const Ref<Scene>& GetCurrentSceneContext();

		static void OnCreateEntity(SceneEntity entity);
		static void OnUpdateEntity(SceneEntity entity, Timestep ts);


		static void OnScriptComponentDestroyed(Entity entity);

		static bool ModuleExists(const std::string& moduleName);
		static void InitScriptEntity(SceneEntity entity);
		static void ShutdownScriptEntity(SceneEntity entity, const std::string& moduleName);
		static void InstantiateEntityClass(SceneEntity entity);

	};

}