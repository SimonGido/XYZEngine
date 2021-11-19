#include "stdafx.h"
#include "OpenGLFence.h"

namespace XYZ {
	OpenGLFence::OpenGLFence(uint64_t timeOut)
	{
		m_Fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		auto value = glClientWaitSync(m_Fence, GL_SYNC_FLUSH_COMMANDS_BIT, timeOut);
	}

	OpenGLFence::~OpenGLFence()
	{
		glDeleteSync(m_Fence);
	}

}