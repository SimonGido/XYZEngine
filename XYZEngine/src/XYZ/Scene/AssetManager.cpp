#include "stdafx.h"
#include "AssetManager.h"

#include "XYZ/ECS/Entity.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/Texture.h"
#include "XYZ/Scene/Scene.h"
#include "XYZ/Renderer/MeshFactory.h"

#include <yaml-cpp/yaml.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::quat>
	{
		static Node encode(const glm::quat& rhs)
		{
			Node node;
			node.push_back(rhs.w);
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::quat& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.w = node[0].as<float>();
			rhs.x = node[1].as<float>();
			rhs.y = node[2].as<float>();
			rhs.z = node[3].as<float>();
			return true;
		}
	};
}


namespace XYZ {
	static enum class FieldType
	{
		Float,
		Float2,
		Float3,
		Float4,
		Int,
		None
	};

	static FieldType FindType(const std::string& str)
	{
		char tokenComma = ',';
		size_t numCommas = std::count(str.begin(), str.end(), tokenComma);
		char tokenDot = '.';
		size_t numDots = std::count(str.begin(), str.end(), tokenDot);

		switch (numCommas)
		{
		case 0:
			if (numDots)
				return FieldType::Float;
			else
				return FieldType::Int;
		case 1:
			return FieldType::Float2;
		case 2:
			return FieldType::Float3;
		case 3:
			return FieldType::Float4;
		}

		XYZ_ASSERT(false, "Wrong type");
		return FieldType::None;
	}

	static TextureWrap IntToTextureWrap(int wrap)
	{
		
		if (wrap == ToUnderlying(TextureWrap::Clamp))
			return TextureWrap::Clamp;
		if (wrap == ToUnderlying(TextureWrap::Repeat))
			return TextureWrap::Repeat;
		

		return TextureWrap::None;
	}

