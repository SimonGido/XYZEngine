#include "stdafx.h"
#include "MeshSource.h"

#include "XYZ/Animation/Animation.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>



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
		m_InverseTransform = glm::inverse(Utils::Mat4FromAssimpMat4(scene->mRootNode->mTransformation));
		loadMeshes(m_Scene);
		loadBones(m_Scene);

		if (m_IsAnimated)
		{
			loadAnimations(m_Scene);		
			m_VertexBuffer = VertexBuffer::Create(m_AnimatedVertices.data(), static_cast<uint32_t>(m_AnimatedVertices.size() * sizeof(AnimatedVertex)));
		}
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
	const std::vector<Ref<Animation>>& MeshSource::GetAnimations() const
	{
		return m_Animations;
	}
	SceneEntity MeshSource::CreateBoneHierarchy(Ref<Scene> scene)
	{
		SceneEntity root;
		createNodeHierarchy(m_Scene->mRootNode, SceneEntity(), scene, root);
		return root;
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

	void MeshSource::loadAnimations(const aiScene* scene)
	{
		for (uint32_t i = 0; i < scene->mNumAnimations; ++i)
		{
			const aiAnimation* aiAnim = scene->mAnimations[i];
			Ref<Animation> animation = m_Animations.emplace_back(Ref<Animation>::Create());
			readNodeHierarchy(scene->mRootNode, aiAnim, animation, "");
			readNodeHierarchyTransforms(scene->mRootNode, aiAnim, glm::mat4(1.0f));
		}
	}

	void MeshSource::readNodeHierarchy(const aiNode* node, const aiAnimation* aiAnim, Ref<Animation>& animation, const std::string& parentPath)
	{
		std::string name(node->mName.data);
		glm::mat4 nodeTransform(Utils::Mat4FromAssimpMat4(node->mTransformation));
		const aiNodeAnim* nodeAnim = findNodeAnim(aiAnim, name);

		std::string path;
		if (m_BoneMapping.find(name) != m_BoneMapping.end())
		{
			path = name;

			if (!parentPath.empty())
				path = parentPath + "/" + path;

			if (nodeAnim)
			{
				if (nodeAnim->mNumPositionKeys > 1)
				{
					auto prop = animation->AddProperty<BoneComponent, glm::vec3, 0>(path);
					for (uint32_t key = 0; key < nodeAnim->mNumPositionKeys; ++key)
					{
						const uint32_t frame = nodeAnim->mPositionKeys[key].mTime / animation->GetFrameLength();
						const auto val = nodeAnim->mPositionKeys[key].mValue;
						glm::vec3 value = { val.x , val.y , val.z };
						prop->AddKeyFrame({ value, frame });
					}
				}
				if (nodeAnim->mNumRotationKeys > 1)
				{
					auto prop = animation->AddProperty<BoneComponent, glm::quat, 1>(path);
					for (uint32_t key = 0; key < nodeAnim->mNumRotationKeys; ++key)
					{
						const uint32_t frame = nodeAnim->mRotationKeys[key].mTime / animation->GetFrameLength();
						auto rotV = nodeAnim->mRotationKeys[key].mValue;
						glm::quat value = glm::quat(rotV.w, rotV.x, rotV.y, rotV.z);
						prop->AddKeyFrame({ value, frame });
					}
				}
				if (nodeAnim->mNumPositionKeys > 1)
				{
					auto prop = animation->AddProperty<BoneComponent, glm::vec3, 2>(path);
					for (uint32_t key = 0; key < nodeAnim->mNumScalingKeys; ++key)
					{
						const uint32_t frame = nodeAnim->mScalingKeys[key].mTime / animation->GetFrameLength();
						const auto val = nodeAnim->mScalingKeys[key].mValue;
						glm::vec3 value = { val.x , val.y , val.z };
						prop->AddKeyFrame({ value, frame });
					}
				}
			}
		}
		if (node != m_Scene->mRootNode)
		{
			for (uint32_t i = 0; i < node->mNumChildren; i++)
				readNodeHierarchy(node->mChildren[i], aiAnim, animation, path);
		}
		else
		{
			for (uint32_t i = 0; i < node->mNumChildren; i++)
				readNodeHierarchy(node->mChildren[i], aiAnim, animation, "");
		}
	}

	void MeshSource::readNodeHierarchyTransforms(const aiNode* node, const aiAnimation* aiAnim, const glm::mat4& parentTransform)
	{
		std::string name(node->mName.data);
		glm::mat4 nodeTransform(Utils::Mat4FromAssimpMat4(node->mTransformation));
		const aiNodeAnim* nodeAnim = findNodeAnim(aiAnim, name);
		
		glm::vec3 translation{}, scale{};
		glm::quat rotation{};
		
		if (nodeAnim)
		{		
			auto transV = nodeAnim->mPositionKeys[0].mValue;
			auto rotV = nodeAnim->mRotationKeys[0].mValue;
			auto scaleV = nodeAnim->mScalingKeys[0].mValue;

			translation = { transV.x, transV.y, transV.z };
			rotation = { rotV.w, rotV.x, rotV.y, rotV.z };
			scale = { scaleV.x, scaleV.y, scaleV.z };
			nodeTransform = glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), scale);
		}
		glm::mat4 transform = parentTransform * nodeTransform;

		if (m_BoneMapping.find(name) != m_BoneMapping.end() && nodeAnim)
		{
			uint32_t BoneIndex = m_BoneMapping[name];
			m_BoneInfo[BoneIndex].Translation = translation;
			m_BoneInfo[BoneIndex].Rotation = rotation;
			m_BoneInfo[BoneIndex].Scale = scale;

			m_BoneInfo[BoneIndex].Transformation = nodeTransform * m_BoneInfo[BoneIndex].BoneOffset;
			m_BoneInfo[BoneIndex].FinalTransformation = m_InverseTransform * transform * m_BoneInfo[BoneIndex].BoneOffset;
		}
		for (uint32_t i = 0; i < node->mNumChildren; i++)
			readNodeHierarchyTransforms(node->mChildren[i], aiAnim, transform);
	}

	void MeshSource::createNodeHierarchy(const aiNode* node, SceneEntity parentEntity, Ref<Scene>& scene, SceneEntity& root)
	{
		std::string name(node->mName.data);
		glm::mat4 nodeTransform(Utils::Mat4FromAssimpMat4(node->mTransformation));
		SceneEntity newEntity;

		if (m_BoneMapping.find(name) != m_BoneMapping.end())
		{
			if (parentEntity.IsValid())
			{
				newEntity = scene->CreateEntity(name, parentEntity, GUID());
			}
			else
			{
				newEntity = scene->CreateEntity(name, GUID());
				root = newEntity;
			}
			auto& boneComponent = newEntity.EmplaceComponent<BoneComponent>();
			uint32_t BoneIndex = m_BoneMapping[name];
			boneComponent.DecomposeTransform(m_BoneInfo[BoneIndex].Transformation);
			boneComponent.Scale = m_BoneInfo[BoneIndex].Scale;
			boneComponent.BoneIndex = BoneIndex;
			boneComponent.WorldTransform = m_BoneInfo[BoneIndex].FinalTransformation;
		}
		
		for (uint32_t i = 0; i < node->mNumChildren; i++)
			createNodeHierarchy(node->mChildren[i], newEntity, scene, root);
	}

	const aiNodeAnim* MeshSource::findNodeAnim(const aiAnimation* animation, const std::string& nodeName) const
	{
		for (uint32_t i = 0; i < animation->mNumChannels; i++)
		{
			const aiNodeAnim* nodeAnim = animation->mChannels[i];
			if (std::string(nodeAnim->mNodeName.data) == nodeName)
				return nodeAnim;
		}
		return nullptr;
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