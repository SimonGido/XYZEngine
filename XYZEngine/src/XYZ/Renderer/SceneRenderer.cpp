#include "stdafx.h"
#include "SceneRenderer.h"

#include "Renderer2D.h"
#include "Renderer.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Debug/Profiler.h"
#include "XYZ/ImGui/ImGui.h"

#include <glm/gtx/transform.hpp>

#include <imgui/imgui.h>

namespace XYZ {

	static ThreadPool s_ThreadPool;

	SceneRenderer::SceneRenderer(Ref<Scene> scene, SceneRendererSpecification specification)
		:
		m_Specification(specification),
		m_ActiveScene(scene)
	{
		Init();
	}

	SceneRenderer::~SceneRenderer()
	{
		s_ThreadPool.EraseThread(m_ThreadIndex);
	}


	void SceneRenderer::Init()
	{
		XYZ_PROFILE_FUNC("SceneRenderer::Init");
		m_ThreadIndex = s_ThreadPool.PushThread();
		if (m_Specification.SwapChainTarget)
			m_CommandBuffer = Renderer::GetAPIContext()->GetRenderCommandBuffer();
		else
			m_CommandBuffer = RenderCommandBuffer::Create(0, "SceneRenderer");

		m_CommandBuffer->CreateTimestampQueries(GPUTimeQueries::Count());

		m_Renderer2D = Ref<Renderer2D>::Create(m_CommandBuffer);
	
		createCompositePipeline();

		// Light pass
		{
			FramebufferSpecification specs;
			specs.ClearColor = { 0.0f,0.0f,0.0f,0.0f };
			specs.Attachments = {
				FramebufferTextureSpecification(ImageFormat::RGBA16F, true),
				FramebufferTextureSpecification(ImageFormat::DEPTH24STENCIL8)
			};
			Ref<Framebuffer> fbo = Framebuffer::Create(specs);
			m_LightPass = RenderPass::Create({ fbo });
		}
		// Geometry pass
		{
			FramebufferSpecification specs;
			specs.ClearColor = { 0.0f,0.0f,0.0f,0.0f };
			specs.Attachments = {
				FramebufferTextureSpecification(ImageFormat::RGBA16F),
				FramebufferTextureSpecification(ImageFormat::RGBA16F),
				FramebufferTextureSpecification(ImageFormat::DEPTH24STENCIL8)
			};
			Ref<Framebuffer> fbo = Framebuffer::Create(specs);
			m_GeometryPass = RenderPass::Create({ fbo });
		}
		// Bloom pass
		{
			FramebufferSpecification specs;
			specs.ClearColor = { 0.0f,0.0f,0.0f, 0.0f };
			specs.Attachments = {
				FramebufferTextureSpecification(ImageFormat::RGBA16F),
				FramebufferTextureSpecification(ImageFormat::DEPTH24STENCIL8)
			};
			Ref<Framebuffer> fbo = Framebuffer::Create(specs);
			m_BloomPass = RenderPass::Create({ fbo });
		}

		auto shaderLibrary = Renderer::GetShaderLibrary();
		m_LightShader = shaderLibrary->Get("LightShader");
		m_BloomComputeShader = shaderLibrary->Get("Bloom");

		//m_BloomTexture[0] = Texture2D::Create(ImageFormat::RGBA32F, 1280, 720, {});
		//m_BloomTexture[1] = Texture2D::Create(ImageFormat::RGBA32F, 1280, 720, {});
		//m_BloomTexture[2] = Texture2D::Create(ImageFormat::RGBA32F, 1280, 720, {});

		//m_LightStorageBuffer = StorageBuffer::Create(sc_MaxNumberOfLights * sizeof(SceneRenderer::PointLight), 1);
		//m_SpotLightStorageBuffer = StorageBuffer::Create(sc_MaxNumberOfLights * sizeof(SceneRenderer::SpotLight), 2);
		//m_CameraUniformBuffer = UniformBuffer::Create(sizeof(CameraData), 0);
	}

	void SceneRenderer::SetScene(Ref<Scene> scene)
	{
		m_ActiveScene = scene;
	}


