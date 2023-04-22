#include "stdafx.h"
#include "AssetSerializer.h"

#include "XYZ/Scene/SceneSerializer.h"
#include "XYZ/Scene/Prefab.h"

#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/Mesh.h"
#include "XYZ/Renderer/SubTexture.h"

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


		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Name" << shader->GetName();
		out << YAML::Key << "FilePath" << shader->GetPath();
		out << YAML::Key << "SourceHash" << shaderAsset->GetSourceHash();
		out << YAML::EndMap;
		std::ofstream fout(metadata.FilePath);
		fout << out.c_str();
		fout.flush();
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


		asset = Ref<ShaderAsset>::Create(name, filePath, sourceHash);
		return true;
	}

	void MaterialAssetSerializer::Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const
	{
		WeakRef<MaterialAsset> material = asset.As<MaterialAsset>();
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Shader" << material->GetShaderAsset()->GetHandle();
		out << YAML::Key << "Opaque" << material->IsOpaque();
		out << YAML::Key << "Textures" << YAML::BeginSeq;
		for (const auto& texture : material->GetTextures())
		{
			if (texture.Texture.Raw())
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Name" << texture.Name;
				out << YAML::Key << "Handle" << texture.Texture->GetHandle();
				out << YAML::EndMap;
			}
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
				if (texture.Raw())
					out << texture->GetHandle();
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;


		out << YAML::Key << "Specialization" << YAML::BeginSeq;
		for (const auto& spec : material->GetSpecialization().GetValues())
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << spec.Name;
			out << YAML::Key << "Data" << static_cast<uint32_t>(*spec.Data.data());
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::Key << "MaterialData" << YAML::BeginSeq;
		auto shader = material->GetShader();
		for (auto& [name, buffer] : shader->GetBuffers())
		{
			for (auto& [uniName, uni] : buffer.Uniforms)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Name" << uniName;
				out << YAML::Key << "Type" << static_cast<uint32_t>(uni.GetDataType());
				switch (uni.GetDataType())
				{
				case ShaderUniformDataType::Int:     out << YAML::Key << "Value" << material->Get<int>(uniName);  break;
				case ShaderUniformDataType::UInt:    out << YAML::Key << "Value" << material->Get<uint32_t>(uniName);  break;
				case ShaderUniformDataType::Float:   out << YAML::Key << "Value" << material->Get<float>(uniName);  break;
				case ShaderUniformDataType::Vec2:    out << YAML::Key << "Value" << material->Get<glm::vec2>(uniName);  break;
				case ShaderUniformDataType::Vec3:    out << YAML::Key << "Value" << material->Get<glm::vec3>(uniName);  break;
				case ShaderUniformDataType::Vec4:    out << YAML::Key << "Value" << material->Get<glm::vec4>(uniName);  break;
				//case ShaderUniformDataType::Mat3:    materialAsset->Set<glm::mat3>(name, uniform["Value"].as<glm::mat3>()); break;
				//case ShaderUniformDataType::Mat4:    materialAsset->Set<glm::mat4>(name, uniform["Value"].as<glm::mat4>()); break;
				}
				out << YAML::EndMap;
			}
		}
		out << YAML::EndSeq;	
		out << YAML::EndMap;

		std::ofstream fout(metadata.FilePath);
		fout << out.c_str();
		fout.flush();
	}
	bool MaterialAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		AssetHandle shaderHandle = AssetHandle(data["Shader"].as<std::string>());
		Ref<ShaderAsset> shaderAsset = AssetManager::GetAssetWait<ShaderAsset>(shaderHandle);
		Ref<MaterialAsset> materialAsset = Ref<MaterialAsset>::Create(shaderAsset);

		auto opaque = data["Opaque"];
		if (opaque)
		{
			materialAsset->SetOpaque(opaque.as<bool>());
		}

		for (auto texture : data["Textures"])
		{
			GUID handle(texture["Handle"].as<std::string>());
			auto name = texture["Name"].as<std::string>();
			if (AssetManager::Exist(handle))
			{
				materialAsset->SetTexture(name, AssetManager::GetAssetWait<Texture2D>(handle));
			}
			else
			{
				XYZ_CORE_WARN("Missing texture!");
				materialAsset->SetTexture(name, Renderer::GetDefaultResources().RendererAssets.at("WhiteTexture").As<Texture2D>());
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
					materialAsset->SetTexture(name, AssetManager::GetAssetWait<Texture2D>(handle), index);
				}
				else
				{
					XYZ_CORE_WARN("Missing texture!");
					materialAsset->SetTexture(name, Renderer::GetDefaultResources().RendererAssets.at("WhiteTexture").As<Texture2D>(), index);
				}
				index++;
			}
		}

		auto specialization = data["Specialization"];
		if (specialization)
		{
			for (auto& spec : specialization)
			{
				const std::string name = spec["Name"].as<std::string>();
				const uint32_t data = spec["Data"].as<uint32_t>();
				materialAsset->Specialize(name, data);
			}
		}

		auto materialData = data["MaterialData"];
		if (materialData)
		{
			for (auto& uniform : materialData)
			{
				const std::string name = uniform["Name"].as<std::string>();
				const ShaderUniformDataType type = static_cast<ShaderUniformDataType>(uniform["Type"].as<uint32_t>());
				switch (type)
				{
				case ShaderUniformDataType::Int:     materialAsset->Set<int>(	   name, uniform["Value"].as<int>()); break;
				case ShaderUniformDataType::UInt:    materialAsset->Set<uint32_t>( name, uniform["Value"].as<uint32_t>()); break;
				case ShaderUniformDataType::Float:   materialAsset->Set<float>(	   name, uniform["Value"].as<float>()); break;
				case ShaderUniformDataType::Vec2:    materialAsset->Set<glm::vec2>(name, uniform["Value"].as<glm::vec2>()); break;
				case ShaderUniformDataType::Vec3:    materialAsset->Set<glm::vec3>(name, uniform["Value"].as<glm::vec3>()); break;
				case ShaderUniformDataType::Vec4:    materialAsset->Set<glm::vec4>(name, uniform["Value"].as<glm::vec4>()); break;
				//case ShaderUniformDataType::Mat3:    materialAsset->Set<glm::mat3>(name, uniform["Value"].as<glm::mat3>()); break;
				//case ShaderUniformDataType::Mat4:    materialAsset->Set<glm::mat4>(name, uniform["Value"].as<glm::mat4>()); break;
				}
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
		fout.flush();
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
			// TODO: use obj file?
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
		fout.flush();
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
			// TODO: use obj file?
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
		return true;
	}
	void SkeletonAssetSerializer::Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const
	{
		WeakRef<SkeletonAsset> anim = asset.As<SkeletonAsset>();
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "SourceFilePath" << anim->GetFilePath();

		out << YAML::EndMap;

		std::ofstream fout(metadata.FilePath);
		fout << out.c_str();
		fout.flush();
	}
	bool SkeletonAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		auto sourceFilePath = data["SourceFilePath"];
		asset = Ref<SkeletonAsset>::Create(sourceFilePath.as<std::string>());

		return true;
	}

	void AnimationAssetSerializer::Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const
	{
		WeakRef<AnimationAsset> anim = asset.As<AnimationAsset>();
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "SourceFilePath" << anim->GetFilePath();
		out << YAML::Key << "AnimationName" << anim->GetName();
		out << YAML::Key << "Skeleton" << anim->GetSkeleton()->GetHandle();
		out << YAML::EndMap;

		std::ofstream fout(metadata.FilePath);
		fout << out.c_str();
		fout.flush();
	}
	bool AnimationAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		auto sourceFilePath = data["SourceFilePath"].as<std::string>();
		auto animationName = data["AnimationName"].as<std::string>();
		AssetHandle skeletonHandle(data["Skeleton"].as<std::string>());
		
		Ref<SkeletonAsset> skeleton = AssetManager::GetAssetWait<SkeletonAsset>(skeletonHandle);
		asset = Ref<AnimationAsset>::Create(sourceFilePath, animationName, skeleton);

		return true;
	}

	void StaticMeshAssetSerializer::Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const
	{
		WeakRef<StaticMesh> mesh = asset.As<StaticMesh>();
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "MeshSource" << mesh->GetMeshSource()->GetHandle();


		out << YAML::EndMap;

		std::ofstream fout(metadata.FilePath);
		fout << out.c_str();
		fout.flush();
	}



	bool StaticMeshAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		AssetHandle meshSourceHandle(data["MeshSource"].as<std::string>());
		asset = Ref<StaticMesh>::Create(AssetManager::GetAssetWait<MeshSource>(meshSourceHandle));
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
		fout.flush();
	}

	bool AnimatedMeshAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		AssetHandle meshSourceHandle(data["MeshSource"].as<std::string>());
		asset = Ref<AnimatedMesh>::Create(AssetManager::GetAssetWait<MeshSource>(meshSourceHandle));
		return true;
	}
	void PrefabAssetSerializer::Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const
	{
		WeakRef<Prefab> prefab = asset.As<Prefab>();

		SceneSerializer serializer;
		serializer.Serialize(metadata.FilePath.string(), prefab->m_Scene);
	}
	bool PrefabAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		Ref<Prefab> prefab = Ref<Prefab>::Create();

		SceneSerializer serializer;
		prefab->m_Scene = serializer.Deserialize(metadata.FilePath.string());
		prefab->m_Entity = prefab->m_Scene->GetEntityByGUID(data["FirstChild"].as<std::string>());
		prefab->m_Scene->GetRegistry().each([&](const entt::entity entity) {
			prefab->m_Entities.push_back({ entity, prefab->m_Scene.Raw() });
		});
		asset = prefab;
		return true;
	}
	void SubTextureSerializer::Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const
	{
		WeakRef<SubTexture> subTexture = asset.As<SubTexture>();
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Texture" << subTexture->GetTexture()->GetHandle();
		out << YAML::Key << "TexCoords" << subTexture->GetTexCoords();

		out << YAML::EndMap;

		std::ofstream fout(metadata.FilePath);
		fout << out.c_str();
		fout.flush();
	}
	bool SubTextureSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());



		AssetHandle textureHandle(data["Texture"].as<std::string>());
		Ref<Texture2D> texture = AssetManager::GetAssetWait<Texture2D>(textureHandle);
		glm::vec4 texCoords = data["TexCoords"].as<glm::vec4>();
		asset = Ref<SubTexture>::Create(texture, texCoords);

		return true;
	}
	void AnimationControllerAssetSerializer::Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const
	{
		WeakRef<AnimationController> controller = asset.As<AnimationController>();
		YAML::Emitter out;
		out << YAML::BeginMap;

		auto skeleton = controller->GetSkeleton();
		if (skeleton.Raw())
			out << YAML::Key << "Skeleton" << controller->GetSkeleton()->GetHandle();
		else
			out << YAML::Key << "Skeleton" << "";

		out << YAML::Key << "Animations" << YAML::BeginSeq;
		for (size_t i = 0; i < controller->GetAnimationStates().size(); ++i)
		{
			const Ref<AnimationAsset>& anim = controller->GetAnimationStates()[i];
			const std::string& stateName = controller->GetStateNames()[i];

			out << YAML::BeginMap;
			out << YAML::Key << "Name" << stateName;
			out << YAML::Key << "Handle" << anim->GetHandle();
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		std::ofstream fout(metadata.FilePath);
		fout << out.c_str();
		fout.flush();
	}
	bool AnimationControllerAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		Ref<AnimationController> controller = Ref<AnimationController>::Create();

		auto skeletonData = data["Skeleton"].as<std::string>();
		if (!skeletonData.empty())
		{
			AssetHandle skeletonHandle(skeletonData);
			controller->SetSkeletonAsset(AssetManager::GetAssetWait<SkeletonAsset>(skeletonHandle));
		}
		
		auto animationsData = data["Animations"];
		for (auto animData : animationsData)
		{
			std::string stateName = animData["Name"].as<std::string>();
			AssetHandle animHandle(animData["Handle"].as<std::string>());
			Ref<AnimationAsset> animation = AssetManager::GetAssetWait<AnimationAsset>(animHandle);
			controller->AddState(stateName, animation);
		}

		asset = controller;
		return true;
	}
	void ParticleSystemSerializerGPU::Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const
	{
		WeakRef<ParticleSystemGPU> particleSystem = asset.As<ParticleSystemGPU>();
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "MaxParticles" << particleSystem->GetMaxParticles();
		out << YAML::Key << "Speed" << particleSystem->Speed;
		out << YAML::Key << "Loop" << particleSystem->Loop;
		
		out << YAML::Key << "InputLayout" << YAML::BeginMap;
		out << YAML::Key << "Name" << particleSystem->GetInputLayout().GetName();
		out << YAML::Key << "Stride" << particleSystem->GetInputLayout().GetStride();
		out << YAML::Key << "Variables" << YAML::BeginSeq;
		
		for (const auto& var : particleSystem->GetInputLayout().GetVariables())
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << var.Name;
			out << YAML::Key << "TypeName" << var.Type.Name;
			out << YAML::Key << "TypeSize" << var.Type.Size;
			out << YAML::Key << "IsArray" << var.IsArray;
			out << YAML::Key << "Offset" << var.Offset;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap; // InputLayout


		out << YAML::Key << "OutputLayout" << YAML::BeginMap;
		out << YAML::Key << "Name" << particleSystem->GetOutputLayout().GetName();
		out << YAML::Key << "Stride" << particleSystem->GetOutputLayout().GetStride();
		out << YAML::Key << "Variables" << YAML::BeginSeq;

		for (const auto& var : particleSystem->GetOutputLayout().GetVariables())
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << var.Name;
			out << YAML::Key << "TypeName" << var.Type.Name;
			out << YAML::Key << "TypeSize" << var.Type.Size;
			out << YAML::Key << "IsArray" << var.IsArray;
			out << YAML::Key << "Offset" << var.Offset;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap; // OutputLayout

		
		out << YAML::EndMap; // ParticleSystem
		std::ofstream fout(metadata.FilePath);
		fout << out.c_str();
		fout.flush();
	}
	bool ParticleSystemSerializerGPU::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());
		

		const uint32_t maxParticles = data["MaxParticles"].as<uint32_t>();
		const float speed			= data["Speed"].as<float>();
		const bool loop				= data["Loop"].as<bool>();


		std::vector<ParticleVariable> inputVariables;
		
		auto inputLayoutData		= data["InputLayout"];
		const std::string inputName	= inputLayoutData["Name"].as<std::string>();
		const uint32_t inputStride	= inputLayoutData["Stride"].as<uint32_t>();

		for (auto& var : inputLayoutData["Variables"])
		{
			ParticleVariable& variable = inputVariables.emplace_back();
			variable.Name		= var["Name"].as<std::string>();
			variable.Type.Name	= var["TypeName"].as<std::string>();
			variable.Type.Size	= var["TypeSize"].as<uint32_t>();
			variable.IsArray	= var["IsArray"].as<bool>();
			variable.Offset		= var["Offset"].as<uint32_t>();		
		}

		std::vector<ParticleVariable> outputVariables;

		auto outputLayoutData			= data["OutputLayout"];
		const std::string outputName	= outputLayoutData["Name"].as<std::string>();
		const uint32_t outputStride		= outputLayoutData["Stride"].as<uint32_t>();

		for (auto& var : outputLayoutData["Variables"])
		{
			ParticleVariable& variable = outputVariables.emplace_back();
			variable.Name		= var["Name"].as<std::string>();
			variable.Type.Name	= var["TypeName"].as<std::string>();
			variable.Type.Size	= var["TypeSize"].as<uint32_t>();
			variable.IsArray	= var["IsArray"].as<bool>();
			variable.Offset		= var["Offset"].as<uint32_t>();
		}


		ParticleSystemLayout inputLayout(inputName, inputVariables, inputStride);
		ParticleSystemLayout outputLayout(outputName, outputVariables, outputStride);

		Ref<ParticleSystemGPU> particleSystem = Ref<ParticleSystemGPU>::Create(inputLayout, outputLayout, maxParticles);
		particleSystem->Speed = speed;
		particleSystem->Loop = loop;

		asset = particleSystem;
		return true;
	}
	void VoxelMeshSourceSerializer::Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const
	{
	}
	bool VoxelMeshSourceSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		return true;
	}
}