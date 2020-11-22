#pragma once


#include <string>

namespace XYZ {

	class GUID
	{
	public:
		GUID();
		GUID(const GUID& other);
		
		GUID& operator=(const std::string uuid)
		{
			m_UUID = uuid;
			return *this;
		}

		operator std::string& () { return m_UUID; }
		operator const  std::string& () const { return m_UUID; }

	private:
		std::string m_UUID;
	};
}