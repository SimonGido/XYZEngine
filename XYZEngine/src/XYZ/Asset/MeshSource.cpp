#include "stdafx.h"
#include "MeshSource.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

#include <glm/glm.hpp>

namespace XYZ {
	namespace Utils {

		glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4& matrix)
		{
			glm::mat4 result;
			//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
			result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
			result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
			result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
			result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
			return result;
		}

	}

	static const uint32_t s_MeshImportFlags =
		aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
		aiProcess_Triangulate |             // Make sure we're triangles
		aiProcess_SortByPType |             // Split meshes by primitive type
		aiProcess_GenNormals |              // Make sure we have legit normals
		aiProcess_GenUVCoords |             // Convert UVs if required 
		aiProcess_OptimizeMeshes |          // Batch draws where possible
		aiProcess_JoinIdenticalVertices |
		aiProcess_GlobalScale |             // e.g. convert cm to m for fbx import (and other formats where cm is native)
		aiProcess_ValidateDataStructure;    // Validation

	struct LogStream : public Assimp::LogStream
	{
		static void Initialize()
		{
			if (Assimp::DefaultLogger::isNullLogger())
			{
				Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
				Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
			}
		}

		virtual void write(const char* message) override
		{
			XYZ_ERROR("Assimp error: {0}", message);
		}
	};

	MeshSource::MeshSource(const std::string& filepath)
		:
		m_SourceFilePath(filepath)
	{
		LogStream::Initialize();
		m_Importer = std::make_unique<Assimp::Importer>();
		const aiScene* scene = m_Importer->ReadFile(m_SourceFilePath, s_MeshImportFlags);
		if (!scene || !scene->HasMeshes() || scene->mNumMeshes > 1)
		{
			XYZ_ERROR("Failed to load mesh file: {0}", m_SourceFilePath);
			SetFlag(AssetFlag::Invalid);
			return;
		}

		m_Scene = scene;

		m_IsAnimated = scene->mAnimations != nullptr;
		m_IsAnimated = false;
		m_InverseTransform = glm::inverse(Utils::Mat4FromAssimpMat4(scene->mRootNode->mTransformation));
		loadMeshes(m_Scene);
		loadBones(m_Scene);

		if (m_IsAnimated)
			m_VertexBuffer = VertexBuffer::Create(m_AnimatedVertices.data(), static_cast<uint32_t>(m_AnimatedVertices.size() * sizeof(AnimatedVertex)));
		else
			m_VertexBuffer = VertexBuffer::Create(m_StaticVertices.data(), static_cast<uint32_t>(m_StaticVertices.size() * sizeof(Vertex)));
		
		m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), static_cast<uint32_t>(m_Indices.size()));
	}
	MeshSource::MeshSource(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
		:
		m_StaticVertices(std::move(vertices)),
		m_Indices(std::move(indices)),
		m_InverseTransform(1.0f),
		m_Scene(nullptr),
		m_IsAnimated(false)
	{
		m_VertexBuffer = VertexBuffer::Create(m_StaticVertices.data(), static_cast<uint32_t>(m_StaticVertices.size() * sizeof(Vertex)));
		m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), static_cast<uint32_t>(m_Indices.size()));
	}
	MeshSource::MeshSource(std::vector<AnimatedVertex> vertices, std::vector<uint32_t> indices)
		:
		m_AnimatedVertices(std::move(vertices)),
		m_Indices(std::move(indices)),
		m_InverseTransform(1.0f),
		m_Scene(nullptr),
		m_IsAnimated(true)
	{
		m_VertexBuffer = VertexBuffer::Create(m_AnimatedVertices.data(), static_cast<uint32_t>(m_AnimatedVertices.size() * sizeof(AnimatedVertex)));
		m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), static_cast<uint32_t>(m_Indices.size()));
	}
	void MeshSource::loadMeshes(const aiScene* scene)
	{
		for (unsigned m = 0; m < scene->mNumMeshes; m++)
		{
			aiMesh* mesh = scene->mMeshes[m];
			if (m_IsAnimated)
			{
				for (size_t i = 0; i < mesh->mNumVertices; i++)
				{
					AnimatedVertex vertex;
					vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
					if (mesh->HasTextureCoords(0))
						vertex.TexCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

					m_AnimatedVertices.push_back(vertex);
				}
			}
			else
			{
				for (size_t i = 0; i < mesh->mNumVertices; i++)
				{
					Vertex vertex;
					vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
					if (mesh->HasTextureCoords(0))
						vertex.TexCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

					m_StaticVertices.push_back(vertex);
				}
			}
			for (size_t i = 0; i < mesh->mNumFaces; i++)
			{
				XYZ_ASSERT(mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices.");
				m_Indices.push_back(mesh->mFaces[i].mIndices[0]);
				m_Indices.push_back(mesh->mFaces[i].mIndices[1]);
				m_Indices.push_back(mesh->mFaces[i].mIndices[2]);
			}
		}
	}

	void MeshSource::loadBones(const aiScene* scene)
	{
		if (m_IsAnimated)
		{
			for (size_t m = 0; m < scene->mNumMeshes; m++)
			{
				aiMesh* mesh = scene->mMeshes[m];
				for (size_t i = 0; i < mesh->mNumBones; i++)
				{
					aiBone* bone = mesh->mBones[i];
					std::string boneName(bone->mName.data);
					int boneIndex = 0;

					if (m_BoneMapping.find(boneName) == m_BoneMapping.end())
					{
						// Allocate an index for a new bone
						boneIndex = m_BoneCount;
						m_BoneCount++;
						BoneInfo bi;
						m_BoneInfo.push_back(bi);
						m_BoneInfo[boneIndex].BoneOffset = Utils::Mat4FromAssimpMat4(bone->mOffsetMatrix);
						m_BoneMapping[boneName] = boneIndex;
					}
					else
					{
						boneIndex = m_BoneMapping[boneName];
					}

					for (size_t j = 0; j < bone->mNumWeights; j++)
					{
						int VertexID = bone->mWeights[j].mVertexId;
						float Weight = bone->mWeights[j].mWeight;
						m_AnimatedVertices[VertexID].AddBoneData(boneIndex, Weight);
					}
				}
			}
		}
	}

	void AnimatedVertex::AddBoneData(uint32_t boneID, float weight)
	{
		for (size_t i = 0; i < 4; i++)
		{
			if (Weights[i] == 0.0)
			{
				IDs[i] = boneID;
				Weights[i] = weight;
				return;
			}
		}
		XYZ_WARN("Vertex has more than four bones/weights affecting it, extra data will be discarded (BoneID={0}, Weight={1})", boneID, weight);
	}

}