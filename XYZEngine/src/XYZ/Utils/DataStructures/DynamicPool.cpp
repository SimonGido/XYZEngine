#include "stdafx.h"
#include "DynamicPool.h"


namespace XYZ {
	DynamicPool::DynamicPool(uint32_t capacity)
		:
		m_Size(0),
		m_Capacity(capacity)
	{
		m_Data = nullptr;
		if (m_Capacity)
			m_Data = new uint8_t[m_Capacity];
	}
	DynamicPool::DynamicPool(const DynamicPool& other)
		:
		m_Size(other.m_Size),
		m_Capacity(other.m_Capacity)
	{
		destroy();
		m_Handles = other.m_Handles;
		m_Data = new uint8_t[m_Capacity];
		for (const size_t handle : m_Handles)
		{
			Base* base = reinterpret_cast<Base*>(&other.m_Data[m_Capacity]);
			base->OnCopy(&m_Data[handle]);
		}
	}
	DynamicPool::DynamicPool(DynamicPool&& other) noexcept
		:
		m_Size(other.m_Size),
		m_Capacity(other.m_Capacity)
	{
		destroy();
		m_Handles = std::move(other.m_Handles);
		m_Data = other.m_Data;
		other.m_Data = nullptr;
		other.m_Size = 0;
		other.m_Capacity = 0;
	}
	DynamicPool::~DynamicPool()
	{
		destroy();
	}
	void DynamicPool::Erase(size_t index)
	{
		size_t handle = m_Handles[index];
		Base* base = reinterpret_cast<Base*>(&m_Data[handle]);
		base->~Base();
		if (index + 1 < m_Handles.size())
		{
			const size_t elementSize = m_Handles[index + 1] - handle;
			m_Size -= elementSize;
			for (size_t i = index + 1; i < m_Handles.size(); ++i)
			{
				const size_t nextHandle = m_Handles[i];
				m_Handles[i] -= elementSize;
				Base* next = reinterpret_cast<Base*>(&m_Data[nextHandle]);
				next->OnCopy(&m_Data[handle]);
				handle = nextHandle;
			}
		}	
		m_Handles.erase(m_Handles.begin() + index);
	}
	DynamicPool::Base& DynamicPool::Back()
	{
		return *reinterpret_cast<Base*>(&m_Data[m_Handles.back()]);
	}
	const DynamicPool::Base& DynamicPool::Back() const
	{
		return *reinterpret_cast<Base*>(&m_Data[m_Handles.back()]);
	}
	DynamicPool::Base& DynamicPool::operator[](size_t index)
	{
		return *reinterpret_cast<Base*>(&m_Data[m_Handles[index]]);
	}
	const DynamicPool::Base& DynamicPool::operator[](size_t index) const
	{
		return *reinterpret_cast<Base*>(&m_Data[m_Handles[index]]);
	}
	void DynamicPool::destroy()
	{
		if (m_Data)
		{
			for (const size_t handle : m_Handles)
			{
				Base* base = reinterpret_cast<Base*>(&m_Data[handle]);
				base->~Base();
			}
			delete[]m_Data;
		}
	}
	void DynamicPool::reallocate(size_t minSize)
	{
		m_Capacity = minSize + (m_Capacity * sc_CapacityMultiplier);
		uint8_t* tmp = new uint8_t[m_Capacity];
		if (m_Data)
		{
			for (const size_t handle : m_Handles)
			{
				Base* base = reinterpret_cast<Base*>(&m_Data[handle]);
				base->OnCopy(&tmp[handle]);
			}
			delete[] m_Data;
		}
		m_Data = tmp;
	}
}