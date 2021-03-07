#pragma once

#include <array>


namespace XYZ {

	class GUID
	{
	public:
		GUID();
		GUID(const std::string& str);
		GUID(const GUID& other);

		GUID& operator=(const std::string& str);

		bool operator==(const GUID& other) const
		{
			return m_Data1 == other.m_Data1
				&& m_Data2 == other.m_Data2
				&& m_Data3 == other.m_Data3
				&& m_Data4[0] == other.m_Data4[0]
				&& m_Data4[1] == other.m_Data4[1]
				&& m_Data4[2] == other.m_Data4[2]
				&& m_Data4[3] == other.m_Data4[3]
				&& m_Data4[4] == other.m_Data4[4]
				&& m_Data4[5] == other.m_Data4[5]
				&& m_Data4[6] == other.m_Data4[6]
				&& m_Data4[7] == other.m_Data4[7];
		}

		operator std::string() const;
		operator std::string();

		inline size_t Hash() const
		{
			size_t seed = 0;
			std::hash<size_t> hasher;
			seed ^= hasher(m_Data1) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= hasher(m_Data2) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= hasher(m_Data3) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			for (uint32_t i = 0; i < 8; ++i)
				seed ^= hasher(m_Data4[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

			return hasher(seed);
		}

	private:
		uint64_t m_Data1;
		uint16_t m_Data2;
		uint16_t m_Data3;
		uint8_t  m_Data4[8];
	};
}