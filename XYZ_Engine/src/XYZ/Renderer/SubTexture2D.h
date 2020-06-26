#pragma once
#include "Texture.h"

#include <glm/glm.hpp>

namespace XYZ {

	class SubTexture2D
	{
	public:
		SubTexture2D(std::shared_ptr<Texture2D> texture, const glm::vec2& coords, const glm::vec2& size);

		const glm::vec4& GetTexCoords() const { return m_TexCoords; }
	private:
		std::shared_ptr<Texture2D> m_Texture;
		glm::vec4 m_TexCoords;
	};
}