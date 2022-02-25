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

#include "XYZ/Scene/Components.h"

#include "ScriptEngineRegistry.h"


namespace XYZ {

	static MonoDomain* s_MonoDomain = nullptr;
	static std::string s_AssemblyPath;
	static Ref<Scene>  s_SceneContext;

	static MonoMethod* s_ExceptionMethod = nullptr;
	static MonoClass*  s_EntityClass = nullptr;

	static std::unordered_map<std::string, MonoClass*> s_Classes;

	// Assembly images
	MonoImage* s_AppAssemblyImage = nullptr;
	MonoImage* s_CoreAssemblyImage = nullptr;


	static MonoMethod* GetMethod(MonoImage* image, const std::string& methodDesc);

	struct EntityScriptClass
	{
		std::string FullName;
		std::string ClassName;
		std::string NamespaceName;

		MonoClass*  Class = nullptr;
		MonoMethod* Constructor = nullptr;
		MonoMethod* OnCreateMethod = nullptr;
		MonoMethod* OnDestroyMethod = nullptr;
		MonoMethod* OnUpdateMethod = nullptr;

		void InitClassMethods(MonoImage* image)
		{
			Constructor = GetMethod(s_CoreAssemblyImage, "XYZ.Entity:.ctor(uint)");
			OnCreateMethod = GetMethod(image, FullName + ":OnCreate()");
			OnDestroyMethod = GetMethod(image, FullName + ":OnDestroy()");
			OnUpdateMethod = GetMethod(image, FullName + ":OnUpdate(single)");
		}
	};


	

	static MonoAssembly* LoadAssemblyFromFile(const char* filepath)
	{
		if (filepath == NULL)
		{
			return NULL;
		}

		const HANDLE file = CreateFileA(filepath, FILE_READ_ACCESS, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file == INVALID_HANDLE_VALUE)
		{
			return NULL;
		}

		const DWORD file_size = GetFileSize(file, NULL);
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
		const auto assemb = mono_assembly_load_from_full(image, filepath, &status, 0);
		free(file_data);
		CloseHandle(file);
		mono_image_close(image);
		return assemb;
	}

	static void DestroyInstance(uint32_t handle)
	{
		mono_gchandle_free(handle);
	}

	static MonoAssembly* LoadAssembly(const std::string& path)
	{
		MonoAssembly* assembly = LoadAssemblyFromFile(path.c_str());

		if (!assembly)
			XYZ_CORE_ERROR("Could not load assembly: {0}", path);
		else
			XYZ_CORE_INFO("Successfully loaded assembly: {0}", path);

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
			XYZ_CORE_ERROR("mono_assembly_get_image failed");

		return image;
	}

	static MonoClass* GetClass(MonoImage* image, const EntityScriptClass& scriptClass)
	{
		MonoClass* monoClass = mono_class_from_name(image, scriptClass.NamespaceName.c_str(), scriptClass.ClassName.c_str());
		if (!monoClass)
			XYZ_CORE_ERROR("mono_class_from_name failed");

		return monoClass;
	}

	static uint32_t Instantiate(EntityScriptClass& scriptClass)
	{
		MonoObject* instance = mono_object_new(s_MonoDomain, scriptClass.Class);
		if (!instance)
			XYZ_CORE_ERROR("mono_object_new failed");

		mono_runtime_object_init(instance);
		const uint32_t handle = mono_gchandle_new(instance, false);
		return handle;
	}

	static MonoMethod* GetMethod(MonoImage* image, const std::string& methodDesc)
	{
		MonoMethodDesc* desc = mono_method_desc_new(methodDesc.c_str(), NULL);
		if (!desc)
			XYZ_CORE_ERROR("mono_method_desc_new failed");

		MonoMethod* method = mono_method_desc_search_in_image(desc, image);
		if (!method)
			XYZ_CORE_ERROR("mono_method_desc_search_in_image failed");

		return method;
	}

	static std::string GetStringProperty(const char* propertyName, MonoClass* classType, MonoObject* object)
	{
		MonoProperty* property = mono_class_get_property_from_name(classType, propertyName);
		MonoMethod* getterMethod = mono_property_get_get_method(property);
		MonoString* string = (MonoString*)mono_runtime_invoke(getterMethod, object, NULL, NULL);
		return string != nullptr ? std::string(mono_string_to_utf8(string)) : "";
	}

