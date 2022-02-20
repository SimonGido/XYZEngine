#include "stdafx.h"
#include "AssetSerializer.h"

#include "XYZ/Scene/SceneSerializer.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/Mesh.h"

#include "Renderer/MeshSource.h"
#include "Renderer/MaterialAsset.h"
#include "Renderer/ShaderAsset.h"
#include "AssetManager.h"

#include "XYZ/Utils/YamlUtils.h"

namespace XYZ {
	namespace Utils {
		static std::string ImageFormatToString(ImageFormat format)
		{
			switch (format)
			{
			case XYZ::ImageFormat::None:
				break;
			case XYZ::ImageFormat::RED32F:
				return "RED32F";
				break;
			case XYZ::ImageFormat::RGB:
				return "RGB";
				break;
			case XYZ::ImageFormat::RGBA:
				return "RGBA";
				break;
			case XYZ::ImageFormat::RGBA16F:
				return "RGBA16F";
				break;
			case XYZ::ImageFormat::RGBA32F:
				return "RGBA32F";
				break;
			case XYZ::ImageFormat::RG16F:
				return "RG16F";
				break;
			case XYZ::ImageFormat::RG32F:
				return "RG32F";
				break;
			case XYZ::ImageFormat::SRGB:
				return "SRGB";
				break;
			case XYZ::ImageFormat::DEPTH32F:
				return "DEPTH32F";
				break;
			case XYZ::ImageFormat::DEPTH24STENCIL8:
				return "DEPTH24STENCIL8";
				break;
			}
			XYZ_ASSERT(false, "");
			return std::string();
		}
		static ImageFormat StringToImageFormat(const std::string& format)
		{
			if (format == "RED32F")
				return ImageFormat::RED32F;
			if (format == "RGB")
				return ImageFormat::RGB;
			if (format == "RGBA")
				return ImageFormat::RGBA;
			if (format == "RGBA16F")
				return ImageFormat::RGBA16F;
			if (format == "RGBA32F")
				return ImageFormat::RGBA32F;
			if (format == "RG16F")
				return ImageFormat::RG16F;
			if (format == "RG32F")
				return ImageFormat::RG32F;
			if (format == "SRGB")
				return ImageFormat::SRGB;
			if (format == "DEPTH32F")
				return ImageFormat::DEPTH32F;
			if (format == "DEPTH24STENCIL8")
				return ImageFormat::DEPTH24STENCIL8;
		}
	}


