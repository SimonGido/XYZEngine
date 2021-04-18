#include "stdafx.h"
#include "Collision.h"

#include "XYZ/Utils/Math/Math.h"

namespace XYZ {
	namespace Collision {
		bool AABBvsAABB(Manifold& m)
		{
			glm::vec2 n = m.B->GetPosition() - m.A->GetPosition();

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
					float sizeX = bBox.Max.x - bBox.Min.x;
					float sizeY = bBox.Max.y - bBox.Min.y;
					m.ContactCount = 2;
					if (xOverlap < yOverlap)
					{
						if (n.x < 0)
						{
							m.Normal = glm::vec2(-1.0f, 0.0f);
							m.Contacts[0] = bBox.Max - glm::vec3(0.0f, sizeY, 0.0f);
							m.Contacts[1] = bBox.Max;
						}
						else
						{
							m.Normal = glm::vec2(1.0f, 0.0f);
							m.Contacts[0] = bBox.Min;
							m.Contacts[1] = bBox.Min + glm::vec3(0.0f, sizeY, 0.0f);
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
							m.Contacts[0] = bBox.Max - glm::vec3(sizeX, 0.0f, 0.0f);
							m.Contacts[1] = bBox.Max;
						}
						else
						{
							m.Normal = glm::vec2(0.0f, 1.0f);
							m.Contacts[0] = bBox.Min;
							m.Contacts[1] = bBox.Min + glm::vec3(sizeX, 0.0f, 0.0f);
							
						}
						m.PenetrationDepth = yOverlap;
						return true;
					}
				}
			}
			return false;
		}
		bool CirclevsCircle(Manifold& m)
		{
			// Calculate translational vector, which is normal
			const CircleShape* ShapeA = static_cast<const CircleShape*>(m.A->GetShape());
			const CircleShape* ShapeB = static_cast<const CircleShape*>(m.B->GetShape());
			
			glm::vec2 normal = m.B->GetPosition() - m.A->GetPosition();

			float distSqr = normal.x * normal.x + normal.y * normal.y;
			float radius = ShapeA->Radius + ShapeB->Radius;

			// Not in contact
			if (distSqr >= radius * radius)
			{
				m.ContactCount = 0;
				return false;
			}

			float distance = std::sqrt(distSqr);
			m.ContactCount = 1;

			if (distance == 0.0f)
			{
				m.PenetrationDepth = ShapeA->Radius;
				m.Normal = glm::vec2(1, 0);
				m.Contacts[0] = m.A->GetPosition();
			}
			else
			{
				m.PenetrationDepth = radius - distance;
				m.Normal = normal / distance; // Faster than using Normalized since we already performed sqrt
				m.Contacts[0] = m.Normal * ShapeA->Radius + m.A->GetPosition();
			}
			return true;
		}
		bool CirclevsAABB(Manifold& m)
		{
			m.ContactCount = 0;

			const CircleShape* circleShape = static_cast<const CircleShape*>(m.A->GetShape());
			const BoxShape2D* boxShape = static_cast<const BoxShape2D*>(m.B->GetShape());
			// Transform circle center to Polygon model space
			glm::vec2 center = m.A->GetPosition();
			
			const AABB& box = boxShape->GetAABB();

			// Find edge with minimum penetration
			// Exact concept as using support points in Polygon vs Polygon
			float separation = -FLT_MAX;

			float s1 = Math::Dot(glm::vec2(0.0f, -1.0f), center - glm::vec2(box.Min.x, box.Min.y));
			float s2 = Math::Dot(glm::vec2(1.0f, 0.0f), center - glm::vec2(box.Max.x, box.Min.y));
			float s3 = Math::Dot(glm::vec2(0.0f, 1.0f), center - glm::vec2(box.Max.x, box.Max.y));
			float s4 = Math::Dot(glm::vec2(-1.0f, 0.0f), center - glm::vec2(box.Min.x, box.Max.y));

			if (s1 > circleShape->Radius || s2 > circleShape->Radius || s3 > circleShape->Radius || s4 > circleShape->Radius)
				return false;

			glm::vec2 faceNormal = glm::vec2(0.0f);
			glm::vec2 v1 = glm::vec2(0.0f), v2 = glm::vec2(0.0f);
			if (s1 > separation)
			{
				separation = s1;
				faceNormal = glm::vec2(0.0f, -1.0f);
				v1 = glm::vec2(box.Min.x, box.Min.y);
				v2 = glm::vec2(box.Max.x, box.Min.y);
			}
			if (s2 > separation)
			{
				separation = s2;
				faceNormal = glm::vec2(1.0f, 0.0f);
				v1 = glm::vec2(box.Max.x, box.Min.y);
				v2 = glm::vec2(box.Max.x, box.Max.y);
			}
			if (s3 > separation)
			{
				separation = s3;
				faceNormal = glm::vec2(0.0f, 1.0f);
				v1 = glm::vec2(box.Max.x, box.Max.y);
				v2 = glm::vec2(box.Min.x, box.Max.y);
			}
			if (s4 > separation)
			{
				separation = s4;
				faceNormal = glm::vec2(-1.0f, 0.0f);
				v1 = glm::vec2(box.Min.x, box.Max.y);
				v2 = glm::vec2(box.Min.x, box.Min.y);
			}

			// Check to see if center is within polygon
			if (separation < FLT_EPSILON)
			{
				m.ContactCount = 1;
				m.Normal = -faceNormal;
				m.Contacts[0] = m.Normal * circleShape->Radius + m.A->GetPosition();
				m.PenetrationDepth = circleShape->Radius;
				return true;
			}

			// Determine which voronoi region of the edge center of circle lies within
			float dot1 = Math::Dot(center - v1, v2 - v1);
			float dot2 = Math::Dot(center - v2, v1 - v2);
			m.PenetrationDepth = circleShape->Radius - separation;

			// Closest to v1
			if (dot1 <= 0.0f)
			{
				float distSqr = Math::Dot(center - v1, center - v1);
				if (distSqr > circleShape->Radius * circleShape->Radius)
					return false;

				m.ContactCount = 1;
				m.Normal = v1 - center;
				Math::Normalize(m.Normal);
				v1 = v1 + m.B->GetPosition();
				m.Contacts[0] = v1;
			}
			// Closest to v2
			else if (dot2 <= 0.0f)
			{
				float distSqr = Math::Dot(center - v2, center - v2);
				if (distSqr > circleShape->Radius * circleShape->Radius)
					return false;

				m.ContactCount = 1;
				m.Normal = v2 - center;
				Math::Normalize(m.Normal);
				v2 += m.B->GetPosition();
				m.Contacts[0] = v2;
			}
			// Closest to face
			else
			{
				if (Math::Dot(center - v1, faceNormal) > circleShape->Radius)
					return false;
				m.Normal = -faceNormal;
				m.Contacts[0] = m.Normal * circleShape->Radius + m.A->GetPosition();
				m.ContactCount = 1;
			}
			return true;
		}
		bool PolygonvsPolygon(Manifold& m)
		{
			return false;
		}
	}
}


