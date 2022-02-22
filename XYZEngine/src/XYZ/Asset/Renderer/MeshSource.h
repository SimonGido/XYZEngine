#pragma once
#include "XYZ/Asset/Asset.h"
#include "XYZ/Asset/Animation/SkeletonAsset.h"

#include "XYZ/Renderer/Buffer.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

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
		float Weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		void AddBoneData(uint32_t boneID, float weight);
	};

	struct BoneInfo
	{
		glm::mat4 BoneOffset;
		uint32_t  JointIndex;
	};


	class MeshSource : public Asset
	{
	public:
		MeshSource(const std::string& filepath);
		MeshSource(const aiScene* scene, const std::string& filepath);
		MeshSource(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		MeshSource(std::vector<AnimatedVertex> vertices, std::vector<uint32_t> indices);

		const std::vector<AnimatedVertex>& GetAnimatedVertices() const { return m_AnimatedVertices; }
		const std::vector<Vertex>&		   GetVertices() const { return m_StaticVertices; }
		const std::vector<uint32_t>&	   GetIndices() const { return m_Indices; }
		
		const std::unordered_map<std::string, uint32_t> GetBoneMapping() const { return m_BoneMapping; }
		const std::vector<BoneInfo>						GetBoneInfo() const { return m_BoneInfo; }
		const std::string& GetSourceFilePath()   const { return m_SourceFilePath; }
		const glm::mat4&   GetInverseTransform() const { return m_InverseTransform; }
		const glm::mat4&   GetTransform()		 const { return m_Transform; }
		const aiScene*	   GetScene()			 const { return m_Scene; }
		bool			   IsAnimated()			 const { return m_IsAnimated; }


		Ref<VertexBuffer>   GetVertexBuffer() const { return m_VertexBuffer; }
		Ref<IndexBuffer>    GetIndexBuffer()  const { return m_IndexBuffer; }

		static AssetType	GetStaticType() { return AssetType::MeshSource; }

	private:		
		void loadSkeleton(const aiScene* scene);
		void loadMeshes(const aiScene* scene);
		void loadBoneInfo(const aiScene* scene);
		void traverseNodes(aiNode* node, const glm::mat4& parentTransform);

		uint32_t findJointIndex(const std::string& name) const;
	private:
		std::string						  m_SourceFilePath;
		std::unique_ptr<Assimp::Importer> m_Importer;

		std::vector<AnimatedVertex> m_AnimatedVertices;
		std::vector<Vertex>			m_StaticVertices;
		std::vector<uint32_t>		m_Indices;

		ozz::unique_ptr<ozz::animation::Skeleton> m_Skeleton;
		std::unordered_map<std::string, uint32_t> m_BoneMapping;
		glm::mat4								  m_InverseTransform;
		const aiScene*							  m_Scene;
		bool									  m_IsAnimated;
		
		Ref<VertexBuffer>	  m_VertexBuffer;
		Ref<IndexBuffer>	  m_IndexBuffer;

		std::vector<BoneInfo> m_BoneInfo;
		uint32_t			  m_BoneCount = 0;

		glm::mat4			  m_Transform;
	};
}