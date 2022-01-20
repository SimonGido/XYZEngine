#pragma once

#include "XYZ/ECS/Entity.h"
#include "XYZ/Scene/Scene.h"

extern "C" {

	typedef struct _MonoObject MonoObject;
	typedef struct _MonoClassField MonoClassField;
	typedef struct _MonoDomain MonoDomain;
}



namespace XYZ {
	
	struct EntityScriptClass;

	struct ScriptEntityInstance
	{
		uint32_t				 Handle = 0;
		Scene*					 SceneInstance = nullptr;
		EntityScriptClass*	     ScriptClass = nullptr;
		std::vector<PublicField> Fields;

		MonoObject* GetInstance();
		bool		IsRuntimeAvailable() const;
	};

	class ScriptEngine
	{
	public:
		static void Init(const std::string& assemblyPath);
		static void Shutdown();

		static void LoadRuntimeAssembly(const std::string& path);
		static void ReloadAssembly(const std::string& path);

		static void SetSceneContext(const Ref<Scene>& scene);
		static const Ref<Scene>& GetCurrentSceneContext();

		static void OnCreateEntity(const SceneEntity& entity);
		static void OnDestroyEntity(const SceneEntity& entity);
		static void OnUpdateEntity(const SceneEntity& entity, Timestep ts);
		static MonoObject* Construct(const std::string& fullName, bool callConstructor = true, void** parameters = nullptr);

		static void CreateModule(const std::string& moduleName);
		static bool ModuleExists(const std::string& moduleName);

		static void CreateScriptEntityInstance(const SceneEntity& entity);
		static void DestroyScriptEntityInstance(const SceneEntity& entity);
		static void CopyPublicFieldsToRuntime(const SceneEntity& entity);

		
		static MonoDomain* GetMonoDomain();

		static const SparseArray<ScriptEntityInstance>& GetScriptEntityInstances();
		static const std::vector<std::string>&			GetEntityClasses();
	private:
		static void createModules();
	
	};

}