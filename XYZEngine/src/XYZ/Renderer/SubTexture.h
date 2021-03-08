#pragma once
#include "Texture.h"
#include "XYZ/Asset/Asset.h"

#include <glm/glm.hpp>

namespace XYZ {

	class SubTexture : public Asset
	{
	public:
		SubTexture(const Ref<Texture>& texture, const glm::vec2& coords, const glm::vec2& size);
		SubTexture(const Ref<Texture>& texture, const glm::vec4& texCoords);

		void SetCoords(const glm::vec4& texCoords) { std::cout << "set"; m_TexCoords = texCoords; };
		void SetCoordsWithSize(const glm::vec2& coords,const glm::vec2& size);
		void SetTexture(const Ref<Texture>& texture) { m_Texture = texture; };
		const glm::vec4& GetTexCoords() const { return m_TexCoords; }

		const Ref<Texture>& GetTexture() const { return m_Texture; }

	private:
		Ref<Texture> m_Texture;
		glm::vec4 m_TexCoords;
	};
}