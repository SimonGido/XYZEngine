#pragma once
#include "BasicUIAllocator.h"
#include "BasicUIRenderer.h"

#include "XYZ/Event/InputEvent.h"

namespace XYZ {

	class bUIData
	{
	public:
		bUIAllocator& CreateAllocator(const std::string& name, const std::string& filepath, size_t size);

		void Reload();
		void Update();
		void BuildMesh(bUIRenderer& renderer);

		bUIAllocator& GetAllocator(const std::string& name);
		const bUIAllocator& GetAllocator(const std::string& name) const;

		bool Exist(const std::string& name) const;
	private:
		void buildMesh(bUIRenderer& renderer, bUIAllocator& allocator, bUIElement* element, uint32_t scissorID);

	private:
		std::vector<bUIAllocator> m_Allocators;
		std::unordered_map<std::string, size_t> m_AllocatorMap;
		std::vector<std::string> m_UIPaths;

		friend class bUILoader;
		friend class bUIInput;
	};

}