#include "stdafx.h"
#include "InGuiAllocator.h"

namespace XYZ {
	
	IGPool::IGPool(const std::initializer_list<IGElementType>& types, size_t ** handles)
		:
		m_Size(0),
		m_Elements(types)
	{
		m_Capacity = 0;
		*handles = new size_t[types.size()];

		uint32_t counter = 0;
		for (auto it : types)
		{
			switch (it)
			{
			case XYZ::IGElementType::Window:
				*handles[counter++] = m_Capacity;
				m_Handles.push_back(m_Capacity);
				m_Capacity += sizeof(IGWindow);
				break;
			case XYZ::IGElementType::ImageWindow:
				*handles[counter++] = m_Capacity;
				m_Handles.push_back(m_Capacity);
				m_Capacity += sizeof(IGImageWindow);
				break;
			case XYZ::IGElementType::Button:
				*handles[counter++] = m_Capacity;
				m_Handles.push_back(m_Capacity);
				m_Capacity += sizeof(IGButton);
				break;
			case XYZ::IGElementType::Checkbox:
				*handles[counter++] = m_Capacity;
				m_Handles.push_back(m_Capacity);
				m_Capacity += sizeof(IGCheckbox);
				break;
			case XYZ::IGElementType::Slider:
				*handles[counter++] = m_Capacity;
				m_Handles.push_back(m_Capacity);
				m_Capacity += sizeof(IGSlider);
				break;
			case XYZ::IGElementType::None:
				break;
			default:
				break;
			}
		}
		m_Data = new uint8_t[m_Capacity];
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
			case XYZ::IGElementType::ImageWindow:
				destroy<IGImageWindow>(offset);
				offset += sizeof(IGImageWindow);
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

	size_t IGAllocator::CreatePool(const std::initializer_list<IGElementType>& types, size_t ** handles)
	{
		m_Pools.emplace_back(types, handles);
		return m_Pools.size() - 1;
	}

}