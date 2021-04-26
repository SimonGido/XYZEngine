#pragma once
#include "BasicUITypes.h"

#include "XYZ/Utils/DataStructures/ByteBuffer.h"


namespace XYZ {
	class bUIQueue
	{
	public:
		bUIQueue(uint32_t size);
		~bUIQueue();

		template <typename T>
		uint32_t Allocate(const T& element)
		{
			static_assert(std::is_base_of<bUIElement, T>::value, "");
			new(&m_Buffer[m_Next])T(element);
			uint32_t id = m_Next;
			m_IDs.push_back(id);
			m_Next += sizeof(T);
			return id;
		}

		bUIElement& GetElement(uint32_t index)
		{
			return *reinterpret_cast<bUIElement*>(&m_Buffer[m_IDs[index]]);
		}

		size_t Size() const
		{
			return m_IDs.size();
		}

	private:
		ByteBuffer m_Buffer;
		uint32_t m_Next;
		std::vector<uint32_t> m_IDs;
	};
}