	static TextureParam IntToTextureParam(int param)
	{
		if (param == ToUnderlying(TextureParam::Nearest))
			return TextureParam::Nearest;
		if (param == ToUnderlying(TextureParam::Linear))
			return TextureParam::Linear;
		
		return TextureParam::None;
	}


	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::quat& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.w << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}
	static std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition(const glm::mat4& transform)
	{
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose(transform, scale, orientation, translation, skew, perspective);

		return { translation, orientation, scale };
	}

	template<>
	void Asset<Texture2D>::Serialize(AssetManager& manager)
	{
		auto& texture = Handle;
		XYZ_LOG_INFO("Serializing texture ", texture->GetFilepath());

		YAML::Emitter out;
		out << YAML::BeginMap; // Texture
		out << YAML::Key << "Texture" << YAML::Value << texture->GetName();

		out << YAML::Key << "Wrap" << YAML::Value << ToUnderlying(texture->GetSpecification().Wrap);
		out << YAML::Key << "Param Min" << YAML::Value << ToUnderlying(texture->GetSpecification().MinParam);
		out << YAML::Key << "Param Max" << YAML::Value << ToUnderlying(texture->GetSpecification().MagParam);

		out << YAML::EndMap; // Texture

		std::ofstream fout(texture->GetFilepath() + ".meta");
		fout << out.c_str();
	}


	template<>
	void Asset<Texture2D>::Deserialize(const std::string& filepath, AssetManager& manager)
	{
		XYZ_LOG_INFO("Deserializing texture ",filepath);
		TextureWrap wrap = TextureWrap::None;
		TextureParam min = TextureParam::None;
		TextureParam max = TextureParam::None;

		std::ifstream stream(filepath + ".meta");
		if (stream.is_open())
		{
			std::stringstream strStream;
			strStream << stream.rdbuf();

			YAML::Node data = YAML::Load(strStream.str());

			XYZ_ASSERT(data["Texture"], "Incorrect file format");
			wrap = IntToTextureWrap(data["Wrap"].as<int>());
			min = IntToTextureParam(data["Param Min"].as<int>());
			max = IntToTextureParam(data["Param Max"].as<int>());

		}
		else
		{
			XYZ_LOG_WARN("Missing texture meta data, setting default");
		}

		Handle = Texture2D::Create(wrap, min, max, filepath);
	}

	template <>
	void Asset<SubTexture2D>::Serialize(AssetManager& manager)
	{
		auto subTexture = Handle;
		XYZ_LOG_INFO("Serializing subtexture ", subTexture->GetFilepath());

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "SubTexture" << YAML::Value << subTexture->GetName();
		out << YAML::Key << "TextureAssetPath" << YAML::Value << subTexture->GetTexture()->GetFilepath();
		out << YAML::Key << "TexCoords" << YAML::Value << subTexture->GetTexCoords();
		out << YAML::EndMap;

		std::ofstream fout(subTexture->GetFilepath());
		fout << out.c_str();
	}


	template <>
	void Asset<SubTexture2D>::Deserialize(const std::string& filepath,AssetManager& manager)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());

		XYZ_LOG_INFO("Deserializing subtexture ", filepath);
		XYZ_ASSERT(data["SubTexture"], "Incorrect file format ");

		std::string path = data["TextureAssetPath"].as<std::string>();
		manager.LoadAsset<Texture2D>(path);

		glm::vec4 texCoords = data["TexCoords"].as<glm::vec4>();

		Handle = Ref<SubTexture2D>::Create(manager.GetAsset<Texture2D>(path), texCoords);
	}

	template <>
	void Asset<Material>::Serialize(AssetManager& manager)
	{
		auto& material = Handle;
		XYZ_LOG_INFO("Serializing material ", material->GetFilepath());
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Material" << YAML::Value << material->GetName();
		out << YAML::Key << "ShaderAssetPath" << YAML::Value << material->GetShader()->GetPath();


		out << YAML::Key << "Textures";
		out << YAML::Value << YAML::BeginSeq;
		uint32_t counter = 0;
		for (auto& texture : material->GetTextures())
		{
			out << YAML::BeginMap;
			out << YAML::Key << "TextureAssetPath";
			out << YAML::Value << texture->GetFilepath();
			out << YAML::Key << "TextureIndex";
			out << YAML::Value << counter++;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::Key << "Values";
		out << YAML::Value << YAML::BeginSeq;
		for (auto& uniform : material->GetShader()->GetUniforms())
		{
			switch (uniform.Type)
			{
			case UniformDataType::FLOAT:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(float*)&material->GetBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::FLOAT_VEC2:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(glm::vec2*) & material->GetBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::FLOAT_VEC3:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(glm::vec3*) & material->GetBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::FLOAT_VEC4:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(glm::vec4*) & material->GetBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::INT:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(int*)&material->GetBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::FLOAT_MAT4:
				break;
			};
		}


		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(material->GetFilepath());
		fout << out.c_str();
	}

	template <>
	void Asset<Material>::Deserialize(const std::string& filepath, AssetManager& manager)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());

		XYZ_LOG_INFO("Deserializing material ", filepath);
		XYZ_ASSERT(data["Material"], "Incorrect file format ");



		auto shader = Shader::Create(data["ShaderAssetPath"].as<std::string>());
		Ref<Material> material = Material::Create(shader);

		for (auto& seq : data["Textures"])
		{
			std::string path = seq["TextureAssetPath"].as<std::string>();
			uint32_t index = seq["TextureIndex"].as<uint32_t>();
			manager.LoadAsset<Texture2D>(path);
			material->Set("u_Texture", manager.GetAsset<Texture2D>(path), index);
		}

		for (auto& seq : data["Values"])
		{
			for (auto& val : seq)
			{
				std::stringstream ss;
				ss << val.second;
				auto type = FindType(ss.str());
				switch (type)
				{
				case FieldType::Float:
					material->Set(val.first.as<std::string>(), val.second.as<float>());
					break;
				case FieldType::Float2:
					material->Set(val.first.as<std::string>(), val.second.as<glm::vec2>());
					break;
				case FieldType::Float3:
					material->Set(val.first.as<std::string>(), val.second.as<glm::vec3>());
					break;
				case FieldType::Float4:
					material->Set(val.first.as<std::string>(), val.second.as<glm::vec4>());
					break;
				case FieldType::None:
					break;
				}

			}
		}
		Handle = material;
	}


	static void SerializeEntity(YAML::Emitter& out, AssetManager& manager, Entity entity, int32_t parentIndex)
	{
		if (entity.HasComponent<SceneTagComponent>())
		{
			SceneTagComponent* tag = entity.GetComponent<SceneTagComponent>();
			out << YAML::BeginMap;
			out << YAML::Key << "Entity";
			out << YAML::Value << tag->Name;

			if (parentIndex >= 0)
			{
				out << YAML::Key << "Parent";
				out << YAML::Value << parentIndex;
			}
			if (entity.HasComponent<TransformComponent>())
			{
				out << YAML::Key << "TransformComponent";
				out << YAML::BeginMap;
				auto transform = entity.GetComponent<TransformComponent>();

				auto [pos, rot, scale] = GetTransformDecomposition(*transform);
				out << YAML::Key << "Position" << YAML::Value << pos;
				out << YAML::Key << "Rotation" << YAML::Value << rot;
				out << YAML::Key << "Scale" << YAML::Value << scale;

				out << YAML::EndMap; // TransformComponent
			}
			if (entity.HasComponent<SpriteRenderer>())
			{
				out << YAML::Key << "SpriteRenderer";
				out << YAML::BeginMap; // RenderComponent2D
				auto renderComponent = entity.GetComponent<SpriteRenderer>();
				auto materialPath = renderComponent->Material->GetFilepath();
				if (materialPath.empty())
				{
					manager.RegisterAsset<Material>("Assets/Materials/New_Material.mat", renderComponent->Material);
					materialPath = "Assets/Materials/New_Material.mat";
				}
				else if (!manager.IsRegistered<Material>(materialPath))
				{
					manager.RegisterAsset<Material>(materialPath, renderComponent->Material);
					materialPath = renderComponent->Material->GetFilepath();
				}


				auto subtexturePath = renderComponent->SubTexture->GetFilepath();
				if (subtexturePath.empty())
				{
					manager.RegisterAsset<SubTexture2D>("Assets/SubTextures/New_SubTexture.subtex", renderComponent->SubTexture);
					subtexturePath = "Assets/SubTextures/New_SubTexture.subtex";
				}
				else if (!manager.IsRegistered<SubTexture2D>(subtexturePath))
				{
					manager.RegisterAsset<SubTexture2D>(subtexturePath, renderComponent->SubTexture);
					subtexturePath = renderComponent->SubTexture->GetFilepath();
				}

				out << YAML::Key << "MaterialAssetPath" << YAML::Value << materialPath;
				out << YAML::Key << "SubTextureAssetPath" << YAML::Value << subtexturePath;
				out << YAML::Key << "Color" << YAML::Value << renderComponent->Color;
				out << YAML::Key << "TextureID" << YAML::Value << renderComponent->TextureID;
				out << YAML::Key << "SortLayer" << YAML::Value << renderComponent->SortLayer;
				out << YAML::Key << "IsVisible" << YAML::Value << renderComponent->IsVisible;
				out << YAML::EndMap; // RendererComponent2D
			}

			out << YAML::EndMap; // Entity
		}
	}

	template <>
	void Asset<Scene>::Serialize(AssetManager& manager)
	{
		Ref<Scene> scene = Handle;
		XYZ_LOG_INFO("Serializing scene ", scene->GetFilepath());
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene";
		out << YAML::Value << scene->GetName();

		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		for (auto& object : scene->m_SceneGraph.GetFlatData())
		{
			Entity entity = { object.GetData().Entity,scene.Raw() };
			SerializeEntity(out,manager, entity, object.GetData().Parent);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		std::ofstream fout(scene->GetFilepath());
		fout << out.c_str();
	}

	template <>
	void Asset<Scene>::Deserialize(const std::string& filepath, AssetManager& manager)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		XYZ_ASSERT(data["Scene"], "Incorrect file format");
	

		std::string sceneName = data["Scene"].as<std::string>();
		XYZ_LOG_INFO("Deserializing scene ", sceneName);
		
		Handle = Ref<Scene>::Create(sceneName);

		auto entities = data["Entities"];
		if (entities)
		{
			std::vector<std::pair<Entity,Entity>> deserializedEntities;
			for (auto entity : entities)
			{
				SceneTagComponent tag(entity["Entity"].as<std::string>());

				Entity ent = Handle->CreateEntity(tag);
				auto parent = entity["Parent"];
				if (parent)
				{
					Entity parentEntity(parent.as<uint32_t>(), Handle.Raw());
					deserializedEntities.push_back({ ent, parentEntity });
				}
				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					XYZ_LOG_INFO("Adding transform to entity ", tag.Name);
					auto transform = ent.GetComponent<TransformComponent>();
					glm::vec3 translation = transformComponent["Position"].as<glm::vec3>();
					glm::quat rotation = transformComponent["Rotation"].as<glm::quat>();
					glm::vec3 scale = transformComponent["Scale"].as<glm::vec3>();

					glm::mat4 transformMatrix = glm::translate(glm::mat4(1.0f), translation) *
						glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), scale);
					
					*transform = transformMatrix;
				}
				auto spriteRenderer = entity["SpriteRenderer"];
				if (spriteRenderer)
				{
					XYZ_LOG_INFO("Adding sprite renderer to entity ", tag.Name);
					std::string materialPath = spriteRenderer["MaterialAssetPath"].as<std::string>();
					std::string subtexturePath = spriteRenderer["SubTextureAssetPath"].as<std::string>();
					glm::vec4 color = spriteRenderer["Color"].as<glm::vec4>();
					uint32_t textureID = spriteRenderer["TextureID"].as<uint32_t>();
					uint16_t sortLayer = spriteRenderer["SortLayer"].as<uint16_t>();

					manager.LoadAsset<Material>(materialPath);
					manager.LoadAsset<SubTexture2D>(subtexturePath);
					auto material = manager.GetAsset<Material>(materialPath);
					auto subTexture = manager.GetAsset<SubTexture2D>(subtexturePath);
					ent.EmplaceComponent<SpriteRenderer>(
						material,
						subTexture,
						color,
						textureID,
						sortLayer
					);
				}
			}

			for (auto &entity : deserializedEntities)
			{
				auto [child, parent] = entity;
				Handle->SetParent(parent, child);
			}
		}
	}

	AssetManager::~AssetManager()
	{
		Serialize();
		for (auto storage : m_AssetStorages)
		{
			delete storage.second;
		}
	}
	void AssetManager::Serialize()
	{
		for (auto storage : m_AssetStorages)
		{
			storage.second->Serialize(*this);
		}
	}
}