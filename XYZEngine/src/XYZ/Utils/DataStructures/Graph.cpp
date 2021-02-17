#include "stdafx.h"
#include "Graph.h"


namespace XYZ {
	Graph::Graph(bool bothDirrections)
		:
		m_BothDirrections(bothDirrections)
	{
	}
	int32_t Graph::AddVertex(uint32_t index)
	{
		m_Vertices.push_back({ index });
		return m_Vertices.size() - 1;
	}
	void Graph::AddEdge(int32_t start, int32_t end)
	{
		XYZ_ASSERT(start < m_Vertices.size() && end < m_Vertices.size(), "Edge out of range");
		m_Vertices[start].Connections.push_back({ end });
		if (m_BothDirrections)
			m_Vertices[end].Connections.push_back({ start });
	}
	void Graph::RemoveEdge(int32_t source, int32_t destination)
	{		
		{
			uint32_t counter = 0;
			auto& connections = m_Vertices[source].Connections;
			for (auto it : connections)
			{
				if (it == destination)
				{
					connections.erase(connections.begin() + counter);
					break;
				}
				counter++;
			}
		}
		if (m_BothDirrections)
		{
			uint32_t counter = 0;
			auto& connections = m_Vertices[destination].Connections;
			for (auto it : connections)
			{
				if (it == source)
				{
					connections.erase(connections.begin() + counter);
					break;
				}
				counter++;
			}
		}
	}
	void Graph::RemoveVertex(int32_t index)
	{
		// Remove vertex and all connections to it
		if (m_BothDirrections)
		{
			// TODO
		}
		m_Vertices[index] = std::move(m_Vertices.back());
		m_Vertices.pop_back();
	}
	void Graph::Clear()
	{
		m_Vertices.clear();
	}
}