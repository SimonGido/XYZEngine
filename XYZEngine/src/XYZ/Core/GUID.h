#pragma once

#include <array>
#include <functional>
#include <string>

#include "XYZ/Core/Core.h"

namespace XYZ {

	class XYZ_API GUID
	{
	public:
		GUID();
		GUID(const std::string& str);
		GUID(const GUID& other);

		GUID& operator=(const std::string& str);

		bool operator==(const GUID& other) const
		{
			for (size_t i = 0; i < 2; ++i)
			{
				if (m_Data[i] != other.m_Data[i])
					return false;
			}
			return true;
		}

		bool operator<(const GUID& other) const
		{
			for (size_t i = 0; i < 2; ++i)
			{
				if (m_Data[i] != other.m_Data[i])
					return m_Data[i] < other.m_Data[i];
			}
			return false;
		}

		bool operator >(const GUID& other) const
		{
			return !(*this < other);
		}

		bool operator<=(const GUID& other) const
		{
			return *this < other || *this == other;
		}

		operator std::string() const;
		operator std::string();
		std::string ToString() const
		{
			return (std::string)*this;
		}
		inline size_t Hash() const
		{
			size_t seed = 0;
			const std::hash<size_t> hasher;
			seed ^= hasher(m_Data[0]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= hasher(m_Data[1]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			return hasher(seed);
		}

	private:
		uint64_t m_Data[2];
	};
}


namespace std {

	template <>
	struct hash<XYZ::GUID>
	{
		size_t operator()(const XYZ::GUID& k) const
		{
			return k.Hash();
		}
	};

}