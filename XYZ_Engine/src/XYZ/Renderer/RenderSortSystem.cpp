#include "stdafx.h"
#include "RenderSortSystem.h"

#include "Renderer2D.h"


namespace XYZ {
	void RenderSortSystem::PushRenderData(const RenderComponent* renderable, const Transform2D* transform)
	{
		auto material = renderable->MaterialIns;
		if (!material->IsSet(RenderFlags::InstancedFlag))
		{
			if (material->IsSet(RenderFlags::TransparentFlag))
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
	
		for (auto& elem : m_Opaque)
		{
			Renderer2D::SetMaterial(elem.Renderable->MaterialIns);
			auto quads = elem.Renderable->GetRenderData();
			for (size_t i = 0; i < elem.Renderable->GetCountQuads(); ++i)
				Renderer2D::SubmitQuad(elem.Transform->GetTransformation(), quads[i]);
		}
		Renderer2D::Flush();
		for (auto& elem : m_Transparent)
		{
			Renderer2D::SetMaterial(elem.Renderable->MaterialIns);
			auto quads = elem.Renderable->GetRenderData();
			for (size_t i = 0; i < elem.Renderable->GetCountQuads(); ++i)
				Renderer2D::SubmitQuad(elem.Transform->GetTransformation(), quads[i]);
		}
		Renderer2D::Flush();

		m_Opaque.clear();
		m_Transparent.clear();

	}
}