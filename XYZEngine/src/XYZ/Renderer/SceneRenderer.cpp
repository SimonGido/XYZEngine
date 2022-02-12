#include "stdafx.h"
#include "SceneRenderer.h"

#include "Renderer2D.h"
#include "Renderer.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Debug/Profiler.h"
#include "XYZ/ImGui/ImGui.h"
#include "XYZ/API/Vulkan/VulkanRendererAPI.h"
#include "XYZ/API/Vulkan/VulkanPipelineCompute.h"
#include "XYZ/Scene/SceneEntity.h"

#include <glm/gtx/transform.hpp>

#include <imgui/imgui.h>


#include "XYZ/Renderer/MeshFactory.h"

namespace XYZ {

	static ThreadPool s_ThreadPool;

	static RenderQueue::TransformData Mat4ToTransformData(const glm::mat4& transform)
	{
		RenderQueue::TransformData data;
		data.TransformRow[0] = { transform[0][0], transform[1][0], transform[2][0], transform[3][0] };
		data.TransformRow[1] = { transform[0][1], transform[1][1], transform[2][1], transform[3][1] };
		data.TransformRow[2] = { transform[0][2], transform[1][2], transform[2][2], transform[3][2] };
		return data;
	}
	SceneRenderer::SceneRenderer(Ref<Scene> scene, SceneRendererSpecification specification)
		:
		m_Specification(specification),
		m_ActiveScene(scene)
	{
		m_ViewportSize = { 1280, 720 };
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

		createCompositePass();
		createLightPass();
		createGeometryPass();

		m_Renderer2D = Ref<Renderer2D>::Create(m_CommandBuffer);
		m_Renderer2D->SetTargetRenderPass(m_GeometryPass);
		m_WhiteTexture = Renderer::GetDefaultResources().WhiteTexture;

		
		auto shaderLibrary = Renderer::GetShaderLibrary();
		m_CompositeRenderPipeline.Init(m_CompositePass, shaderLibrary->Get("CompositeShader"));
		m_LightRenderPipeline.Init(m_LightPass, shaderLibrary->Get("LightShader"));


		auto bloomShader = shaderLibrary->Get("Bloom");
		m_BloomComputePipeline = PipelineCompute::Create(bloomShader);
		m_BloomComputeMaterial = Material::Create(bloomShader);

		TextureProperties props;
		props.Storage = true;
		props.SamplerWrap = TextureWrap::Clamp;
		m_BloomTexture[0] = Texture2D::Create(ImageFormat::RGBA32F, 1280, 720, nullptr, props);
		m_BloomTexture[1] = Texture2D::Create(ImageFormat::RGBA32F, 1280, 720, nullptr, props);
		m_BloomTexture[2] = Texture2D::Create(ImageFormat::RGBA32F, 1280, 720, nullptr, props);
		
		m_InstanceVertexBuffer = VertexBuffer::Create(sc_InstanceVertexBufferSize);
		m_TransformVertexBuffer = VertexBuffer::Create(sc_TransformBufferCount * sizeof(RenderQueue::TransformData));
		m_TransformData.resize(sc_TransformBufferCount);
		m_InstanceData.resize(sc_InstanceVertexBufferSize);



		m_TestMesh = MeshFactory::CreateBox(glm::vec3(2.0f));
		m_TestMaterial = Material::Create(shaderLibrary->Get("MeshShader"));
		m_TestMaterial->SetImage("u_Texture", m_WhiteTexture->GetImage());
		m_Renderer2D->GetCameraBufferSet()->CreateDescriptors(m_TestMaterial->GetShader());
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
		m_SceneCamera = camera;

		// Viewport size is changed at the beginning of the frame, so we do not delete texture that is currently use for rendering
		updateViewportSize();
		m_CameraBuffer.ViewProjectionMatrix = m_SceneCamera.Camera.GetProjectionMatrix() * m_SceneCamera.ViewMatrix;
		m_CameraBuffer.ViewMatrix = m_SceneCamera.ViewMatrix;
		m_CameraBuffer.ViewPosition = glm::vec4(camera.ViewPosition, 0.0f);
		
		//m_CameraUniformBuffer->Update(&m_CameraBuffer, sizeof(CameraData), 0);
	}
	void SceneRenderer::BeginScene(const glm::mat4& viewProjectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& viewPosition)
	{
		// Viewport size is changed at the beginning of the frame, so we do not delete texture that is currently use for rendering
		updateViewportSize();

		m_CameraBuffer.ViewProjectionMatrix = viewProjectionMatrix;
		m_CameraBuffer.ViewMatrix = viewMatrix;
		m_CameraBuffer.ViewPosition = glm::vec4(viewPosition, 0.0f);

		//m_CameraUniformBuffer->Update(&m_CameraBuffer, sizeof(CameraData), 0);		
	}
	void SceneRenderer::EndScene()
	{
		SubmitMesh(m_TestMesh, m_TestMaterial, glm::mat4(1.0f));
		SubmitMesh(m_TestMesh, m_TestMaterial, glm::translate(glm::mat4(1.0f), glm::vec3(7.0f, 0.0f, 0.0f)));

		preRender();
		m_CommandBuffer->Begin();
		m_GPUTimeQueries.GPUTime = m_CommandBuffer->BeginTimestampQuery();
		
		geometryPass(m_Queue, true);
		geometryPass2D(m_Queue, false);
		lightPass();
		bloomPass();
		compositePass();

		m_CommandBuffer->EndTimestampQuery(m_GPUTimeQueries.GPUTime);

		m_CommandBuffer->End();
		m_CommandBuffer->Submit();

		m_Queue.SpriteDrawCommands.clear();
		m_Queue.BillboardDrawCommands.clear();
		m_Queue.MeshDrawCommands.clear();
		m_Queue.InstanceMeshDrawCommands.clear();
	}


