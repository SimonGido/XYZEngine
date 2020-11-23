#pragma once
#include "XYZ/Core/Ref.h"
#include "Serializable.h"
#include "Serializer.h"


namespace XYZ {
	
	class AssetManager
	{
	public:
		template <typename T>
		Ref<T> GetAsset(const std::string& filepath)
		{
			auto it = m_Assets.find(filepath);
			if (it != m_Assets.end())
				return Ref<T>((T*)it->second);

			Ref<T> ref = Serializer::DeserializeResource<T>(filepath);
			m_Assets[filepath] = ref.Raw();
			return ref;
		}

	private:
		std::unordered_map<std::string, RefCount*> m_Assets;
	};
}