#include "stdafx.h"
#include "BasicUIAllocator.h"

namespace XYZ {
	
	bUIAllocator::bUIAllocator(size_t capacity)
		:
		m_Size(0),
		m_Capacity(capacity)
	{
		m_Data = new uint8_t[capacity];
	}
	bUIAllocator::bUIAllocator(bUIAllocator&& other) noexcept
		:
		m_Capacity(other.m_Capacity),
		m_Size(other.m_Size)
	{
		Reset();
		m_Elements = std::move(other.m_Elements);
		m_Tree = std::move(other.m_Tree);
		m_Data = other.m_Data;
		other.m_Data = nullptr;
		other.m_Size = 0;
		other.m_Capacity = 0;
	}
	bUIAllocator::~bUIAllocator()
	{
		if (m_Data)
		{
			size_t offset = 0;
			for (Element& element : m_Elements)
				destroy(element.Offset);

			delete[]m_Data;
		}
	}

	bUIAllocator& bUIAllocator::operator=(bUIAllocator&& other) noexcept
	{
		Reset();
		m_Capacity = other.m_Capacity;
		m_Size = other.m_Size;
		m_Elements = std::move(other.m_Elements);
		m_Tree = std::move(other.m_Tree);
		m_Data = other.m_Data;

		other.m_Data = nullptr;
		other.m_Size = 0;
		other.m_Capacity = 0;
		return *this;
	}

	void bUIAllocator::Reset()
	{
		if (m_Data)
		{
			for (Element& element : m_Elements)
				destroy(element.Offset);

			m_Elements.clear();
			m_Tree.Clear();
			m_Size = 0;
		}	
	}

	void bUIAllocator::Reserve(size_t size)
	{
		Reset();
		m_Data = new uint8_t[size];
		m_Capacity = size;
	}
	
	void bUIAllocator::resize(size_t minimalSize)
	{
		m_Capacity *= sc_CapacityMultiplier;
		if (m_Capacity < minimalSize)
			m_Capacity = minimalSize * sc_CapacityMultiplier;

		uint8_t* tmp = new uint8_t[m_Capacity];
		for (Element element : m_Elements)
		{
			copy(element.Type, element.Offset, tmp);
			destroy(element.Offset);
		}
		if (m_Data)
			delete[]m_Data;
		m_Data = tmp;
	}

	void bUIAllocator::destroy(size_t offset)
	{
		bUIElement* tmp = reinterpret_cast<bUIElement*>(&m_Data[offset]);
		tmp->~bUIElement();
	}
	void bUIAllocator::copy(bUIElementType type, size_t offset, uint8_t* buffer)
	{
		switch (type)
		{
		case XYZ::bUIElementType::Button:
			new(&buffer[offset])bUIButton(*reinterpret_cast<bUIButton*>(&m_Data[offset]));
			break;
		case XYZ::bUIElementType::Checkbox:
			new(&buffer[offset])bUICheckbox(*reinterpret_cast<bUICheckbox*>(&m_Data[offset]));
			break;
		case XYZ::bUIElementType::Slider:
			new(&buffer[offset])bUISlider(*reinterpret_cast<bUISlider*>(&m_Data[offset]));
			break;
		case XYZ::bUIElementType::Group:
			new(&buffer[offset])bUIGroup(*reinterpret_cast<bUIGroup*>(&m_Data[offset]));
			break;
		case XYZ::bUIElementType::Scrollbox:
			new(&buffer[offset])bUIScrollbox(*reinterpret_cast<bUIScrollbox*>(&m_Data[offset]));
			break;
		default:
			break;
		}
	}
}