	void SceneRenderer::SubmitBillboard(const Ref<Material>& material, const Ref<SubTexture>& subTexture, uint32_t sortLayer, const glm::vec4& color, const glm::vec3& position, const glm::vec2& size)
	{
		RenderQueue::SpriteKey key{ material->GetHandle() };

		auto& command = m_Queue.BillboardDrawCommands[key];

		uint32_t textureIndex = command.setTexture(subTexture->GetTexture());

		command.Material = material;
		command.BillboardData.push_back({ textureIndex, subTexture->GetTexCoords(), color, position, size });
	}

	void SceneRenderer::SubmitSprite(const Ref<Material>& material, const Ref<SubTexture>& subTexture, const glm::vec4& color, const glm::mat4& transform)
	{
		RenderQueue::SpriteKey key{ material->GetHandle() };
		auto& command = m_Queue.SpriteDrawCommands[key];
		
		uint32_t textureIndex = command.setTexture(subTexture->GetTexture());
		
		command.Material = material;
		command.SpriteData.push_back({ textureIndex, subTexture->GetTexCoords(), color, transform });
	}

	void SceneRenderer::SubmitMesh(const Ref<Mesh>& mesh, const Ref<Material>& material, const glm::mat4& transform)
	{
		RenderQueue::MeshKey key{ mesh->GetHandle(), material->GetHandle() };

		auto& dc = m_Queue.MeshDrawCommands[key];
		dc.Mesh = mesh;
		dc.Material = material;
		dc.InstanceCount++;
		dc.TransformData.push_back(Mat4ToTransformData(transform));
	}

	void SceneRenderer::SubmitMesh(const Ref<Mesh>& mesh, const Ref<Material>& material, const glm::mat4& transform, const void* instanceData, uint32_t instanceCount, uint32_t instanceSize)
	{
		RenderQueue::InstanceMeshKey key{ mesh->GetHandle(), material->GetHandle(), instanceSize };
		
		auto& dc = m_Queue.InstanceMeshDrawCommands[key];
		dc.Mesh = mesh;
		dc.Material = material;
		dc.InstanceCount++;
		dc.TransformData.push_back(Mat4ToTransformData(transform));

		size_t oldSize = dc.InstanceData.size();
		size_t instanceDataSize = static_cast<size_t>(instanceCount * instanceSize);
		dc.InstanceData.resize(oldSize + instanceDataSize);
		memcpy(dc.InstanceData.data() + oldSize, &transform, instanceDataSize);
	}


	void SceneRenderer::SetGridProperties(const GridProperties& props)
	{
		m_GridProps = props;
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
		return m_CompositeRenderPipeline.Pipeline->GetSpecification().RenderPass;
	}

