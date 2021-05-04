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
		m_Capacity(other.m_Capacity)
	{
		Reset();
		if (m_Data)
			delete[]m_Data;

		m_Size = other.m_Size;
		m_Elements = std::move(other.m_Elements);
		m_ElementMap = std::move(other.m_ElementMap);
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
		if (m_Data)
			delete[]m_Data;
		m_Capacity = other.m_Capacity;
		m_Size = other.m_Size;
		m_Elements = std::move(other.m_Elements);
		m_ElementMap = std::move(other.m_ElementMap);
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
			m_ElementMap.clear();
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
		uint32_t counter = 0;
		for (Element& element : m_Elements)
		{
			moveElement(tmp, element.Type, element.Offset);
			bUIElement* movedElement = reinterpret_cast<bUIElement*>(&tmp[element.Offset]);
			m_Tree.SetData(movedElement->ID, movedElement);
			destroy(element.Offset);
		}
		if (m_Data)
			delete[]m_Data;
		m_Data = tmp;


		counter = 0;
		for (Element& element : m_Elements)
		{
			bUIElement* movedElement = GetElement<bUIElement>(counter++);
			movedElement->Parent = static_cast<bUIElement*>(m_Tree.GetParentData(movedElement->ID));
		}
	}

	void bUIAllocator::destroy(size_t offset)
	{
		bUIElement* tmp = reinterpret_cast<bUIElement*>(&m_Data[offset]);
		tmp->~bUIElement();
	}

	void bUIAllocator::moveElement(uint8_t* dest, bUIElementType type, size_t offset)
	{
		switch (type)
		{
		case XYZ::bUIElementType::None:
			XYZ_ASSERT(false, "Type is none");
			break;
		case XYZ::bUIElementType::Button:
			move<bUIButton>(dest, offset);
			break;
		case XYZ::bUIElementType::Checkbox:
			move<bUICheckbox>(dest, offset);
			break;
		case XYZ::bUIElementType::Slider:
			move<bUISlider>(dest,offset);
			break;
		case XYZ::bUIElementType::Window:
			move<bUIWindow>(dest, offset);
			break;
		case XYZ::bUIElementType::Scrollbox:
			move<bUIScrollbox>(dest, offset);
			break;
		case XYZ::bUIElementType::Float:
			move<bUIFloat>(dest, offset);
			break;
		case XYZ::bUIElementType::Int:
			move<bUIInt>(dest, offset);
			break;
		case XYZ::bUIElementType::String:
			move<bUIString>(dest, offset);
			break;
		case XYZ::bUIElementType::Tree:
			move<bUITree>(dest, offset);
			break;
		case XYZ::bUIElementType::Dropdown:
			move<bUIDropdown>(dest, offset);
			break;
		case XYZ::bUIElementType::Image:
			move<bUIImage>(dest, offset);
			break;
		case XYZ::bUIElementType::Text:
			move<bUIText>(dest, offset);
			break;	
		case XYZ::bUIElementType::Timeline:
			move<bUITimeline>(dest, offset);
			break;
		default:
			break;
		}
	}
	void bUIAllocator::storeElement(bUIElement* parent, bUIElement* element)
	{
		XYZ_ASSERT(m_ElementMap.find(element->Name) == m_ElementMap.end(), "Element with the name ", element->Name, " already exists");
		m_Elements.push_back({ element->Type, m_Size });
		m_ElementMap[element->Name] = m_Elements.size() - 1;
		element->Parent = parent;
		if (parent)
			static_cast<bUIElement*>(element)->ID = m_Tree.InsertAtEnd(element, parent->ID);
		else
			static_cast<bUIElement*>(element)->ID = m_Tree.InsertAtEnd(element);
	}
}