#pragma once
#include "XYZ/Renderer/Fence.h"

#include <GL/glew.h>

namespace XYZ {
	class OpenGLFence : public Fence
	{
	public:
		OpenGLFence(uint64_t timeOut);
		virtual ~OpenGLFence() override;

	private:
		GLsync m_Fence;
	};
		
}