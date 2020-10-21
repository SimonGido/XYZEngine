#pragma once
#include <XYZ.h>

namespace XYZ {

	class EditorScene
	{
	public:
		void SetContext(const Ref<Scene>& context);


	private:
		Ref<Scene> m_Context;
	};

}