	void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
	{
		if (m_ViewportSize.x != width || m_ViewportSize.y != height)
		{
			m_ViewportSize = glm::ivec2(width, height);
			m_ViewportSizeChanged = true;
		}
	}
	void SceneRenderer::BeginScene(const SceneRendererCamera& camera)
	{
		XYZ_ASSERT(!m_ActiveScene.Raw(), "No Scene set");
		m_SceneCamera = camera;

		// Viewport size is changed at the beginning of the frame, so we do not delete texture that is currently use for rendering
		UpdateViewportSize();
		m_CameraBuffer.ViewProjectionMatrix = m_SceneCamera.Camera.GetProjectionMatrix() * m_SceneCamera.ViewMatrix;
		m_CameraBuffer.ViewMatrix = m_SceneCamera.ViewMatrix;
		m_CameraBuffer.ViewPosition = glm::vec4(camera.ViewPosition, 0.0f);
		
		//m_CameraUniformBuffer->Update(&m_CameraBuffer, sizeof(CameraData), 0);
	}
	void SceneRenderer::BeginScene(const glm::mat4& viewProjectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& viewPosition)
	{
		XYZ_ASSERT(m_ActiveScene.Raw(), "No Scene set");

		// Viewport size is changed at the beginning of the frame, so we do not delete texture that is currently use for rendering
		UpdateViewportSize();

		m_CameraBuffer.ViewProjectionMatrix = viewProjectionMatrix;
		m_CameraBuffer.ViewMatrix = viewMatrix;
		m_CameraBuffer.ViewPosition = glm::vec4(viewPosition, 0.0f);

		//m_CameraUniformBuffer->Update(&m_CameraBuffer, sizeof(CameraData), 0);		
	}
	void SceneRenderer::EndScene()
	{
		m_CommandBuffer->Begin();
		m_GPUTimeQueries.GPUTime = m_CommandBuffer->BeginTimestampQuery();
		flush();
		flushDefaultQueue();
		m_CommandBuffer->EndTimestampQuery(m_GPUTimeQueries.GPUTime);

		m_CommandBuffer->End();
		m_CommandBuffer->Submit();
	}

	void SceneRenderer::SubmitSprite(const Ref<Material>& material, const Ref<SubTexture>& subTexture, uint32_t sortLayer, const glm::vec4& color, const glm::mat4& transform)
	{
		m_Queue.m_SpriteDrawList.push_back({
			   material, subTexture, sortLayer, color, transform
			});
	}

	void SceneRenderer::SubmitBillboard(const Ref<Material>& material, const Ref<SubTexture>& subTexture, uint32_t sortLayer, const glm::vec4& color, const glm::vec3& position, const glm::vec2& size)
	{
		m_Queue.m_BillboardDrawList.push_back({
			   material,subTexture, sortLayer, color, position, size
			});
	}


