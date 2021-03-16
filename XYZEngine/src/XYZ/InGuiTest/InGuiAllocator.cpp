#include "stdafx.h"
#include "InGuiAllocator.h"
#include "InGuiUIElements.h"

namespace XYZ {
	
	IGPool::IGPool(const std::initializer_list<IGHierarchy>& hierarchy, size_t ** handles)
		:
		m_Size(0)
	{
		m_Capacity = 0;
		resolveHandles(hierarchy, handles);
		allocateMemory();

		uint32_t counter = 0;
		for (auto& h : hierarchy)
		{
			int32_t parentID = m_Hierarchy.Insert(Get<void>(m_Handles[counter++]));
			m_ParentHierarchyIDs.push_back(parentID);
			for (auto it : h.Children)
			{
				m_Hierarchy.Insert(Get<void>(m_Handles[counter++]), parentID);
			}
		}
	}

	IGPool::~IGPool()
	{
		size_t offset = 0;
		for (auto it : m_Elements)
		{
			switch (it)
			{
			case XYZ::IGElementType::Window:
				destroy<IGWindow>(offset);
				offset += sizeof(IGWindow);
				break;
			case XYZ::IGElementType::Button:
				destroy<IGButton>(offset);
				offset += sizeof(IGButton);
				break;
			case XYZ::IGElementType::Checkbox:
				destroy<IGCheckbox>(offset);
				offset += sizeof(IGCheckbox);
				break;
			case XYZ::IGElementType::Slider:
				destroy<IGSlider>(offset);
				offset += sizeof(IGSlider);
				break;
			case XYZ::IGElementType::None:
				break;
			default:
				break;
			}
		}
	}

	void IGPool::resolveHandles(const std::initializer_list<IGHierarchy>& hierarchy, size_t**handles)
	{
		uint32_t counter = 0;
		for (auto& it : hierarchy)
			counter += it.Children.size() + 1;
		*handles = new size_t[counter];

		counter = 0;
		for (auto h : hierarchy)
		{
			(*handles)[counter++] = m_Capacity;
			m_Handles.push_back(m_Capacity);	
			m_Elements.push_back(h.Parent);
			switch (h.Parent)
			{
			case XYZ::IGElementType::Window:
				m_Capacity += sizeof(IGWindow);
				break;
			}
			for (auto& it : h.Children)
			{
				(*handles)[counter++] = m_Capacity;
				m_Handles.push_back(m_Capacity);
				m_Elements.push_back(it);
				switch (it)
				{
				case XYZ::IGElementType::Button:		
					m_Capacity += sizeof(IGButton);
					break;
				case XYZ::IGElementType::Checkbox:		
					m_Capacity += sizeof(IGCheckbox);
					break;
				case XYZ::IGElementType::Slider:
					m_Capacity += sizeof(IGSlider);
					break;
				case XYZ::IGElementType::None:
					break;
				default:
					break;
				}
			}
		}
	}

	void IGPool::allocateMemory()
	{
		m_Data = new uint8_t[m_Capacity];
		IGWindow* lastParent = nullptr;
		for (auto it : m_Elements)
		{				
			switch (it)
			{
			case XYZ::IGElementType::Window:
			{
				lastParent = Allocate<IGWindow>(glm::vec2(0.0f), glm::vec2(200.0f,400.0f), glm::vec4(1.0f)).first;
				break;
			}
			case XYZ::IGElementType::Button:
			{	
				auto [element, handle] = Allocate<IGButton>(glm::vec2(0.0f), glm::vec2(50.0f), glm::vec4(1.0f));
				element->Parent = lastParent;
				break;
			}
			case XYZ::IGElementType::Checkbox:
			{	
				auto [element, handle] = Allocate<IGCheckbox>(glm::vec2(0.0f), glm::vec2(50.0f), glm::vec4(1.0f));
				element->Parent = lastParent;
				break;
			}
			case XYZ::IGElementType::Slider:

				break;
			case XYZ::IGElementType::None:
				break;
			default:
				break;
			}
		}
	}

	std::pair<size_t, size_t> IGAllocator::CreatePool(const std::initializer_list<IGHierarchy>& hierarchy, size_t ** handles)
	{
		m_Pools.emplace_back(hierarchy, handles);
		return { m_Pools.size() - 1, m_Pools.back().Size() };
	}

}