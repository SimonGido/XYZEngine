#include "stdafx.h"
#include "GridCollisionSystem.h"

#define OUT_OF_GRID (1 << 31)
namespace XYZ {
	GridCollisionSystem::GridCollisionSystem()
		: m_NumRows(1), m_NumCols(1), m_CellSize(1), m_PositionX(0), m_PositionY(0)
	{
		m_Signature.set(XYZ::ECSManager::GetComponentType<GridBody>());
		m_Signature.set(XYZ::ECSManager::GetComponentType<CollisionComponent>());
	}
	void GridCollisionSystem::ClearGrid()
	{
		for (int i = 0; i < m_NumRows; ++i)
			for (int j = 0; j < m_NumCols; ++j)
				m_Cells[i][j].Mask = 0;
	}
	void GridCollisionSystem::ResizeGrid(int numRows, int numCols, int cellSize, int positionX, int positionY)
	{
		m_NumRows = numRows;
		m_NumCols = numCols;
		m_CellSize = cellSize; 
		m_PositionX = positionX; 
		m_PositionY = positionY;

		m_Cells.resize(m_NumRows);
		for (int i = 0; i < m_NumRows; ++i)
			m_Cells[i].resize(m_NumCols);
	}
	void GridCollisionSystem::MoveGrid(int positionX, int positionY)
	{
		m_PositionX = positionX;
		m_PositionY = positionY;
	}
	void GridCollisionSystem::Update(float dt)
	{
		for (auto& it : m_Components)
		{
			if ((it.ActiveComponent->ActiveComponents & m_Signature) == m_Signature)
			{
				auto mask = it.Collision->Layer;
				int32_t result = Move(
					*it.GridBody,
					it.Collision->CollisionLayers,
					it.Collision->Layer);

				// Store mask of layer it collides with
				it.Collision->CurrentCollisions = result;

				// No collisions , free to move
				if (!result && result != OUT_OF_GRID)
				{
					it.GridBody->Col += it.GridBody->NextCol;
					it.GridBody->Row += it.GridBody->NextRow;
				}
				else
				{
					//it.GridBody.Get().NextCol = 0;
					//it.GridBody.Get().NextRow = 0;
				}
			}
		}

	}
	void GridCollisionSystem::Add(uint32_t entity)
	{
		Component component;
		component.ActiveComponent = ECSManager::GetComponent<ActiveComponent>(entity);
		component.GridBody = ECSManager::GetComponent<GridBody>(entity);
		component.Collision = ECSManager::GetComponent<CollisionComponent>(entity);
		component.Ent = entity;

		auto layer = component.Collision->Layer;
		auto collisionLayer = component.Collision->CollisionLayers;
		if (Insert(*component.GridBody,collisionLayer, layer))
		{
			m_Components.push_back(component);
			XYZ_LOG_INFO("Entity with ID ", entity, " added");
		}
		else
			XYZ_LOG_WARN("Entity with ID ", entity, " not added");
	}
	void GridCollisionSystem::Remove(uint32_t entity)
	{
		auto it = std::find(m_Components.begin(), m_Components.end(), entity);
		if (it != m_Components.end())
		{
			XYZ_LOG_INFO("Entity with id ", entity, " removed");
			auto mask = (*it).Collision->Layer;
			Remove(*(*it).GridBody, mask);
			*it = std::move(m_Components.back());
			m_Components.pop_back();
		}
	}
	bool GridCollisionSystem::Contains(uint32_t entity)
	{
		auto it = std::find(m_Components.begin(), m_Components.end(), entity);
		if (it != m_Components.end())
			return true;
		return false;
	}
	bool GridCollisionSystem::Insert(const GridBody& body, int32_t collisionMask, int32_t layerMask)
	{
		if (body.Row + body.Height < m_NumRows && body.Col + body.Width < m_NumCols
			&& body.Row >= m_PositionY && body.Col >= m_PositionX)
		{
			for (int i = body.Row; i < body.Row + body.Height; ++i)
				for (int j = body.Col; j < body.Col + body.Width; ++j)
					if (m_Cells[i][j].Mask & collisionMask)
						return false;

			for (int i = body.Row; i < body.Row + body.Height; ++i)
				for (int j = body.Col; j < body.Col + body.Width; ++j)
					m_Cells[i][j].Mask |= layerMask;


			return true;
		}
		return false;
	}
	int32_t GridCollisionSystem::Move(const GridBody& oldBody,int32_t collisionMask, int32_t layerMask)
	{
		GridBody newBody = oldBody;
		newBody.Col += newBody.NextCol;
		newBody.Row += newBody.NextRow;

		if (newBody.Row + newBody.Height < m_NumRows && newBody.Col + newBody.Width < m_NumCols
			&& newBody.Row >= m_PositionY && newBody.Col >= m_PositionX)
		{
			// No changes to the body
			if (oldBody == newBody)
				return 0;

			if (newBody.Col < oldBody.Col)
			{
				// Handle diagonal movement
				if (newBody.Row != oldBody.Row)
				{
					if (m_Cells[oldBody.Row][oldBody.Col - 1].Mask & collisionMask)
					{
						return m_Cells[oldBody.Row][oldBody.Col - 1].Mask & collisionMask;
					}
					else if (m_Cells[newBody.Row][newBody.Col + newBody.Width].Mask & collisionMask)
					{
						return m_Cells[newBody.Row][newBody.Col + newBody.Width].Mask & collisionMask;
					}
				}
				else
				{
					// Check all cells to the top of the body
					for (int i = newBody.Row; i < newBody.Row + newBody.Height; ++i)
						if (m_Cells[i][newBody.Col].Mask & collisionMask)
							return m_Cells[i][newBody.Col].Mask & collisionMask;
				}
			}
			else if (newBody.Col > oldBody.Col)
			{
				// Handle diagonal movement
				if (newBody.Row != oldBody.Row)
				{
					if (m_Cells[oldBody.Row][oldBody.Col + oldBody.Width].Mask & collisionMask)
					{
						return m_Cells[oldBody.Row][oldBody.Col + oldBody.Width].Mask & collisionMask;
					}
					else if (m_Cells[newBody.Row][newBody.Col - 1].Mask & collisionMask)
					{
						return m_Cells[newBody.Row][newBody.Col - 1].Mask & collisionMask;
					}
				}
				else
				{
					// Check all cells to the top of the body and one cell in the width of the new body
					for (int i = newBody.Row; i < newBody.Row + newBody.Height; ++i)
						if (m_Cells[i][newBody.Col + newBody.Width - 1].Mask & collisionMask)
							return m_Cells[i][newBody.Col + newBody.Width - 1].Mask & collisionMask;
				}
			}
			else if (newBody.Row < oldBody.Row)
			{
				// Check all cells to the width of the body
				for (int i = newBody.Col; i < newBody.Col + newBody.Width; ++i)
					if (m_Cells[newBody.Row][i].Mask & collisionMask)
						return m_Cells[newBody.Row][i].Mask & collisionMask;
			}
			else
			{
				// Check all cells to the width of the body and one to the top
				for (int i = newBody.Col; i < newBody.Col + newBody.Width; ++i)
					if (m_Cells[newBody.Row + newBody.Height - 1][i].Mask & collisionMask)
						return m_Cells[newBody.Row + newBody.Height - 1][i].Mask & collisionMask;
			}

			// Clear all the cells occupied by the oldBody
			for (int i = oldBody.Row; i < oldBody.Row + oldBody.Height; ++i)
				for (int j = oldBody.Col; j < oldBody.Col + oldBody.Width; ++j)
					m_Cells[i][j].Mask &= (~layerMask);

			// Set all the cells occupied by the newBody
			for (int i = newBody.Row; i < newBody.Row + newBody.Height; ++i)
				for (int j = newBody.Col; j < newBody.Col + newBody.Width; ++j)
					m_Cells[i][j].Mask |= layerMask;


			// Movement was successful return empty collision mask
			return 0;
		}
		return 1 << 31;
	}
	void GridCollisionSystem::Remove(const GridBody& body, int32_t layerMask)
	{
		if (body.Row + body.Height < m_NumRows && body.Col + body.Width < m_NumCols
			&& body.Row >= m_PositionY && body.Col >= m_PositionX)
		{
			// Clear all cells occupied by the body
			for (int i = body.Row; i < body.Row + body.Height; ++i)
				for (int j = body.Col; j < body.Col + body.Width; ++j)
					m_Cells[i][j].Mask &= (~layerMask);
		}
	}
}