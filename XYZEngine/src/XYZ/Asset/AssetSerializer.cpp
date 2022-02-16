#include "stdafx.h"
#include "AssetSerializer.h"

#include "XYZ/Scene/SceneSerializer.h"
#include "XYZ/Animation/AnimatorController.h"
#include "XYZ/Renderer/Renderer.h"

#include "MaterialAsset.h"
#include "ShaderAsset.h"
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

		Ref<Shader> shader = Shader::Create(name, filePath, layouts);
		asset = Ref<ShaderAsset>::Create(shader);
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

	template <typename T>
	static void SerializeProperties(YAML::Emitter& out, const std::vector<Property<T>>& props, const std::string& name)
	{
		out << YAML::Key << name << YAML::Value << YAML::BeginSeq;
		for (const auto& prop : props)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Path" << prop.GetPath();
			out << YAML::Key << "ComponentName" << prop.GetComponentName();
			out << YAML::Key << "ValueName" << prop.GetValueName();
			out << YAML::Key << "Keys" << YAML::Value << YAML::BeginSeq;
			for (const auto& key : prop.Keys)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Frame" << key.Frame;
				out << YAML::Key << "Value" << key.Value;
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
	}



	template <uint16_t valIndex, typename T, typename CompType>
	void AddPropFromRefl(Reflection<CompType> refl, Ref<Animation>& anim, const std::string_view path)
	{
		anim->AddProperty<CompType, T, valIndex>(path);
	}

	template <typename T>
	static void AddProperty(Ref<Animation>& anim, const std::string_view path, const std::string_view componentName, const std::string_view valName)
	{
		Utils::For([&](auto j) {
			if (ReflectedComponents::sc_ClassNames[j.value] == componentName)
			{
				auto reflClass = ReflectedComponents::Get<j.value>();
				Utils::For([&](auto i) {
					if (reflClass.sc_VariableNames[i.value] == valName)
					{
						AddPropFromRefl<i.value, T>(reflClass, anim, path);
					}
				}, std::make_index_sequence<reflClass.sc_NumVariables>());
			}
		}, std::make_index_sequence<ReflectedComponents::sc_NumClasses>());
	}

	template <typename T>
	static void DeserializeProperties(YAML::Node& data, Ref<Animation>& anim)
	{
		std::vector<Property<T>>& props = anim->GetProperties<T>();
		for (auto prop : data)
		{		
			std::string path = prop["Path"].as<std::string>();
			std::string componentName = prop["ComponentName"].as<std::string>();
			std::string valueName = prop["ValueName"].as<std::string>();

		
			AddProperty<T>(anim, path, componentName, valueName);
			auto& last = props.back();
			for (auto key : prop["Keys"])
			{
				uint32_t frame = key["Frame"].as<uint32_t>();
				T value = key["Value"].as<T>();
				last.Keys.push_back({ value, frame });
			}
		}
	}

	void AnimationAssetSerializer::Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const
	{
		WeakRef<Animation> animation = asset.As<Animation>();
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "NumFrames" << animation->GetNumFrames();
		out << YAML::Key << "Frequency" << animation->GetFrequency();
		out << YAML::Key << "Repeat" << animation->GetRepeat();

		SerializeProperties(out, animation->GetProperties<glm::vec4>(), "Vec4Properties");
		SerializeProperties(out, animation->GetProperties<glm::vec3>(), "Vec3Properties");
		SerializeProperties(out, animation->GetProperties<glm::vec2>(), "Vec2Properties");
		SerializeProperties(out, animation->GetProperties<float>(),	   "FloatProperties");

		out << YAML::EndMap;

		std::ofstream fout(metadata.FilePath);
		fout << out.c_str();
	}
	bool AnimationAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{		
		Ref<Animation> anim = Ref<Animation>::Create();
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		const uint32_t numFrames = data["NumFrames"].as<uint32_t>();
		const uint32_t frequency = data["Frequency"].as<uint32_t>();
		const bool     repeat = data["Repeat"].as<bool>();

		DeserializeProperties<glm::vec4>(data["Vec4Properties"], anim);
		DeserializeProperties<glm::vec3>(data["Vec3Properties"], anim);
		DeserializeProperties<glm::vec2>(data["Vec2Properties"], anim);
		DeserializeProperties<float>(data["FloatProperties"], anim);

		asset = anim;
		return true;
	}

}