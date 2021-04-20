#include "stdafx.h"
#include "BasicUIData.h"

#include "BasicUI.h"

#include "XYZ/Core/Input.h"

namespace XYZ {
	bUIAllocator& bUIData::CreateAllocator(const std::string& name, size_t size)
	{
		auto it = m_AllocatorMap.find(name);
		XYZ_ASSERT(it == m_AllocatorMap.end(), "Allocator with name ", name, " already exists");
		m_Allocators.emplace_back(size);
		m_AllocatorMap[name] = m_Allocators.size() - 1;
		return m_Allocators.back();
	}
	void bUIData::Update()
	{
		for (bUIAllocator& allocator : m_Allocators)
		{
			for (size_t i = 0; i < allocator.Size(); ++i)
			{
				allocator.GetElement<bUIElement>(i)->OnUpdate();
			}
		}
	}
	void bUIData::BuildMesh(bUIRenderer& renderer)
	{
		for (bUIAllocator& allocator : m_Allocators)
		{	
			Tree& tree = allocator.m_Tree;
			tree.TraverseNodeSiblings(tree.GetRoot(), [&](void* parent, void* child)->bool {

				bUIElement* childElement = static_cast<bUIElement*>(child);
				if (childElement->Visible)
					buildMesh(renderer, allocator, childElement);
				return false;
			});		
		}
	}
	bUIAllocator& bUIData::GetAllocator(const std::string& name)
	{
		auto it = m_AllocatorMap.find(name);
		XYZ_ASSERT(it != m_AllocatorMap.end(), "Allocator with name ", name, " already exists");
		return m_Allocators[it->second];
	}
	const bUIAllocator& bUIData::GetAllocator(const std::string& name) const
	{
		auto it = m_AllocatorMap.find(name);
		XYZ_ASSERT(it != m_AllocatorMap.end(), "Allocator with name ", name, " already exists");
		return m_Allocators[it->second];
	}
	bool bUIData::Exist(const std::string& name) const
	{
		return (m_AllocatorMap.find(name) != m_AllocatorMap.end());
	}
	void bUIData::buildMesh(bUIRenderer& renderer, bUIAllocator& allocator, bUIElement* element)
	{
		element->PushQuads(renderer);
		Tree& tree = allocator.m_Tree;
		if (element->ChildrenVisible)
		{
			tree.TraverseNodeChildren(element->ID, [&](void* parent, void* child)->bool {

				bUIElement* childElement = static_cast<bUIElement*>(child);
				if (childElement->Visible)
					buildMesh(renderer, allocator, childElement);
				return false;
			});
		}
	}
}