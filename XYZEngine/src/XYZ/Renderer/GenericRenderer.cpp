#include "stdafx.h"
#include "GenericRenderer.h"

namespace XYZ {
	struct GenericRendererData
	{
		uint8_t* m_QuadDataBase;
	};

	static GenericRendererData s_Data;

	void GenericRenderer::Init()
	{
	}
	void GenericRenderer::Shutdown()
	{
	}
}