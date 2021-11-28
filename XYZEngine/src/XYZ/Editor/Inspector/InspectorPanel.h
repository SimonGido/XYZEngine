#pragma once
#include "InspectorContext.h"

namespace XYZ {
	namespace Editor {
		class InspectorPanel
		{
		public:
			InspectorPanel();
			
			void OnImGuiRender(bool &open, Ref<Renderer2D> renderer);
	
			void SetContext(InspectorContext* context);

		private:
			InspectorContext* m_Context;
		};
	}
}