	void SceneRenderer::SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform)
	{
		//m_Queues[mesh->GetMaterial()->GetRenderQueueID()].m_MeshCommandList.push_back({ mesh, transform });
	}
	void SceneRenderer::SubmitMeshInstanced(const Ref<Mesh>& mesh, const glm::mat4& transform, uint32_t count)
	{
		//m_Queues[mesh->GetMaterial()->GetRenderQueueID()].m_InstancedMeshCommandList.push_back({ mesh, transform, count });
	}
	void SceneRenderer::SubmitMeshInstanced(const Ref<Mesh>& mesh, const std::vector<glm::mat4>& transforms, uint32_t count)
	{
	}
	//void SceneRenderer::SubmitLight(const PointLight2D& light, const glm::mat4& transform)
	//{
	//	XYZ_ASSERT(m_PointLightsList.size() + 1 < sc_MaxNumberOfLights, "Max number of lights per scene is ", sc_MaxNumberOfLights);
	//	SubmitLight(light, glm::vec3(transform[3][0], transform[3][1], 0.0f));
	//}
	//void SceneRenderer::SubmitLight(const SpotLight2D& light, const glm::mat4& transform)
	//{
	//	XYZ_ASSERT(m_SpotLightsList.size() + 1 < sc_MaxNumberOfLights, "Max number of lights per scene is ", sc_MaxNumberOfLights);
	//	SubmitLight(light, glm::vec3(transform[3][0], transform[3][1], 0.0f));
	//}
	//void SceneRenderer::SubmitLight(const PointLight2D& light, const glm::vec3& position)
	//{
	//	XYZ_ASSERT(m_PointLightsList.size() + 1 < sc_MaxNumberOfLights, "Max number of lights per scene is ", sc_MaxNumberOfLights);
	//
	//	PointLight lightData;
	//	lightData.Position = glm::vec2(position.x, position.y);
	//	lightData.Color = glm::vec4(light.Color, 0.0f);
	//	lightData.Radius = light.Radius;
	//	lightData.Intensity = light.Intensity;
	//	m_PointLightsList.push_back(lightData);
	//}
	//void SceneRenderer::SubmitLight(const SpotLight2D& light, const glm::vec3& position)
	//{
	//	XYZ_ASSERT(m_SpotLightsList.size() + 1 < sc_MaxNumberOfLights, "Max number of lights per scene is ", sc_MaxNumberOfLights);
	//
	//	SpotLight lightData;
	//	lightData.Position = glm::vec2(position.x, position.y);
	//	lightData.Color = glm::vec4(light.Color, 0.0f);
	//	lightData.Radius = light.Radius;
	//	lightData.Intensity = light.Intensity;
	//	lightData.InnerAngle = light.InnerAngle;
	//	lightData.OuterAngle = light.OuterAngle;
	//	m_SpotLightsList.push_back(lightData);
	//}
	void SceneRenderer::SetGridProperties(const GridProperties& props)
	{
		m_GridProps = props;
	}

	void SceneRenderer::UpdateViewportSize()
	{
		if (m_ViewportSizeChanged)
		{
			const uint32_t width = (uint32_t)m_ViewportSize.x;
			const uint32_t height = (uint32_t)m_ViewportSize.y;
			m_GeometryPass->GetSpecification().TargetFramebuffer->Resize(width, height);
			m_LightPass->GetSpecification().TargetFramebuffer->Resize(width, height);
			m_BloomPass->GetSpecification().TargetFramebuffer->Resize(width, height);

			m_BloomTexture[0] = Texture2D::Create(ImageFormat::RGBA32F, width, height, {});
			m_BloomTexture[1] = Texture2D::Create(ImageFormat::RGBA32F, width, height, {});
			m_BloomTexture[2] = Texture2D::Create(ImageFormat::RGBA32F, width, height, {});
			m_ViewportSizeChanged = false;
		}
	}

	void SceneRenderer::OnImGuiRender()
	{
		XYZ_PROFILE_FUNC("SceneRenderer::OnImGuiRender");
		
		if (ImGui::Begin("Scene Renderer"))
		{
			if (ImGui::BeginTable("##Viewport", 4, ImGuiTableFlags_SizingFixedFit))
			{
				UI::TextTableRow(
					"%s", "Viewport Width:", "%llu", m_ViewportSize.x,
					"%s", "Viewport Height:", "%llu", m_ViewportSize.y
				);
				ImGui::EndTable();
			}
			uint32_t frameIndex = Renderer::GetCurrentFrame();
			if (UI::BeginTreeNode("GPU measurements"))
			{
				if (ImGui::BeginTable("##GPUTime", 2, ImGuiTableFlags_SizingFixedFit))
				{
					UI::TextTableRow("%s", "GPU time:", "%.3fms", m_CommandBuffer->GetExecutionGPUTime(frameIndex, m_GPUTimeQueries.GPUTime));
					UI::TextTableRow("%s", "Renderer2D Pass:", "%.3fms", m_CommandBuffer->GetExecutionGPUTime(frameIndex, m_GPUTimeQueries.Renderer2DPassQuery));
					ImGui::EndTable();
				}
				
				UI::EndTreeNode();
			}
			if (UI::BeginTreeNode("Pipeline Statistics"))
			{
				const PipelineStatistics& pipelineStats = m_CommandBuffer->GetPipelineStatistics(frameIndex);
				if (ImGui::BeginTable("##GPUTime", 2, ImGuiTableFlags_SizingFixedFit))
				{
					UI::TextTableRow("%s", "Input Assembly Vertices:", "%llu", pipelineStats.InputAssemblyVertices);		
					UI::TextTableRow("%s", "Input Assembly Primitives:", "%llu", pipelineStats.InputAssemblyPrimitives);
					UI::TextTableRow("%s", "Vertex Shader Invocations:", "%llu", pipelineStats.VertexShaderInvocations);
					UI::TextTableRow("%s", "Clipping Invocations:", "%llu", pipelineStats.ClippingInvocations);
					UI::TextTableRow("%s", "Clipping Primitives:", "%llu", pipelineStats.ClippingPrimitives);
					UI::TextTableRow("%s", "Fragment Shader Invocations:", "%llu", pipelineStats.FragmentShaderInvocations);
					UI::TextTableRow("%s", "Compute Shader Invocations:", "%llu", pipelineStats.ComputeShaderInvocations);

					ImGui::EndTable();
				}
				UI::EndTreeNode();
			}
		}
		ImGui::End();
	}

	Ref<RenderPass> SceneRenderer::GetFinalRenderPass() const
	{	
		// TODO: temporary;
		return m_Renderer2D->GetTargetRenderPass();
		//return m_CompositePipeline->GetSpecification().RenderPass;
	}

	Ref<Image2D> SceneRenderer::GetFinalPassImage() const
	{
		return m_Renderer2D->GetTargetRenderPass()->GetSpecification().TargetFramebuffer->GetImage();
		//return m_CompositePipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->GetImage();
	}
	SceneRendererOptions& SceneRenderer::GetOptions()
	{
		return m_Options;
	}
	void SceneRenderer::flush()
	{
		XYZ_PROFILE_FUNC("SceneRenderer::flush");
		//flushLightQueue();
		
		
		//Renderer::BeginRenderPass(m_CommandBuffer, m_CompositePipeline->GetSpecification().RenderPass, true);
		//
		////m_LightPass->GetSpecification().TargetFramebuffer->BindTexture(0, 0);
		////m_BloomTexture[2]->Bind(1);
		//
		//auto geometryImage = m_Renderer2D->GetTargetRenderPass()->GetSpecification().TargetFramebuffer->GetImage();
		//
		//m_CompositeMaterial->Set("u_GeometryTexture", geometryImage);
		//m_CompositeMaterial->Set("u_BloomTexture", geometryImage);
		//Renderer::BindPipeline(m_CommandBuffer, m_CompositePipeline, nullptr, m_CompositeMaterial);
		//Renderer::SubmitFullscreenQuad(m_CommandBuffer, m_CompositePipeline, m_CompositeMaterial);
		//
		//Renderer::EndRenderPass(m_CommandBuffer);

		//auto [width, height] = Input::GetWindowSize();
		//Renderer::SetViewPort(0, 0, (uint32_t)width, (uint32_t)height);
	}
	void SceneRenderer::flushLightQueue()
	{
		XYZ_PROFILE_FUNC("SceneRenderer::flushLightQueue");
		RenderQueue& queue = m_Queue;
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
		RenderQueue& queue = m_Queue;
		sortQueue(queue);

		geometryPass2D(queue, true);
	
		queue.m_SpriteDrawList.clear();
		queue.m_BillboardDrawList.clear();
		queue.m_MeshCommandList.clear();
		queue.m_InstancedMeshCommandList.clear();
	}

	void SceneRenderer::sortQueue(RenderQueue& queue)
	{
		XYZ_PROFILE_FUNC("SceneRenderer::sortQueue");
		std::sort(queue.m_SpriteDrawList.begin(), queue.m_SpriteDrawList.end(),
			[](const RenderQueue::SpriteDrawCommand& a, const RenderQueue::SpriteDrawCommand& b) {
			if (a.SortLayer == b.SortLayer)
				return a.Material->GetShader()->GetHash() < b.Material->GetShader()->GetHash();
			return a.SortLayer < b.SortLayer;
		});
	}

	
	void SceneRenderer::geometryPass2D(RenderQueue& queue, bool clear)
	{
		m_GPUTimeQueries.Renderer2DPassQuery = m_CommandBuffer->BeginTimestampQuery();
		m_Renderer2D->BeginScene(m_CameraBuffer.ViewProjectionMatrix, m_CameraBuffer.ViewMatrix);
		for (auto& dc : queue.m_SpriteDrawList)
		{
			m_Renderer2D->SetQuadMaterial(dc.Material);
			m_Renderer2D->SubmitQuad(dc.Transform, dc.SubTexture, dc.Color);
		}
		for (auto& dc : queue.m_BillboardDrawList)
		{
			m_Renderer2D->SetQuadMaterial(dc.Material);
			m_Renderer2D->SubmitQuadBillboard(dc.Position, dc.Size, dc.SubTexture, dc.Color);
		}

		m_Renderer2D->EndScene();
		m_CommandBuffer->EndTimestampQuery(m_GPUTimeQueries.Renderer2DPassQuery);
	}
	void SceneRenderer::lightPass()
	{
		Renderer::BeginRenderPass(m_CommandBuffer, m_LightPass, true);
	
		m_LightShader->Bind();
		m_LightShader->SetInt("u_NumberPointLights", (int)m_PointLightsList.size());
		m_LightShader->SetInt("u_NumberSpotLights", (int)m_SpotLightsList.size());
	
		m_GeometryPass->GetSpecification().TargetFramebuffer->BindTexture(0, 0);
		m_GeometryPass->GetSpecification().TargetFramebuffer->BindTexture(1, 1);
	
		Renderer::SubmitFullscreenQuad();
	
		Renderer::EndRenderPass(m_CommandBuffer);
	}

	void SceneRenderer::bloomPass()
	{
		m_BloomComputeShader->Bind();
		m_BloomComputeShader->SetFloat("u_FilterTreshold", 1.0f);
		m_BloomComputeShader->SetFloat("u_FilterKnee", 0.1f);

		const uint32_t workGroupSize = 4;
		uint32_t workGroupsX = (uint32_t)glm::ceil(m_ViewportSize.x / workGroupSize);
		uint32_t workGroupsY = (uint32_t)glm::ceil(m_ViewportSize.y / workGroupSize);
	
		// Filter stage
		m_BloomComputeShader->SetInt("u_Mode", 0);

		m_BloomTexture[0]->BindImage(0, 0, BindImageType::Write); // o_Image
		m_LightPass->GetSpecification().TargetFramebuffer->BindTexture(0, 1); // u_Texture
		m_BloomComputeShader->Compute(workGroupsX, workGroupsY, 1, ComputeBarrierType::ShaderImageAccessBarrier);
		
		// Downsample stage
		m_BloomComputeShader->SetInt("u_Mode", 1);
		const uint32_t mips = m_BloomTexture[0]->GetMipLevelCount() - 2;
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
	void SceneRenderer::createCompositePipeline()
	{
		auto shaderLibrary = Renderer::GetShaderLibrary();
		FramebufferSpecification compFramebufferSpec;
		compFramebufferSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
		compFramebufferSpec.SwapChainTarget = m_Specification.SwapChainTarget;

		// No depth for swapchain
		if (m_Specification.SwapChainTarget)
			compFramebufferSpec.Attachments = { ImageFormat::RGBA };
		else
			compFramebufferSpec.Attachments = { ImageFormat::RGBA, ImageFormat::Depth };

		Ref<Framebuffer> framebuffer = Framebuffer::Create(compFramebufferSpec);

		PipelineSpecification pipelineSpecification;
		pipelineSpecification.Layout = {
			{0, ShaderDataType::Float3, "a_Position" },
			{1, ShaderDataType::Float2, "a_TexCoord" }
		};

		pipelineSpecification.BackfaceCulling = false;
		pipelineSpecification.Shader = shaderLibrary->Get("CompositeShader");

		RenderPassSpecification renderPassSpec;
		renderPassSpec.TargetFramebuffer = framebuffer;
		pipelineSpecification.RenderPass = RenderPass::Create(renderPassSpec);
		pipelineSpecification.DebugName = "SceneComposite";
		pipelineSpecification.DepthWrite = false;
		
		m_CompositePipeline = Pipeline::Create(pipelineSpecification);
		m_CompositeMaterial = Material::Create(shaderLibrary->Get("CompositeShader"));
	}
}