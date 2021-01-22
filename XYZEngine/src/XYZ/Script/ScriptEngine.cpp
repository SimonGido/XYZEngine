#include "stdafx.h"
#include "ScriptEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>

#include <iostream>
#include <chrono>
#include <thread>

#include <Windows.h>
#include <winioctl.h>

#include "XYZ/Scene/SceneEntity.h"
#include "ScriptEngineRegistry.h"

namespace XYZ {

	static MonoDomain* s_MonoDomain = nullptr;
	static std::string s_AssemblyPath;
	static Ref<Scene> s_SceneContext;

	// Assembly images
	MonoImage* s_AppAssemblyImage = nullptr;
	MonoImage* s_CoreAssemblyImage = nullptr;


	static MonoMethod* GetMethod(MonoImage* image, const std::string& methodDesc);

	struct EntityScriptClass
	{
		std::string FullName;
		std::string ClassName;
		std::string NamespaceName;

		MonoClass* Class = nullptr;
		MonoMethod* OnCreateMethod = nullptr;
		MonoMethod* OnDestroyMethod = nullptr;
		MonoMethod* OnUpdateMethod = nullptr;

		void InitClassMethods(MonoImage* image)
		{
			OnCreateMethod = GetMethod(image, FullName + ":OnCreate()");
			OnUpdateMethod = GetMethod(image, FullName + ":OnUpdate(single)");
		}
	};
	static std::unordered_map<std::string, EntityScriptClass*> s_EntityClassMap;

	static MonoObject* GetInstance(uint32_t handle)
	{
		XYZ_ASSERT(handle, "Entity has not been instantiated!");
		return mono_gchandle_get_target(handle);
	}

	MonoAssembly* LoadAssemblyFromFile(const char* filepath)
	{
		if (filepath == NULL)
		{
			return NULL;
		}

		HANDLE file = CreateFileA(filepath, FILE_READ_ACCESS, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file == INVALID_HANDLE_VALUE)
		{
			return NULL;
		}

		DWORD file_size = GetFileSize(file, NULL);
		if (file_size == INVALID_FILE_SIZE)
		{
			CloseHandle(file);
			return NULL;
		}

		void* file_data = malloc(file_size);
		if (file_data == NULL)
		{
			CloseHandle(file);
			return NULL;
		}

		DWORD read = 0;
		if (ReadFile(file, file_data, file_size, &read, NULL))
		{ }
		if (file_size != read)
		{
			free(file_data);
			CloseHandle(file);
			return NULL;
		}

		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(reinterpret_cast<char*>(file_data), file_size, 1, &status, 0);
		if (status != MONO_IMAGE_OK)
		{
			return NULL;
		}
		auto assemb = mono_assembly_load_from_full(image, filepath, &status, 0);
		free(file_data);
		CloseHandle(file);
		mono_image_close(image);
		return assemb;
	}



	static MonoAssembly* LoadAssembly(const std::string& path)
	{
		MonoAssembly* assembly = LoadAssemblyFromFile(path.c_str());

		if (!assembly)
			XYZ_LOG_ERR("Could not load assembly: ", path);
		else
			XYZ_LOG_INFO("Successfully loaded assembly: ", path);

		return assembly;
	}

	static void InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		auto domain = mono_jit_init("XYZEngine");

