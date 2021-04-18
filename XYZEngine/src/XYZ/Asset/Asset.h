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
		Script,
		SkeletalMesh
	};


	class Asset : public RefCount
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
	
	class AssetDirectory
	{
	public:
		GUID						Handle;
		GUID						ParentHandle;
		std::string					FilePath;
		std::vector<GUID>			SubDirectoryHandles;
	};

}