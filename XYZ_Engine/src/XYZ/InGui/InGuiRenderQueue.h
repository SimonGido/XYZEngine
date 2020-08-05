#pragma once
#include "XYZ/Renderer/InGuiRenderer.h"


namespace XYZ {
	namespace InGui {

		class InGuiRenderQueue
		{
		public:
			void Push(InGuiMesh* mesh,uint8_t priority);
			void SubmitToRenderer();

		private:
			struct InGuiDrawable
			{
				InGuiMesh* Mesh;
				uint8_t Priority;
			};
			std::vector<InGuiDrawable> m_DrawList;
		};
		
	}
}