#pragma once
#include "FreeList.h"


// Very simple version of graph, it is required to update this, it is not even real graph

namespace XYZ {

	
	class Graph
	{	
	public:
		struct Vertex
		{
			uint32_t Index;
			std::vector<int32_t> Connections;
		};
	public:
		Graph(bool bothDirrections);

		int32_t AddVertex(uint32_t index);	
		void AddEdge(int32_t start, int32_t end);	
		void RemoveEdge(int32_t source, int32_t destination);
		void RemoveVertex(int32_t index);
		void Clear();

		template <typename Func>
		void Traverse(const Func& func)
		{
			for (int32_t i = 0; i < m_Vertices.size(); ++i)
			{			
				for (auto connection : m_Vertices[i].Connections)
				{
					func(i, connection, m_Vertices[i].Index, m_Vertices[connection].Index);
				}
			}
		}

		const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
	private:
		
		std::vector<Vertex> m_Vertices;
		const bool m_BothDirrections;
	};
}