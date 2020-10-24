#include "stdafx.h"
#include "SceneRenderer.h"

#include "Renderer2D.h"
#include "Renderer.h"


#include <glm/gtx/transform.hpp>

namespace XYZ {
	struct SceneRendererData
	{
		const Scene* ActiveScene = nullptr;
		
		SceneRendererCamera SceneCamera;
		SceneRendererOptions Options;
		GridProperties GridProps;

		Ref<RenderPass> GeometryPass;

		struct SpriteDrawCommand
		{
			SpriteRenderer* Sprite;
			glm::mat4 Transform;
		};
		std::vector<SpriteDrawCommand> DrawList;
	};

	static SceneRendererData s_Data;


	void SceneRenderer::Init()
	{
		Ref<FrameBuffer> fbo = FrameBuffer::Create({ 1280, 720,{0.2f,0.2f,0.2f,1.0f} });
		fbo->CreateColorAttachment(FrameBufferFormat::RGBA16F);
		fbo->CreateDepthAttachment();
		fbo->Resize();
		s_Data.GeometryPass = RenderPass::Create({ fbo });
	}

	void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
	{
		auto& specs = s_Data.GeometryPass->GetSpecification().TargetFramebuffer->GetSpecification();
		specs.Width = width;
		specs.Height = height;
		s_Data.GeometryPass->GetSpecification().TargetFramebuffer->Resize();
	}
	void SceneRenderer::BeginScene(const Scene* scene, const SceneRendererCamera& camera)
	{
		XYZ_ASSERT(!s_Data.ActiveScene, "Missing end scene");
		s_Data.ActiveScene = scene;
		s_Data.SceneCamera = camera;
	}
	void SceneRenderer::EndScene()
	{
		XYZ_ASSERT(s_Data.ActiveScene, "Missing begin scene");
		s_Data.ActiveScene = nullptr;
		
		FlushDrawList();
	}
	void SceneRenderer::SubmitSprite(SpriteRenderer* sprite, const glm::mat4& transform)
	{
		s_Data.DrawList.push_back({ sprite,transform });
	}
	void SceneRenderer::SetGridProperties(const GridProperties& props)
	{
		s_Data.GridProps = props;
	}

	Ref<RenderPass> SceneRenderer::GetFinalRenderPass()
	{
		return s_Data.GeometryPass;
	}
	
	uint32_t SceneRenderer::GetFinalColorBufferRendererID()
	{
		return  s_Data.GeometryPass->GetSpecification().TargetFramebuffer->GetColorAttachment(0).RendererID;
	}
	SceneRendererOptions& SceneRenderer::GetOptions()
	{
		return s_Data.Options;
	}
	void SceneRenderer::FlushDrawList()
	{
		GeometryPass();
		s_Data.DrawList.clear();
	}
	void SceneRenderer::GeometryPass()
	{
		glm::mat4 viewProjectionMatrix = s_Data.SceneCamera.Camera.GetProjectionMatrix() * s_Data.SceneCamera.ViewMatrix;
		
		Renderer::BeginRenderPass(s_Data.GeometryPass, true);
		Renderer2D::BeginScene(viewProjectionMatrix);
		
		if (s_Data.Options.ShowGrid)
		{
			Renderer2D::SubmitGrid(s_Data.GridProps.Transform, s_Data.GridProps.Scale, s_Data.GridProps.LineWidth);
		}

		std::sort(s_Data.DrawList.begin(), s_Data.DrawList.end(), 
			[](const SceneRendererData::SpriteDrawCommand& a, const SceneRendererData::SpriteDrawCommand& b) {
				if (a.Sprite->SortLayer == b.Sprite->SortLayer)
					return a.Sprite->Material->GetSortKey() < b.Sprite->Material->GetSortKey();
				return a.Sprite->SortLayer < b.Sprite->SortLayer;
		});

		
		for (auto& dc : s_Data.DrawList)
		{
			Renderer2D::SetMaterial(dc.Sprite->Material);
			Renderer2D::SubmitQuad(dc.Transform, dc.Sprite->SubTexture->GetTexCoords(), dc.Sprite->TextureID, dc.Sprite->Color);
		}
		Renderer2D::Flush();
		Renderer2D::FlushLines();
		Renderer2D::EndScene();

		Renderer::EndRenderPass();
		Renderer::WaitAndRender();
	}
}