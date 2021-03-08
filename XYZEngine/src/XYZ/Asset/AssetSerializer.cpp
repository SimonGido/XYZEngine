#include "stdafx.h"
#include "AssetSerializer.h"

#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Scene/Scene.h"

#include "XYZ/Scene/SceneSerializer.h"
#include "XYZ/Utils/FileSystem.h"
#include "AssetManager.h"

#include <yaml-cpp/yaml.h>


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
	static YAML::Emitter& ToVec2(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}
	
	static YAML::Emitter& ToVec3(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}
	
	static YAML::Emitter& ToVec4(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	enum class FieldType
	{
		Float,
		Float2,
		Float3,
		Float4,
		Int,
		None
	};
	static bool HasExtension(const std::string& path, const char* extension)
	{
		auto lastDot = path.rfind('.');
		auto count = path.size() - lastDot;

		std::string_view view(path.c_str() + lastDot + 1, count);
		if (!view.compare(0, view.size() - 1, extension))
			return true;
		return false;
	}
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
	static void SerializeUniformList(YAML::Emitter& out, const uint8_t* buffer, const std::vector<Uniform>& uniformList)
	{
		for (auto& uniform : uniformList)
		{
			switch (uniform.DataType)
			{
			case UniformDataType::FLOAT:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(float*)&buffer[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::VEC2:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value;
				ToVec2(out, *(glm::vec2*)&buffer[uniform.Offset]);
				out << YAML::EndMap;
				break;
			case UniformDataType::VEC3:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value;
				ToVec3(out, *(glm::vec3*)&buffer[uniform.Offset]);
				out << YAML::EndMap;
				break;
			case UniformDataType::VEC4:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value;
				ToVec4(out, *(glm::vec4*)&buffer[uniform.Offset]);
				out << YAML::EndMap;
				break;
			case UniformDataType::INT:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(int*)&buffer[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::MAT4:
				break;
			};
		}
	}



	template<>
	void AssetSerializer::serialize<Texture2D>(const Ref<Asset>& asset)
	{
		XYZ_ASSERT(!asset->FilePath.empty(), "Filepath is empty");
		Ref<Texture2D> texture = Ref<Texture2D>((Texture2D*)asset.Raw());
		
		YAML::Emitter out;
		out << YAML::BeginMap;
		
		out << YAML::Key << "Texture" << YAML::Value << asset->FileName;
		out << YAML::Key << "Wrap" << YAML::Value << ToUnderlying(texture->GetSpecification().Wrap);
		out << YAML::Key << "Param Min" << YAML::Value << ToUnderlying(texture->GetSpecification().MinParam);
		out << YAML::Key << "Param Max" << YAML::Value << ToUnderlying(texture->GetSpecification().MagParam);
		
		std::ofstream fout(asset->FilePath);
		fout << out.c_str();
	}


	template <>
	void AssetSerializer::serialize<SubTexture>(const Ref<Asset>& asset)
	{
		XYZ_ASSERT(!asset->FilePath.empty(), "Filepath is empty");
		Ref<SubTexture> subTexture = Ref<SubTexture>((SubTexture*)asset.Raw());
		
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "SubTexture" << YAML::Value << subTexture->FileName;
		out << YAML::Key << "TextureAsset" << YAML::Value << subTexture->GetTexture()->Handle;
		out << YAML::Key << "TexCoords" << YAML::Value;
		ToVec4(out, subTexture->GetTexCoords());
		
		out << YAML::EndMap;
		
		std::ofstream fout(subTexture->FilePath);
		fout << out.c_str();
	}

	template <>
	void AssetSerializer::serialize<Shader>(const Ref<Asset>& asset)
	{
		XYZ_ASSERT(!asset->FilePath.empty(), "Filepath is empty");
		Ref<Shader> shader = Ref<Shader>((Shader*)asset.Raw());
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Shader" << YAML::Value << asset->FileName;
		out << YAML::Key << "FilePath" << YAML::Value << shader->GetPath();
		std::ofstream fout(asset->FilePath);
		fout << out.c_str();
	}

	template <>
	void AssetSerializer::serialize<Material>(const Ref<Asset>& asset)
	{
		XYZ_ASSERT(!asset->FilePath.empty(), "Filepath is empty");
		Ref<Material> material = Ref<Material>((Material*)asset.Raw());

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Material" << YAML::Value << asset->FileName;
		out << YAML::Key << "ShaderAsset" << YAML::Value << material->GetShader()->Handle;

		out << YAML::Key << "Textures";
		out << YAML::Value << YAML::BeginSeq;
		uint32_t counter = 0;
		for (auto& texture : material->GetTextures())
		{
			if (!texture.Raw())
			{
				counter++;
				continue;
			}
			out << YAML::BeginMap;
			out << YAML::Key << "TextureAsset" << YAML::Value << texture->Handle; 
			out << YAML::Key << "TextureIndex" << YAML::Value << counter++;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::Key << "Values";
		out << YAML::Value << YAML::BeginSeq;

		SerializeUniformList(out, material->GetVSUniformBuffer(), material->GetShader()->GetVSUniformList().Uniforms);
		SerializeUniformList(out, material->GetFSUniformBuffer(), material->GetShader()->GetFSUniformList().Uniforms);

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(material->FilePath);
		fout << out.c_str();
	}


	template <>
	void AssetSerializer::serialize<Font>(const Ref<Asset>& asset)
	{
		XYZ_ASSERT(!asset->FilePath.empty(), "Filepath is empty");
		Ref<Font> font = Ref<Font>((Font*)asset.Raw());
		YAML::Emitter out;
		out << YAML::BeginMap; // Font
		
		out << YAML::Key << "Font" << YAML::Value << font->FileName;
		out << YAML::Key << "FontPath" << YAML::Value << font->GetFilepath();
		out << YAML::Key << "PixelSize" << YAML::Value << font->GetPixelsize();

		out << YAML::EndMap; // Font

		std::ofstream fout(asset->FilePath);
		fout << out.c_str();
	}

	template <>
	void AssetSerializer::serialize<Scene>(const Ref<Asset>& asset)
	{
		XYZ_ASSERT(!asset->FilePath.empty(), "Filepath is empty");
		Ref<Scene> scene = Ref<Scene>((Scene*)asset.Raw());
		
		SceneSerializer sceneSerializer(scene);
		sceneSerializer.Serialize();
	}

	template <>
	Ref<Asset> AssetSerializer::deserialize<Texture2D>(Ref<Asset> asset)
	{
		std::ifstream stream(asset->FilePath);
		TextureSpecs specs;
		if (stream.is_open())
		{
			std::stringstream strStream;
			strStream << stream.rdbuf();
			YAML::Node data = YAML::Load(strStream.str());
	
			XYZ_ASSERT(data["Texture"], "Incorrect file format");
			specs.Wrap = IntToTextureWrap(data["Wrap"].as<int>());
			specs.MinParam = IntToTextureParam(data["Param Min"].as<int>());
			specs.MagParam = IntToTextureParam(data["Param Max"].as<int>());
		}
		else
		{
			XYZ_LOG_WARN("Missing texture meta data, setting default");
		}
	
		auto texture = Texture2D::Create(specs, asset->FilePath);
		return texture.As<Asset>();
	}

	template <>
	Ref<Asset> AssetSerializer::deserialize<SubTexture>(Ref<Asset> asset)
	{
		std::ifstream stream(asset->FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		XYZ_ASSERT(data["SubTexture"], "Incorrect file format ");
		GUID textureHandle(data["TextureAsset"].as<std::string>());

		auto texture = AssetManager::GetAsset<Texture2D>(textureHandle);
		glm::vec4 texCoords = data["TexCoords"].as<glm::vec4>();

		auto ref = Ref<SubTexture>::Create(texture, texCoords);
		return ref.As<SubTexture>();
	}

	template <>
	Ref<Asset> AssetSerializer::deserialize<Shader>(Ref<Asset> asset)
	{
		std::ifstream stream(asset->FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		return Shader::Create(data["FilePath"].as<std::string>());
	}

	template <>
	Ref<Asset> AssetSerializer::deserialize<Material>(Ref<Asset> asset)
	{
		std::ifstream stream(asset->FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		GUID shaderHandle(data["ShaderAsset"].as<std::string>());
		auto shader = AssetManager::GetAsset<Shader>(shaderHandle);

		Ref<Material> material = Ref<Material>::Create(shader);

		for (auto& seq : data["Textures"])
		{
			GUID textureHandle(seq["TextureAsset"].as<std::string>());
			uint32_t index = seq["TextureIndex"].as<uint32_t>();
			Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(textureHandle);
			material->Set("u_Texture", texture, index);
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
		return material;
	}
	template <>
	Ref<Asset> AssetSerializer::deserialize<Font>(Ref<Asset> asset)
	{
		std::ifstream stream(asset->FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		XYZ_ASSERT(data["Font"], "Incorrect file format");
		uint32_t pixelSize = data["PixelSize"].as<uint32_t>();
		std::string fontPath = data["FontPath"].as<std::string>();

		auto ref = Ref<Font>::Create(pixelSize, fontPath);
		return ref.As<Font>();
	}

	template <>
	Ref<Asset> AssetSerializer::deserialize<Scene>(Ref<Asset> asset)
	{
		Ref<Scene> result;
		SceneSerializer sceneSerializer(result);
		sceneSerializer.Deserialize();

		return result;
	}

	Ref<Asset> AssetSerializer::LoadAssetMeta(const std::string& filepath, const GUID& directoryHandle, AssetType type)
	{
		Ref<Asset> asset = Ref<Asset>::Create();

		std::string extension = Utils::GetExtension(filepath);
		asset->FilePath = filepath;
		std::replace(asset->FilePath.begin(), asset->FilePath.end(), '\\', '/');

		bool hasMeta = FileSystem::Exists(asset->FilePath + ".meta");
		if (hasMeta)
		{
			AssetSerializer::loadMetaFile(asset);
		}
		else
		{
			asset->Handle = GUID();
			asset->Type = type;
		}

		asset->FileExtension = extension;
		asset->FileName = Utils::RemoveExtension(Utils::GetFilename(filepath));
		asset->DirectoryHandle = directoryHandle;
		asset->IsLoaded = false;

		if (!hasMeta)
			createMetaFile(asset);

		return asset;

	}
	Ref<Asset> AssetSerializer::LoadAssetData(Ref<Asset>& asset)
	{
		switch (asset->Type)
		{
		case AssetType::Scene:
			return deserialize<Scene>(asset);
		case AssetType::Texture:
			return deserialize<Texture2D>(asset);
		case AssetType::SubTexture:
			return deserialize<SubTexture>(asset);
		case AssetType::Material:
			return deserialize<Material>(asset);
		case AssetType::Shader:
			return deserialize<Shader>(asset);
		case AssetType::Font:
			return deserialize<Font>(asset);
		}
	}
	void AssetSerializer::loadMetaFile(Ref<Asset> asset)
	{
		std::ifstream stream(asset->FilePath + ".meta");
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		XYZ_ASSERT(data["Asset"], "Invalid File Format");

		asset->Handle = GUID(data["Asset"].as<std::string>());
		asset->FilePath = data["FilePath"].as<std::string>();
		asset->Type = (AssetType)data["Type"].as<int>();
	}
	void AssetSerializer::createMetaFile(Ref<Asset> asset)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Asset" << YAML::Value << (std::string)asset->Handle;
		out << YAML::Key << "FilePath" << YAML::Value << asset->FilePath;
		out << YAML::Key << "Type" << YAML::Value << (int)asset->Type;
		out << YAML::EndMap;

		std::ofstream fout(asset->FilePath + ".meta");
		fout << out.c_str();

	}
}