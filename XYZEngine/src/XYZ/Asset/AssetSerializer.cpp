#include "stdafx.h"
#include "AssetSerializer.h"

#include "XYZ/Scene/SceneSerializer.h"

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


	void SceneAssetSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		SceneSerializer serializer(asset.As<Scene>());
		serializer.Serialize(metadata.FilePath.string());
	}
	bool SceneAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset, bool allocate) const
	{
		SceneSerializer serializer(asset.As<Scene>());
		asset = serializer.Deserialize(metadata.FilePath.string());
		return true;
	}
	void MaterialAssetSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
	}
	bool MaterialAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset, bool allocate) const
	{
		if (allocate)
		{
			// asset = Material::Create() ...
		}
		else
		{
			
		}
		return false;
	}

	

	void TextureAssetSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<Texture2D> texture = asset.As<Texture2D>();
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
		out << YAML::Key << "SRGB" << props.GenerateMips;
		out << YAML::Key << "Storage" << props.Storage;

		out << YAML::EndMap;

		std::ofstream fout(metadata.FilePath);
		fout << out.c_str();
	}
	bool TextureAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset, bool allocate) const
	{
		std::ifstream stream(metadata.FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		std::string imagePath;



		if (allocate)
		{
			
		}
		else
		{
			TextureProperties properties{}; // Load properties from file
			Texture2D::Create(asset.As<Texture2D>(), "blabla", properties);
		}
		return false;
	}
}