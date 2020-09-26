#pragma once
#include "FreeList.h"


// Very simple version of graph, it is required to update this, it is not even real graph

namespace XYZ {
	
	struct Connection
	{
		uint32_t Start;
		uint32_t End;
	};

	class Graph
	{
	public:
		void Connect(const Connection& connection);
		void Disconnect(uint32_t index);
	

		const std::vector<Connection>& GetConnections() const { return m_Connections; }

	private:
		std::vector<Connection> m_Connections;
	};
}