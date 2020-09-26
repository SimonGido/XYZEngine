#include "stdafx.h"
#include "Graph.h"


namespace XYZ {
	void Graph::Connect(const Connection& connection)
	{
		m_Connections.push_back(connection);
	}
	void Graph::Disconnect(uint32_t index)
	{
		m_Connections.erase(m_Connections.begin() + index);
	}
}