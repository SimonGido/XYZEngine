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
		SkeletalMesh,
		Animation,
		Animator,
		None
	};


	class Asset : public RefCount
	{
	public:
		virtual ~Asset() = default;
	
		GUID		Handle;
		AssetType   Type;
		std::string FilePath;
		std::string FileName;
		std::string FileExtension;
		GUID		DirectoryHandle;
		bool		IsLoaded = false;

		static AssetType GetStaticType() { return AssetType::None; }
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