#pragma once
#include "XYZ/Asset/Asset.h"

namespace XYZ {

	class Plugin : public Asset
	{
	public:
		Plugin(std::filesystem::path path);

		virtual AssetType GetAssetType() const override { return AssetType::Plugin; }


		const std::filesystem::path& GetFilepath() const { return m_Filepath; }

		static AssetType GetStaticType() { return AssetType::Plugin; }
	private:
		std::filesystem::path m_Filepath;
	};

}