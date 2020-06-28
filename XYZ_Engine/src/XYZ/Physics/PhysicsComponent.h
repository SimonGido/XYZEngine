#pragma once
#include "XYZ/ECS/Component.h"

#include <glm/glm.hpp>


namespace XYZ {
	template<typename T>
	using vector2D = std::vector<std::vector<T>>;


	struct RigidBody2D : public Type<RigidBody2D>
	{
		RigidBody2D(const glm::vec2& velocity)
			: Velocity(velocity), BounceValue(0)
		{}

		glm::vec2 Velocity = glm::vec2(0);
		int BounceValue = 0;
	};

	struct GridBody : public Type<GridBody>
	{
		GridBody(int row, int col, int width, int height)
			: Row(row), Col(col), Width(width), Height(height), NextRow(0), NextCol(0)
		{}

		GridBody() = default;

		int Row = 0;
		int Col = 0;
		int Width = 0;
		int Height = 0;
		int NextRow = 0;
		int NextCol = 0;

		bool operator ==(const GridBody& other) const
		{
			return (Col == other.Col && Row == other.Row);
		}

		bool operator !=(const GridBody& other)
		{
			return (Col != other.Col || Row != other.Row);
		}
	};

	struct InterpolatedMovement : public Type<InterpolatedMovement>
	{
		InterpolatedMovement(const glm::vec2& velocity)
			: Distance(glm::vec2(0)), Velocity(velocity), InProgress(false)
		{}

		glm::vec2 Distance = glm::vec2(0);
		glm::vec2 Velocity = glm::vec2(0);
		bool InProgress = false;
	};

	struct CollisionComponent : public Type<CollisionComponent>
	{
		CollisionComponent(int32_t layer, int32_t collisionLayers)
			: Layer(layer), CollisionLayers(collisionLayers), CurrentCollisions(0)
		{}


		int32_t Layer = 0;

		// layers that might collide with layer
		int32_t CollisionLayers = 0;

		// current collisions
		int32_t CurrentCollisions = 0;
	};


	struct RealGridBody : public Type<RealGridBody>
	{
		RealGridBody(float left, float right, float bottom, float top)
			: Left(left), Right(right), Bottom(bottom), Top(top)
		{}

		RealGridBody() = default;

		bool Collide(const RealGridBody& other)
		{
			if (Left >= other.Right || other.Left >= Right)
			{
				return false;
			}
			if (Top <= other.Bottom || other.Top <= Bottom)
			{
				return false;
			}
			return true;
		}

		void Move(const glm::vec2& pos)
		{
			Left += pos.x;
			Right += pos.x;
			Top += pos.y;
			Bottom += pos.y;
		}

		float Left = 0.0f;
		float Right = 0.0f;
		float Bottom = 0.0f;
		float Top = 0.0f;
	};

}


