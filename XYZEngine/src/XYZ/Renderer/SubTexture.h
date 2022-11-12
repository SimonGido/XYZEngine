#pragma once
#include "Texture.h"
#include "XYZ/Asset/Asset.h"

#include <glm/glm.hpp>

namespace XYZ {

	class XYZ_API SubTexture : public Asset
	{
	public:
		SubTexture(const Ref<Texture2D>& texture, const glm::vec4& texCoords);
		SubTexture(const Ref<Texture2D>& texture);

		void SetTexture(const Ref<Texture2D>& texture) { m_Texture = texture; };
		void SetTexCoords(const glm::vec4& texCoords);
		void Upside();

		const glm::vec4& GetTexCoords() const;
		glm::vec4 GetTexCoordsUpside() const;

		const Ref<Texture2D>& GetTexture() const { return m_Texture; }

		static AssetType GetStaticType() { return AssetType::SubTexture; }
	private:
		Ref<Texture2D> m_Texture;
		glm::vec4 m_TexCoords;
	};
}