#pragma once
#include <glm/glm.hpp>

#include "XYZ/Renderer/SubTexture.h"

namespace XYZ {

	struct bUIQuad
	{
		glm::vec4 Color;
		glm::vec4 TexCoord;
		glm::vec3 Position;
		glm::vec2 Size;
		uint32_t  TextureID;
	};

	struct bUILine
	{
		glm::vec4 Color;
		glm::vec3 P0;
		glm::vec3 P1;
	};

	struct bUIMesh
	{
		std::vector<bUIQuad> Quads;
		std::vector<bUILine> Lines;
	};

	class bUIRenderer
	{
	public:
		void Begin();
		
		template <typename T>
		void Submit(const T& element, const Ref<SubTexture>& subTexture);


		const bUIMesh& GetMesh()const { return m_Mesh; }
	private:
		bUIMesh m_Mesh;
	};
}