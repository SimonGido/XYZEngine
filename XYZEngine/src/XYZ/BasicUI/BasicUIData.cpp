#include "stdafx.h"
#include "BasicUIData.h"

#include "BasicUI.h"
#include "BasicUILoader.h"

#include "XYZ/Core/Input.h"

namespace XYZ {
	bUIAllocator& bUIData::CreateAllocator(const std::string& name, const std::string& filepath, size_t size)
	{
		auto it = m_AllocatorMap.find(name);
		XYZ_ASSERT(it == m_AllocatorMap.end(), "Allocator with name ", name, " already exists");
		m_Allocators.emplace_back(size);
		m_AllocatorMap[name] = { m_Allocators.size() - 1, filepath };
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
	void bUIData::Reload()
	{
		for (auto& [name, data] : m_AllocatorMap)
		{
			if (bUILoader::Load(data.Filepath))
			{
				if (data.OnReload)
					data.OnReload(m_Allocators[data.Index]);
			}
		}
	}
	void bUIData::Reload(const std::string& name)
	{
		auto it = m_AllocatorMap.find(name);
		XYZ_ASSERT(it != m_AllocatorMap.end(), "Allocator with name ", name, " does not exist");
		if (bUILoader::Load(it->second.Filepath))
		{
			if (it->second.OnReload)
				it->second.OnReload(m_Allocators[it->second.Index]);
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
					buildMesh(renderer, allocator, childElement, 0);
				return false;
			});		
		}
	}
	void bUIData::SetOnReloadCallback(const std::string& name, const bUIAllocatorReloadCallback& callback)
	{
		m_AllocatorMap[name].OnReload = callback;
	}
	bUIAllocator& bUIData::GetAllocator(const std::string& name)
	{
		auto it = m_AllocatorMap.find(name);
		XYZ_ASSERT(it != m_AllocatorMap.end(), "Allocator with name ", name, " does not exist");
		return m_Allocators[it->second.Index];
	}
	const bUIAllocator& bUIData::GetAllocator(const std::string& name) const
	{
		auto it = m_AllocatorMap.find(name);
		XYZ_ASSERT(it != m_AllocatorMap.end(), "Allocator with name ", name, " does not exist");
		return m_Allocators[it->second.Index];
	}
	bool bUIData::Exist(const std::string& name) const
	{
		return (m_AllocatorMap.find(name) != m_AllocatorMap.end());
	}
	void bUIData::buildMesh(bUIRenderer& renderer, bUIAllocator& allocator, bUIElement* element, uint32_t scissorID)
	{
		element->PushQuads(renderer, scissorID);
		Tree& tree = allocator.m_Tree;
		if (element->ChildrenVisible && element->Visible)
		{
			tree.TraverseNodeChildren(element->ID, [&](void* parent, void* child)->bool {

				bUIElement* childElement = static_cast<bUIElement*>(child);
				buildMesh(renderer, allocator, childElement, scissorID);
				return false;
			});
		}
	}
}