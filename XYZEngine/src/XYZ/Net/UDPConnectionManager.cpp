#include "stdafx.h"

#include "UDPConnectionManager.h"


namespace XYZ {
	bool UDPConnectionManager::AddConnection(const KeyType& key, std::shared_ptr<UDPConnection> connection)
	{
		auto it = m_Connections.find(key);
		if (it != m_Connections.end())
			return false;

		m_Connections[key] = connection;
		return true;
	}
	void UDPConnectionManager::RemoveConnection(const KeyType& key)
	{
		auto it = m_Connections.find(key);
		if (it != m_Connections.end())
			m_Connections.erase(key);
	}
	bool UDPConnectionManager::HasConnection(const KeyType& key)
	{
		return (m_Connections.find(key) != m_Connections.end());
	}
	std::size_t UDPConnectionManager::getHash(const asio::ip::udp::endpoint& endpoint)
	{
		std::ostringstream stream;
		stream << endpoint;
		std::hash<std::string> hasher;
		return hasher(stream.str());
	}

}