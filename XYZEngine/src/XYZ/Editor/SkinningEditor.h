#pragma once

#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Core/Timestep.h"


namespace XYZ {


	class SkinningEditor
	{
	public:
		SkinningEditor(Ref<SubTexture> texture);

		void Update();
		void OnInGuiRender();

	private:

	};
}