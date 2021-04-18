#include "stdafx.h"
#include "BasicUIData.h"

namespace XYZ {
	bUIData::bUIData()
		:
		m_Size(0),
		m_Capacity(0),
		m_Data(nullptr)
	{
	}
	bUIData::bUIData(size_t capacity)
		:
		m_Size(0),
		m_Capacity(capacity)
	{
		m_Data = new uint8_t[capacity];
	}
	bUIData::~bUIData()
	{
		if (m_Data)
		{
			size_t offset = 0;
			for (Element& element : m_Elements)
				destroy(element.Offset);

			delete[]m_Data;
		}
	}

	void bUIData::Reset()
	{
		if (m_Data)
		{
			for (Element& element : m_Elements)
				destroy(element.Offset);

			m_Elements.clear();
			m_Size = 0;
		}	
	}

	void bUIData::Reserve(size_t size)
	{
		Reset();
		m_Data = new uint8_t[size];
		m_Capacity = size;
	}
	
	void bUIData::resize(size_t minimalSize)
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

	void bUIData::destroy(size_t offset)
	{
		bUIElement* tmp = reinterpret_cast<bUIElement*>(&m_Data[offset]);
		tmp->~bUIElement();
	}
	void bUIData::copy(bUIElementType type, size_t offset, uint8_t* buffer)
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
		default:
			break;
		}
	}
}