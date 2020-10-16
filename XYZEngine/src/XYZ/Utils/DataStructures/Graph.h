#pragma once
#include "FreeList.h"


// Very simple version of graph, it is required to update this, it is not even real graph

namespace XYZ {

	
	class Graph
	{
	private:
		
	
	public:
		Graph(bool bothDirrections);

		int32_t AddVertex();
		
		void AddEdge(int32_t start, int32_t end);
		
		void RemoveEdge(int32_t source, int32_t destination);
	
		void RemoveVertex(int32_t index);
		


		template <typename Func>
		void Traverse(const Func& func)
		{
			// Traverse graph parent -> all its children.... next parent - > all its children
			if (m_Visited.Num() != m_AdjList.Num())
				m_Visited.SetNum(m_AdjList.Num());
			for (auto& it : m_Visited)
				it = false;

			uint32_t counter = 0;
			for (auto& adj : m_AdjList)
			{
				for (int32_t i = 0; i < adj.Num(); ++i)
				{
					VertexData* next = nullptr;
					VertexData* previous = nullptr;
					int32_t nextIndex = -1;
					int32_t previousIndex = -1;
					if (i < adj.Num() - 1)
					{
						next = &m_Data[adj[i + 1].Index];
						nextIndex = i + 1;
					}
					else if (i != 0)
					{
						next = &m_Data[adj[0].Index];
						nextIndex = 0;
					}
					if (i > 0)
					{
						previous = &m_Data[adj[i - 1].Index];
						previousIndex = i - 1;
					}
					else
					{
						previous = &m_Data[adj[adj.Num() - 1].Index];
						previousIndex = adj.Num() - 1;
					}
					// Functions takes as param parent data, child data, parent data index / adj index, child index relative to parent, and child data index / adj index
					func(m_Data[counter], m_Data[adj[i].Index], counter, adj[i].Index, next, previous, nextIndex, previousIndex, i, m_Visited[adj[i].Index], adj.Num() == 1);
				}

				m_Visited[counter] = true;
				counter++;
			}
		}

	private:
		struct Vertex
		{
			std::vector<int32_t> Connections;
		};

		FreeList<Vertex> m_Vertices;
		bool m_BothDirrections;
	};
}