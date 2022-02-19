#pragma once
#include "Asset.h"

#include "XYZ/Renderer/Buffer.h"

#include <glm/glm.hpp>

struct aiNode;
struct aiAnimation;
struct aiNodeAnim;
struct aiScene;

namespace Assimp {
	class Importer;
}


namespace XYZ {
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
	};

	struct AnimatedVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
		uint32_t IDs[4] = { 0,0,0,0 };
		float Weights[4] = { 1.0f, 0.0f, 0.0f, 0.0f };

		void AddBoneData(uint32_t boneID, float weight);
	};
	struct BoneInfo
	{
		glm::mat4 BoneOffset;
		glm::mat4 FinalTransformation;
	};
	class MeshSource : public Asset
	{
	public:
		MeshSource(const std::string& filepath);
		MeshSource(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		MeshSource(std::vector<AnimatedVertex> vertices, std::vector<uint32_t> indices);

		const std::vector<AnimatedVertex>& GetAnimatedVertices() const { return m_AnimatedVertices; }
		const std::vector<Vertex>&		   GetVertices() const { return m_StaticVertices; }
		const std::vector<uint32_t>&	   GetIndices() const { return m_Indices; }

		const std::unordered_map<std::string, uint32_t> GetBoneMapping() const { return m_BoneMapping; }
		const std::vector<glm::mat4>					GetBoneTransforms() const { return m_BoneTransforms; }
		
		const std::string& GetSourceFilePath()   const { return m_SourceFilePath; }
		const glm::mat4&   GetInverseTransform() const { return m_InverseTransform; }
		bool			   IsAnimated()			 const { return m_IsAnimated; }


		Ref<VertexBuffer>   GetVertexBuffer() const { return m_VertexBuffer; }
		Ref<IndexBuffer>    GetIndexBuffer()  const { return m_IndexBuffer; }

		static AssetType	GetStaticType() { return AssetType::MeshSource; }

	private:
		void loadMeshes(const aiScene* scene);
		void loadBones(const aiScene* scene);

	private:
		std::string m_SourceFilePath;
		std::unique_ptr<Assimp::Importer> m_Importer;

		std::vector<AnimatedVertex> m_AnimatedVertices;
		std::vector<Vertex>			m_StaticVertices;
		std::vector<uint32_t>		m_Indices;

		std::unordered_map<std::string, uint32_t> m_BoneMapping;
		std::vector<glm::mat4>					  m_BoneTransforms;
		glm::mat4								  m_InverseTransform;
		const aiScene*							  m_Scene;
		bool									  m_IsAnimated;
		
		Ref<VertexBuffer>	  m_VertexBuffer;
		Ref<IndexBuffer>	  m_IndexBuffer;

		std::vector<BoneInfo> m_BoneInfo;
		uint32_t			  m_BoneCount = 0;
	};
}