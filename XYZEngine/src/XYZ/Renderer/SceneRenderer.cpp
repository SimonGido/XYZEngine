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
#include "XYZ/Asset/AssetManager.h"

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
		m_CommandBuffer = PrimaryRenderCommandBuffer::Create(0, "SceneRenderer");

		m_CommandBuffer->CreateTimestampQueries(GPUTimeQueries::Count());

		createCompositePass();
		createLightPass();
		createGeometryPass();

		m_Renderer2D = Ref<Renderer2D>::Create(m_CommandBuffer);
		m_Renderer2D->SetTargetRenderPass(m_GeometryPass);
		m_WhiteTexture = Renderer::GetDefaultResources().WhiteTexture;

		Ref<ShaderAsset> compositeShaderAsset = AssetManager::GetAsset<ShaderAsset>("Resources/Shaders/CompositeShader.shader");
		Ref<ShaderAsset> lightShaderAsset	 = AssetManager::GetAsset<ShaderAsset>("Resources/Shaders/LightShader.shader");
		Ref<ShaderAsset> bloomShaderAsset = AssetManager::GetAsset<ShaderAsset>("Resources/Shaders/Bloom.shader");
		Ref<ShaderAsset> meshShaderAsset = AssetManager::GetAsset<ShaderAsset>("Resources/Shaders/MeshShader.shader");

		m_CompositeRenderPipeline.Init(m_CompositePass, compositeShaderAsset->GetShader());
		m_LightRenderPipeline.Init(m_LightPass, lightShaderAsset->GetShader());


		m_BloomComputePipeline = PipelineCompute::Create(bloomShaderAsset->GetShader());
		m_BloomComputeMaterial = Material::Create(bloomShaderAsset->GetShader());
		m_BloomComputeMaterialInstance = Ref<MaterialInstance>::Create(m_BloomComputeMaterial);

		TextureProperties props;
		props.Storage = true;
		props.SamplerWrap = TextureWrap::Clamp;
		m_BloomTexture[0] = Texture2D::Create(ImageFormat::RGBA32F, 1280, 720, nullptr, props);
		m_BloomTexture[1] = Texture2D::Create(ImageFormat::RGBA32F, 1280, 720, nullptr, props);
		m_BloomTexture[2] = Texture2D::Create(ImageFormat::RGBA32F, 1280, 720, nullptr, props);
		
		m_InstanceVertexBufferSet = Ref<VertexBufferSet>::Create(Renderer::GetConfiguration().FramesInFlight, sc_InstanceVertexBufferSize);

		m_TransformVertexBufferSet = Ref<VertexBufferSet>::Create(Renderer::GetConfiguration().FramesInFlight, sc_TransformBufferCount * sizeof(RenderQueue::TransformData));
		m_TransformData.resize(sc_TransformBufferCount);
		m_InstanceData.resize(sc_InstanceVertexBufferSize);
		m_BoneTransformsData.resize(sc_MaxBoneTransforms);

		m_BoneTransformsStorageSet = StorageBufferSet::Create(Renderer::GetConfiguration().FramesInFlight);
		m_BoneTransformsStorageSet->Create(sc_MaxBoneTransforms * sizeof(RenderQueue::BoneTransforms), 2, 0);


		m_TestMesh = MeshFactory::CreateBox(glm::vec3(2.0f));
		m_TestMaterial = Ref<MaterialAsset>::Create(meshShaderAsset);
		m_TestMaterial->SetTexture("u_Texture", m_WhiteTexture);
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

		m_CameraBuffer.ViewProjectionMatrix = m_SceneCamera.Camera.GetProjectionMatrix() * m_SceneCamera.ViewMatrix;
		m_CameraBuffer.ViewMatrix = m_SceneCamera.ViewMatrix;
		m_CameraBuffer.ViewPosition = glm::vec4(camera.ViewPosition, 0.0f);
		
		//m_CameraUniformBuffer->Update(&m_CameraBuffer, sizeof(CameraData), 0);
	}
	void SceneRenderer::BeginScene(const glm::mat4& viewProjectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& viewPosition)
	{
		m_CameraBuffer.ViewProjectionMatrix = viewProjectionMatrix;
		m_CameraBuffer.ViewMatrix = viewMatrix;
		m_CameraBuffer.ViewPosition = glm::vec4(viewPosition, 0.0f);

		//m_CameraUniformBuffer->Update(&m_CameraBuffer, sizeof(CameraData), 0);		
	}
	void SceneRenderer::EndScene()
	{
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
		m_Queue.AnimatedMeshDrawCommands.clear();
		m_Queue.InstanceMeshDrawCommands.clear();
		m_PointLights.clear();
		m_SpotLights.clear();

		updateViewportSize();
	}


	void SceneRenderer::SubmitBillboard(const Ref<MaterialAsset>& material, const Ref<SubTexture>& subTexture, uint32_t sortLayer, const glm::vec4& color, const glm::vec3& position, const glm::vec2& size)
	{
		RenderQueue::SpriteKey key{ material->GetHandle() };

		auto& command = m_Queue.BillboardDrawCommands[key];

		uint32_t textureIndex = command.setTexture(subTexture->GetTexture());

		command.Material = material;
		command.BillboardData.push_back({ textureIndex, subTexture->GetTexCoords(), color, position, size });
	}

	void SceneRenderer::SubmitSprite(const Ref<MaterialAsset>& material, const Ref<SubTexture>& subTexture, const glm::vec4& color, const glm::mat4& transform)
	{
		RenderQueue::SpriteKey key{ material->GetHandle() };
		auto& command = m_Queue.SpriteDrawCommands[key];
		
		uint32_t textureIndex = command.setTexture(subTexture->GetTexture());
		
		command.Material = material;
		command.SpriteData.push_back({ textureIndex, subTexture->GetTexCoords(), color, transform });
	}

	void SceneRenderer::SubmitMesh(const Ref<Mesh>& mesh, const Ref<MaterialAsset>& material, const glm::mat4& transform, const Ref<MaterialInstance>& overrideMaterial)
	{
		RenderQueue::BatchMeshKey key{ mesh->GetHandle(), material->GetHandle() };
		
		auto& dc = m_Queue.MeshDrawCommands[key];
		dc.Mesh = mesh;
		dc.MaterialAsset = material;

		if (overrideMaterial.Raw())
		{
			auto& dcOverride = dc.OverrideCommands.emplace_back();
			dcOverride.OverrideMaterial = overrideMaterial;
			dcOverride.Transform = transform;
		}
		else
		{		
			dc.OverrideMaterial = material->GetMaterialInstance();
			dc.TransformInstanceCount++;
			dc.TransformData.push_back(Mat4ToTransformData(transform));
		}
	}

	void SceneRenderer::SubmitMesh(const Ref<Mesh>& mesh, const Ref<MaterialAsset>& material, const void* instanceData, uint32_t instanceCount, uint32_t instanceSize, const Ref<MaterialInstance>& overrideMaterial)
	{
		RenderQueue::BatchMeshKey key{ mesh->GetHandle(), material->GetHandle() };

		auto& dc = m_Queue.InstanceMeshDrawCommands[key];
		dc.Mesh = mesh;
		dc.MaterialAsset = material;
		dc.Transform = glm::mat4(1.0f);

		if (overrideMaterial.Raw())
		{
			XYZ_ASSERT(false, "Not implemented");
		}
		else
		{
			dc.OverrideMaterial = material->GetMaterialInstance();
			dc.InstanceCount += instanceCount;
		
			size_t offset = dc.InstanceData.size();
			size_t instanceDataSize = static_cast<size_t>(instanceCount) * instanceSize;
			dc.InstanceData.resize(offset + instanceDataSize);
			memcpy(dc.InstanceData.data() + offset, instanceData, instanceDataSize);
		}
	}

	void SceneRenderer::SubmitMesh(const Ref<AnimatedMesh>& mesh, const Ref<MaterialAsset>& material, const glm::mat4& transform, const std::vector<ozz::math::Float4x4>& boneTransforms, const Ref<MaterialInstance>& overrideMaterial)
	{
		RenderQueue::BatchMeshKey key{ mesh->GetHandle(), material->GetHandle() };

		auto& dc = m_Queue.AnimatedMeshDrawCommands[key];
		dc.Mesh = mesh;
		dc.MaterialAsset = material;

		if (overrideMaterial.Raw())
		{
			auto& dcOverride = dc.OverrideCommands.emplace_back();
			dcOverride.OverrideMaterial = overrideMaterial;
			dcOverride.Transform = transform * mesh->GetMeshSource()->GetTransform();
			copyToBoneStorage(dcOverride.BoneTransforms, boneTransforms, mesh);
		}
		else
		{
			dc.OverrideMaterial = material->GetMaterialInstance();
			dc.TransformInstanceCount++;
			dc.TransformData.push_back(Mat4ToTransformData(transform));
			auto& boneStorage = dc.BoneData.emplace_back();
			copyToBoneStorage(boneStorage, boneTransforms, mesh);
		}
	}

	void SceneRenderer::SubmitLight(const PointLight2D& light, const glm::vec2& position)
	{
		m_PointLights.push_back({
			glm::vec4(light.Color, 1.0f),
			position,
			light.Radius,
			light.Intensity
		});
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
			if (ImGui::BeginTable("##Specification", 2, ImGuiTableFlags_SizingFixedFit))
			{
				UI::TextTableRow("%s", "Max Light Count:", "%u", sc_MaxNumberOfLights);
				UI::TextTableRow("%s", "Max Transform Instances:", "%u", sc_TransformBufferCount);
				UI::TextTableRow("%s", "Max Instance Data Size:", "%u", sc_InstanceVertexBufferSize);

				ImGui::EndTable();
			}
			if (UI::BeginTreeNode("Render Statistics"))
			{
				if (ImGui::BeginTable("##RenderStatistics", 2, ImGuiTableFlags_SizingFixedFit))
				{
					UI::TextTableRow("%s", "Sprite Draw Count:", "%u", m_RenderStatistics.SpriteDrawCommandCount);
					UI::TextTableRow("%s", "Mesh Draw Count:", "%u", m_RenderStatistics.MeshDrawCommandCount);

					UI::TextTableRow("%s", "Mesh Override Draw Count:", "%u", m_RenderStatistics.MeshOverrideDrawCommandCount);
					UI::TextTableRow("%s", "Instance Mesh Draw Count:", "%u", m_RenderStatistics.InstanceMeshDrawCommandCount);

					UI::TextTableRow("%s", "Point Light2D Count:", "%u", m_RenderStatistics.PointLight2DCount);
					UI::TextTableRow("%s", "Spot Light2D Count:", "%u", m_RenderStatistics.SpotLight2DCount);

					UI::TextTableRow("%s", "Transform Instances:", "%u", m_RenderStatistics.TransformInstanceCount);
					UI::TextTableRow("%s", "Instance Data Size:", "%u", m_RenderStatistics.InstanceDataSize);
					
					ImGui::EndTable();
				}
				UI::EndTreeNode();
			}
			const uint32_t frameIndex = Renderer::GetCurrentFrame();
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
				command.MaterialAsset->GetMaterial()
			);

			Renderer::RenderMesh(
				m_CommandBuffer,
				command.Pipeline,
				command.OverrideMaterial,
				command.Mesh->GetVertexBuffer(),
				command.Mesh->GetIndexBuffer(),
				glm::mat4(1.0f),
				m_TransformVertexBufferSet,
				command.TransformOffset,
				command.TransformInstanceCount
			);
			for (auto& dcOverride : command.OverrideCommands)
			{
				Renderer::RenderMesh(
					m_CommandBuffer,
					command.Pipeline,
					dcOverride.OverrideMaterial,
					command.Mesh->GetVertexBuffer(),
					command.Mesh->GetIndexBuffer(),
					dcOverride.Transform
				);
			}
		}

		for (auto& [key, command] : queue.AnimatedMeshDrawCommands)
		{
			Renderer::BindPipeline(
				m_CommandBuffer,
				command.Pipeline,
				m_Renderer2D->GetCameraBufferSet(),
				m_BoneTransformsStorageSet,
				command.MaterialAsset->GetMaterial()
			);
			Renderer::RenderMesh(
				m_CommandBuffer,
				command.Pipeline,
				command.OverrideMaterial,
				command.Mesh->GetVertexBuffer(),
				command.Mesh->GetIndexBuffer(),
				{ glm::mat4(1.0f), command.BoneTransformsIndex },
				m_TransformVertexBufferSet,
				command.TransformOffset,
				command.TransformInstanceCount
			);
			for (auto& dcOverride : command.OverrideCommands)
			{
				Renderer::RenderMesh(
					m_CommandBuffer,
					command.Pipeline,
					dcOverride.OverrideMaterial,
					command.Mesh->GetVertexBuffer(),
					command.Mesh->GetIndexBuffer(),
					{ dcOverride.Transform, dcOverride.BoneTransformsIndex },
					m_TransformVertexBufferSet,
					command.TransformOffset,
					command.TransformInstanceCount
				);
			}
		}
		uint32_t index = Renderer::GetCurrentFrame();
		for (auto& [key, command] : queue.InstanceMeshDrawCommands)
		{
			Renderer::BindPipeline(
				m_CommandBuffer,
				command.Pipeline,
				m_Renderer2D->GetCameraBufferSet(),
				nullptr,
				command.MaterialAsset->GetMaterial()
			);

			Renderer::RenderMesh(
				m_CommandBuffer,
				command.Pipeline,
				command.OverrideMaterial,
				command.Mesh->GetVertexBuffer(),
				command.Mesh->GetIndexBuffer(),
				command.Transform,
				m_InstanceVertexBufferSet,
				command.InstanceOffset,
				command.InstanceCount
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
			Ref<Material> material = command.Material->GetMaterial();
			for (uint32_t i = 0; i < command.TextureCount; ++i)
				material->SetImageArray("u_Texture", command.Textures[i]->GetImage(), i);
			for (uint32_t i = command.TextureCount; i < Renderer2D::GetMaxTextures(); ++i)
				material->SetImageArray("u_Texture", m_WhiteTexture->GetImage(), i);

			for (const auto& data : command.SpriteData)		
				m_Renderer2D->SubmitQuad(data.Transform, data.TexCoords, data.TextureIndex, data.Color);

			m_Renderer2D->Flush();
		}

		for (auto& [key, command] : queue.BillboardDrawCommands)
		{
			m_Renderer2D->SetQuadMaterial(command.Material);
			Ref<Material> material = command.Material->GetMaterial();
			for (uint32_t i = 0; i < command.TextureCount; ++i)
				material->SetImageArray("u_Texture", command.Textures[i]->GetImage(), i);
			for (uint32_t i = command.TextureCount; i < Renderer2D::GetMaxTextures(); ++i)
				material->SetImageArray("u_Texture", m_WhiteTexture->GetImage(), i);

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

		const uint32_t pointLightsSize = static_cast<uint32_t>(m_PointLights.size());
		const uint32_t spotLightsSize =  static_cast<uint32_t>(m_SpotLights.size());

		const int realNumPointLights = pointLightsSize < sc_MaxNumberOfLights ? pointLightsSize : sc_MaxNumberOfLights;
		const int realNumSpotLights = spotLightsSize < sc_MaxNumberOfLights ? spotLightsSize : sc_MaxNumberOfLights;

		m_LightRenderPipeline.MaterialInstance->Set("u_Uniforms.NumberPointLights", realNumPointLights);
		m_LightRenderPipeline.MaterialInstance->Set("u_Uniforms.NumberSpotLights", realNumSpotLights);

		
		Renderer::BindPipeline(
			m_CommandBuffer, 
			m_LightRenderPipeline.Pipeline, 
			m_CameraUniformBuffer, 
			m_LightStorageBufferSet, 
			m_LightRenderPipeline.Material
		);
		
		Renderer::SubmitFullscreenQuad(m_CommandBuffer, m_LightRenderPipeline.Pipeline, m_LightRenderPipeline.MaterialInstance);
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
		Ref<MaterialInstance> computeMaterialInst = m_BloomComputeMaterialInstance;
		Renderer::Submit([computeMaterialInst, bloomSettings = m_BloomSettings, prefilter]() mutable {
			computeMaterialInst->Set("u_Uniforms.FilterTreshold", bloomSettings.FilterTreshold);
			computeMaterialInst->Set("u_Uniforms.FilterKnee", bloomSettings.FilterKnee);
			computeMaterialInst->Set("u_Uniforms.Mode", prefilter);
			//computeMaterial->Set("u_Uniforms.LOD", 0.0f);			
		});
		computeMaterial->SetImage("o_Image", m_BloomTexture[0]->GetImage(), 0);
		computeMaterial->SetImage("u_Texture", lightPassImage);
		computeMaterial->SetImage("u_BloomTexture", lightPassImage);

		Renderer::BeginPipelineCompute(m_CommandBuffer, m_BloomComputePipeline, nullptr, nullptr, m_BloomComputeMaterial);		
		Renderer::DispatchCompute(m_BloomComputePipeline, computeMaterialInst, workGroupsX, workGroupsY, 1);
		imageBarrier(vulkanPipeline, m_BloomTexture[0]->GetImage());

		Renderer::Submit([computeMaterialInst, downsample]() mutable {
			computeMaterialInst->Set("u_Uniforms.Mode", downsample);
		});

		const uint32_t mips = m_BloomTexture[0]->GetMipLevelCount() - 2;
		for (uint32_t mip = 1; mip < mips; ++mip)
		{
			auto [mipWidth, mipHeight] = m_BloomTexture[0]->GetMipSize(mip);
			workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)workGroupSize);
			workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)workGroupSize);
	
			computeMaterial->SetImage("o_Image", m_BloomTexture[1]->GetImage(), mip);
			computeMaterial->SetImage("u_Texture", m_BloomTexture[0]->GetImage());
			Renderer::Submit([computeMaterialInst, mip]() mutable {
				computeMaterialInst->Set("u_Uniforms.LOD", (float)mip - 1.0f);
			});

			Renderer::UpdateDescriptors(m_BloomComputePipeline, m_BloomComputeMaterial, nullptr, nullptr);
			Renderer::DispatchCompute(m_BloomComputePipeline, computeMaterialInst, workGroupsX, workGroupsY, 1);
			imageBarrier(vulkanPipeline, m_BloomTexture[1]->GetImage());


			computeMaterial->SetImage("o_Image", m_BloomTexture[0]->GetImage(), mip);
			computeMaterial->SetImage("u_Texture", m_BloomTexture[1]->GetImage());
			Renderer::Submit([computeMaterialInst, mip]() mutable {
				computeMaterialInst->Set("u_Uniforms.LOD", (float)mip);
			});
			Renderer::UpdateDescriptors(m_BloomComputePipeline, m_BloomComputeMaterial, nullptr, nullptr);
			Renderer::DispatchCompute(m_BloomComputePipeline, computeMaterialInst, workGroupsX, workGroupsY, 1);
			imageBarrier(vulkanPipeline, m_BloomTexture[0]->GetImage());
		}
		Renderer::Submit([computeMaterialInst, mips, upsamplefirst]() mutable {
			computeMaterialInst->Set("u_Uniforms.Mode", upsamplefirst);
			computeMaterialInst->Set("u_Uniforms.LOD", mips - 2.0f);
		});
		
		m_BloomComputeMaterial->SetImage("o_Image", m_BloomTexture[2]->GetImage(), mips - 2);
		m_BloomComputeMaterial->SetImage("u_Texture", m_BloomTexture[0]->GetImage());
		
		auto [mipWidth, mipHeight] = m_BloomTexture[2]->GetMipSize(mips - 2);
		workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)workGroupSize);
		workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)workGroupSize);
		
		Renderer::UpdateDescriptors(m_BloomComputePipeline, m_BloomComputeMaterial, nullptr, nullptr);
		Renderer::DispatchCompute(m_BloomComputePipeline, computeMaterialInst, workGroupsX, workGroupsY, 1);	
		imageBarrier(vulkanPipeline, m_BloomTexture[2]->GetImage());
		
		// Upsample stage
		Renderer::Submit([computeMaterialInst, upsample]() mutable {
			computeMaterialInst->Set("u_Uniforms.Mode", upsample);
		});

		for (int32_t mip = mips - 3; mip >= 0; mip--)
		{
			auto [mipWidth, mipHeight] = m_BloomTexture[2]->GetMipSize(mip);
			workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)workGroupSize);
			workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)workGroupSize);
		
			m_BloomComputeMaterial->SetImage("o_Image", m_BloomTexture[2]->GetImage(), mip);
			m_BloomComputeMaterial->SetImage("u_Texture", m_BloomTexture[0]->GetImage());
			m_BloomComputeMaterial->SetImage("u_BloomTexture", m_BloomTexture[2]->GetImage());
			Renderer::Submit([computeMaterialInst, mip]() mutable {

				computeMaterialInst->Set("u_Uniforms.LOD", (float)mip);
			});
		
			Renderer::UpdateDescriptors(m_BloomComputePipeline, m_BloomComputeMaterial, nullptr, nullptr);
			Renderer::DispatchCompute(m_BloomComputePipeline, computeMaterialInst, workGroupsX, workGroupsY, 1);
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
		Renderer::SubmitFullscreenQuad(m_CommandBuffer, m_CompositeRenderPipeline.Pipeline, m_CompositeRenderPipeline.MaterialInstance);
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
		m_CompositePass = RenderPass::Create(renderPassSpec);
	}
	void SceneRenderer::createLightPass()
	{
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
	}

	void SceneRenderer::createGeometryPass()
	{
		FramebufferSpecification framebufferSpec;
		framebufferSpec.Attachments = {
				FramebufferTextureSpecification(ImageFormat::RGBA32F),
				FramebufferTextureSpecification(ImageFormat::RGBA32F),
				FramebufferTextureSpecification(ImageFormat::DEPTH24STENCIL8)
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
			if (width != 0 && height != 0)
			{
				m_GeometryPass->GetSpecification().TargetFramebuffer->Resize(width, height);
				m_LightPass->GetSpecification().TargetFramebuffer->Resize(width, height);
				m_CompositePass->GetSpecification().TargetFramebuffer->Resize(width, height);

				TextureProperties props;
				props.Storage = true;
				props.SamplerWrap = TextureWrap::Clamp;
				// TODO: resizing
				m_BloomTexture[0] = Texture2D::Create(ImageFormat::RGBA32F, width, height, nullptr, props);
				m_BloomTexture[1] = Texture2D::Create(ImageFormat::RGBA32F, width, height, nullptr, props);
				m_BloomTexture[2] = Texture2D::Create(ImageFormat::RGBA32F, width, height, nullptr, props);
			}
			m_ViewportSizeChanged = false;
		}
	}
	void SceneRenderer::preRender()
	{
		prepareInstances();
		prepareLights();
		m_RenderStatistics.SpriteDrawCommandCount = static_cast<uint32_t>(m_Queue.SpriteDrawCommands.size());
	}
	void SceneRenderer::prepareInstances()
	{
		// Prepare transforms
		size_t overrideCount = 0;
		uint32_t transformsCount = 0;
		for (auto& [key, dc] : m_Queue.MeshDrawCommands)
		{
			dc.Pipeline = getGeometryPipeline(dc.MaterialAsset->GetMaterial(), dc.MaterialAsset->IsOpaque());
			dc.TransformOffset = transformsCount * sizeof(RenderQueue::TransformData);
			overrideCount += dc.OverrideCommands.size();
			for (const auto& transform : dc.TransformData)
			{
				m_TransformData[transformsCount] = transform;
				transformsCount++;
			}
		}


		uint32_t boneTransformsCount = 0;
		for (auto& [key, dc] : m_Queue.AnimatedMeshDrawCommands)
		{
			auto material = 
			dc.Pipeline = getGeometryPipeline(dc.MaterialAsset->GetMaterial(), dc.MaterialAsset->IsOpaque());
			dc.TransformOffset = transformsCount * sizeof(RenderQueue::TransformData);
			dc.BoneTransformsIndex = boneTransformsCount;
			overrideCount += dc.OverrideCommands.size();
			for (const auto& transform : dc.TransformData)
			{
				m_TransformData[transformsCount] = transform;
				transformsCount++;
			}
			for (const auto& bones : dc.BoneData)
			{
				const size_t offset = boneTransformsCount * bones.size();
				memcpy(&m_BoneTransformsData[offset], bones.data(), sizeof(RenderQueue::BoneTransforms));
				boneTransformsCount++;
			}
			for (auto& overrideDc : dc.OverrideCommands)
			{
				const auto& bones = overrideDc.BoneTransforms;
				const size_t offset = boneTransformsCount * bones.size();
				memcpy(&m_BoneTransformsData[offset], bones.data(), sizeof(RenderQueue::BoneTransforms));
				overrideDc.BoneTransformsIndex = boneTransformsCount;
				boneTransformsCount++;
			}
		}

		// Prepare transforms and instance data
		uint32_t instanceOffset = 0;
		for (auto& [key, dc] : m_Queue.InstanceMeshDrawCommands)
		{
			dc.Pipeline = getGeometryPipeline(dc.MaterialAsset->GetMaterial(), dc.MaterialAsset->IsOpaque());
			dc.InstanceOffset = instanceOffset;
			memcpy(&m_InstanceData.data()[instanceOffset], dc.InstanceData.data(), dc.InstanceData.size());
			instanceOffset += dc.InstanceData.size();
		}	
		uint32_t index = Renderer::GetCurrentFrame();
		m_TransformVertexBufferSet->Update(m_TransformData.data(), transformsCount * sizeof(RenderQueue::TransformData));
		m_InstanceVertexBufferSet->Update(m_InstanceData.data(), instanceOffset);
		m_BoneTransformsStorageSet->Update(m_BoneTransformsData.data(), boneTransformsCount * sizeof(RenderQueue::BoneTransforms), 0, 0, 2);
	
		
		m_RenderStatistics.MeshDrawCommandCount = static_cast<uint32_t>(m_Queue.MeshDrawCommands.size());
		m_RenderStatistics.MeshOverrideDrawCommandCount = static_cast<uint32_t>(overrideCount);
		m_RenderStatistics.InstanceMeshDrawCommandCount = static_cast<uint32_t>(m_Queue.InstanceMeshDrawCommands.size());

		m_RenderStatistics.TransformInstanceCount = transformsCount;
		m_RenderStatistics.InstanceDataSize = instanceOffset;
	}
	void SceneRenderer::prepareLights()
	{
		// Prepare lights
		auto& registry = m_ActiveScene->GetRegistry();

		m_SpotLights.reserve(registry.storage<SpotLight2D>().size());
		m_PointLights.reserve(registry.storage<PointLight2D>().size());

		// Spot lights
		auto spotLight2DView = registry.view<TransformComponent, SpotLight2D>();
		for (auto entity : spotLight2DView)
		{
			// Render previous frame data
			auto& [transform, light] = spotLight2DView.get<TransformComponent, SpotLight2D>(entity);
			auto [trans, rot, scale] = transform.GetWorldComponents();

			SpotLight lightData;
			lightData.Color = glm::vec4(light.Color, 1.0f);
			lightData.Position = trans;
			lightData.Radius = light.Radius;
			lightData.Intensity = light.Intensity;
			lightData.InnerAngle = light.InnerAngle;
			lightData.OuterAngle = light.OuterAngle;

			m_SpotLights.push_back(lightData);
		}

		// Point Lights
		auto pointLight2DView = registry.view<TransformComponent, PointLight2D>();
		for (auto entity : pointLight2DView)
		{
			auto& [transform, light] = pointLight2DView.get<TransformComponent, PointLight2D>(entity);
			auto [trans, rot, scale] = transform.GetWorldComponents();
			PointLight lightData;
			lightData.Color = glm::vec4(light.Color, 1.0f);
			lightData.Position = trans;
			lightData.Radius = light.Radius;
			lightData.Intensity = light.Intensity;
			m_PointLights.push_back(lightData);
		}
		
		const uint32_t pointLightsSize = static_cast<uint32_t>(m_PointLights.size());
		const uint32_t spotLightsSize = static_cast<uint32_t>(m_SpotLights.size());

		const uint32_t realNumPointLights = pointLightsSize < sc_MaxNumberOfLights ? pointLightsSize : sc_MaxNumberOfLights;
		const uint32_t realNumSpotLights = spotLightsSize < sc_MaxNumberOfLights ? spotLightsSize : sc_MaxNumberOfLights;

		Ref<StorageBufferSet> instance = m_LightStorageBufferSet;
		Renderer::Submit([instance, pLights = m_PointLights, sLights = m_SpotLights, realNumPointLights, realNumSpotLights]() mutable {
			const uint32_t frame = Renderer::GetCurrentFrame();
			instance->Get(1, 0, frame)->RT_Update(pLights.data(), realNumPointLights * sizeof(PointLight));
			instance->Get(2, 0, frame)->RT_Update(sLights.data(), realNumSpotLights * sizeof(SpotLight));
		});

		m_RenderStatistics.PointLight2DCount = static_cast<uint32_t>(m_PointLights.size());
		m_RenderStatistics.SpotLight2DCount = static_cast<uint32_t>(m_SpotLights.size());
	}
	Ref<Pipeline> SceneRenderer::getGeometryPipeline(const Ref<Material>& material, bool opaque)
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
		spec.DepthTest = true;
		spec.DepthWrite = true;

		auto& pipeline = m_GeometryPipelines[shader->GetHash()];
		pipeline = Pipeline::Create(spec);
		return pipeline;
	}

	void SceneRenderer::copyToBoneStorage(RenderQueue::BoneTransforms& storage, const std::vector<ozz::math::Float4x4>& boneTransforms, const Ref<AnimatedMesh>& mesh)
	{
		if (boneTransforms.empty())
		{
			for (auto& bone : storage)
				bone = ozz::math::Float4x4::identity();
		}
		else
		{
			const auto& boneInfo = mesh->GetMeshSource()->GetBoneInfo();
			for (size_t i = 0; i < boneTransforms.size(); ++i)
			{
				const uint32_t jointIndex = boneInfo[i].JointIndex;
				storage[i] = boneInfo[i].InverseTransform * boneTransforms[jointIndex] * boneInfo[i].BoneOffset;
			}
		}
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
		this->MaterialInstance = Ref<XYZ::MaterialInstance>::Create(this->Material);
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