	void SceneAssetSerializer::Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const
	{
		SceneSerializer serializer;
		serializer.Serialize(metadata.FilePath.string(), asset.As<Scene>());
	}
	bool SceneAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		SceneSerializer serializer;
		asset = serializer.Deserialize(metadata.FilePath.string());
		return true;
	}

	void ShaderAssetSerializer::Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const
	{
		WeakRef<ShaderAsset> shaderAsset = asset.As<ShaderAsset>();
		Ref<Shader> shader = shaderAsset->GetShader();
		const auto& layouts = shaderAsset->GetLayouts();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Name" << shader->GetName();
		out << YAML::Key << "FilePath" << shader->GetPath();
		out << YAML::Key << "SourceHash" << shaderAsset->GetSourceHash();
		out << YAML::Key << "Layouts" << YAML::BeginSeq;
		for (const auto& layout : layouts)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Instanced" << layout.Instanced();
			out << YAML::Key << "Elements" << YAML::BeginSeq;
			out << YAML::Flow;
			for (const auto& element : layout)
			{
				out << static_cast<uint32_t>(element.Type);
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		std::ofstream fout(metadata.FilePath);
		fout << out.c_str();
	}
	bool ShaderAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		std::string name = data["Name"].as<std::string>();
		std::string filePath = data["FilePath"].as<std::string>();
		const size_t sourceHash = data["SourceHash"].as<size_t>();

		std::vector<BufferLayout> layouts;
		for (auto layoutData : data["Layouts"])
		{
			bool instanced = layoutData["Instanced"].as<bool>();
			std::vector<BufferElement> elements;
			for (auto elementData : layoutData["Elements"])
			{
				ShaderDataType type = static_cast<ShaderDataType>(elementData.as<uint32_t>());
				elements.push_back({ type, "" });
			}
			layouts.emplace_back(elements, instanced);
		}


		asset = Ref<ShaderAsset>::Create(name, filePath, sourceHash, layouts);
		return true;
	}

	void MaterialAssetSerializer::Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const
	{
		WeakRef<MaterialAsset> material = asset.As<MaterialAsset>();
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Shader" << material->GetShaderAsset()->GetHandle();
		out << YAML::Key << "Textures" << YAML::BeginSeq;
		for (const auto& texture : material->GetTextures())
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << texture.Name;
			out << YAML::Key << "Handle" << texture.Texture->GetHandle();
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::Key << "TextureArrays" << YAML::BeginSeq;
		for (const auto& textureArr : material->GetTextureArrays())
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << textureArr.Name;
			out << YAML::Key << "Textures" << YAML::BeginSeq;
			out << YAML::Flow;
			for (const auto& texture : textureArr.Textures)
			{
				out << texture->GetHandle();
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(metadata.FilePath);
		fout << out.c_str();
	}
	bool MaterialAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		AssetHandle shaderHandle = AssetHandle(data["Shader"].as<std::string>());
		Ref<ShaderAsset> shaderAsset = AssetManager::GetAsset<ShaderAsset>(shaderHandle);
		Ref<MaterialAsset> materialAsset = Ref<MaterialAsset>::Create(shaderAsset);

		for (auto texture : data["Textures"])
		{
			GUID handle(texture["Handle"].as<std::string>());
			auto name = texture["Name"].as<std::string>();
			if (AssetManager::Exist(handle))
			{
				materialAsset->SetTexture(name, AssetManager::GetAsset<Texture2D>(handle));
			}
			else
			{
				XYZ_WARN("Missing texture!");
				materialAsset->SetTexture(name, Renderer::GetDefaultResources().WhiteTexture);
			}
		}
		auto textureArrays = data["TextureArrays"];
		for (auto textureArr : textureArrays)
		{
			auto name = textureArr["Name"].as<std::string>();
			uint32_t index = 0;
			for (auto texture : textureArr["Textures"])
			{
				GUID handle(texture.as<std::string>());
				if (AssetManager::Exist(handle))
				{
					materialAsset->SetTexture(name, AssetManager::GetAsset<Texture2D>(handle), index);
				}
				else
				{
					XYZ_WARN("Missing texture!");
					materialAsset->SetTexture(name, Renderer::GetDefaultResources().WhiteTexture, index);
				}
				index++;
			}
		}
		asset = materialAsset;
		return true;
	}

	

	void TextureAssetSerializer::Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const
	{
		WeakRef<Texture2D> texture = asset.As<Texture2D>();
		YAML::Emitter out;
		out << YAML::BeginMap;
	
		out << YAML::Key << "Image Path" << texture->GetPath();
		out << YAML::Key << "Format" << static_cast<uint32_t>(texture->GetFormat());
		out << YAML::Key << "Width" << texture->GetWidth();
		out << YAML::Key << "Height" << texture->GetHeight();
		out << YAML::Key << "MipLevelCount" << texture->GetMipLevelCount();
		const auto& props = texture->GetProperties();
		out << YAML::Key << "SamplerWrap" << static_cast<uint32_t>(props.SamplerWrap);
		out << YAML::Key << "SamplerFilter" << static_cast<uint32_t>(props.SamplerFilter);
		out << YAML::Key << "GenerateMips" << props.GenerateMips;
		out << YAML::Key << "SRGB" << props.SRGB;
		out << YAML::Key << "Storage" << props.Storage;

		out << YAML::EndMap;

		std::ofstream fout(metadata.FilePath);
		fout << out.c_str();
	}
	bool TextureAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		std::string imagePath = data["Image Path"].as<std::string>();
		uint32_t width = data["Width"].as<uint32_t>();
		uint32_t height = data["Height"].as<uint32_t>();
		
		TextureProperties props{};
		ImageFormat format = static_cast<ImageFormat>(data["Format"].as<uint32_t>());
		props.SamplerWrap = static_cast<TextureWrap>(data["SamplerWrap"].as<uint32_t>());
		props.SamplerFilter = static_cast<TextureFilter>(data["SamplerFilter"].as<uint32_t>());

		props.GenerateMips = data["GenerateMips"].as<bool>();
		props.SRGB = data["SRGB"].as<bool>();
		props.Storage = data["Storage"].as<bool>();

		
		asset = Texture2D::Create(imagePath, props);
		return true;
	}

	void MeshSourceAssetSerializer::Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const
	{
		WeakRef<MeshSource> meshSource = asset.As<MeshSource>();
		YAML::Emitter out;
		out << YAML::BeginMap;

		if (!meshSource->GetSourceFilePath().empty())
			out << YAML::Key << "SourceFilePath" << meshSource->GetSourceFilePath();
		else
		{
			out << YAML::Key << "Vertices" << YAML::Value << YAML::BeginSeq;
			for (auto& vertex : meshSource->GetVertices())
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Position" << vertex.Position;
				out << YAML::Key << "TexCoord" << vertex.TexCoord;
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::Key << "AnimatedVertices" << YAML::Value << YAML::BeginSeq;
			for (auto& vertex : meshSource->GetAnimatedVertices())
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Position" << vertex.Position;
				out << YAML::Key << "TexCoord" << vertex.TexCoord;
				out << YAML::Key << "BoneIDs" << glm::ivec4(vertex.IDs[0], vertex.IDs[1], vertex.IDs[2], vertex.IDs[3]);
				out << YAML::Key << "Weights" << glm::vec4(vertex.Weights[0], vertex.Weights[1], vertex.Weights[2], vertex.Weights[3]);
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::Key << "Indices" << YAML::Value << YAML::BeginSeq;
			out << YAML::Flow;
			for (auto index : meshSource->GetIndices())
			{
				out << index;
			}
			out << YAML::EndSeq;
		}
		

		out << YAML::EndMap;

		std::ofstream fout(metadata.FilePath);
		fout << out.c_str();
	}

	bool MeshSourceAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		auto sourceFilePath = data["SourceFilePath"];
		if (sourceFilePath)
		{
			asset = Ref<MeshSource>::Create(sourceFilePath.as<std::string>());
		}
		else
		{
			std::vector<Vertex> vertices;
			std::vector<AnimatedVertex> animVertices;
			std::vector<uint32_t> indices;

			for (auto it : data["Vertices"])
			{
				Vertex vertex;
				vertex.Position = it["Position"].as<glm::vec3>();
				vertex.TexCoord = it["TexCoord"].as<glm::vec2>();
				vertices.push_back(vertex);
			}
			for (auto it : data["AnimatedVertices"])
			{
				AnimatedVertex vertex;
				vertex.Position = it["Position"].as<glm::vec3>();
				vertex.TexCoord = it["TexCoord"].as<glm::vec2>();
				glm::ivec4 ids = it["BoneIDs"].as<glm::ivec4>();
				glm::vec4 weights = it["Weights"].as<glm::vec4>();
				vertex.IDs[0] = ids.x;
				vertex.IDs[1] = ids.y;
				vertex.IDs[2] = ids.z;
				vertex.IDs[3] = ids.w;

				vertex.Weights[0] = weights.x;
				vertex.Weights[1] = weights.y;
				vertex.Weights[2] = weights.z;
				vertex.Weights[3] = weights.w;

				animVertices.push_back(vertex);
			}
			for (auto it : data["Indices"])
			{
				indices.push_back(it.as<uint32_t>());
			}
			if (!vertices.empty())
				asset = Ref<MeshSource>::Create(vertices, indices);
			else
				asset = Ref<MeshSource>::Create(animVertices, indices);
		}
		return false;
	}

	void MeshAssetSerializer::Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const
	{
		WeakRef<Mesh> mesh = asset.As<Mesh>();
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "MeshSource" << mesh->GetMeshSource()->GetHandle();


		out << YAML::EndMap;

		std::ofstream fout(metadata.FilePath);
		fout << out.c_str();
	}

	bool MeshAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		AssetHandle meshSourceHandle(data["MeshSource"].as<std::string>());
		asset = Ref<Mesh>::Create(AssetManager::GetAsset<MeshSource>(meshSourceHandle));
		return true;
	}

	void AnimatedMeshAssetSerializer::Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const
	{
		WeakRef<AnimatedMesh> mesh = asset.As<AnimatedMesh>();
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "MeshSource" << mesh->GetMeshSource()->GetHandle();


		out << YAML::EndMap;

		std::ofstream fout(metadata.FilePath);
		fout << out.c_str();
	}

	bool AnimatedMeshAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		AssetHandle meshSourceHandle(data["MeshSource"].as<std::string>());
		asset = Ref<AnimatedMesh>::Create(AssetManager::GetAsset<MeshSource>(meshSourceHandle));
		return true;
	}

}