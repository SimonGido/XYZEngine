#include "stdafx.h"
#include "SceneRenderer.h"

#include "Renderer2D.h"
#include "Renderer.h"

#include "XYZ/Core/Input.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {
	struct SceneRendererData
	{
		const Scene* ActiveScene = nullptr;

		SceneRendererCamera SceneCamera;
		SceneRendererOptions Options;
		GridProperties GridProps;
		glm::mat4 ViewProjectionMatrix;

		Ref<RenderPass> CompositePass;
		Ref<RenderPass> LightPass;
		Ref<RenderPass> GeometryPass;
		Ref<RenderPass> BloomPass;
		Ref<RenderPass> GaussianBlurPass;

		Ref<Shader> GaussianBlurShader;
		Ref<Shader> BloomShader;
		Ref<Shader> CompositeShader;
		Ref<Shader> LightShader;


		Ref<ShaderStorageBuffer> LightStorageBuffer;
		

		struct SpriteDrawCommand
		{
			SpriteRenderer* Sprite;
			TransformComponent* Transform;
		};
		struct ParticleDrawCommand
		{
			ParticleComponent* Particle;
			TransformComponent* Transform;
		};

		struct CollisionDrawCommand
		{
			TransformComponent* Transform;
			uint32_t ID;
		};

		struct PointLight
		{	
			glm::vec4 Position;
			glm::vec3 Color;
			float Intensity = 1.0f;
		};


		std::vector<CollisionDrawCommand> CollisionList;
		std::vector<SpriteDrawCommand> SpriteDrawList;
		std::vector<ParticleDrawCommand> ParticleDrawList;
		std::vector<PointLight> LightsList;


		glm::vec2 ViewportSize;

		const uint32_t MaxNumberOfLights = 100;
	};

	static SceneRendererData s_Data;


	void SceneRenderer::Init()
	{
		// Composite pass
		{
			FramebufferSpecs specs;
			specs.ClearColor = { 0.1f,0.1f,0.1f,1.0f };
			specs.Attachments = {
				FramebufferTextureSpecs(FramebufferTextureFormat::RGBA16F),
				FramebufferTextureSpecs(FramebufferTextureFormat::DEPTH24STENCIL8)
			};
			Ref<Framebuffer> fbo = Framebuffer::Create(specs);
			s_Data.CompositePass = RenderPass::Create({ fbo });
		}

		// Light pass
		{
			FramebufferSpecs specs;
			specs.ClearColor = { 0.0f,0.0f,0.0f,1.0f };
			specs.Attachments = {
				FramebufferTextureSpecs(FramebufferTextureFormat::RGBA16F),
				FramebufferTextureSpecs(FramebufferTextureFormat::DEPTH24STENCIL8)
			};
			Ref<Framebuffer> fbo = Framebuffer::Create(specs);
			s_Data.LightPass = RenderPass::Create({ fbo });
		}
		// Geometry pass
		{
			FramebufferSpecs specs;
			specs.ClearColor = { 0.1f,0.1f,0.1f,1.0f };
			specs.Attachments = {
				FramebufferTextureSpecs(FramebufferTextureFormat::RGBA16F),
				FramebufferTextureSpecs(FramebufferTextureFormat::RGBA16F),
				FramebufferTextureSpecs(FramebufferTextureFormat::R32I),
				FramebufferTextureSpecs(FramebufferTextureFormat::DEPTH24STENCIL8)
			};
			Ref<Framebuffer> fbo = Framebuffer::Create(specs);
			s_Data.GeometryPass = RenderPass::Create({ fbo });
		}
		// Bloom pass
		{
			FramebufferSpecs specs;
			specs.ClearColor = { 0.0f,0.0f,0.0f,1.0f };
			specs.Attachments = {
				FramebufferTextureSpecs(FramebufferTextureFormat::RGBA16F),
				FramebufferTextureSpecs(FramebufferTextureFormat::DEPTH24STENCIL8)
			};
			Ref<Framebuffer> fbo = Framebuffer::Create(specs);
			s_Data.BloomPass = RenderPass::Create({ fbo });
		}
		// Gausian blur pass
		{
			FramebufferSpecs specs;
			specs.ClearColor = { 0.0f,0.0f,0.0f,1.0f };
			specs.Attachments = {
				FramebufferTextureSpecs(FramebufferTextureFormat::RGBA16F),
				FramebufferTextureSpecs(FramebufferTextureFormat::DEPTH24STENCIL8)
			};
			Ref<Framebuffer> fbo = Framebuffer::Create(specs);
			s_Data.GaussianBlurPass = RenderPass::Create({ fbo });
		}
		s_Data.GaussianBlurShader = Shader::Create("Assets/Shaders/GaussianBlurShader.glsl");
		s_Data.BloomShader = Shader::Create("Assets/Shaders/BloomShader.glsl");
		s_Data.CompositeShader = Shader::Create("Assets/Shaders/CompositeShader.glsl");
		s_Data.LightShader = Shader::Create("Assets/Shaders/LightShader.glsl");
		s_Data.LightStorageBuffer = ShaderStorageBuffer::Create(s_Data.MaxNumberOfLights * sizeof(SceneRendererData::PointLight));
	}

	void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
	{
		s_Data.ViewportSize = glm::vec2(width, height);
	
		s_Data.GeometryPass->GetSpecification().TargetFramebuffer->Resize(width, height);	
		s_Data.LightPass->GetSpecification().TargetFramebuffer->Resize(width, height);
		s_Data.GaussianBlurPass->GetSpecification().TargetFramebuffer->Resize(width, height);
		s_Data.BloomPass->GetSpecification().TargetFramebuffer->Resize(width, height);
		s_Data.CompositePass->GetSpecification().TargetFramebuffer->Resize(width, height);	
	}
	void SceneRenderer::BeginScene(const Scene* scene, const SceneRendererCamera& camera)
	{
		XYZ_ASSERT(!s_Data.ActiveScene, "Missing end scene");
		s_Data.ActiveScene = scene;
		s_Data.SceneCamera = camera;

	
		s_Data.ViewProjectionMatrix = s_Data.SceneCamera.Camera.GetProjectionMatrix() * s_Data.SceneCamera.ViewMatrix;
	}
	void SceneRenderer::BeginScene(const Scene* scene, const glm::mat4 viewProjectionMatrix)
	{
		XYZ_ASSERT(!s_Data.ActiveScene, "Missing end scene");
		s_Data.ActiveScene = scene;

		s_Data.ViewProjectionMatrix = viewProjectionMatrix;
	}
	void SceneRenderer::EndScene()
	{
		XYZ_ASSERT(s_Data.ActiveScene, "Missing begin scene");
		s_Data.ActiveScene = nullptr;

		FlushDrawList();
	}
	static SkeletalMesh* s_Mesh;

	void SceneRenderer::SubmitSkeletalMesh(SkeletalMesh* mesh)
	{
		s_Mesh = mesh;
	}

	void SceneRenderer::SubmitSprite(SpriteRenderer* sprite, TransformComponent* transform)
	{
		s_Data.SpriteDrawList.push_back({ sprite,transform });
	}
	void SceneRenderer::SubmitCollision(TransformComponent* transform, uint32_t collisionID)
	{
		s_Data.CollisionList.push_back({ transform, collisionID });
	}
	void SceneRenderer::SubmitParticles(ParticleComponent* particle, TransformComponent* transform)
	{
		s_Data.ParticleDrawList.push_back({ particle,transform });
	}
	void SceneRenderer::SubmitLight(PointLight2D* light, const glm::mat4& transform)
	{
		XYZ_ASSERT(s_Data.LightsList.size() + 1 < s_Data.MaxNumberOfLights, "Max number of lights per scene is ", s_Data.MaxNumberOfLights);
		SceneRendererData::PointLight lightData;
		lightData.Position = glm::vec4(transform[3][0], transform[3][1], transform[3][2], 0.0f);
		lightData.Color = light->Color;
		lightData.Intensity = light->Intensity;
		s_Data.LightsList.push_back(lightData);
	}

	void SceneRenderer::SetGridProperties(const GridProperties& props)
	{
		s_Data.GridProps = props;
	}

	Ref<RenderPass> SceneRenderer::GetFinalRenderPass()
	{
		return s_Data.CompositePass;
	}

	Ref<RenderPass> SceneRenderer::GetCollisionRenderPass()
	{
		return s_Data.GeometryPass;
	}

	uint32_t SceneRenderer::GetFinalColorBufferRendererID()
	{
		return s_Data.CompositePass->GetSpecification().TargetFramebuffer->GetColorAttachmentRendererID(0);
	}
	SceneRendererOptions& SceneRenderer::GetOptions()
	{
		return s_Data.Options;
	}
	void SceneRenderer::FlushDrawList()
	{
		std::sort(s_Data.SpriteDrawList.begin(), s_Data.SpriteDrawList.end(),
			[](const SceneRendererData::SpriteDrawCommand& a, const SceneRendererData::SpriteDrawCommand& b) {
				return a.Transform->Translation.z < b.Transform->Translation.z;

				if (a.Sprite->SortLayer == b.Sprite->SortLayer)
					return a.Sprite->Material->GetFlags() < b.Sprite->Material->GetFlags();
				return a.Sprite->SortLayer < b.Sprite->SortLayer;
			});
		
		std::sort(s_Data.ParticleDrawList.begin(), s_Data.ParticleDrawList.end(),
			[](const SceneRendererData::ParticleDrawCommand& a, const SceneRendererData::ParticleDrawCommand& b) {
				return a.Particle->RenderMaterial->GetFlags() < b.Particle->RenderMaterial->GetFlags();
			});

		GeometryPass();
		LightPass();
		BloomPass();
		//GaussianBlurPass();
		CompositePass();

		auto [width, height] = Input::GetWindowSize();
		Renderer::SetViewPort(0, 0, (uint32_t)width, (uint32_t)height);
		Renderer::WaitAndRender();

		s_Data.CollisionList.clear();
		s_Data.SpriteDrawList.clear();
		s_Data.ParticleDrawList.clear();
		s_Data.LightsList.clear();
	}

	void SceneRenderer::GeometryPass()
	{
		Renderer::BeginRenderPass(s_Data.GeometryPass, true);
		Renderer2D::BeginScene(s_Data.ViewProjectionMatrix);
		//int clearValue = -1;
		//s_Data.GeometryPass->GetSpecification().TargetFramebuffer->ClearColorAttachment(2, &clearValue);

		
		if (s_Data.Options.ShowGrid)
		{
			Renderer2D::SubmitGrid(s_Data.GridProps.Transform, s_Data.GridProps.Scale, s_Data.GridProps.LineWidth);
		}

		for (auto& dc : s_Data.CollisionList)
		{
			Renderer2D::SubmitCollisionQuad(dc.Transform->GetTransform(), dc.ID);
		}

		for (auto& dc : s_Data.SpriteDrawList)
		{
			Renderer2D::SetMaterial(dc.Sprite->Material);
			uint32_t textureID = Renderer2D::SetTexture(dc.Sprite->SubTexture->GetTexture());
			Renderer2D::SubmitQuad(dc.Transform->GetTransform(), dc.Sprite->SubTexture->GetTexCoords(), textureID, dc.Sprite->Color);
		}
		s_Mesh->Render();

		Renderer2D::Flush();
		Renderer2D::FlushLines();
		Renderer2D::FlushCollisions();

		for (auto& dc : s_Data.ParticleDrawList)
		{
			auto material = dc.Particle->RenderMaterial->GetParentMaterial();
			auto materialInstace = dc.Particle->RenderMaterial;

			material->Set("u_ViewProjectionMatrix", s_Data.ViewProjectionMatrix);
			material->Bind();
			materialInstace->Set("u_Transform", dc.Transform->GetTransform());
			materialInstace->Bind();
			Renderer2D::SubmitParticles(dc.Transform->GetTransform(), dc.Particle->ParticleEffect);
		}

		
		Renderer2D::EndScene();
		Renderer::EndRenderPass();		
	}

	void SceneRenderer::LightPass()
	{
		Renderer::BeginRenderPass(s_Data.LightPass, true);

		s_Data.LightShader->Bind();
		s_Data.LightShader->SetInt("u_NumberOfLights", s_Data.LightsList.size());
		
		if (s_Data.LightsList.size())
		{
			s_Data.LightStorageBuffer->Update(s_Data.LightsList.data(), s_Data.LightsList.size() * sizeof(SceneRendererData::PointLight));
			s_Data.LightStorageBuffer->BindRange(0, s_Data.LightsList.size() * sizeof(SceneRendererData::PointLight), 0);
		}

		s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindTexture(0, 0);
		s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindTexture(1, 1);
	
		Renderer::SubmitFullsceenQuad();
		
		Renderer::EndRenderPass();
	}

	void SceneRenderer::BloomPass()
	{
		Renderer::BeginRenderPass(s_Data.BloomPass, true);
		float exposure = 0.7f;

		s_Data.BloomShader->Bind();
		s_Data.BloomShader->SetFloat("u_Exposure", exposure);
		s_Data.LightPass->GetSpecification().TargetFramebuffer->BindTexture(0, 0);

		Renderer::SubmitFullsceenQuad();

		Renderer::EndRenderPass();
	}
	void SceneRenderer::GaussianBlurPass()
	{
		Renderer::BeginRenderPass(s_Data.GaussianBlurPass, true);
		uint32_t amount = 2;

		s_Data.GaussianBlurShader->Bind();
		s_Data.GaussianBlurShader->SetInt("u_Horizontal", 0);
		for (uint32_t i = 1; i <= amount; i++)
		{
			s_Data.BloomPass->GetSpecification().TargetFramebuffer->BindTexture(0, 0);
			Renderer::SubmitFullsceenQuad();
		}

		s_Data.GaussianBlurShader->SetInt("u_Horizontal", 5);
		for (uint32_t i = 1; i <= amount; i++)
		{
			s_Data.BloomPass->GetSpecification().TargetFramebuffer->BindTexture(0, 0);
			Renderer::SubmitFullsceenQuad();
		}
		Renderer::EndRenderPass();
	}

	void SceneRenderer::CompositePass()
	{
		Renderer::BeginRenderPass(s_Data.CompositePass, true);

		s_Data.CompositeShader->Bind();
		s_Data.LightPass->GetSpecification().TargetFramebuffer->BindTexture(0, 0);
		s_Data.BloomPass->GetSpecification().TargetFramebuffer->BindTexture(0, 1);

		Renderer::SubmitFullsceenQuad();	
		Renderer::EndRenderPass();
	}
}