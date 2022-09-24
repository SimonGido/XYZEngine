#include "stdafx.h"
#include "GeometryRenderQueue.h"


namespace XYZ {
	uint32_t GeometryRenderQueue::SpriteDrawCommand::SetTexture(const Ref<Texture2D>& texture)
	{
		for (uint32_t i = 0; i < TextureCount; i++)
		{
			if (Textures[i]->GetHandle() == texture->GetHandle())
			{
				return i;
			}
		}
		uint32_t result = TextureCount;
		Textures[result] = texture;
		TextureCount++;
		return result;
	}
}