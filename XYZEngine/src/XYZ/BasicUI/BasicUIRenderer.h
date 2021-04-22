#pragma once
#include <glm/glm.hpp>

#include "XYZ/Renderer/Buffer.h"
#include "XYZ/Renderer/SubTexture.h"

namespace XYZ {

	struct bUIQuad
	{
		glm::vec4 Color;
		glm::vec4 TexCoord;
		glm::vec3 Position;
		glm::vec2 Size;
		uint32_t  TextureID;
		uint32_t  ScissorID;
	};

	struct bUILine
	{
		glm::vec4 Color;
		glm::vec3 P0;
		glm::vec3 P1;
	};

	struct bUIScissor
	{
		float X;
		float Y;
		float Width;
		float Height;
	};

	struct bUIMesh
	{
		std::vector<bUIQuad>	Quads;
		std::vector<bUILine>	Lines;
		std::vector<bUIScissor> Scissors;
	};

	class bUIRenderer
	{
	public:
		void Begin();
		void BindCustomTextures();
		void UpdateScissorBuffer(Ref<ShaderStorageBuffer> scissorBuffer);

		template <typename T, typename ...Args>
		void Submit(const T& element, uint32_t& scissorID, const Args& ...);


		const bUIMesh& GetMesh()const { return m_Mesh; }

	private:
		bUIMesh m_Mesh;
		std::vector<Ref<Texture2D>> m_CustomTextures;
	};
}