	Ref<Image2D> SceneRenderer::GetFinalPassImage() const
	{
		return m_CompositeRenderPipeline.Pipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->GetImage();
	}
	SceneRendererOptions& SceneRenderer::GetOptions()
	{
		return m_Options;
	}
	void SceneRenderer::geometryPass(RenderQueue& queue, bool clear)
	{
		XYZ_PROFILE_FUNC("SceneRenderer::geometryPass");
		Renderer::BeginRenderPass(m_CommandBuffer, m_GeometryPass, clear);
		for (auto& [key, command] : queue.MeshDrawCommands)
		{
			Renderer::BindPipeline(
				m_CommandBuffer,
				command.Pipeline,
				m_Renderer2D->GetCameraBufferSet(),
				nullptr,
				command.Material
			);
			Renderer::RenderMesh(
				m_CommandBuffer,
				command.Pipeline,
				command.Material,
				command.Mesh->GetVertexBuffer(),
				command.Mesh->GetIndexBuffer(),
				m_TransformVertexBuffer,
				command.TransformOffset,
				command.InstanceCount);
		}


		for (auto& [key, command] : queue.InstanceMeshDrawCommands)
		{
			Renderer::BindPipeline(
				m_CommandBuffer,
				command.Pipeline,
				m_Renderer2D->GetCameraBufferSet(),
				nullptr,
				command.Material
			);
			Renderer::RenderMeshInstanced(
				m_CommandBuffer,
				command.Pipeline,
				command.Material,
				command.Mesh->GetVertexBuffer(),
				command.Mesh->GetIndexBuffer(),
				m_TransformVertexBuffer,
				command.TransformOffset,
				command.InstanceCount,
				m_InstanceVertexBuffer,
				command.InstanceOffset,
				static_cast<uint32_t>(command.InstanceData.size())
			);
		}
		Renderer::EndRenderPass(m_CommandBuffer);
	}
	void SceneRenderer::geometryPass2D(RenderQueue& queue, bool clear)
	{
		XYZ_PROFILE_FUNC("SceneRenderer::geometryPass2D");

		m_GPUTimeQueries.Renderer2DPassQuery = m_CommandBuffer->BeginTimestampQuery();
		m_Renderer2D->BeginScene(m_CameraBuffer.ViewProjectionMatrix, m_CameraBuffer.ViewMatrix, clear);
		
		for (auto& [key, command] : queue.SpriteDrawCommands)
		{
			m_Renderer2D->SetQuadMaterial(command.Material);
			for (uint32_t i = 0; i < command.TextureCount; ++i)
				command.Material->SetImageArray("u_Texture", command.Textures[i]->GetImage(), i);
			for (uint32_t i = command.TextureCount; i < Renderer2D::GetMaxTextures(); ++i)
				command.Material->SetImageArray("u_Texture", m_WhiteTexture->GetImage(), i);

			for (const auto& data : command.SpriteData)		
				m_Renderer2D->SubmitQuad(data.Transform, data.TexCoords, data.TextureIndex, data.Color);

			m_Renderer2D->Flush();
		}

		for (auto& [key, command] : queue.BillboardDrawCommands)
		{
			m_Renderer2D->SetQuadMaterial(command.Material);
			for (uint32_t i = 0; i < command.TextureCount; ++i)
				command.Material->SetImageArray("u_Texture", command.Textures[i]->GetImage(), i);
			for (uint32_t i = command.TextureCount; i < Renderer2D::GetMaxTextures(); ++i)
				command.Material->SetImageArray("u_Texture", m_WhiteTexture->GetImage(), i);

			for (const auto& data : command.BillboardData)
				m_Renderer2D->SubmitQuadBillboard(data.Position, data.Size, data.TexCoords, data.TextureIndex, data.Color);
			
			m_Renderer2D->Flush();
		}

		m_Renderer2D->EndScene();
		m_CommandBuffer->EndTimestampQuery(m_GPUTimeQueries.Renderer2DPassQuery);
	}
	void SceneRenderer::lightPass()
	{
		XYZ_PROFILE_FUNC("SceneRenderer::lightPass");

		Renderer::BeginRenderPass(m_CommandBuffer, m_LightPass, true);

		Ref<Framebuffer>& renderer2DFramebuffer = m_Renderer2D->GetTargetRenderPass()->GetSpecification().TargetFramebuffer;
		Ref<Image2D> geometryColorImage = renderer2DFramebuffer->GetImage(0);
		Ref<Image2D> geometryPositionImage = renderer2DFramebuffer->GetImage(1);

		m_LightRenderPipeline.Material->SetImageArray("u_Texture", geometryColorImage, 0);
		m_LightRenderPipeline.Material->SetImageArray("u_Texture", geometryPositionImage, 1);

		m_LightRenderPipeline.Material->Set("u_Uniforms.NumberPointLights", (int)m_NumPointLights);
		m_LightRenderPipeline.Material->Set("u_Uniforms.NumberSpotLights", (int)m_NumSpotLights);

		

		Renderer::BindPipeline(
			m_CommandBuffer, 
			m_LightRenderPipeline.Pipeline, 
			m_CameraUniformBuffer, 
			m_LightStorageBufferSet, 
			m_LightRenderPipeline.Material
		);
		
		Renderer::SubmitFullscreenQuad(m_CommandBuffer, m_LightRenderPipeline.Pipeline, m_LightRenderPipeline.Material);
		Renderer::EndRenderPass(m_CommandBuffer);
	}

