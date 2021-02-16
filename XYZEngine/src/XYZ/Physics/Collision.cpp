#include "stdafx.h"
#include "Collision.h"

namespace XYZ {
	namespace Collision {
		bool AABBvsAABB(Manifold& m)
		{
			glm::vec2 n = m.B->m_Position - m.A->m_Position;

			const AABB& aBox = m.A->GetShape()->GetAABB();
			const AABB& bBox = m.B->GetShape()->GetAABB();
			float aExtentX = (aBox.Max.x - aBox.Min.x) / 2.0f;
			float bExtentX = (bBox.Max.x - bBox.Min.x) / 2.0f;
			float xOverlap = aExtentX + bExtentX - abs(n.x);
			if (xOverlap > 0.0f)
			{
				float aExtentY = (aBox.Max.y - aBox.Min.y) / 2.0f;
				float bExtentY = (bBox.Max.y - bBox.Min.y) / 2.0f;

				// Calculate overlap on y axis
				float yOverlap = aExtentY + bExtentY - abs(n.y);
				if (yOverlap > 0.0f)
				{
					m.ContactCount = 2;
					if (xOverlap < yOverlap)
					{
						if (n.x < 0)
						{
							m.Normal = glm::vec2(-1.0f, 0.0f);
							m.Contacts[0] = bBox.Min;
							m.Contacts[1] = bBox.Min + glm::vec3(0.0f, bBox.Max.y, 0.0f);
						}
						else
						{
							m.Normal = glm::vec2(1.0f, 0.0f);
							m.Contacts[0] = bBox.Max - glm::vec3(0.0f, bBox.Max.y, 0.0f);
							m.Contacts[1] = bBox.Max;
						}
						m.PenetrationDepth = xOverlap;
						return true;
					}
					else
					{
						// Point toward B knowing that n points from A to B
						if (n.y < 0)
						{
							m.Normal = glm::vec2(0.0f, -1.0f);
							m.Contacts[0] = bBox.Min;
							m.Contacts[1] = bBox.Min + glm::vec3(bBox.Max.x, 0.0f, 0.0f);
						}
						else
						{
							m.Normal = glm::vec2(0.0f, 1.0f);
							m.Contacts[0] = bBox.Max - glm::vec3(bBox.Max.x, 0.0f, 0.0f);
							m.Contacts[1] = bBox.Max;
						}
						m.PenetrationDepth = yOverlap;
						return true;
					}
				}
			}
			return false;
		}
	}
}


