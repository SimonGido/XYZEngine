#pragma once
#include "Texture.h"

#include "Framebuffer.h"

namespace XYZ {
	class RenderTexture : public Texture
	{
	public:
		virtual Ref<Framebuffer> GetRenderTarget() = 0;

		static Ref<RenderTexture> Create(const Ref<Framebuffer>& renderTarget);
	};

}