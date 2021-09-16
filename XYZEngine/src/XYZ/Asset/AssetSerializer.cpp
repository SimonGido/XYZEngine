#include "stdafx.h"
#include "AssetSerializer.h"

#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Renderer/SkeletalMesh.h"
#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Animation/Animation.h"


#include "XYZ/Scene/SceneSerializer.h"
#include "XYZ/Utils/FileSystem.h"
#include "XYZ/Utils/YamlUtils.h"
#include "AssetManager.h"

#include <yaml-cpp/yaml.h>


#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>


namespace YAML {

	template<>
	struct convert<XYZ::TreeNode>
	{
		static Node encode(const XYZ::TreeNode& rhs)
		{
			Node node;
			node.push_back(rhs.ID);
			node.push_back(rhs.Parent);
			node.push_back(rhs.FirstChild);
			node.push_back(rhs.NextSibling);
			node.push_back(rhs.PreviousSibling);
	
			return node;
		}

		static bool decode(const Node& node, XYZ::TreeNode& rhs)
		{
			if (!node.IsSequence() || node.size() != 5)
				return false;

			rhs.ID = node[0].as<int32_t>();
			rhs.Parent = node[1].as<int32_t>();
			rhs.FirstChild = node[2].as<int32_t>();
			rhs.NextSibling = node[3].as<int32_t>();
			rhs.PreviousSibling = node[4].as<int32_t>();

			return true;
		}
	};
	template<>
	struct convert<XYZ::AnimatedVertex>
	{
		static Node encode(const XYZ::AnimatedVertex& rhs)
		{
			Node node;
			node.push_back(rhs.Position.x);
			node.push_back(rhs.Position.y);
			node.push_back(rhs.Position.z);
			node.push_back(rhs.TexCoord.x);
			node.push_back(rhs.TexCoord.y);
			for (size_t i = 0; i < 4; ++i)
			{
				node.push_back(rhs.BoneData.Weights[i]);
				node.push_back(rhs.BoneData.IDs[i]);
			}
		
			return node;
		}

		static bool decode(const Node& node, XYZ::AnimatedVertex& rhs)
		{
			if (!node.IsSequence() || node.size() != 13)
				return false;

			rhs.Position.x = node[0].as<float>();
			rhs.Position.y = node[1].as<float>();
			rhs.Position.z = node[2].as<float>();
			rhs.TexCoord.x = node[3].as<float>();
			rhs.TexCoord.y = node[4].as<float>();

			size_t counter = 5;
			for (size_t i = 0; i < 4; ++i)
			{
				rhs.BoneData.Weights[i] = node[counter++].as<float>();
				rhs.BoneData.IDs[i] = node[counter++].as<uint32_t>();
			}
			
			return true;
		}
	};
}


