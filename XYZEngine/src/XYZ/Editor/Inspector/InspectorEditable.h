#pragma once
#include "XYZ/Renderer/EditorRenderer.h"

namespace XYZ {

	class InspectorEditable
	{
	public:
		virtual ~InspectorEditable() = default;

		virtual bool OnEditorRender(Ref<EditorRenderer> renderer) = 0;
	};

}