	void SceneRenderer::bloomPass()
	{
		XYZ_PROFILE_FUNC("SceneRenderer::bloomPass");

		constexpr int prefilter = 0;
		constexpr int downsample = 1;
		constexpr int upsamplefirst = 2;
		constexpr int upsample = 3;

		auto vulkanPipeline = m_BloomComputePipeline;

		auto imageBarrier = [](Ref<VulkanPipelineCompute> pipeline, Ref<VulkanImage2D> image) {

			Renderer::Submit([pipeline, image]() {
				VkImageMemoryBarrier imageMemoryBarrier = {};
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
				imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
				imageMemoryBarrier.image = image->GetImageInfo().Image;
				imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, image->GetSpecification().Mips, 0, 1 };
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				vkCmdPipelineBarrier(
					pipeline->GetActiveCommandBuffer(),
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &imageMemoryBarrier);
			});
		};

		const uint32_t workGroupSize = 4;
		uint32_t workGroupsX = (uint32_t)glm::ceil(m_ViewportSize.x / workGroupSize);
		uint32_t workGroupsY = (uint32_t)glm::ceil(m_ViewportSize.y / workGroupSize);
		Ref<Image2D> lightPassImage = m_LightPass->GetSpecification().TargetFramebuffer->GetImage();
		// Renderer::ClearImage(m_CommandBuffer, m_BloomTexture[2]->GetImage());

		Ref<Material> computeMaterial = m_BloomComputeMaterial;
		Renderer::Submit([computeMaterial, bloomSettings = m_BloomSettings, prefilter]() mutable {
			computeMaterial->Set("u_Uniforms.FilterTreshold", bloomSettings.FilterTreshold);
			computeMaterial->Set("u_Uniforms.FilterKnee", bloomSettings.FilterKnee);
			computeMaterial->Set("u_Uniforms.Mode", prefilter);
			//computeMaterial->Set("u_Uniforms.LOD", 0.0f);			
		});
		computeMaterial->SetImage("o_Image", m_BloomTexture[0]->GetImage(), 0);
		computeMaterial->SetImage("u_Texture", lightPassImage);
		computeMaterial->SetImage("u_BloomTexture", lightPassImage);

		Renderer::BeginPipelineCompute(m_CommandBuffer, m_BloomComputePipeline, nullptr, nullptr, m_BloomComputeMaterial);		
		Renderer::DispatchCompute(m_BloomComputePipeline, m_BloomComputeMaterial, workGroupsX, workGroupsY, 1);
		imageBarrier(vulkanPipeline, m_BloomTexture[0]->GetImage());

		Renderer::Submit([computeMaterial, downsample]() mutable {
			computeMaterial->Set("u_Uniforms.Mode", downsample);
		});

