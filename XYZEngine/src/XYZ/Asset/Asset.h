#pragma once
#include "XYZ/Core/Ref.h"
#include "XYZ/Core/GUID.h"

namespace XYZ {

	enum class AssetType
	{
		Scene,
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
	
		GUID		Handle;
		AssetType   Type;
		std::string FilePath;
		std::string FileName;
		std::string FileExtension;
		GUID		DirectoryHandle;
		bool		IsLoaded = false;
	};

}