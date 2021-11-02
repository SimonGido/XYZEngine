#include "stdafx.h"
#include "SceneRenderer.h"

#include "Renderer2D.h"
#include "Renderer.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Debug/Profiler.h"
#include "XYZ/Renderer/Renderer.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {

	static ThreadPool s_ThreadPool;

	SceneRenderer::SceneRenderer()
		:
		m_ActiveScene(nullptr)
	{
		m_ThreadIndex = s_ThreadPool.PushThread();
		// Composite pass
		{
			FramebufferSpecs specs;
			specs.ClearColor = { 0.0f,0.0f,0.0f,1.0f };
			specs.Attachments = {
				FramebufferTextureSpecs(ImageFormat::RGBA16F),
				FramebufferTextureSpecs(ImageFormat::DEPTH24STENCIL8)
			};

			#ifdef IMGUI_BUILD
			{
				specs.SwapChainTarget = false;
			}
			#else
			{
				specs.SwapChainTarget = true;
			}
			#endif
			Ref<Framebuffer> fbo = Framebuffer::Create(specs);
			m_CompositePass = RenderPass::Create({ fbo });
		}

		// Light pass
		{
			FramebufferSpecs specs;
			specs.ClearColor = { 0.0f,0.0f,0.0f,0.0f };
			specs.Attachments = {
				FramebufferTextureSpecs(ImageFormat::RGBA16F, true),
				FramebufferTextureSpecs(ImageFormat::DEPTH24STENCIL8)
			};
			Ref<Framebuffer> fbo = Framebuffer::Create(specs);
			m_LightPass = RenderPass::Create({ fbo });
		}
		// Geometry pass
		{
			FramebufferSpecs specs;
			specs.ClearColor = { 0.0f,0.0f,0.0f,0.0f };
			specs.Attachments = {
				FramebufferTextureSpecs(ImageFormat::RGBA16F),
				FramebufferTextureSpecs(ImageFormat::RGBA16F),
				FramebufferTextureSpecs(ImageFormat::DEPTH24STENCIL8)
			};
			Ref<Framebuffer> fbo = Framebuffer::Create(specs);
			m_GeometryPass = RenderPass::Create({ fbo });
		}
		// Bloom pass
		{
			FramebufferSpecs specs;
			specs.ClearColor = { 0.0f,0.0f,0.0f, 0.0f };
			specs.Attachments = {
				FramebufferTextureSpecs(ImageFormat::RGBA16F),
				FramebufferTextureSpecs(ImageFormat::DEPTH24STENCIL8)
			};
			Ref<Framebuffer> fbo = Framebuffer::Create(specs);
			m_BloomPass = RenderPass::Create({ fbo });
		}

		auto shaderLibrary   = Renderer::GetShaderLibrary();
		m_CompositeShader	 = shaderLibrary->Get("CompositeShader");
		m_LightShader		 = shaderLibrary->Get("LightShader");
		m_BloomComputeShader = shaderLibrary->Get("Bloom");


		m_BloomTexture[0] = Texture2D::Create(ImageFormat::RGBA32F, 1280, 720, {});
		m_BloomTexture[1] = Texture2D::Create(ImageFormat::RGBA32F, 1280, 720, {});
		m_BloomTexture[2] = Texture2D::Create(ImageFormat::RGBA32F, 1280, 720, {});

		m_LightStorageBuffer     = ShaderStorageBuffer::Create(sc_MaxNumberOfLights * sizeof(SceneRenderer::PointLight), 1);
		m_SpotLightStorageBuffer = ShaderStorageBuffer::Create(sc_MaxNumberOfLights * sizeof(SceneRenderer::SpotLight), 2);
		m_CameraUniformBuffer    = UniformBuffer::Create(sizeof(CameraData), 0);
	}

	SceneRenderer::~SceneRenderer()
	{
		s_ThreadPool.EraseThread(m_ThreadIndex);
	}

	void SceneRenderer::Release() const
	{
	}

	void SceneRenderer::SetRenderer2D(const Ref<Renderer2D>& renderer2D)
	{
		m_Renderer2D = renderer2D;
	}

	void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
	{
		if (m_ViewportSize.x != width || m_ViewportSize.y != height)
		{
			m_ViewportSize = glm::ivec2(width, height);
			m_ViewportSizeChanged = true;
		}
	}
	void SceneRenderer::BeginScene(const Scene* scene, const SceneRendererCamera& camera)
	{
		XYZ_ASSERT(!m_ActiveScene, "Missing end scene");
		m_ActiveScene = scene;
		m_SceneCamera = camera;

		// Viewport size is changed at the beginning of the frame, so we do not delete texture that is currently use for rendering
		UpdateViewportSize();
		m_CameraBuffer.ViewProjectionMatrix = m_SceneCamera.Camera.GetProjectionMatrix() * m_SceneCamera.ViewMatrix;
		m_CameraBuffer.ViewMatrix = m_SceneCamera.ViewMatrix;
		m_CameraBuffer.ViewPosition = glm::vec4(camera.ViewPosition, 0.0f);
		
		m_CameraUniformBuffer->Update(&m_CameraBuffer, sizeof(CameraData), 0);
	}
	void SceneRenderer::BeginScene(const Scene* scene, const glm::mat4& viewProjectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& viewPosition)
	{
		XYZ_ASSERT(!m_ActiveScene, "Missing end scene");
		m_ActiveScene = scene;

		// Viewport size is changed at the beginning of the frame, so we do not delete texture that is currently use for rendering
		UpdateViewportSize();

		m_CameraBuffer.ViewProjectionMatrix = viewProjectionMatrix;
		m_CameraBuffer.ViewMatrix = viewMatrix;
		m_CameraBuffer.ViewPosition = glm::vec4(viewPosition, 0.0f);

		m_CameraUniformBuffer->Update(&m_CameraBuffer, sizeof(CameraData), 0);
	}
	void SceneRenderer::EndScene()
	{
		XYZ_ASSERT(m_ActiveScene, "Missing begin scene");
		m_ActiveScene = nullptr;

		flush();
	}

	void SceneRenderer::SubmitSprite(Ref<Material> material, Ref<SubTexture> subTexture, uint32_t sortLayer, const glm::vec4& color, const glm::mat4& transform)
	{
		m_Queues[material->GetRenderQueueID()].m_SpriteDrawList.push_back({
			   material, subTexture, sortLayer, color, transform
			});
	}

	void SceneRenderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform)
	{
		m_Queues[mesh->GetMaterial()->GetRenderQueueID()].m_MeshCommandList.push_back({ mesh, transform });
	}
	void SceneRenderer::SubmitMeshInstanced(Ref<Mesh> mesh, const glm::mat4& transform, uint32_t count)
	{
		m_Queues[mesh->GetMaterial()->GetRenderQueueID()].m_InstancedMeshCommandList.push_back({ mesh, transform, count });
	}
	void SceneRenderer::SubmitMeshInstanced(Ref<Mesh> mesh, const std::vector<glm::mat4>& transforms, uint32_t count)
	{
	}
	void SceneRenderer::SubmitLight(const PointLight2D& light, const glm::mat4& transform)
	{
		XYZ_ASSERT(m_PointLightsList.size() + 1 < sc_MaxNumberOfLights, "Max number of lights per scene is ", sc_MaxNumberOfLights);
		SubmitLight(light, glm::vec3(transform[3][0], transform[3][1], 0.0f));
	}
	void SceneRenderer::SubmitLight(const SpotLight2D& light, const glm::mat4& transform)
	{
		XYZ_ASSERT(m_SpotLightsList.size() + 1 < sc_MaxNumberOfLights, "Max number of lights per scene is ", sc_MaxNumberOfLights);
		SubmitLight(light, glm::vec3(transform[3][0], transform[3][1], 0.0f));
	}
	void SceneRenderer::SubmitLight(const PointLight2D& light, const glm::vec3& position)
	{
		XYZ_ASSERT(m_PointLightsList.size() + 1 < sc_MaxNumberOfLights, "Max number of lights per scene is ", sc_MaxNumberOfLights);

		PointLight lightData;
		lightData.Position = glm::vec2(position.x, position.y);
		lightData.Color = glm::vec4(light.Color, 0.0f);
		lightData.Radius = light.Radius;
		lightData.Intensity = light.Intensity;
		m_PointLightsList.push_back(lightData);
	}
	void SceneRenderer::SubmitLight(const SpotLight2D& light, const glm::vec3& position)
	{
		XYZ_ASSERT(m_SpotLightsList.size() + 1 < sc_MaxNumberOfLights, "Max number of lights per scene is ", sc_MaxNumberOfLights);

		SpotLight lightData;
		lightData.Position = glm::vec2(position.x, position.y);
		lightData.Color = glm::vec4(light.Color, 0.0f);
		lightData.Radius = light.Radius;
		lightData.Intensity = light.Intensity;
		lightData.InnerAngle = light.InnerAngle;
		lightData.OuterAngle = light.OuterAngle;
		m_SpotLightsList.push_back(lightData);
	}
	void SceneRenderer::SetGridProperties(const GridProperties& props)
	{
		m_GridProps = props;
	}

	void SceneRenderer::UpdateViewportSize()
	{
		if (m_ViewportSizeChanged)
		{
			uint32_t width = (uint32_t)m_ViewportSize.x;
			uint32_t height = (uint32_t)m_ViewportSize.y;
			m_GeometryPass->GetSpecification().TargetFramebuffer->Resize(width, height);
			m_LightPass->GetSpecification().TargetFramebuffer->Resize(width, height);
			m_BloomPass->GetSpecification().TargetFramebuffer->Resize(width, height);
			m_CompositePass->GetSpecification().TargetFramebuffer->Resize(width, height);
			m_BloomTexture[0] = Texture2D::Create(ImageFormat::RGBA32F, width, height, {});
			m_BloomTexture[1] = Texture2D::Create(ImageFormat::RGBA32F, width, height, {});
			m_BloomTexture[2] = Texture2D::Create(ImageFormat::RGBA32F, width, height, {});
			m_ViewportSizeChanged = false;
		}
	}

	Ref<RenderPass> SceneRenderer::GetFinalRenderPass()
	{
		return m_CompositePass;
	}

	uint32_t SceneRenderer::GetFinalColorBufferRendererID()
	{
		return m_CompositePass->GetSpecification().TargetFramebuffer->GetColorAttachmentRendererID(0);
	}
	SceneRendererOptions& SceneRenderer::GetOptions()
	{
		return m_Options;
	}
	void SceneRenderer::flush()
	{
		XYZ_PROFILE_FUNC("SceneRenderer::flush");
		flushLightQueue();
		flushDefaultQueue();
		Renderer::BeginRenderPass(m_CompositePass, true);

		m_CompositeShader->Bind();
		m_LightPass->GetSpecification().TargetFramebuffer->BindTexture(0, 0);
		m_BloomTexture[2]->Bind(1);

		Renderer::SubmitFullscreenQuad();
		Renderer::EndRenderPass();

		auto [width, height] = Input::GetWindowSize();
		Renderer::SetViewPort(0, 0, (uint32_t)width, (uint32_t)height);
	}
	void SceneRenderer::flushLightQueue()
	{
		XYZ_PROFILE_FUNC("SceneRenderer::flushLightQueue");
		RenderQueue& queue = m_Queues[LightQueue];
		sortQueue(queue);


		if (m_PointLightsList.size())
		{
			m_LightStorageBuffer->Update(m_PointLightsList.data(), (uint32_t)m_PointLightsList.size() * (uint32_t)sizeof(PointLight));
			m_LightStorageBuffer->BindRange(0, (uint32_t)m_PointLightsList.size() * (uint32_t)sizeof(PointLight));
		}

		if (m_SpotLightsList.size())
		{
			m_SpotLightStorageBuffer->Update(m_SpotLightsList.data(), (uint32_t)m_SpotLightsList.size() * (uint32_t)sizeof(SpotLight));
			m_SpotLightStorageBuffer->BindRange(0, (uint32_t)m_SpotLightsList.size() * (uint32_t)sizeof(SpotLight));
		}
		geometryPass(queue, m_GeometryPass, true);
		lightPass();
		bloomPass();

		queue.m_SpriteDrawList.clear();
		queue.m_MeshCommandList.clear();
		queue.m_InstancedMeshCommandList.clear();
		m_PointLightsList.clear();
		m_SpotLightsList.clear();
	}
	void SceneRenderer::flushDefaultQueue()
	{
		XYZ_PROFILE_FUNC("SceneRenderer::flushDefaultQueue");
		RenderQueue& queue = m_Queues[DefaultQueue];
		sortQueue(queue);

		geometryPass(queue, m_LightPass, false);

		queue.m_SpriteDrawList.clear();
		queue.m_MeshCommandList.clear();
		queue.m_InstancedMeshCommandList.clear();
	}

	void SceneRenderer::sortQueue(RenderQueue& queue)
	{
		XYZ_PROFILE_FUNC("SceneRenderer::sortQueue");
		std::sort(queue.m_SpriteDrawList.begin(), queue.m_SpriteDrawList.end(),
			[](const RenderQueue::SpriteDrawCommand& a, const RenderQueue::SpriteDrawCommand& b) {
			if (a.SortLayer == b.SortLayer)
				return a.Material->GetShader()->GetRendererID() < b.Material->GetShader()->GetRendererID();
			return a.SortLayer < b.SortLayer;
		});
	}

	void SceneRenderer::geometryPass(RenderQueue& queue, const Ref<RenderPass>& pass, bool clear)
	{
		Renderer::BeginRenderPass(pass, clear);
		
		for (auto& dc : queue.m_MeshCommandList)
		{
			auto material = dc.Mesh->GetMaterial();
			auto shader = material->GetShader();
			material->Bind();
			shader->SetMat4("u_Transform", dc.Transform);
			dc.Mesh->GetVertexArray()->Bind();
			Renderer::DrawIndexed(PrimitiveType::Triangles, dc.Mesh->GetIndexCount(), 0);
		}
		
		m_Renderer2D->BeginScene();
		for (auto& dc : queue.m_SpriteDrawList)
		{
			m_Renderer2D->SetMaterial(dc.Material);
			uint32_t textureID = m_Renderer2D->SetTexture(dc.SubTexture->GetTexture());
			m_Renderer2D->SubmitQuad(dc.Transform, dc.SubTexture->GetTexCoords(), textureID, dc.Color);		
		}

		m_Renderer2D->FlushAll();
		m_Renderer2D->EndScene();

		// TODO: last should be renderer2D
		for (auto& dc : queue.m_InstancedMeshCommandList)
		{
			auto& material = dc.Mesh->GetMaterial();
			auto shader = material->GetShader();
			material->Bind();
			shader->SetMat4("u_Transform", dc.Transform);
			dc.Mesh->GetVertexArray()->Bind();
			Renderer::DrawInstanced(PrimitiveType::Triangles, dc.Mesh->GetIndexCount(), dc.Count, 0);
		}
		
		Renderer::EndRenderPass();
	}
	void SceneRenderer::lightPass()
	{
		Renderer::BeginRenderPass(m_LightPass, true);
	
		m_LightShader->Bind();
		m_LightShader->SetInt("u_NumberPointLights", (int)m_PointLightsList.size());
		m_LightShader->SetInt("u_NumberSpotLights", (int)m_SpotLightsList.size());
	
		m_GeometryPass->GetSpecification().TargetFramebuffer->BindTexture(0, 0);
		m_GeometryPass->GetSpecification().TargetFramebuffer->BindTexture(1, 1);
	
		Renderer::SubmitFullscreenQuad();
	
		Renderer::EndRenderPass();
	}

	void SceneRenderer::bloomPass()
	{
		m_BloomComputeShader->Bind();
		m_BloomComputeShader->SetFloat("u_FilterTreshold", 1.0f);
		m_BloomComputeShader->SetFloat("u_FilterKnee", 0.1f);
		
		uint32_t workGroupSize = 4;
		uint32_t workGroupsX = (uint32_t)glm::ceil(m_ViewportSize.x / workGroupSize);
		uint32_t workGroupsY = (uint32_t)glm::ceil(m_ViewportSize.y / workGroupSize);
	
		// Filter stage
		m_BloomComputeShader->SetInt("u_Mode", 0);

		m_BloomTexture[0]->BindImage(0, 0, BindImageType::Write); // o_Image
		m_LightPass->GetSpecification().TargetFramebuffer->BindTexture(0, 1); // u_Texture
		m_BloomComputeShader->Compute(workGroupsX, workGroupsY, 1, ComputeBarrierType::ShaderImageAccessBarrier);
		
		// Downsample stage
		m_BloomComputeShader->SetInt("u_Mode", 1);
		uint32_t mips = m_BloomTexture[0]->GetMipLevelCount() - 2;
		for (uint32_t i = 1; i < mips; ++i)
		{
			auto [mipWidth, mipHeight] = m_BloomTexture[0]->GetMipSize(i);
			
			m_BloomTexture[1]->BindImage(0, i, BindImageType::Write); // o_Image
			m_BloomTexture[0]->Bind(1);							   // u_Texture
	
			workGroupsX = (uint32_t)glm::ceil((float)mipWidth /  (float)workGroupSize);
			workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)workGroupSize);
	
			m_BloomComputeShader->SetFloat("u_LOD", i - 1.0f);
			m_BloomComputeShader->Compute(workGroupsX, workGroupsY, 1, ComputeBarrierType::ShaderImageAccessBarrier);
		
	
			m_BloomTexture[0]->BindImage(0, i, BindImageType::Write); // o_Image
			m_BloomTexture[1]->Bind(1);							   // u_Texture

			m_BloomComputeShader->SetFloat("u_LOD", i);
			m_BloomComputeShader->Compute(workGroupsX, workGroupsY, 1, ComputeBarrierType::ShaderImageAccessBarrier);
		}

		// Upsample first
		m_BloomComputeShader->SetInt("u_Mode", 2);

		m_BloomComputeShader->SetFloat("u_LOD", mips - 2.0f);
		m_BloomTexture[2]->BindImage(0, mips - 2, BindImageType::Write); // o_Image
		m_BloomTexture[0]->Bind(1);									  // u_Texture 

	
		auto [mipWidth, mipHeight] = m_BloomTexture[2]->GetMipSize(mips - 2);
		workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)workGroupSize);
		workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)workGroupSize);
		m_BloomComputeShader->Compute(workGroupsX, workGroupsY, 1, ComputeBarrierType::ShaderImageAccessBarrier);
		
		// Upsample stage
		m_BloomComputeShader->SetInt("u_Mode", 3);
		for (int32_t mip = mips - 3; mip >= 0; mip--)
		{
			auto [mipWidth, mipHeight] = m_BloomTexture[2]->GetMipSize(mip);
			workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)workGroupSize);
			workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)workGroupSize);
			
			m_BloomTexture[2]->BindImage(0, mip, BindImageType::Write); // o_Image
			m_BloomTexture[0]->Bind(1);							     // u_Texture 
			m_BloomTexture[2]->Bind(2);								 // u_BloomTexture
	
			m_BloomComputeShader->SetFloat("u_LOD", mip);
			m_BloomComputeShader->Compute(workGroupsX, workGroupsY, 1, ComputeBarrierType::ShaderImageAccessBarrier);
		}
	}
}