namespace XYZ {
	YAML::Emitter& operator<<(YAML::Emitter& out, const TreeNode& node)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq
			<< node.ID
			<< node.Parent
			<< node.FirstChild
			<< node.NextSibling
			<< node.PreviousSibling
			<< YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Tree& tree)
	{		
		out << YAML::Value << YAML::BeginSeq;
		for (int32_t i = 0; i < tree.GetFlatNodes().Range(); ++i)
		{
			if (tree.IsNodeValid(i))
			{
				const TreeNode& node = tree.GetFlatNodes()[i];
				out << node;
			}
		}
		out << YAML::EndSeq;
		return out;
	}


	YAML::Emitter& operator<<(YAML::Emitter& out, const AnimatedVertex& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq 
			<< v.Position.x << v.Position.y << v.Position.z 
			<< v.TexCoord.x << v.TexCoord.y
			<< v.BoneData.Weights[0] << v.BoneData.IDs[0]
			<< v.BoneData.Weights[1] << v.BoneData.IDs[1]
			<< v.BoneData.Weights[2] << v.BoneData.IDs[2]
			<< v.BoneData.Weights[3] << v.BoneData.IDs[3]
			<< YAML::EndSeq;
		return out;
	}

	// TODO: Temporary
	static void CopyAsset(Ref<Asset>& target, const Ref<Asset>& source)
	{
		target->DirectoryHandle = source->DirectoryHandle;
		target->FileExtension = source->FileExtension;
		target->FilePath = source->FilePath;
		target->FileName = source->FileName;
		target->Type = source->Type;
		target->Handle = source->Handle;
		target->IsLoaded = source->IsLoaded;
	}
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

	static YAML::Emitter& ToQuat(YAML::Emitter& out, const glm::quat& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.w << v.x << v.y << v.z << YAML::EndSeq;
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
			case UniformDataType::Float:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(float*)&buffer[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::Vec2:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value;
				ToVec2(out, *(glm::vec2*)&buffer[uniform.Offset]);
				out << YAML::EndMap;
				break;
			case UniformDataType::Vec3:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value;
				ToVec3(out, *(glm::vec3*)&buffer[uniform.Offset]);
				out << YAML::EndMap;
				break;
			case UniformDataType::Vec4:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value;
				ToVec4(out, *(glm::vec4*)&buffer[uniform.Offset]);
				out << YAML::EndMap;
				break;
			case UniformDataType::Int:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(int*)&buffer[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::Mat4:
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
		out << YAML::Key << "TextureFilePath" << YAML::Value << texture->GetFilepath();
		out << YAML::Key << "Width" << YAML::Value << texture->GetWidth();
		out << YAML::Key << "Height" << YAML::Value << texture->GetHeight();
		out << YAML::Key << "Channels" << YAML::Value << texture->GetChannels();
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
		out << YAML::Key << "TexCoords" << YAML::Value << subTexture->GetTexCoords();
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
		out << YAML::Key << "ShaderFilePath" << YAML::Value << shader->GetPath();
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
		out << YAML::Key << "RenderQueueID" << YAML::Value << material->GetRenderQueueID();
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
	void AssetSerializer::serialize<SkeletalMesh>(const Ref<Asset>& asset)
	{
		XYZ_ASSERT(!asset->FilePath.empty(), "Filepath is empty");
		Ref<SkeletalMesh> mesh = Ref<SkeletalMesh>((SkeletalMesh*)asset.Raw());
		YAML::Emitter out;
		out << YAML::BeginMap; // Skeletal Mesh

		
		out << YAML::Key << "SkeletalMesh" << YAML::Value << asset->FileName;
		out << YAML::Key << "MaterialAsset" << YAML::Value << mesh->GetMaterial()->Handle;
		out << YAML::Key << "AnimatedVertices" << YAML::Value << mesh->GetVertices();
		out << YAML::Key << "Indices" << YAML::Value << mesh->GetIndicies();
		//out << YAML::Key << "Hierarchy" << YAML::Value << mesh->GetBoneHierarchy();
		//out << YAML::Key << "Bones" << YAML::Value << mesh->GetBones();

		out << YAML::EndMap; // Skeletal Mesh

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
	void AssetSerializer::serialize<Animation>(const Ref<Asset>& asset)
	{
		XYZ_ASSERT(!asset->FilePath.empty(), "Filepath is empty");
		Ref<Animation> anim = Ref<Animation>((Animation*)asset.Raw());
		YAML::Emitter out;
		out << YAML::BeginMap; // Animation
		
		out << YAML::Key << "NumFrames" << YAML::Value << anim->GetNumFrames();
		out << YAML::Key << "FrameLength" << YAML::Value << anim->GetFrameLength();
		out << YAML::Key << "Repeat" << YAML::Value << anim->GetRepeat();

		out << YAML::Key << "Tracks";
		out << YAML::Value << YAML::BeginSeq;
		
		out << YAML::EndSeq;

		out << YAML::EndMap; // Animation

		std::ofstream fout(asset->FilePath);
		fout << out.c_str();	
	}

	template <>
	Ref<Asset> AssetSerializer::deserialize<Texture2D>(const Ref<Asset>& asset)
	{
		std::ifstream stream(asset->FilePath);
		TextureSpecs specs;
		
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());
	
		XYZ_ASSERT(data["Texture"], "Incorrect file format");

		uint32_t width = data["Width"].as<uint32_t>();
		uint32_t height = data["Height"].as<uint32_t>();
		uint32_t channels = data["Channels"].as<uint32_t>();

		specs.Wrap = IntToTextureWrap(data["Wrap"].as<int>());
		specs.MinParam = IntToTextureParam(data["Param Min"].as<int>());
		specs.MagParam = IntToTextureParam(data["Param Max"].as<int>());
			
		std::string textureFilePath = data["TextureFilePath"].as<std::string>();
		Ref<Texture2D> texture;
		if (!textureFilePath.empty())
		{
			texture = Texture2D::Create(specs, textureFilePath);			
		}
		else
		{
			texture = Texture2D::Create(width, height, channels, specs);
		}
		CopyAsset(texture.As<Asset>(), asset);
		return texture.As<Asset>();
	}

	template <>
	Ref<Asset> AssetSerializer::deserialize<SubTexture>(const Ref<Asset>& asset)
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
		CopyAsset(ref.As<Asset>(), asset);
		
		return ref.As<Asset>();
	}

	template <>
	Ref<Asset> AssetSerializer::deserialize<Shader>(const Ref<Asset>& asset)
	{
		std::ifstream stream(asset->FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		Ref<Shader> shader =Shader::Create(data["ShaderFilePath"].as<std::string>());
		CopyAsset(shader.As<Asset>(), asset);
		return shader;
	}

	template <>
	Ref<Asset> AssetSerializer::deserialize<Material>(const Ref<Asset>& asset)
	{
		std::ifstream stream(asset->FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		GUID shaderHandle(data["ShaderAsset"].as<std::string>());
		auto shader = AssetManager::GetAsset<Shader>(shaderHandle);

		Ref<Material> material = Ref<Material>::Create(shader);
		CopyAsset(material.As<Asset>(), asset);
		material->SetRenderQueueID(data["RenderQueueID"].as<uint8_t>());
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
	Ref<Asset> AssetSerializer::deserialize<Font>(const Ref<Asset>& asset)
	{
		std::ifstream stream(asset->FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		XYZ_ASSERT(data["Font"], "Incorrect file format");
		uint32_t pixelSize = data["PixelSize"].as<uint32_t>();
		std::string fontPath = data["FontPath"].as<std::string>();

		auto ref = Ref<Font>::Create(pixelSize, fontPath);
		CopyAsset(ref.As<Asset>(), asset);
		return ref.As<Font>();
	}

	
	template <>
	Ref<Asset> AssetSerializer::deserialize<SkeletalMesh>(const Ref<Asset>& asset)
	{
		std::ifstream stream(asset->FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		GUID materialHandle(data["MaterialAsset"].as<std::string>());
		Ref<Material> material = AssetManager::GetAsset<Material>(materialHandle);

		std::vector<AnimatedVertex> vertices = data["AnimatedVertices"].as<std::vector<AnimatedVertex>>();
		std::vector<uint32_t> indices = data["Indices"].as<std::vector<uint32_t>>();

		auto ref = Ref<SkeletalMesh>::Create(
			std::move(vertices), 
			std::move(indices), 
			material
		);
		CopyAsset(ref.As<Asset>(), asset);
		return ref.As<Font>();
	}

	template <>
	Ref<Asset> AssetSerializer::deserialize<Scene>(const Ref<Asset>& asset)
	{
		Ref<Scene> result = Ref<Scene>::Create("");
		CopyAsset(result.As<Asset>(), asset);

		SceneSerializer sceneSerializer(result);
		sceneSerializer.Deserialize();

		return result;
	}

	template <>
	Ref<Asset> AssetSerializer::deserialize<Animation>(const Ref<Asset>& asset)
	{
		std::ifstream stream(asset->FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());


		return Ref<Asset>();
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
	void AssetSerializer::SerializeAsset(const Ref<Asset>& asset)
	{
		switch (asset->Type)
		{
		case AssetType::Scene:
			serialize<Scene>(asset);
			break;
		case AssetType::Texture:
			serialize<Texture2D>(asset);
			break;
		case AssetType::SubTexture:
			serialize<SubTexture>(asset);
			break;
		case AssetType::Material:
			serialize<Material>(asset);
			break;
		case AssetType::Shader:
			serialize<Shader>(asset);
			break;
		case AssetType::Font:
			serialize<Font>(asset);
			break;
		case AssetType::SkeletalMesh:
			serialize<SkeletalMesh>(asset);
			break;
		case AssetType::Animation:
			serialize<Animation>(asset);
			break;
		}
		createMetaFile(asset);
	}
	Ref<Asset> AssetSerializer::LoadAsset(Ref<Asset>& asset)
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
		case AssetType::SkeletalMesh:
			return deserialize<SkeletalMesh>(asset);
		case AssetType::Animation:
			return deserialize<Animation>(asset);
		}	
		XYZ_ASSERT(false, "");
		return Ref<Asset>();
	}
	void AssetSerializer::loadMetaFile(Ref<Asset>& asset)
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
	void AssetSerializer::createMetaFile(const Ref<Asset>& asset)
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