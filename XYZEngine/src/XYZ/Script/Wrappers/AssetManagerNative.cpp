#include "stdafx.h"
#include "AssetManagerNative.h"

#include "XYZ/Asset/AssetManager.h"

#include "XYZ/Asset/Renderer/ShaderAsset.h"
#include "XYZ/Asset/Renderer/MaterialAsset.h"
#include "XYZ/Asset/Renderer/MeshSource.h"


#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#define REGISTER_ASSET_TYPE(Type) \
	{\
		MonoType* type = mono_reflection_type_from_name("XYZ." #Type, s_CoreAssemblyImage);\
	    XYZ_ASSERT(type != nullptr, "No C# asset class found for " #Type "!");\
		s_NativeGetAssetFuncs[type].Connect<&Utils::GetAsset<Type>>();\
	}

namespace XYZ {
	extern MonoImage* s_CoreAssemblyImage;

	namespace Script {
		namespace Utils {
			template <typename T>
			void* GetAsset(const std::string& strPath)
			{		
				Ref<T> asset = AssetManager::TryGetAsset<T>(strPath);
				if (asset.Raw())
					return new Ref<T>(asset);
				return nullptr;
			}
		}

		static std::unordered_map<MonoType*, Delegate<void*(const std::string& strPath)>> s_NativeGetAssetFuncs;

		static void* GetAsset(MonoString* path, MonoReflectionType* type)
		{
			std::string strPath(mono_string_to_utf8(path));
			MonoType* mType = mono_reflection_type_get_type(type);
			return s_NativeGetAssetFuncs[mType](strPath);
		}

		void AssetManagerNative::Register()
		{
			REGISTER_ASSET_TYPE(ShaderAsset);
			REGISTER_ASSET_TYPE(MaterialAsset);

			mono_add_internal_call("XYZ.AssetManager::GetAsset_Native", GetAsset);
		}
	}
}