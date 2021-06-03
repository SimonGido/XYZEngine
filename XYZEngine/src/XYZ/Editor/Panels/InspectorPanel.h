#pragma once
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	namespace Editor {
		class InspectorPanel
		{
		public:
			InspectorPanel();

			void OnImGuiRender();
	

			void SetContext(SceneEntity context);

		private:
			void drawComponents(SceneEntity entity);

		private:
			SceneEntity m_Context;
		};
	}
}