#pragma once
#include "Texture.h"
#include "XYZ/Core/Ref.h"

#include <glm/glm.hpp>

namespace XYZ {

	class SubTexture2D : public RefCount
	{
	public:
		SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& coords, const glm::vec2& size);

		void SetCoords(const glm::vec2& coords,const glm::vec2& size);
		const glm::vec4& GetTexCoords() const { return m_TexCoords; }
	private:
		Ref<Texture2D> m_Texture;
		glm::vec4 m_TexCoords;
	};
}