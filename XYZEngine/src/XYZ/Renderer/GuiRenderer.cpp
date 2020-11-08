#include "stdafx.h"
#include "GuiRenderer.h"

#include "Renderer.h"
#include "Renderer2D.h"
#include "RenderPass.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {
	struct GuiRendererData
	{
		glm::vec2 ViewportSize;

		struct WidgetDrawCommand
		{
			CanvasRenderer* Renderer;
			glm::vec3	    Position;
			glm::vec2		Size;
		};

		std::vector<WidgetDrawCommand> WidgetDrawList;
	};

	GuiRendererData s_Data;

	void GuiRenderer::Init()
	{
	}
	void GuiRenderer::BeginScene(const glm::vec2& viewportSize)
	{
		s_Data.ViewportSize = viewportSize;
	}

	void GuiRenderer::EndScene()
	{
		flushDrawList();
		s_Data.WidgetDrawList.clear();
	}
	void GuiRenderer::SetMaterial(const Ref<Material>& material)
	{
	}
	void GuiRenderer::SubmitWidget(CanvasRenderer* canvasRenderer, const glm::vec3& position, const glm::vec2& size)
	{
		s_Data.WidgetDrawList.push_back({ canvasRenderer, position, size });
	}
	void GuiRenderer::flushDrawList()
	{
		Renderer2D::BeginScene(glm::mat4(1.0f), s_Data.ViewportSize);
		
		Ref<Material> currentMaterial = nullptr;
		if (s_Data.WidgetDrawList.size())
		{
			currentMaterial = s_Data.WidgetDrawList.back().Renderer->Material;
			currentMaterial->Set("u_ViewportSize", s_Data.ViewportSize);
			Renderer2D::SetMaterial(currentMaterial);
		}
		for (auto& dc : s_Data.WidgetDrawList)
		{
			if (currentMaterial->GetSortKey() != dc.Renderer->Material->GetSortKey())
			{
				currentMaterial = dc.Renderer->Material;
				currentMaterial->Set("u_ViewportSize", s_Data.ViewportSize);
				Renderer2D::SetMaterial(currentMaterial);
			}		
			Renderer2D::SubmitQuad(dc.Position, dc.Size, dc.Renderer->SubTexture->GetTexCoords(), dc.Renderer->TextureID, dc.Renderer->Color);
		}
		Renderer2D::Flush();
		Renderer2D::FlushLines();
		Renderer2D::EndScene();
	}
}
