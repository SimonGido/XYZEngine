#include "stdafx.h"
#include "RealGridCollisionSystem.h"


namespace XYZ {
	RealGridCollisionSystem::RealGridCollisionSystem(ECSManager* ecs)
		:
		m_ECS(ecs),
		m_NumCols(1),m_NumRows(1),m_CellSize(1)
	{		
		m_Signature.set(m_ECS->GetComponentType<RealGridBody>());
		m_Signature.set(m_ECS->GetComponentType<RigidBody2D>());
	}
	RealGridCollisionSystem::~RealGridCollisionSystem()
	{
	
	}
	void RealGridCollisionSystem::CreateGrid(int width, int height, int cellSize)
	{
		m_NumRows = width;
		m_NumCols = height;
		m_CellSize = cellSize;

		m_Cells.resize(m_NumRows);
		for (int i = 0; i < m_NumRows; ++i)
			m_Cells[i].resize(m_NumCols);
	}
	void RealGridCollisionSystem::Update(float dt)
	{	
		for (int i = 0; i < m_Components.Range(); ++i)
		{		
			auto body = m_Components[i].RealGridBody;
			int gridLeft = (int)floor(body->Left / m_CellSize);
			int gridRight = (int)floor(body->Right / m_CellSize);
			int gridTop = (int)floor(body->Top / m_CellSize);
			int gridBottom = (int)floor(body->Bottom / m_CellSize);
		
			body->Move(m_Components[i].RigidBody->Velocity);

			int newGridLeft = (int)floor(body->Left / m_CellSize);
			int newGridRight = (int)floor(body->Right / m_CellSize);
			int newGridTop = (int)floor(body->Top / m_CellSize);
			int newGridBottom = (int)floor(body->Bottom / m_CellSize);
				

			for (int y = gridBottom; y < gridTop; ++y)
			{
				for (int x = gridLeft; x < gridRight; ++x)
				{
					auto it = std::find(m_Cells[y][x].indices.begin(), m_Cells[y][x].indices.end(),i);
					if (it != m_Cells[y][x].indices.end())
						m_Cells[y][x].indices.erase(it);
				}
			}
			for (int y = newGridBottom; y < newGridTop; ++y)
			{
				for (int x = newGridLeft; x < newGridRight; ++x)
				{		
					for (auto index : m_Cells[y][x].indices)
					{	
						if (m_Components[i].RealGridBody->Collide(*m_Components[index].RealGridBody))
						{
							std::cout << "LOL" << std::endl;
						}				
					}
					m_Cells[y][x].indices.push_back(i);
				}
			}
		}
	}
	void RealGridCollisionSystem::Add(uint32_t entity)
	{
		Component component;
		component.ActiveComponent = m_ECS->GetComponent<ActiveComponent>(entity);
		component.RealGridBody = m_ECS->GetComponent<RealGridBody>(entity);
		component.RigidBody = m_ECS->GetComponent<RigidBody2D>(entity);
		component.Ent = entity;

		int index = m_Components.Insert(component);

		auto body = m_ECS->GetComponent<RealGridBody>(entity);
		auto gridLeft = (int)floor(body->Left / m_CellSize);
		auto gridRight = (int)floor(body->Right / m_CellSize);
		auto gridTop = (int)floor(body->Top / m_CellSize);
		auto gridBottom = (int)floor(body->Bottom / m_CellSize);

		for (int y = gridBottom; y < gridTop; ++y)
		{
			for (int x = gridLeft; x < gridRight; ++x)
			{
				m_Cells[y][x].indices.push_back(index);
			}
		}
		XYZ_LOG_INFO("Entity with ID ", entity, " added");
	}
	void RealGridCollisionSystem::Remove(uint32_t entity)
	{
		if (m_ECS->Contains<RealGridBody>(entity))
		{
			auto body = m_ECS->GetComponent<RealGridBody>(entity);

			auto gridLeft = (int)floor(body->Left / m_CellSize);
			auto gridRight = (int)floor(body->Right / m_CellSize);
			auto gridTop = (int)floor(body->Top / m_CellSize);
			auto gridBottom = (int)floor(body->Bottom / m_CellSize);


			for (int y = gridBottom; y <= gridTop; ++y)
			{
				for (int x = gridLeft; x <= gridRight; ++x)
				{
					for (auto index : m_Cells[y][x].indices)
					{
						if (m_Components[index].Ent == entity)
						{
							m_Components.Erase(index);
							m_Cells[y][x].indices.erase(m_Cells[y][x].indices.begin()+index);
							break;
						}
					}
				}
			}
		}
	}
	bool RealGridCollisionSystem::Contains(uint32_t entity)
	{
		//if (ECSManager::Contains<RealGridBody>(entity))
		//{
		//	auto body = ECSManager::GetComponent<RealGridBody>(entity);
		//
		//	auto gridLeft = (int)floor(body->left / m_CellSize);
		//	auto gridRight = (int)floor(body->right / m_CellSize);
		//	auto gridTop = (int)floor(body->top / m_CellSize);
		//	auto gridBottom = (int)floor(body->bottom / m_CellSize);
		//
		//
		//	for (int y = gridBottom; y <= gridTop; ++y)
		//	{
		//		for (int x = gridLeft; x <= gridRight; ++x)
		//		{
		//			GridRow* row = &m_Grid.rows[y];
		//			int* link = &row->cells[x];
		//			while (row->elem[*link].entity != entity)
		//				link = &row->elem[*link].nextIndex;
		//			
		//			
		//		}
		//	}
		//}

		
		return false;
	}
}