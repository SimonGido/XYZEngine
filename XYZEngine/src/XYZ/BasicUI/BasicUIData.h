#pragma once
#include "BasicUIAllocator.h"
#include "BasicUIRenderer.h"

#include "XYZ/Event/InputEvent.h"

namespace XYZ {

	using bUIAllocatorReloadCallback = std::function<void(bUIAllocator&)>;
	class bUIData
	{
	public:
		bUIAllocator& CreateAllocator(const std::string& name, const std::string& filepath, size_t size);

		void Update();
		void Reload();
		void Reload(const std::string& name);
		void BuildMesh(bUIRenderer& renderer);
		void SetOnReloadCallback(const std::string& name, const bUIAllocatorReloadCallback& callback);

		bUIAllocator& GetAllocator(const std::string& name);
		const bUIAllocator& GetAllocator(const std::string& name) const;

		bool Exist(const std::string& name) const;
	private:
		void buildMesh(bUIRenderer& renderer, bUIAllocator& allocator, bUIElement* element, uint32_t scissorID);

	private:
		std::vector<bUIAllocator> m_Allocators;
		struct AllocatorData
		{
			size_t Index;
			std::string Filepath;
			bUIAllocatorReloadCallback OnReload;
		};
		std::unordered_map<std::string, AllocatorData> m_AllocatorMap;

		friend class bUILoader;
		friend class bUIInput;
	};

}