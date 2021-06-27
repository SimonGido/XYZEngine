#pragma once


namespace XYZ {

	class InspectorEditable
	{
	public:
		virtual ~InspectorEditable() = default;

		virtual bool OnEditorRender() = 0;
	};

}