	static MonoObject* CallMethod(MonoObject* object, MonoMethod* method, void** params = nullptr)
	{
		MonoObject* pException = NULL;
		MonoObject* result = mono_runtime_invoke(method, object, params, &pException);
		if (pException)
		{
			MonoClass* exceptionClass = mono_object_get_class(pException);
			MonoType* exceptionType = mono_class_get_type(exceptionClass);
			const char* typeName = mono_type_get_name(exceptionType);
			std::string message = GetStringProperty("Message", exceptionClass, pException);
			std::string stackTrace = GetStringProperty("StackTrace", exceptionClass, pException);

			// TODO: console logging
			XYZ_ERROR("{0}: {1}. Stack Trace: {2}", typeName, message, stackTrace);

			void* args[] = { pException };
			MonoObject* result = mono_runtime_invoke(s_ExceptionMethod, nullptr, args, nullptr);
		}
		return result;
	}

	static std::vector<MonoClass*> GetAssemblyClasses(MonoImage* image)
	{
		std::vector<MonoClass*> class_vec;

		const MonoTableInfo* table_info = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);

		int rows = mono_table_info_get_rows(table_info);

		/* For each row, get some of its values */
		for (int i = 0; i < rows; i++)
		{
			MonoClass* _class = nullptr;
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(table_info, i, cols, MONO_TYPEDEF_SIZE);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
			const char* name_space = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			_class = mono_class_from_name(image, name_space, name);
			class_vec.push_back(_class);
		}
		return class_vec;
	}

	static std::vector<std::string> GetModules(MonoImage* image)
	{
		std::vector<std::string> class_vec;
		const MonoTableInfo* table_info = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);

		int rows = mono_table_info_get_rows(table_info);

		/* For each row, get some of its values */
		for (int i = 0; i < rows; i++)
		{
			MonoClass* _class = nullptr;
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(table_info, i, cols, MONO_TYPEDEF_SIZE);
			const char* name_space = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
			
			std::string moduleName(name_space);
			moduleName += ".";
			moduleName += name;

			class_vec.push_back(moduleName);
		}
		return class_vec;
	}

	static PublicFieldType GetXYZFieldType(MonoType* monoType)
	{
		const int type = mono_type_get_type(monoType);
		switch (type)
		{
		case MONO_TYPE_R4: return PublicFieldType::Float;
		case MONO_TYPE_I4: return PublicFieldType::Int;
		case MONO_TYPE_U4: return PublicFieldType::UnsignedInt;
		case MONO_TYPE_STRING: return PublicFieldType::String;
		case MONO_TYPE_VALUETYPE:
		{
			const char* name = mono_type_get_name(monoType);
			if (strcmp(name, "XYZ.Vector2") == 0) return PublicFieldType::Vec2;
			if (strcmp(name, "XYZ.Vector3") == 0) return PublicFieldType::Vec3;
			if (strcmp(name, "XYZ.Vector4") == 0) return PublicFieldType::Vec4;
		}
		}
		return PublicFieldType::None;
	}
	

	static std::vector<std::string>							  s_EntityClasses;
	static std::unordered_map<std::string, EntityScriptClass> s_EntityClassMap;
	static SparseArray<ScriptEntityInstance>				  s_ScriptEntityInstances;

	static MonoAssembly* s_AppAssembly = nullptr;
	static MonoAssembly* s_CoreAssembly = nullptr;



	MonoObject* ScriptEntityInstance::GetInstance()
	{
		XYZ_ASSERT(Handle, "Entity has not been instantiated!");
		return mono_gchandle_get_target(Handle);
	}

	bool ScriptEntityInstance::IsRuntimeAvailable() const
	{
		return Handle != 0;
	}


	void ScriptEngine::Init(const std::string& assemblyPath)
	{
		s_AssemblyPath = assemblyPath;

		InitMono();
		LoadRuntimeAssembly(s_AssemblyPath);
	}

	void ScriptEngine::Shutdown()
	{
		for (auto& it : s_ScriptEntityInstances)
			DestroyInstance(it.Handle);


		s_ScriptEntityInstances.Clear();
		s_EntityClassMap.clear();

		s_SceneContext.Reset();
	}

	void ScriptEngine::LoadRuntimeAssembly(const std::string& path)
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

		MonoAssembly* appAssembly = nullptr;
		appAssembly = LoadAssembly(path);

		const auto appAssemblyImage = GetAssemblyImage(appAssembly);
		ScriptEngineRegistry::RegisterAll();

		if (cleanup)
		{
			mono_domain_unload(s_MonoDomain);
			s_MonoDomain = domain;
		}
		s_ExceptionMethod = GetMethod(s_CoreAssemblyImage, "XYZ.RuntimeException:OnException(object)");
		s_EntityClass = mono_class_from_name(s_CoreAssemblyImage, "XYZ", "Entity");

		s_AppAssembly = appAssembly;
		s_AppAssemblyImage = appAssemblyImage;
		createModules();
	}
	void ScriptEngine::ReloadAssembly(const std::string& path)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		XYZ_ASSERT(scene.Raw(), "No active scene!");
		auto scriptView = scene->m_Registry.view<ScriptComponent>();
		for (auto ent : scriptView)
		{
			DestroyScriptEntityInstance({ ent, scene.Raw() });
		}

		LoadRuntimeAssembly(path);
		for (auto ent : scriptView)
		{
			CreateScriptEntityInstance({ ent, scene.Raw() });
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
	void ScriptEngine::OnCreateEntity(const SceneEntity& entity)
	{
		ScriptEntityInstance& instance = s_ScriptEntityInstances.GetData(entity);
		if (instance.ScriptClass->OnCreateMethod)
			CallMethod(instance.GetInstance(), instance.ScriptClass->OnCreateMethod);
	}
	void ScriptEngine::OnDestroyEntity(const SceneEntity& entity)
	{
		ScriptEntityInstance& instance = s_ScriptEntityInstances.GetData(entity);
		if (instance.ScriptClass->OnDestroyMethod)
			CallMethod(instance.GetInstance(), instance.ScriptClass->OnDestroyMethod);
	}
	void ScriptEngine::OnUpdateEntity(const SceneEntity& entity, Timestep ts)
	{
		ScriptEntityInstance& instance = s_ScriptEntityInstances.GetData(entity);
		if (instance.ScriptClass->OnUpdateMethod)
		{
			void* args[] = { &ts };
			CallMethod(instance.GetInstance(), instance.ScriptClass->OnUpdateMethod, args);
		}
	}

	MonoObject* ScriptEngine::Construct(const std::string& fullName, bool callConstructor, void** parameters)
	{
		std::string namespaceName;
		std::string className;
		std::string parameterList;

		if (fullName.find(".") != std::string::npos)
		{
			namespaceName = fullName.substr(0, fullName.find_first_of('.'));
			className = fullName.substr(fullName.find_first_of('.') + 1, (fullName.find_first_of(':') - fullName.find_first_of('.')) - 1);
		}

		if (fullName.find(":") != std::string::npos)
		{
			parameterList = fullName.substr(fullName.find_first_of(':'));
		}

		MonoClass* clazz = mono_class_from_name(s_CoreAssemblyImage, namespaceName.c_str(), className.c_str());
		XYZ_ASSERT(clazz, "Could not find class!");
		MonoObject* obj = mono_object_new(mono_domain_get(), clazz);

		if (callConstructor)
		{
			MonoMethodDesc* desc = mono_method_desc_new(parameterList.c_str(), NULL);
			MonoMethod* constructor = mono_method_desc_search_in_class(desc, clazz);
			MonoObject* exception = nullptr;
			mono_runtime_invoke(constructor, obj, parameters, &exception);
			XYZ_ASSERT(exception == nullptr, "");
		}

		return obj;
	}
	MonoClass* ScriptEngine::GetCoreClass(const std::string& fullName)
	{
		if (s_Classes.find(fullName) != s_Classes.end())
			return s_Classes[fullName];

		std::string namespaceName = "";
		std::string className;

		if (fullName.find('.') != std::string::npos)
		{
			namespaceName = fullName.substr(0, fullName.find_last_of('.'));
			className = fullName.substr(fullName.find_last_of('.') + 1);
		}
		else
		{
			className = fullName;
		}

		MonoClass* monoClass = mono_class_from_name(s_CoreAssemblyImage, namespaceName.c_str(), className.c_str());
		if (!monoClass)
			XYZ_ERROR("mono_class_from_name failed");

		s_Classes[fullName] = monoClass;

		return monoClass;
	}
	void ScriptEngine::CreateModule(const std::string& moduleName)
	{
		EntityScriptClass& scriptClass = s_EntityClassMap[moduleName];

		if (moduleName.find('.') != std::string::npos)
		{
			scriptClass.NamespaceName = moduleName.substr(0, moduleName.find_last_of('.'));
			scriptClass.ClassName = moduleName.substr(moduleName.find_last_of('.') + 1);
		}
		else
		{
			scriptClass.ClassName = moduleName;
		}
		scriptClass.FullName = moduleName;
		scriptClass.Class = GetClass(s_AppAssemblyImage, scriptClass);
		scriptClass.InitClassMethods(s_AppAssemblyImage);
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
		if (monoClass == nullptr)
			return false;

		auto isEntitySubclass = mono_class_is_subclass_of(monoClass, s_EntityClass, 0);
		return isEntitySubclass;
	}

	void ScriptEngine::CreateScriptEntityInstance(const SceneEntity& entity)
	{
		const auto& scriptComponent = entity.GetComponent<ScriptComponent>();
		s_ScriptEntityInstances.Emplace(entity);
		ScriptEntityInstance& instance = s_ScriptEntityInstances.GetData(entity);
		instance.ScriptClass = &s_EntityClassMap[scriptComponent.ModuleName];
		instance.Handle = Instantiate(*instance.ScriptClass);
		
		uint32_t id = static_cast<uint32_t>(entity.m_ID);
		void* param[] = { &id };
		CallMethod(instance.GetInstance(), instance.ScriptClass->Constructor, param);

		instance.FieldsData.Clear();

		// Store public fields inside instance
		MonoClassField* iter;
		void* ptr = 0;
		while ((iter = mono_class_get_fields(instance.ScriptClass->Class, &ptr)) != NULL)
		{
			const char* name = mono_field_get_name(iter);
			uint32_t flags = mono_field_get_flags(iter);
			if ((flags & MONO_FIELD_ATTR_PUBLIC) == 0)
				continue;

			MonoType* fieldType = mono_field_get_type(iter);
			const PublicFieldType xyzFieldType = GetXYZFieldType(fieldType);
			if (xyzFieldType == PublicFieldType::None)
				continue;

			// TODO: Attributes
			MonoCustomAttrInfo* attr = mono_custom_attrs_from_field(instance.ScriptClass->Class, iter);
			instance.FieldsData.AddField(name, xyzFieldType, instance.Handle, iter);
		}
		instance.FieldsData.CreateBuffer();
	}

	void ScriptEngine::DestroyScriptEntityInstance(const SceneEntity& entity)
	{
		DestroyInstance(s_ScriptEntityInstances.GetData(entity).Handle);
		s_ScriptEntityInstances.Erase(entity);
	}

	void ScriptEngine::CopyPublicFieldsToRuntime(const SceneEntity& entity)
	{
		ScriptEntityInstance& instance = s_ScriptEntityInstances.GetData(entity);
		for (auto& field : instance.FieldsData.GetFields())
			field.CopyStoredValueToRuntime();
	}


	MonoDomain* ScriptEngine::GetMonoDomain()
	{
		return s_MonoDomain;
	}

	const std::vector<PublicField>& ScriptEngine::GetPublicFields(const SceneEntity& entity)
	{
		return s_ScriptEntityInstances.GetData(static_cast<size_t>(entity.ID())).FieldsData.GetFields();
	}

	const SparseArray<ScriptEntityInstance>& ScriptEngine::GetScriptEntityInstances()
	{
		return s_ScriptEntityInstances;
	}

	const std::vector<std::string>& ScriptEngine::GetEntityClasses()
	{
		return s_EntityClasses;
	}

	void ScriptEngine::createModules()
	{
		s_EntityClasses = GetModules(s_AppAssemblyImage);
		s_EntityClasses.erase(s_EntityClasses.begin()); //TODO: Weird module name at the beginning
		for (const auto& mod : s_EntityClasses)
		{
			if (ModuleExists(mod))
				CreateModule(mod);
		}
	}

}