#pragma once
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	namespace Editor {
		class InspectorPanel
		{
		public:
			InspectorPanel();

			void OnUpdate();
	

			void SetContext(SceneEntity context);

		private:
			SceneEntity m_Context;
		};
	}
}