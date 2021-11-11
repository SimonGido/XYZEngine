#pragma once
#include "XYZ/Renderer/RendererAPI.h"

namespace XYZ {

	class VulkanRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
	};
}