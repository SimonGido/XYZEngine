#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "PhysicsComponent.h"

#include "XYZ/Utils/DataStructures/FreeList.h"

namespace XYZ {
	/*! @class RealGridCollisionSystem
	*	@brief Real grid collisions , allows free movement of objects
	*/
	class RealGridCollisionSystem : public System
	{
	public:
		RealGridCollisionSystem();
		~RealGridCollisionSystem();

		void CreateGrid(int width, int height, int cellSize);
		virtual void Update(float dt);
		virtual void Add(uint32_t entity) override;
		virtual void Remove(uint32_t entity) override;
		virtual bool Contains(uint32_t entity) override;


	private:
		struct Component : public System::Component
		{
			RealGridBody* RealGridBody;
			RigidBody2D* RigidBody;
		};
	
		struct Cell
		{
			// Change to raw array on heap int*
			std::vector<int> indices;
		};

		int m_NumRows;
		int m_NumCols;
		int m_CellSize;

		vector2D<Cell> m_Cells;
		FreeList<Component> m_Components;
	};
}