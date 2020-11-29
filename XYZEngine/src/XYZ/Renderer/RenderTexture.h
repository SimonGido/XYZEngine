#pragma once
#include "Texture.h"

#include "FrameBuffer.h"

namespace XYZ {
	class RenderTexture : public Texture
	{
	public:
		virtual Ref<FrameBuffer> GetRenderTarget() = 0;

		static Ref<RenderTexture> Create(const Ref<FrameBuffer>& renderTarget);
	};

}