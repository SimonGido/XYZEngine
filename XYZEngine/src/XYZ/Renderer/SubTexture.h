#pragma once
#include "Texture.h"
#include "XYZ/Asset/Asset.h"

#include <glm/glm.hpp>

namespace XYZ {

	class SubTexture : public Asset
	{
	public:
		SubTexture(const Ref<Texture>& texture, const glm::vec2& coords, const glm::vec2& size);
		SubTexture(const Ref<Texture>& texture, const glm::vec2& coords);

		void SetCoords(const glm::vec2& coords, const glm::vec2& size);
		void SetTexture(const Ref<Texture>& texture) { m_Texture = texture; };
		
		const glm::vec4& GetTexCoords() const;
		glm::vec4 GetTexCoordsUpside() const;
		const glm::vec2& GetCoords() const { return m_Coords; }
		const glm::vec2& GetSize() const { return m_Size; }

		const Ref<Texture>& GetTexture() const { return m_Texture; }
	private:
		void calculatePixelCorrectedTexCoords();

	private:
		Ref<Texture> m_Texture;
		glm::vec2 m_Coords;
		glm::vec2 m_Size;
		glm::vec4 m_TexCoords;
	};
}