		char* name = (char*)"XYZRuntime";
		s_MonoDomain = mono_domain_create_appdomain(name, nullptr);
	}

	static void ShutdownMono()
	{
		mono_jit_cleanup(s_MonoDomain);
	}
	static MonoImage* GetAssemblyImage(MonoAssembly* assembly)
	{
		MonoImage* image = mono_assembly_get_image(assembly);
		if (!image)
			XYZ_LOG_ERR("mono_assembly_get_image failed");

		return image;
	}

	static MonoClass* GetClass(MonoImage* image, const EntityScriptClass& scriptClass)
	{
		MonoClass* monoClass = mono_class_from_name(image, scriptClass.NamespaceName.c_str(), scriptClass.ClassName.c_str());
		if (!monoClass)
			XYZ_LOG_ERR("mono_class_from_name failed");

		return monoClass;
	}

	static uint32_t Instantiate(EntityScriptClass& scriptClass)
	{
		MonoObject* instance = mono_object_new(s_MonoDomain, scriptClass.Class);
		if (!instance)
			XYZ_LOG_ERR("mono_object_new failed");

		mono_runtime_object_init(instance);
		uint32_t handle = mono_gchandle_new(instance, false);
		return handle;
	}

	static MonoMethod* GetMethod(MonoImage* image, const std::string& methodDesc)
	{
		MonoMethodDesc* desc = mono_method_desc_new(methodDesc.c_str(), NULL);
		if (!desc)
			XYZ_LOG_ERR("mono_method_desc_new failed");

		MonoMethod* method = mono_method_desc_search_in_image(desc, image);
		if (!method)
			XYZ_LOG_ERR("mono_method_desc_search_in_image failed");

		return method;
	}

	static MonoObject* CallMethod(MonoObject* object, MonoMethod* method, void** params = nullptr)
	{
		MonoObject* pException = NULL;
		MonoObject* result = mono_runtime_invoke(method, object, params, &pException);
		return result;
	}


	void ScriptEngine::Init(const std::string& assemblyPath)
	{
		s_AssemblyPath = assemblyPath;

		InitMono();
		LoadXYZRuntimeAssembly(s_AssemblyPath);
	}

	void ScriptEngine::Shutdown()
	{
		s_SceneContext = nullptr;
		for (auto it : s_EntityClassMap)
			delete it.second;
	}


	static MonoAssembly* s_AppAssembly = nullptr;
	static MonoAssembly* s_CoreAssembly = nullptr;

	void ScriptEngine::LoadXYZRuntimeAssembly(const std::string& path)
	{
		MonoDomain* domain = nullptr;
		bool cleanup = false;
		if (s_MonoDomain)
		{
			domain = mono_domain_create_appdomain("XYZ Runtime", nullptr);
			mono_domain_set(domain, false);

			cleanup = true;
		}

		s_CoreAssembly = LoadAssembly("Assets/Scripts/XYZScriptCore.dll");
		s_CoreAssemblyImage = GetAssemblyImage(s_CoreAssembly);

		auto appAssembly = LoadAssembly(path);
		auto appAssemblyImage = GetAssemblyImage(appAssembly);
		ScriptEngineRegistry::RegisterAll();

		if (cleanup)
		{
			mono_domain_unload(s_MonoDomain);
			s_MonoDomain = domain;
		}

		s_AppAssembly = appAssembly;
		s_AppAssemblyImage = appAssemblyImage;
	}
	void ScriptEngine::ReloadAssembly(const std::string& path)
	{
		LoadXYZRuntimeAssembly(path);
		
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		XYZ_ASSERT(scene, "No active scene!");

		ComponentStorage<ScriptComponent>* scriptStorage = scene->m_ECS.GetStorage<ScriptComponent>();
		for (size_t i = 0; i < scriptStorage->Size(); ++i)
		{
			InitScriptEntity(SceneEntity{ scriptStorage->GetEntityAtIndex(i), scene.Raw() });
		}
	}

	void ScriptEngine::SetSceneContext(const Ref<Scene>& scene)
	{
		s_SceneContext = scene;
	}
	const Ref<Scene>& ScriptEngine::GetCurrentSceneContext()
	{
		return s_SceneContext;
	}
	void ScriptEngine::OnCreateEntity(SceneEntity entity)
	{
		ScriptComponent& scriptComponent = entity.GetComponent<ScriptComponent>();
		if (scriptComponent.ScriptClass->OnCreateMethod)
			CallMethod(GetInstance(scriptComponent.Handle), scriptComponent.ScriptClass->OnCreateMethod);
	}
	void ScriptEngine::OnUpdateEntity(SceneEntity entity, Timestep ts)
	{
		auto& scriptComponent = entity.GetComponent<ScriptComponent>();
		if (scriptComponent.ScriptClass->OnUpdateMethod)
		{
			void* args[] = { &ts };
			CallMethod(GetInstance(scriptComponent.Handle), scriptComponent.ScriptClass->OnUpdateMethod, args);
		}
	}

	bool ScriptEngine::ModuleExists(const std::string& moduleName)
	{
		std::string NamespaceName, ClassName;
		if (moduleName.find('.') != std::string::npos)
		{
			NamespaceName = moduleName.substr(0, moduleName.find_last_of('.'));
			ClassName = moduleName.substr(moduleName.find_last_of('.') + 1);
		}
		else
		{
			ClassName = moduleName;
		}

		MonoClass* monoClass = mono_class_from_name(s_AppAssemblyImage, NamespaceName.c_str(), ClassName.c_str());
		return monoClass != nullptr;
	}

	void ScriptEngine::InitScriptEntity(SceneEntity entity)
	{
		Scene* scene = entity.m_Scene;
		GUID id = entity.GetComponent<IDComponent>().ID;
		auto& scriptComponent = entity.GetComponent<ScriptComponent>();
		if (scriptComponent.ModuleName.empty())
			return;

		if (!ModuleExists(scriptComponent.ModuleName))
		{
			XYZ_LOG_ERR("Entity references non-existent script module ", scriptComponent.ModuleName);
			return;
		}

		auto it = s_EntityClassMap.find(scriptComponent.ModuleName);
		if (it == s_EntityClassMap.end())
			s_EntityClassMap[scriptComponent.ModuleName] = new EntityScriptClass();
		
		scriptComponent.ScriptClass = s_EntityClassMap[scriptComponent.ModuleName];
		if (scriptComponent.ModuleName.find('.') != std::string::npos)
		{
			scriptComponent.ScriptClass->NamespaceName = scriptComponent.ModuleName.substr(0, scriptComponent.ModuleName.find_last_of('.'));
			scriptComponent.ScriptClass->ClassName = scriptComponent.ModuleName.substr(scriptComponent.ModuleName.find_last_of('.') + 1);
		}
		else
		{
			scriptComponent.ScriptClass->ClassName = scriptComponent.ModuleName;
		}
		scriptComponent.ScriptClass->FullName = scriptComponent.ModuleName;

		scriptComponent.ScriptClass->Class = GetClass(s_AppAssemblyImage, *scriptComponent.ScriptClass);
		scriptComponent.ScriptClass->InitClassMethods(s_AppAssemblyImage);

		//EntityInstanceData& entityInstanceData = s_EntityInstanceMap[scene->GetUUID()][id];
		//EntityInstance& entityInstance = entityInstanceData.Instance;
		//entityInstance.ScriptClass = &scriptClass;
		//ScriptModuleFieldMap& moduleFieldMap = entityInstanceData.ModuleFieldMap;
		//auto& fieldMap = moduleFieldMap[moduleName];
		//
		//// Save old fields
		//std::unordered_map<std::string, PublicField> oldFields;
		//oldFields.reserve(fieldMap.size());
		//for (auto& [fieldName, field] : fieldMap)
		//	oldFields.emplace(fieldName, std::move(field));
		//fieldMap.clear();

		// Retrieve public fields (TODO: cache these fields if the module is used more than once)
		//{
		//	MonoClassField* iter;
		//	void* ptr = 0;
		//	while ((iter = mono_class_get_fields(scriptClass.Class, &ptr)) != NULL)
		//	{
		//		const char* name = mono_field_get_name(iter);
		//		uint32_t flags = mono_field_get_flags(iter);
		//		if ((flags & MONO_FIELD_ATTR_PUBLIC) == 0)
		//			continue;
		//
		//		MonoType* fieldType = mono_field_get_type(iter);
		//		FieldType hazelFieldType = GetHazelFieldType(fieldType);
		//
		//		// TODO: Attributes
		//		MonoCustomAttrInfo* attr = mono_custom_attrs_from_field(scriptClass.Class, iter);
		//
		//		if (oldFields.find(name) != oldFields.end())
		//		{
		//			fieldMap.emplace(name, std::move(oldFields.at(name)));
		//		}
		//		else
		//		{
		//			PublicField field = { name, hazelFieldType };
		//			field.m_EntityInstance = &entityInstance;
		//			field.m_MonoClassField = iter;
		//			fieldMap.emplace(name, std::move(field));
		//		}
		//	}
		//}
	}

	void ScriptEngine::InstantiateEntityClass(SceneEntity entity)
	{
		Scene* scene = entity.m_Scene;
		GUID id = entity.GetComponent<IDComponent>().ID;
		ScriptComponent& scriptComponent = entity.GetComponent<ScriptComponent>();
		
		XYZ_ASSERT(scriptComponent.ScriptClass, "");
		scriptComponent.Handle = Instantiate(*scriptComponent.ScriptClass);

		MonoProperty* entityIDPropery = mono_class_get_property_from_name(scriptComponent.ScriptClass->Class, "ID");
		mono_property_get_get_method(entityIDPropery);
		MonoMethod* entityIDSetMethod = mono_property_get_set_method(entityIDPropery);
		void* param[] = { &id };
		CallMethod(GetInstance(scriptComponent.Handle), entityIDSetMethod, param);

		//// Set all public fields to appropriate values
		//ScriptModuleFieldMap& moduleFieldMap = entityInstanceData.ModuleFieldMap;
		//if (moduleFieldMap.find(moduleName) != moduleFieldMap.end())
		//{
		//	auto& publicFields = moduleFieldMap.at(moduleName);
		//	for (auto& [name, field] : publicFields)
		//		field.CopyStoredValueToRuntime();
		//}

		// Call OnCreate function (if exists)
		OnCreateEntity(entity);
	}
}