#pragma once

#include <glm/glm.hpp>

namespace XYZ {
	namespace Editor {
		class PreviewBone
		{
		public:
			enum CollideFlags {
				Start = BIT(0), End = BIT(1), Body = BIT(2)
			};

			void Decompose(glm::vec2& start, glm::vec2& end, float& rot, glm::vec2& normal, bool worldTransform = true) const;
			bool Collide(const glm::vec2& position, uint8_t collisionFlags) const;
			void Rotate(const glm::vec2& position);
			void Translate(const glm::vec2& position, const glm::mat4& parentSpace);

			glm::vec2 WorldPosition = glm::vec2(0.0f);
			glm::vec2 LocalPosition = glm::vec2(0.0f);
			glm::vec2 Direction = glm::vec2(0.0f);
			float	  Length = 0.0f;
			int32_t	  ID;

			glm::mat4 WorldTransform;
			glm::mat4 LocalTransform;

			std::string Name;
			glm::vec3   Color;
			bool Open = false;

			static constexpr float PointRadius = 5.0f;
		};
	}
}