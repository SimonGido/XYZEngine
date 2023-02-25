#pragma once

#include "XYZ/Scene/SceneEntity.h"
#include "ScriptPublicField.h"


#include "XYZ/Utils/DataStructures/SparseArray.h"

extern "C" {

	typedef struct _MonoObject MonoObject;
	typedef struct _MonoClassField MonoClassField;
	typedef struct _MonoDomain MonoDomain;
	typedef struct _MonoClass MonoClass;
}



namespace XYZ {
	
	struct EntityScriptClass;

	struct XYZ_API ScriptEntityInstance
	{
		MonoObject* GetInstance();
		bool		IsRuntimeAvailable() const;

		uint32_t				 Handle = 0;
		Scene*					 SceneInstance = nullptr;
		EntityScriptClass*		 ScriptClass = nullptr;
		PublicFieldData			 FieldsData;
	};

	class XYZ_API ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void LoadRuntimeAssembly(const std::string& path);
	
		static void SetSceneContext(const Ref<Scene>& scene);
		static const Ref<Scene>& GetCurrentSceneContext();

		static void SetLogger(const std::shared_ptr<spdlog::logger>& logger);
		static std::shared_ptr<spdlog::logger> GetLogger();

		static void OnCreateEntity(const SceneEntity& entity);
		static void OnDestroyEntity(const SceneEntity& entity);
		static void OnUpdateEntity(const SceneEntity& entity, Timestep ts);
		static MonoObject* Construct(const std::string& fullName, bool callConstructor = true, void** parameters = nullptr);
		static MonoClass* GetCoreClass(const std::string& fullName);

		static void CreateModule(const std::string& moduleName);
		static bool ModuleExists(const std::string& moduleName);

		static void CreateScriptEntityInstance(const SceneEntity& entity);
		static void DestroyScriptEntityInstance(const SceneEntity& entity);
		static void CopyPublicFieldsToRuntime(const SceneEntity& entity);

		
		static MonoDomain* GetMonoDomain();

		static const std::vector<PublicField>&			GetPublicFields(const SceneEntity& entity);
		static const SparseArray<ScriptEntityInstance>& GetScriptEntityInstances();
		static const std::vector<std::string>&			GetEntityClasses();
	private:
		static void createEntityModules();
		static void shutdownRuntimeAssembly();
	};

}