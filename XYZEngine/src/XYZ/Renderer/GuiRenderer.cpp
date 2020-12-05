#include "stdafx.h"
#include "GuiRenderer.h"

#include "Renderer.h"
#include "Renderer2D.h"
#include "RenderPass.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {
	struct GuiRendererData
	{
		GuiRendererCamera RendererCamera;
		glm::vec2 ViewportSize;
		struct WidgetDrawCommand
		{
			CanvasRenderer* Renderer;
			RectTransform*  Transform;
		};

		std::vector<WidgetDrawCommand> WidgetDrawList;
	};

	GuiRendererData s_Data;

	void GuiRenderer::Init()
	{
	}
	void GuiRenderer::BeginScene(const GuiRendererCamera& camera, const glm::vec2& viewportSize)
	{
		s_Data.RendererCamera = camera;
		s_Data.ViewportSize = viewportSize;
	}

	void GuiRenderer::EndScene()
	{
		flushDrawList();
		s_Data.WidgetDrawList.clear();
	}

	void GuiRenderer::SubmitWidget(CanvasRenderer* canvasRenderer, RectTransform* transform)
	{
		if (cullTest(transform->WorldPosition, transform->Scale))
			s_Data.WidgetDrawList.push_back({ canvasRenderer, transform });
	}
	void GuiRenderer::flushDrawList()
	{
		glm::mat4 viewProjectionMatrix = s_Data.RendererCamera.Camera.GetProjectionMatrix() * s_Data.RendererCamera.ViewMatrix;
		Renderer2D::BeginScene(viewProjectionMatrix, s_Data.ViewportSize);
		
		std::sort(s_Data.WidgetDrawList.begin(), s_Data.WidgetDrawList.end(), [](const GuiRendererData::WidgetDrawCommand& a, const GuiRendererData::WidgetDrawCommand& b) {
			return a.Renderer->SortLayer < b.Renderer->SortLayer;
		});
	
		for (auto& dc : s_Data.WidgetDrawList)
		{	
			if (dc.Renderer->Material)
				Renderer2D::SetMaterial(dc.Renderer->Material);	
			uint32_t textureID = Renderer2D::SetTexture(dc.Renderer->SubTexture->GetTexture());
			Renderer2D::SubmitQuads(dc.Transform->GetWorldTransform(), dc.Renderer->Mesh.Vertices.data(), dc.Renderer->Mesh.Vertices.size() / 4.0f, textureID, dc.Renderer->TilingFactor);
		}
		Renderer2D::Flush();
		Renderer2D::FlushLines();
		Renderer2D::EndScene();
	}
	bool GuiRenderer::cullTest(const glm::vec3& position, const glm::vec2& scale)
	{
		//if (position.x - scale.x + s_Data.ViewportSize.x / 2.0f < s_Data.ViewportSize.x 
		// && position.x + scale.x + s_Data.ViewportSize.x / 2.0f > 0.0f
		// && position.y - scale.y + s_Data.ViewportSize.y / 2.0f < s_Data.ViewportSize.y 
		// && position.y + scale.y + s_Data.ViewportSize.y / 2.0f > 0.0f)
			return true;
		//return false;
	}
}
