#pragma once
#include "Asset.h"

struct aiScene;

namespace XYZ {
	class AssimpModel : public Asset
	{
	public:
		AssimpModel(const std::string& filepath);

		const aiScene* GetScene() const;
	private:
		std::string    m_FilePath;
		const aiScene* m_Scene;
	};
}