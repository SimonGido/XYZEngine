#pragma once
#include "Core.h"

#include "UDPConnection.h"

namespace XYZ {
	class UDPConnectionManager
	{
	public:
		using ContainerType = std::unordered_map<
			asio::ip::udp::endpoint, std::shared_ptr<UDPConnection>,
			std::size_t(*)(const asio::ip::udp::endpoint&)>;

		using KeyType = ContainerType::key_type;
		using MappedType = ContainerType::mapped_type;


	public:
		bool AddConnection(const KeyType& key, std::shared_ptr<UDPConnection> connection);

		void RemoveConnection(const KeyType& key);

		bool HasConnection(const KeyType& key);

	private:
		static std::size_t getHash(const asio::ip::udp::endpoint& endpoint);
		

	private:
		ContainerType m_Connections;
	};
}