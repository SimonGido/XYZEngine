#pragma once


namespace XYZ {

	class ByteStream
	{
	public:
		ByteStream();
		ByteStream(const ByteStream& other);
		ByteStream(ByteStream&& other) noexcept;
		~ByteStream();

		template <typename T>
		ByteStream& operator <<(T value)
		{
			static_assert(std::is_trivially_copyable<T>::value, "Value must be trivially copyable");
			size_t size = sizeof(T);
			handleMemorySize(size);
			memcpy(&m_Data[m_Size], &value, size);
			m_Size += size;
			return *this;
		}

		template <typename T>
		const ByteStream& operator >>(T& value) const
		{
			static_assert(std::is_trivially_copyable<T>::value, "Value must be trivially copyable");
			size_t size = sizeof(T);
			memcpy(&value, &m_Data[m_Iterator], size);
			m_Iterator += size;
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
	private:
		void handleMemorySize(size_t sizeReq);

	private:
		uint8_t* m_Data;
		size_t	 m_Size;
		size_t   m_Capacity;
		mutable size_t m_Iterator = 0;

		static constexpr size_t sc_CapacityMultiplier = 2;
	};
}