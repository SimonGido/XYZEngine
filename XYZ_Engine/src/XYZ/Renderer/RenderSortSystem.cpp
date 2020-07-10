#include "stdafx.h"
#include "RenderSortSystem.h"

#include "Renderer2D.h"

namespace XYZ {
	void RenderSortSystem::PushRenderData(const RenderComponent* renderable, const Transform2D* transform)
	{
		auto key = renderable->MaterialIns->GetSortKey();
		if (!(key & RenderFlags::InstancedFlag))
		{
			if (renderable->MaterialIns->GetSortKey() & RenderFlags::TransparentFlag)
			{
				m_Transparent.push_back({ renderable,transform });
			}
			else
			{
				m_Opaque.push_back({ renderable,transform });
			}
		}
	}
	void RenderSortSystem::SubmitToRenderer()
	{
		std::sort(m_Opaque.begin(), m_Opaque.end(), OpaqueComparator());
		std::sort(m_Transparent.begin(), m_Transparent.end(), TransparentComparator());

		if (!m_Opaque.empty())
		{
			std::shared_ptr<Material> material = m_Opaque[0].Renderable->MaterialIns;
			for (size_t i = 0; i < m_Opaque.size(); ++i)
			{
				if (material->GetSortKey() == m_Opaque[i].Renderable->MaterialIns->GetSortKey())
				{
					auto& elem = m_Opaque[i];
					auto quads = elem.Renderable->GetRenderData();
					for (size_t i = 0; i < elem.Renderable->GetCountQuads(); ++i)
						Renderer2D::SubmitQuad(elem.Transform->GetTransformation(), quads[i]);

				}
				else
				{
					material->Bind();
					Renderer2D::Flush();
					material = m_Opaque[i].Renderable->MaterialIns;
					auto& elem = m_Opaque[i];
					auto quads = elem.Renderable->GetRenderData();
					for (size_t i = 0; i < elem.Renderable->GetCountQuads(); ++i)
						Renderer2D::SubmitQuad(elem.Transform->GetTransformation(), quads[i]);
				}
			}
			material->Bind();
			Renderer2D::Flush();
		}


		if (!m_Transparent.empty())
		{
			std::shared_ptr<Material> material = m_Transparent[0].Renderable->MaterialIns;
			for (size_t i = 0; i < m_Transparent.size(); ++i)
			{
				if (material->GetSortKey() == m_Transparent[i].Renderable->MaterialIns->GetSortKey())
				{
					auto& elem = m_Transparent[i];
					auto quads = elem.Renderable->GetRenderData();
					for (size_t i = 0; i < elem.Renderable->GetCountQuads(); ++i)
						Renderer2D::SubmitQuad(elem.Transform->GetTransformation(), quads[i]);
				}
				else
				{
					material->Bind();
					Renderer2D::Flush();
					material = m_Transparent[i].Renderable->MaterialIns;

					auto& elem = m_Transparent[i];
					auto quads = elem.Renderable->GetRenderData();
					for (size_t i = 0; i < elem.Renderable->GetCountQuads(); ++i)
						Renderer2D::SubmitQuad(elem.Transform->GetTransformation(), quads[i]);
				}
			}

			material->Bind();
			Renderer2D::Flush();
		}

		m_Opaque.clear();
		m_Transparent.clear();
	}
}