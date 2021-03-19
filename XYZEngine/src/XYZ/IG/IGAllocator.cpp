#include "stdafx.h"
#include "IGAllocator.h"
#include "IGUIElements.h"

namespace XYZ {
	
	IGPool::IGPool(const std::initializer_list<IGHierarchyElement>& hierarchy, size_t ** handles)
		:
		m_Size(0)
	{
		m_Capacity = 0;
		*handles = new size_t[getSize(hierarchy)];

		size_t counter = 0;
		resolveHandles(hierarchy, handles, counter);

		m_Data = new uint8_t[m_Capacity];
		allocateMemory(hierarchy, nullptr);

		counter = 0;
		insertToHierarchy(-1, hierarchy, counter, 0);
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
			case XYZ::IGElementType::Group:
				destroy<IGGroup>(offset);
				offset += sizeof(IGGroup);
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
			case XYZ::IGElementType::Text:
				destroy<IGText>(offset);
				offset += sizeof(IGText);
				break;
			case XYZ::IGElementType::Float:
				destroy<IGFloat>(offset);
				offset += sizeof(IGFloat);
				break;
			case XYZ::IGElementType::Tree:
				destroy<IGTree>(offset);
				offset += sizeof(IGTree);
				break;
			case XYZ::IGElementType::None:
				break;
			default:
				break;
			}
		}
	}

	void IGPool::resolveHandles(const std::vector<IGHierarchyElement>& hierarchy, size_t** handles, size_t& counter)
	{	
		for (auto h : hierarchy)
		{
			(*handles)[counter] = m_Capacity;
			counter++;
			m_Handles.push_back(m_Capacity);	
			m_Elements.push_back(h.Element);
			switch (h.Element)
			{
			case XYZ::IGElementType::Window:
				m_Capacity += sizeof(IGWindow);
				break;
			case XYZ::IGElementType::Group:
				m_Capacity += sizeof(IGGroup);
				break;
			case XYZ::IGElementType::Button:		
				m_Capacity += sizeof(IGButton);
				break;
			case XYZ::IGElementType::Checkbox:		
				m_Capacity += sizeof(IGCheckbox);
				break;
			case XYZ::IGElementType::Slider:
				m_Capacity += sizeof(IGSlider);
				break;
			case XYZ::IGElementType::Text:
				m_Capacity += sizeof(IGText);
				break;
			case XYZ::IGElementType::Float:
				m_Capacity += sizeof(IGFloat);
				break;
			case XYZ::IGElementType::Tree:
				m_Capacity += sizeof(IGTree);
				break;
			case XYZ::IGElementType::None:
				break;
			default:
				break;
			}
			resolveHandles(h.Children, handles, counter);
		}
	}

	void IGPool::allocateMemory(const std::vector<IGHierarchyElement>& hierarchy, IGElement* parent)
	{	
		for (auto& it : hierarchy)
		{
			switch (it.Element)
			{
			case XYZ::IGElementType::Window:
			{
				auto [element, handle] = Allocate<IGWindow>(glm::vec2(0.0f), glm::vec2(400.0f,500.0f), glm::vec4(1.0f));
				element->Parent = parent;
				allocateMemory(it.Children, element);
				break;
			}
			case XYZ::IGElementType::Group:
			{
				auto [element, handle] = Allocate<IGGroup>(glm::vec2(0.0f), glm::vec2(350.0f,450.0f), glm::vec4(1.0f));
				element->Parent = parent;
				allocateMemory(it.Children, element);
				break;
			}
			case XYZ::IGElementType::Button:
			{	
				auto [element, handle] = Allocate<IGButton>(glm::vec2(0.0f), glm::vec2(50.0f), glm::vec4(1.0f));
				element->Parent = parent;
				allocateMemory(it.Children, element);
				break;
			}
			case XYZ::IGElementType::Checkbox:
			{	
				auto [element, handle] = Allocate<IGCheckbox>(glm::vec2(0.0f), glm::vec2(50.0f), glm::vec4(1.0f));
				element->Parent = parent;
				allocateMemory(it.Children, element);
				break;
			}
			case XYZ::IGElementType::Slider:
			{
				auto [element, handle] = Allocate<IGSlider>(glm::vec2(0.0f), glm::vec2(150.0f, 25.0f) , glm::vec4(1.0f));
				element->Parent = parent;
				allocateMemory(it.Children, element);
				break;
			}
			case XYZ::IGElementType::Text:
			{
				auto [element, handle] = Allocate<IGText>(glm::vec2(0.0f), glm::vec2(50.0f, 35.0f), glm::vec4(1.0f));
				element->Parent = parent;
				allocateMemory(it.Children, element);
				break;
			}
			case XYZ::IGElementType::Float:
			{
				auto [element, handle] = Allocate<IGFloat>(glm::vec2(0.0f), glm::vec2(50.0f, 35.0f), glm::vec4(1.0f));
				element->Parent = parent;
				allocateMemory(it.Children, element);
				break;
			}
			case XYZ::IGElementType::Tree:
			{
				auto [element, handle] = Allocate<IGTree>(glm::vec2(0.0f), glm::vec2(25.0f), glm::vec4(1.0f));
				element->Parent = parent;
				allocateMemory(it.Children, element);
				break;
			}
			case XYZ::IGElementType::None:
				break;
			default:
				break;
			}
		}
	}

	void IGPool::insertToHierarchy(int32_t parentID, const std::vector<IGHierarchyElement>& hierarchy, size_t& counter, uint32_t iteration)
	{
		bool firstIteration = !iteration;
		for (auto& it : hierarchy)
		{
			int32_t newParentID = -1;
			if (parentID != -1)
			{
				newParentID = m_Hierarchy.Insert(Get<void>(m_Handles[counter]), parentID);
				Get<IGElement>(m_Handles[counter])->ID = newParentID;
				counter++;
			}
			else
			{
				newParentID = m_Hierarchy.Insert(Get<void>(m_Handles[counter]));
				Get<IGElement>(m_Handles[counter])->ID = newParentID;
				counter++;
			}
			if (firstIteration)
				m_RootElements.push_back(newParentID);
			insertToHierarchy(newParentID, it.Children, counter, iteration + 1);
		}
		if (parentID != -1)
			m_Hierarchy.ReverseNodeChildren(parentID);
	}

	size_t IGPool::getSize(const std::vector<IGHierarchyElement>& hierarchy)
	{
		size_t size = hierarchy.size();
		for (auto& it : hierarchy)
			size += getSize(it.Children);
		return size;
	}

	std::pair<size_t, size_t> IGAllocator::CreatePool(const std::initializer_list<IGHierarchyElement>& hierarchy, size_t ** handles)
	{
		m_Pools.emplace_back(hierarchy, handles);
		return { m_Pools.size() - 1, m_Pools.back().Size() };
	}

}