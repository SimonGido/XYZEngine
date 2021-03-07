#pragma once
#include "XYZ/Core/GUID.h"
#include "XYZ/Core/Ref.h"

namespace XYZ {

	enum class AssetType
	{
		Scene,
		Mesh,
		Texture,
		SubTexture,
		Material,
		Shader,
		Font,
		Audio,
		Script
	};


	class Asset : RefCount
	{
	public:
		virtual ~Asset() {};
	
		size_t		Handle;
		AssetType   Type;
		std::string FilePath;
		std::string FileName;
		std::string FileExtension;
		size_t		DirectoryID;
		bool		IsLoaded = false;
	};

}