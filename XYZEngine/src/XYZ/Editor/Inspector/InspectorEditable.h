#pragma once
#include "XYZ/Renderer/Renderer2D.h"

namespace XYZ {

	class InspectorEditable
	{
	public:
		virtual ~InspectorEditable() = default;

		virtual bool OnEditorRender(Ref<Renderer2D> renderer) = 0;
	};

}