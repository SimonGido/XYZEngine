#pragma once
#include "Texture.h"
#include "XYZ/Asset/Asset.h"

#include <glm/glm.hpp>

namespace XYZ {

	class SubTexture : public Asset
	{
	public:
		SubTexture(const Ref<Texture>& texture, const glm::vec4& texCoords);
		SubTexture(const Ref<Texture>& texture);

		void SetTexture(const Ref<Texture>& texture) { m_Texture = texture; };
		void SetTexCoords(const glm::vec4& texCoords);
		void Upside();

		const glm::vec4& GetTexCoords() const;
		glm::vec4 GetTexCoordsUpside() const;

		const Ref<Texture>& GetTexture() const { return m_Texture; }

	private:
		Ref<Texture> m_Texture;
		glm::vec4 m_TexCoords;
	};
}