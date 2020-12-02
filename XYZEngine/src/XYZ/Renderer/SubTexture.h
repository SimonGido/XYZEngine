#pragma once
#include "Texture.h"
#include "XYZ/Core/Ref.h"
#include "XYZ/Scene/Serializable.h"

#include <glm/glm.hpp>

namespace XYZ {

	class SubTexture : public RefCount,
						 public Serializable
	{
	public:
		SubTexture(const Ref<Texture>& texture, const glm::vec2& coords, const glm::vec2& size);
		SubTexture(const Ref<Texture>& texture, const glm::vec4& texCoords);

		void SetCoords(const glm::vec2& coords,const glm::vec2& size);
		void SetCoords(const glm::vec4& texCoords) { m_TexCoords = texCoords; };
		void SetTexture(const Ref<Texture>& texture) { m_Texture = texture; };
		const glm::vec4& GetTexCoords() const { return m_TexCoords; }

		const Ref<Texture>& GetTexture() const { return m_Texture; }

	private:
		Ref<Texture> m_Texture;
		glm::vec4 m_TexCoords;
	};
}