		const uint32_t mips = m_BloomTexture[0]->GetMipLevelCount() - 2;
		for (uint32_t mip = 1; mip < mips; ++mip)
		{
			auto [mipWidth, mipHeight] = m_BloomTexture[0]->GetMipSize(mip);
			workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)workGroupSize);
			workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)workGroupSize);
	
			computeMaterial->SetImage("o_Image", m_BloomTexture[1]->GetImage(), mip);
			computeMaterial->SetImage("u_Texture", m_BloomTexture[0]->GetImage());
			Renderer::Submit([computeMaterial, mip]() mutable {
				computeMaterial->Set("u_Uniforms.LOD", (float)mip - 1.0f);
			});

			Renderer::UpdateDescriptors(m_BloomComputePipeline, m_BloomComputeMaterial, nullptr, nullptr);
			Renderer::DispatchCompute(m_BloomComputePipeline, m_BloomComputeMaterial, workGroupsX, workGroupsY, 1);
			imageBarrier(vulkanPipeline, m_BloomTexture[1]->GetImage());


			computeMaterial->SetImage("o_Image", m_BloomTexture[0]->GetImage(), mip);
			computeMaterial->SetImage("u_Texture", m_BloomTexture[1]->GetImage());
			Renderer::Submit([computeMaterial, mip]() mutable {
				computeMaterial->Set("u_Uniforms.LOD", (float)mip);
			});
			Renderer::UpdateDescriptors(m_BloomComputePipeline, m_BloomComputeMaterial, nullptr, nullptr);
			Renderer::DispatchCompute(m_BloomComputePipeline, m_BloomComputeMaterial, workGroupsX, workGroupsY, 1);
			imageBarrier(vulkanPipeline, m_BloomTexture[0]->GetImage());
		}
		Renderer::Submit([computeMaterial, mips, upsamplefirst]() mutable {
			computeMaterial->Set("u_Uniforms.Mode", upsamplefirst);
			computeMaterial->Set("u_Uniforms.LOD", mips - 2.0f);
		});
		
		m_BloomComputeMaterial->SetImage("o_Image", m_BloomTexture[2]->GetImage(), mips - 2);
		m_BloomComputeMaterial->SetImage("u_Texture", m_BloomTexture[0]->GetImage());
		
		auto [mipWidth, mipHeight] = m_BloomTexture[2]->GetMipSize(mips - 2);
		workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)workGroupSize);
		workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)workGroupSize);
		
		Renderer::UpdateDescriptors(m_BloomComputePipeline, m_BloomComputeMaterial, nullptr, nullptr);
		Renderer::DispatchCompute(m_BloomComputePipeline, m_BloomComputeMaterial, workGroupsX, workGroupsY, 1);	
		imageBarrier(vulkanPipeline, m_BloomTexture[2]->GetImage());
		
		// Upsample stage
		Renderer::Submit([computeMaterial, upsample]() mutable {
			computeMaterial->Set("u_Uniforms.Mode", upsample);
		});

		for (int32_t mip = mips - 3; mip >= 0; mip--)
		{
			auto [mipWidth, mipHeight] = m_BloomTexture[2]->GetMipSize(mip);
			workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)workGroupSize);
			workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)workGroupSize);
		
			m_BloomComputeMaterial->SetImage("o_Image", m_BloomTexture[2]->GetImage(), mip);
			m_BloomComputeMaterial->SetImage("u_Texture", m_BloomTexture[0]->GetImage());
			m_BloomComputeMaterial->SetImage("u_BloomTexture", m_BloomTexture[2]->GetImage());
			Renderer::Submit([computeMaterial, mip]() mutable {

				computeMaterial->Set("u_Uniforms.LOD", (float)mip);
			});
		
			Renderer::UpdateDescriptors(m_BloomComputePipeline, m_BloomComputeMaterial, nullptr, nullptr);
			Renderer::DispatchCompute(m_BloomComputePipeline, m_BloomComputeMaterial, workGroupsX, workGroupsY, 1);
			imageBarrier(vulkanPipeline, m_BloomTexture[2]->GetImage());
		}
		
		Renderer::EndPipelineCompute(m_BloomComputePipeline);
	}
	void SceneRenderer::compositePass()
	{
		XYZ_PROFILE_FUNC("SceneRenderer::compositePass");

		Ref<Image2D> lightPassImage = m_LightPass->GetSpecification().TargetFramebuffer->GetImage();
		Renderer::BeginRenderPass(m_CommandBuffer, m_CompositePass, true);
		m_CompositeRenderPipeline.Material->SetImage("u_GeometryTexture", lightPassImage);
		m_CompositeRenderPipeline.Material->SetImage("u_BloomTexture", m_BloomTexture[2]->GetImage());
		Renderer::BindPipeline(
			m_CommandBuffer,
			m_CompositeRenderPipeline.Pipeline,
			nullptr,
			nullptr,
			m_CompositeRenderPipeline.Material
		);
		Renderer::SubmitFullscreenQuad(m_CommandBuffer, m_CompositeRenderPipeline.Pipeline, m_CompositeRenderPipeline.Material);
		Renderer::EndRenderPass(m_CommandBuffer);
	}
	void SceneRenderer::createCompositePass()
	{
		FramebufferSpecification compFramebufferSpec;
		compFramebufferSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
		compFramebufferSpec.SwapChainTarget = m_Specification.SwapChainTarget;
		compFramebufferSpec.ClearOnLoad = false;
		// No depth for swapchain
		if (m_Specification.SwapChainTarget)
			compFramebufferSpec.Attachments = { ImageFormat::RGBA };
		else
			compFramebufferSpec.Attachments = { ImageFormat::RGBA, ImageFormat::Depth };

		Ref<Framebuffer> framebuffer = Framebuffer::Create(compFramebufferSpec);

		RenderPassSpecification renderPassSpec;
		renderPassSpec.TargetFramebuffer = framebuffer;
		m_CompositePass = RenderPass::Create(renderPassSpec);;
	}
	void SceneRenderer::createLightPass()
	{
		auto shaderLibrary = Renderer::GetShaderLibrary();
		auto shader = shaderLibrary->Get("LightShader");
		FramebufferSpecification specs;
		specs.ClearColor = { 0.1f,0.1f,0.1f,0.0f };
		specs.Attachments = {
			FramebufferTextureSpecification(ImageFormat::RGBA32F, true)
		};
		Ref<Framebuffer> fbo = Framebuffer::Create(specs);
		m_LightPass = RenderPass::Create({ fbo });

		m_LightStorageBufferSet = StorageBufferSet::Create(Renderer::GetConfiguration().FramesInFlight);
		m_LightStorageBufferSet->Create(sc_MaxNumberOfLights * sizeof(PointLight), 0, 1);
		m_LightStorageBufferSet->Create(sc_MaxNumberOfLights * sizeof(SpotLight), 0, 2);
		m_LightStorageBufferSet->CreateDescriptors(shader);
	}

	void SceneRenderer::createGeometryPass()
	{
		FramebufferSpecification framebufferSpec;
		framebufferSpec.Attachments = {
				FramebufferTextureSpecification(ImageFormat::RGBA32F),
				FramebufferTextureSpecification(ImageFormat::RGBA32F)
		};
		framebufferSpec.Samples = 1;
		framebufferSpec.ClearOnLoad = false;
		framebufferSpec.ClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };

		Ref<Framebuffer> framebuffer = Framebuffer::Create(framebufferSpec);

		RenderPassSpecification renderPassSpec;
		renderPassSpec.TargetFramebuffer = framebuffer;
		m_GeometryPass = RenderPass::Create(renderPassSpec);
	}
	

	void SceneRenderer::updateViewportSize()
	{
		if (m_ViewportSizeChanged)
		{
			const uint32_t width = (uint32_t)m_ViewportSize.x;
			const uint32_t height = (uint32_t)m_ViewportSize.y;
			m_LightPass->GetSpecification().TargetFramebuffer->Resize(width, height);
			//m_CompositePass->GetSpecification().TargetFramebuffer->Resize(width, height);

			TextureProperties props;
			props.Storage = true;
			props.SamplerWrap = TextureWrap::Clamp;
			// TODO: resizing
			m_BloomTexture[0] = Texture2D::Create(ImageFormat::RGBA32F, width, height, nullptr, props);
			m_BloomTexture[1] = Texture2D::Create(ImageFormat::RGBA32F, width, height, nullptr, props);
			m_BloomTexture[2] = Texture2D::Create(ImageFormat::RGBA32F, width, height, nullptr, props);
			m_ViewportSizeChanged = false;
		}
	}
	void SceneRenderer::preRender()
	{
		prepareInstances();
		prepareLights();
	}
	void SceneRenderer::prepareInstances()
	{
		// Prepare transforms
		uint32_t offset = 0;
		for (auto& [key, dc] : m_Queue.MeshDrawCommands)
		{
			dc.Pipeline = createGeometryPipeline(dc.Material);
			dc.TransformOffset = offset * sizeof(RenderQueue::TransformData);
			for (const auto& transform : dc.TransformData)
			{
				m_TransformData[offset] = transform;
				offset++;
			}
		}

		// Prepare transforms and instance data
		uint32_t instanceOffset = 0;
		for (auto& [key, dc] : m_Queue.InstanceMeshDrawCommands)
		{
			dc.Pipeline = createGeometryPipeline(dc.Material);
			dc.TransformOffset = offset * sizeof(RenderQueue::TransformData);
			dc.InstanceOffset = instanceOffset;
			for (const auto& transform : dc.TransformData)
			{
				m_TransformData[offset] = transform;
				offset++;
			}
			// Copy instance data
			const size_t instanceDataSize = dc.InstanceData.size() * key.InstanceSize;
			memcpy(&m_InstanceData.data()[instanceOffset], dc.InstanceData.data(), instanceDataSize);
			instanceOffset += instanceDataSize;
		}

		m_TransformVertexBuffer->Update(m_TransformData.data(), offset * sizeof(RenderQueue::TransformData));
		m_InstanceVertexBuffer->Update(m_InstanceData.data(), instanceOffset);
	}
	void SceneRenderer::prepareLights()
	{
		// Prepare lights
		auto& ecs = m_ActiveScene->GetECS();
		m_NumSpotLights = ecs.GetStorage<SpotLight2D>().Size();
		m_NumPointLights = ecs.GetStorage<PointLight2D>().Size();

		m_SpotLights.resize(m_NumSpotLights);
		m_PointLights.resize(m_NumPointLights);

		// Spot lights
		uint32_t counter = 0;
		auto spotLight2DView = ecs.CreateView<TransformComponent, SpotLight2D>();
		for (auto entity : spotLight2DView)
		{
			// Render previous frame data
			auto& [transform, light] = spotLight2DView.Get<TransformComponent, SpotLight2D>(entity);
			auto [trans, rot, scale] = transform.GetWorldComponents();

			SpotLight lightData;
			lightData.Color = glm::vec4(light.Color, 1.0f);
			lightData.Position = trans;
			lightData.Radius = light.Radius;
			lightData.Intensity = light.Intensity;
			lightData.InnerAngle = light.InnerAngle;
			lightData.OuterAngle = light.OuterAngle;

			m_SpotLights[counter++] = lightData;
		}

		// Point Lights
		counter = 0;
		auto pointLight2DView = ecs.CreateView<TransformComponent, PointLight2D>();
		for (auto entity : pointLight2DView)
		{
			auto& [transform, light] = pointLight2DView.Get<TransformComponent, PointLight2D>(entity);
			auto [trans, rot, scale] = transform.GetWorldComponents();
			PointLight lightData;
			lightData.Color = glm::vec4(light.Color, 1.0f);
			lightData.Position = trans;
			lightData.Radius = light.Radius;
			lightData.Intensity = light.Intensity;
			m_PointLights[counter] = lightData;
		}

		Ref<StorageBufferSet> instance = m_LightStorageBufferSet;
		Renderer::Submit([instance, pLights = m_PointLights, sLights = m_SpotLights]() mutable {
			const uint32_t frame = Renderer::GetCurrentFrame();
			instance->Get(1, 0, frame)->RT_Update(pLights.data(), pLights.size() * sizeof(PointLight));
			instance->Get(2, 0, frame)->RT_Update(sLights.data(), sLights.size() * sizeof(SpotLight));
		});
	}
	Ref<Pipeline> SceneRenderer::createGeometryPipeline(const Ref<Material>& material, bool instanced)
	{
		Ref<Shader> shader = material->GetShader();
		auto it = m_GeometryPipelines.find(shader->GetHash());
		if (it != m_GeometryPipelines.end())
			return it->second;


		PipelineSpecification spec;
		spec.Layouts = shader->GetLayouts();
		spec.RenderPass = m_GeometryPass;
		spec.Shader = shader;
		spec.Topology = PrimitiveTopology::Triangles;

		auto& pipeline = m_GeometryPipelines[shader->GetHash()];
		pipeline = Pipeline::Create(spec);
		return pipeline;
	}

	void SceneRenderer::SceneRenderPipeline::Init(const Ref<RenderPass>& renderPass, const Ref<Shader>& shader, PrimitiveTopology topology)
	{
		PipelineSpecification specification;
		specification.Shader = shader;
		specification.Layouts = shader->GetLayouts();
		specification.RenderPass = renderPass;
		specification.Topology = topology;
		specification.DepthWrite = false;
		this->Pipeline = Pipeline::Create(specification);
		this->Material = Material::Create(shader);
	}
	uint32_t RenderQueue::SpriteDrawCommand::setTexture(const Ref<Texture2D>& texture)
	{
		for (uint32_t i = 0; i < TextureCount; i++)
		{
			if (Textures[i]->GetHandle() == texture->GetHandle())
			{
				return i;
			}
		}
		uint32_t result = TextureCount;
		Textures[result] = texture;
		TextureCount++;
		return result;
	}
}