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

		Ref<RenderPass> CompositePass;
		Ref<RenderPass> GeometryPass;
		Ref<RenderPass> BloomPass;
		Ref<RenderPass> GaussianBlurPass;
		Ref<Shader> GaussianBlurShader;
		Ref<Shader> BloomShader;
		Ref<Shader> CompositeShader;

		struct SpriteDrawCommand
		{
			SpriteRenderer* Sprite;
			glm::mat4 Transform;
		};
		struct ParticleDrawCommand
		{
			ParticleComponent* Particle;
			glm::mat4 Transform;
		};

		std::vector<SpriteDrawCommand> SpriteDrawList;
		std::vector<ParticleDrawCommand> ParticleDrawList;
	};

	static SceneRendererData s_Data;


	void SceneRenderer::Init()
	{
		// Composite pass
		{
			Ref<FrameBuffer> fbo = FrameBuffer::Create({ 1280, 720,{0.1f,0.1f,0.1f,1.0f} });
			fbo->CreateColorAttachment(FrameBufferFormat::RGBA16F); // Color
			fbo->CreateDepthAttachment();
			fbo->Resize();
			s_Data.CompositePass = RenderPass::Create({ fbo });
		}
		// Geometry pass
		{
			Ref<FrameBuffer> fbo = FrameBuffer::Create({ 1280, 720,{0.1f,0.1f,0.1f,1.0f} });
			fbo->CreateColorAttachment(FrameBufferFormat::RGBA16F); // Color
			fbo->CreateColorAttachment(FrameBufferFormat::RGBA16F); // Bright color
			fbo->CreateDepthAttachment();
			fbo->Resize();
			s_Data.GeometryPass = RenderPass::Create({ fbo });
		}
		// Bloom pass
		{
			Ref<FrameBuffer> fbo = FrameBuffer::Create({ 1280, 720,{0.1f,0.1f,0.1f,1.0f} });
			fbo->CreateColorAttachment(FrameBufferFormat::RGBA16F); // Color
			fbo->CreateDepthAttachment();
			fbo->Resize();
			s_Data.BloomPass = RenderPass::Create({ fbo });
		}
		// Gausian blur pass
		{
			Ref<FrameBuffer> fbo = FrameBuffer::Create({ 1280, 720,{0.1f,0.1f,0.1f,1.0f} });
			fbo->CreateColorAttachment(FrameBufferFormat::RGBA16F);
			fbo->CreateDepthAttachment();
			fbo->Resize();
			s_Data.GaussianBlurPass = RenderPass::Create({ fbo });
		}
		s_Data.GaussianBlurShader = Shader::Create("Assets/Shaders/GaussianBlurShader.glsl");
		s_Data.CompositeShader = Shader::Create("Assets/Shaders/CompositeShader.glsl");
		s_Data.BloomShader = Shader::Create("Assets/Shaders/BloomShader.glsl");
	}

	void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
	{
		// Geometry pass
		{
			auto& specs = s_Data.GeometryPass->GetSpecification().TargetFramebuffer->GetSpecification();
			specs.Width = width;
			specs.Height = height;
			s_Data.GeometryPass->GetSpecification().TargetFramebuffer->Resize();
		}
		// Gausian blur pass
		{
			auto& specs = s_Data.GaussianBlurPass->GetSpecification().TargetFramebuffer->GetSpecification();
			specs.Width = width;
			specs.Height = height;
			s_Data.GaussianBlurPass->GetSpecification().TargetFramebuffer->Resize();
		}
		// Composite pass
		{
			auto& specs = s_Data.BloomPass->GetSpecification().TargetFramebuffer->GetSpecification();
			specs.Width = width;
			specs.Height = height;
			s_Data.BloomPass->GetSpecification().TargetFramebuffer->Resize();
		}
		// Composite pass
		{
			auto& specs = s_Data.CompositePass->GetSpecification().TargetFramebuffer->GetSpecification();
			specs.Width = width;
			specs.Height = height;
			s_Data.CompositePass->GetSpecification().TargetFramebuffer->Resize();
		}
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
		s_Data.SpriteDrawList.push_back({ sprite,transform });
	}
	void SceneRenderer::SubmitParticles(ParticleComponent* particle, const glm::mat4& transform)
	{
		s_Data.ParticleDrawList.push_back({ particle,transform });
	}
	void SceneRenderer::SetGridProperties(const GridProperties& props)
	{
		s_Data.GridProps = props;
	}

	Ref<RenderPass> SceneRenderer::GetFinalRenderPass()
	{
		return s_Data.CompositePass;
	}
	
	uint32_t SceneRenderer::GetFinalColorBufferRendererID()
	{
		return s_Data.CompositePass->GetSpecification().TargetFramebuffer->GetColorAttachment(0).RendererID;
	}
	SceneRendererOptions& SceneRenderer::GetOptions()
	{
		return s_Data.Options;
	}
	void SceneRenderer::FlushDrawList()
	{
		std::sort(s_Data.SpriteDrawList.begin(), s_Data.SpriteDrawList.end(),
			[](const SceneRendererData::SpriteDrawCommand& a, const SceneRendererData::SpriteDrawCommand& b) {
				if (a.Sprite->SortLayer == b.Sprite->SortLayer)
					return a.Sprite->Material->GetSortKey() < b.Sprite->Material->GetSortKey();
				return a.Sprite->SortLayer < b.Sprite->SortLayer;
			});

		std::sort(s_Data.ParticleDrawList.begin(), s_Data.ParticleDrawList.end(),
			[](const SceneRendererData::ParticleDrawCommand& a, const SceneRendererData::ParticleDrawCommand& b) {
				return a.Particle->RenderMaterial->GetSortKey() < b.Particle->RenderMaterial->GetSortKey();
			});

		GeometryPass();
		GaussianBlurPass();
		BloomPass();
		CompositePass();
		Renderer::WaitAndRender();
		s_Data.SpriteDrawList.clear();
		s_Data.ParticleDrawList.clear();
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

		for (auto& dc : s_Data.SpriteDrawList)
		{
			Renderer2D::SetMaterial(dc.Sprite->Material);
			Renderer2D::SubmitQuad(dc.Transform, dc.Sprite->SubTexture->GetTexCoords(), dc.Sprite->TextureID, dc.Sprite->Color);
		}

		Renderer2D::Flush();
		Renderer2D::FlushLines();
		

		for (auto& dc : s_Data.ParticleDrawList)
		{
			auto material = dc.Particle->RenderMaterial->GetParentMaterial();
			auto materialInstace = dc.Particle->RenderMaterial;

			material->Set("u_ViewProjectionMatrix", viewProjectionMatrix);
			material->Bind();
			materialInstace->Set("u_Transform", dc.Transform);
			materialInstace->Bind();
			Renderer2D::SubmitParticles(dc.Transform, dc.Particle->ParticleEffect);
		}

		
		Renderer2D::EndScene();
		Renderer::EndRenderPass();
	}
	void SceneRenderer::CompositePass()
	{
		Renderer::BeginRenderPass(s_Data.CompositePass, false);
	
		s_Data.CompositeShader->Bind();
		Texture2D::BindStatic(s_Data.BloomPass->GetSpecification().TargetFramebuffer->GetColorAttachment(0).RendererID, 0);

		Renderer::SubmitFullsceenQuad();
		Renderer::EndRenderPass();
	}
	void SceneRenderer::BloomPass()
	{
		Renderer::BeginRenderPass(s_Data.BloomPass, false);
		float exposure = 0.8f;

		s_Data.BloomShader->Bind();
		s_Data.BloomShader->SetFloat("u_Exposure", exposure);

		Texture2D::BindStatic(s_Data.GeometryPass->GetSpecification().TargetFramebuffer->GetColorAttachment(0).RendererID, 0);
		Texture2D::BindStatic(s_Data.GaussianBlurPass->GetSpecification().TargetFramebuffer->GetColorAttachment(0).RendererID, 1);
		Renderer::SubmitFullsceenQuad();

		Renderer::EndRenderPass();
	}
	void SceneRenderer::GaussianBlurPass()
	{
		glm::mat4 viewProjectionMatrix = s_Data.SceneCamera.Camera.GetProjectionMatrix() * s_Data.SceneCamera.ViewMatrix;

		Renderer::BeginRenderPass(s_Data.GaussianBlurPass, true);
		uint32_t amount = 2;

		s_Data.GaussianBlurShader->Bind();
		s_Data.GaussianBlurShader->SetInt("u_Horizontal", 0);
		for (uint32_t i = 1; i <= amount; i++)
		{			
			Texture2D::BindStatic(s_Data.GeometryPass->GetSpecification().TargetFramebuffer->GetColorAttachment(1).RendererID, 0);
			Renderer::SubmitFullsceenQuad();
		}

		s_Data.GaussianBlurShader->SetInt("u_Horizontal", 5);
		for (uint32_t i = 1; i <= amount; i++)
		{		
			Texture2D::BindStatic(s_Data.GeometryPass->GetSpecification().TargetFramebuffer->GetColorAttachment(1).RendererID, 0);
			Renderer::SubmitFullsceenQuad();
		}
		Renderer::EndRenderPass();
	}
}