#include "stdafx.h"
#include "Asset.h"


namespace XYZ {
	Asset::AssetInfoContainer  Asset::s_AssetExtensions = {
		"none",
		"xyz",
		"tex",
		"subtex",
		"mat",
		"shader",
		"font",
		"audio",
		"cs",
		"mesh",
		"animmesh",
		"meshsrc",
		"anim",
		"animator",
		"skeleton",
		"prefab"
	};
	
	bool Asset::IsValid() const
	{
		const bool missing = m_Flags & (uint16_t)AssetFlag::Missing;
		const bool invalid = m_Flags & (uint16_t)AssetFlag::Invalid;

		return !missing && !invalid;
	}

	void Asset::SetFlag(AssetFlag flag, bool value)
	{
		if (value)
			m_Flags |= (uint16_t)flag;
		else
			m_Flags &= ~(uint16_t)flag;
	}
}