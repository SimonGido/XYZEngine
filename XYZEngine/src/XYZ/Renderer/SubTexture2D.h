#pragma once
#include "Texture.h"
#include "XYZ/Core/Ref.h"
#include "XYZ/Scene/Serializable.h"

#include <glm/glm.hpp>

namespace XYZ {

	class SubTexture2D : public RefCount,
						 public Serializable
	{
	public:
		SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& coords, const glm::vec2& size);
		SubTexture2D(const Ref<Texture2D>& texture, const glm::vec4& texCoords);

		void SetCoords(const glm::vec2& coords,const glm::vec2& size);
		void SetCoords(const glm::vec4& texCoords) { m_TexCoords = texCoords; };

		const glm::vec4& GetTexCoords() const { return m_TexCoords; }

		const Ref<Texture2D>& GetTexture() const { return m_Texture; }
	private:
		Ref<Texture2D> m_Texture;
		glm::vec4 m_TexCoords;
	};
}