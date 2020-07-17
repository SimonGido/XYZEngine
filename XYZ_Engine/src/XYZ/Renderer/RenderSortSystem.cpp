#include "stdafx.h"
#include "RenderSortSystem.h"

#include "Renderer2D.h"


namespace XYZ {
	void RenderSortSystem::PushRenderData(const RenderComponent2D* renderable, const Transform2D* transform)
	{
		auto material = renderable->Material;
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
			Renderer2D::SetMaterial(elem.Renderable->Material);
			Renderer2D::SubmitMesh(elem.Transform->GetTransformation(), elem.Renderable->Mesh);
		}
		Renderer2D::Flush();
		for (auto& elem : m_Transparent)
		{
			Renderer2D::SetMaterial(elem.Renderable->Material);
			Renderer2D::SubmitMesh(elem.Transform->GetTransformation(), elem.Renderable->Mesh);
		}
		Renderer2D::Flush();

		m_Opaque.clear();
		m_Transparent.clear();

	}
}