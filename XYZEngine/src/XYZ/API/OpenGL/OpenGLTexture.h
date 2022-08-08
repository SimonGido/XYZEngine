#pragma once

#include "XYZ/Renderer/Texture.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"
#include "XYZ/Utils/DataStructures/Queue.h"

#include <GL/glew.h>

namespace XYZ {

	class OpenGLTexture2D
	{
	public:
		
	private:
		Ref<Image2D> m_Image;
		TextureProperties m_Properties;
		uint32_t m_Width, m_Height;

	
		bool m_Locked = false;
		bool m_Loaded = false;

		std::string m_FilePath;

	};
}