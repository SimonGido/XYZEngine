#include "stdafx.h"
#include "MeshFactory.h"


namespace XYZ {


	Ref<Mesh> MeshFactory::CreateSprite(const glm::vec4& color, const glm::vec4& texCoord,int32_t textureID)
	{
		Ref<Mesh> mesh = Ref<Mesh>::Create();
		mesh->TextureID = textureID;
		Vertex vertices[4];
		vertices[0].Position = {- (1.0f / 2.0f),- (1.0f / 2.0f), 0.0f, 1.0f };
		vertices[1].Position = {  (1.0f / 2.0f),- (1.0f / 2.0f), 0.0f, 1.0f };
		vertices[2].Position = {  (1.0f / 2.0f),  (1.0f / 2.0f), 0.0f, 1.0f };
		vertices[3].Position = {- (1.0f / 2.0f),  (1.0f / 2.0f), 0.0f, 1.0f };

		vertices[0].Color = color;
		vertices[1].Color = color;
		vertices[2].Color = color;
		vertices[3].Color = color;

		vertices[0].TexCoord = { texCoord.x,texCoord.y };
		vertices[1].TexCoord = { texCoord.z,texCoord.y };
		vertices[2].TexCoord = { texCoord.z,texCoord.w };
		vertices[3].TexCoord = { texCoord.x,texCoord.w };

		for (auto& vertex : vertices)
			mesh->Vertices.push_back(vertex);
		
		mesh->Indices.resize(6);

		return mesh;
	}
	
}