#pragma once
#include "XYZ/ECS/Entity.h"
#include "XYZ/ECS/Component.h"


namespace XYZ {

	class ByteStream
	{
	public:
		ByteStream();
		ByteStream(const ByteStream& other);
		ByteStream(ByteStream&& other) noexcept;
		~ByteStream();

		ByteStream& operator=(ByteStream&& other) noexcept;

		template <typename T>
		ByteStream& operator <<(T value)
		{
			if (std::is_trivially_copyable<T>::value)
			{
				size_t size = sizeof(T);
				handleMemorySize(size);
				memcpy(&m_Data[m_Size], &value, size);
				m_Size += size;
			}
			else
			{
				XYZ_CORE_WARN("Value must be trivially copyable, data was not deserialized");
			}
			return *this;
		}

		template <typename T>
		const ByteStream& operator >>(T& value) const
		{
			if (std::is_trivially_copyable<T>::value)
			{
				size_t size = sizeof(T);
				memcpy(&value, &m_Data[m_Iterator], size);
				m_Iterator += size;
			}
			else
			{
				XYZ_CORE_WARN("Value must be trivially copyable, data was not serialized");
			}
			return *this;
		}

		template <typename T>
		void Write(T* out, size_t offset, size_t size = sizeof(T))
		{
			memcpy(&m_Data[offset], out, size);
		}

		template <typename T>
		void Read(T* out, size_t offset, size_t size = sizeof(T)) const
		{
			memcpy(out, &m_Data[offset], size);
		}

		void SetIterator(size_t iterator) { m_Iterator = iterator; }

		operator uint8_t* ()
		{
			return m_Data;
		}
		operator uint8_t* () const
		{
			return m_Data;
		}
		size_t Size() const { return m_Size; }
		bool End() const { return m_Iterator == m_Size; }
	private:
		void handleMemorySize(size_t sizeReq);

	private:
		uint8_t* m_Data;
		size_t	 m_Size;
		size_t   m_Capacity;
		mutable size_t m_Iterator = 0;

		static constexpr size_t sc_CapacityMultiplier = 2;
	};

	inline ByteStream& operator << (ByteStream & out, Entity entity)
	{
		out << (uint32_t)entity;
		return out;
	}

	inline const ByteStream& operator >> (const ByteStream& out, Entity& entity)
	{
		out >> (uint32_t&)entity;
		return out;
	}

	inline ByteStream& operator <<(ByteStream& out, const std::vector<bool>& vec)
	{
		out << vec.size();
		for (auto val : vec)
			out << val;
		return out;
	}

	inline ByteStream& operator <<(ByteStream& out, const std::string& val)
	{
		out << val.size();
		for (char c : val)
			out << c;
		return out;
	}
	inline const ByteStream& operator >>(const ByteStream& out, std::string& val)
	{
		size_t size = 0;
		out >> size;
		for (size_t i = 0; i < size; ++i)
		{
			char c;
			out >> c;
			val.push_back(c);
		}
		return out;
	}
	template <typename T>
	inline ByteStream& operator <<(ByteStream& out, const std::vector<T>& vec)
	{
		out << vec.size();
		for (auto& val : vec)
			out << val;
		return out;
	}
	template <typename T>
	inline const ByteStream& operator >>(const ByteStream& out, std::vector<T>& vec)
	{
		size_t size;
		out >> size;
		vec.reserve(size);
		for (size_t i = 0; i < size; ++i)
		{
			T stored;
			out >> stored;
			vec.push_back(stored);
		}
		return out;
	}
}