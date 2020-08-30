#include "stdafx.h"
#include "InGuiRenderQueue.h"

#include "XYZ/Renderer/InGuiRenderer.h"

namespace XYZ {
	namespace InGui {
		void InGuiRenderQueue::Push(InGuiMesh* mesh, uint8_t priority)
		{
			XYZ_ASSERT(mesh, "Pushing null pointer to mesh");
			m_DrawList.push_back({ mesh,priority });
		}
		void InGuiRenderQueue::Push(InGuiLineMesh* mesh)
		{
			InGuiRenderer::SubmitLineMesh(*mesh);
		}
		void InGuiRenderQueue::SubmitToRenderer()
		{
			std::sort(m_DrawList.begin(), m_DrawList.end(),[](const InGuiDrawable& a, const InGuiDrawable& b) {
				return a.Priority < b.Priority;
			});

			for (auto& it : m_DrawList)
			{
				InGuiRenderer::SubmitUI(*it.Mesh);
			}
		
			size_t oldSize = m_DrawList.size();
			m_DrawList.clear();
			m_DrawList.reserve(oldSize);
		}
	}
}