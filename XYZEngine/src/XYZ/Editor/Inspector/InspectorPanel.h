#pragma once
#include "InspectorContext.h"

namespace XYZ {
	namespace Editor {
		class InspectorPanel
		{
		public:
			InspectorPanel();
			
			void OnImGuiRender();
	
			void SetContext(InspectorContext* context);

		private:
			InspectorContext* m_Context;

		};
	}
}