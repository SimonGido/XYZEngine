#include "stdafx.h"
#include "GeometryPass.h"

#include "XYZ/API/Vulkan/VulkanRendererAPI.h"
#include "XYZ/API/Vulkan/VulkanPipelineCompute.h"
#include "XYZ/API/Vulkan/VulkanRenderCommandBuffer.h"

#include "XYZ/Renderer/SceneRenderer.h"

#include "XYZ/Utils/Math/Math.h"

#include "XYZ/Debug/Profiler.h"

namespace XYZ {
	GeometryPass::GeometryPass()
	{
	}
	void GeometryPass::Init(WeakRef<SceneRenderer> renderer)
	{
		m_SceneRenderer = renderer;
		m_InstanceVertexBufferSet = Ref<VertexBufferSet>::Create(Renderer::GetConfiguration().FramesInFlight, sc_InstanceVertexBufferSize);
		m_TransformVertexBufferSet = Ref<VertexBufferSet>::Create(Renderer::GetConfiguration().FramesInFlight, sc_TransformBufferSize);
		
		m_Renderer2D = Ref<Renderer2D>::Create(Renderer2DConfiguration{ m_SceneRenderer->m_CommandBuffer, m_SceneRenderer->m_UniformBufferSet });
		m_WhiteTexture = Renderer::GetDefaultResources().RendererAssets.at("WhiteTexture").As<Texture2D>();

		m_SceneRenderer->m_TransformData.resize(GetTransformBufferCount());
		m_SceneRenderer->m_InstanceData.resize(sc_InstanceVertexBufferSize);

		createDepthResources();
	}
	void GeometryPass::PreDepthPass(
		Ref<PrimaryRenderCommandBuffer> commandBuffer,
		GeometryRenderQueue& queue,
		const glm::mat4& viewMatrix,
		bool clear
	)
	{
		XYZ_PROFILE_FUNC("GeometryPass::PreDepthPass");

		// Pre depth
		uint32_t depthPassQuery = commandBuffer->BeginTimestampQuery();
		Renderer::BeginRenderPass(commandBuffer, m_SceneRenderer->m_DepthRenderPass, false, clear);

		submit2DDepth(queue, commandBuffer, viewMatrix);
		submitStaticMeshesDepth(queue, commandBuffer);
		submitAnimatedMeshesDepth(queue, commandBuffer);
		submitInstancedMeshesDepth(queue, commandBuffer);

		Renderer::EndRenderPass(commandBuffer);
		postDepthPass(commandBuffer);

		commandBuffer->EndTimestampQuery(depthPassQuery);
	}

	void GeometryPass::SubmitCompute(Ref<PrimaryRenderCommandBuffer> commandBuffer, GeometryRenderQueue& queue)
	{
		XYZ_PROFILE_FUNC("GeometryPass::SubmitCompute");

		submitComputeCommands(queue, commandBuffer); // Must be called outside render pass
	}


	void GeometryPass::Submit(
		Ref<PrimaryRenderCommandBuffer> commandBuffer,
		GeometryRenderQueue& queue, 
		const glm::mat4& viewMatrix,
		bool clear
	)
	{
		XYZ_PROFILE_FUNC("GeometryPass::Submit");

		// Geometry
		uint32_t geometryPassQuery = commandBuffer->BeginTimestampQuery();
		Renderer::BeginRenderPass(commandBuffer, m_SceneRenderer->m_GeometryRenderPass, false, clear);

		submit2D(queue, commandBuffer, viewMatrix);
		submitStaticMeshes(queue, commandBuffer);
		submitAnimatedMeshes(queue, commandBuffer);
		submitInstancedMeshes(queue, commandBuffer);
		
		submitIndirectCommands(queue, commandBuffer);

		Renderer::EndRenderPass(commandBuffer);
		commandBuffer->EndTimestampQuery(geometryPassQuery);
	}

	GeometryPassStatistics GeometryPass::PreSubmit(GeometryRenderQueue& queue)
	{
		XYZ_PROFILE_FUNC("GeometryPass::PreSubmit");
		size_t overrideCount = 0;
		size_t animatedOverrideCount = 0;
		uint32_t transformsCount = 0;
		uint32_t boneTransformCount = 0;
		uint32_t instanceOffset = 0;

		prepareComputeCommands(queue);
		prepareIndirectCommands(queue);
		prepareStaticDrawCommands(queue, overrideCount, transformsCount);
		prepareAnimatedDrawCommands(queue, animatedOverrideCount, transformsCount, boneTransformCount);
		prepareInstancedDrawCommands(queue, instanceOffset);
		prepare2DDrawCommands(queue);

		m_TransformVertexBufferSet->Update(m_SceneRenderer->m_TransformData.data(), transformsCount * sizeof(GeometryRenderQueue::TransformData));
		m_InstanceVertexBufferSet->Update(m_SceneRenderer->m_InstanceData.data(), instanceOffset);
		
		m_SceneRenderer->m_StorageBufferSet->Update(m_SceneRenderer->m_BoneTransformSSBO.Data, boneTransformCount * sizeof(GeometryRenderQueue::BoneTransforms), 0, SSBOBoneTransformData::Binding, SSBOBoneTransformData::Set);
		
		return { 
			static_cast<uint32_t>(overrideCount), 
			static_cast<uint32_t>(animatedOverrideCount), 
			transformsCount, 
			instanceOffset,
			boneTransformCount
		};
	}

	void GeometryPass::submitComputeCommands(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer)
	{
		for (auto& [key, com] : queue.ComputeCommands)
		{
			Renderer::BeginPipelineCompute(
				commandBuffer,
				com.Pipeline,
				m_SceneRenderer->m_UniformBufferSet,
				m_SceneRenderer->m_StorageBufferSet,
				com.MaterialCompute->GetMaterial()
			);

			// Regular compute commands
			for (auto& command : com.ComputeCommands)
			{
				for (auto& data : command.AllocationData)
				{
					m_SceneRenderer->m_StorageBufferSet->SetBufferInfo(
						data.Allocation.GetSize(),
						data.Allocation.GetOffset(),
						data.Allocation.GetBinding(),
						data.Allocation.GetSet()
					);
				}
				
				Renderer::UpdateDescriptors(
					com.Pipeline,
					com.MaterialCompute->GetMaterial(),
					m_SceneRenderer->m_UniformBufferSet,
					m_SceneRenderer->m_StorageBufferSet
				);

				Renderer::DispatchCompute(
					com.Pipeline,
					nullptr,
					32, 32, 1,
					command.OverrideUniformData
				);

				Renderer::Submit([renderCommandBuffer = commandBuffer]() mutable
					{
						const uint32_t frameIndex = Renderer::GetCurrentFrame();
						VkMemoryBarrier barrier{};

						barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
						barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
						barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_MEMORY_READ_BIT;

						VkCommandBuffer vulkanCommandBuffer = (const VkCommandBuffer)renderCommandBuffer->CommandBufferHandle(frameIndex);

						vkCmdPipelineBarrier(vulkanCommandBuffer,
							VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
							VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
							0,
							1, &barrier,
							0, nullptr,
							0, nullptr);
					});


			}

			Renderer::EndPipelineCompute(com.Pipeline);
		}
	}


	void GeometryPass::submitIndirectCommands(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer)
	{
		uint32_t index = 0;
		for (auto& [key, dc] : queue.IndirectDrawCommands)
		{
			Renderer::BindPipeline(
				commandBuffer,
				dc.Pipeline,
				m_SceneRenderer->m_UniformBufferSet,
				m_SceneRenderer->m_StorageBufferSet,
				dc.MaterialAsset->GetMaterial()
			);

			for (auto& dcOverride : dc.OverrideCommands)
			{
				m_SceneRenderer->m_StorageBufferSet->SetBufferInfo(
					dcOverride.IndirectCommandAllocation.GetSize(),
					dcOverride.IndirectCommandAllocation.GetOffset(),
					dcOverride.IndirectCommandAllocation.GetBinding(),
					dcOverride.IndirectCommandAllocation.GetSet()
				);

				m_SceneRenderer->m_StorageBufferSet->SetBufferInfo(
					dcOverride.ReadStateAllocation.GetSize(),
					dcOverride.ReadStateAllocation.GetOffset(),
					dcOverride.ReadStateAllocation.GetBinding(),
					dcOverride.ReadStateAllocation.GetSet()
				);

				Renderer::UpdateDescriptors(
					commandBuffer,
					dc.Pipeline,
					dc.MaterialAsset->GetMaterial(),
					m_SceneRenderer->m_UniformBufferSet,
					m_SceneRenderer->m_StorageBufferSet
				);

				Renderer::RenderIndirectMesh(
					commandBuffer,
					dc.Pipeline,
					dc.MaterialAsset->GetMaterialInstance(),
					dcOverride.Mesh->GetVertexBuffer(),
					dcOverride.Mesh->GetIndexBuffer(),
					{ dcOverride.Transform },
					m_SceneRenderer->m_StorageBufferSet,
					dcOverride.IndirectCommandAllocation.GetOffset(),
					1,
					sizeof(IndirectIndexedDrawCommand)
				);
				index++;
			}
		}
	}

	void GeometryPass::submitStaticMeshes(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer)
	{
		for (auto& [key, command] : queue.MeshDrawCommands)
		{
			Renderer::BindPipeline(
				commandBuffer,
				command.Pipeline,
				m_SceneRenderer->m_UniformBufferSet,
				m_SceneRenderer->m_StorageBufferSet,
				command.MaterialAsset->GetMaterial()
			);
			if (command.Count > 0)
			{
				Renderer::RenderMesh(
					commandBuffer,
					command.Pipeline,
					command.OverrideMaterial,
					command.Mesh->GetVertexBuffer(),
					command.Mesh->GetIndexBuffer(),
					glm::mat4(1.0f),
					m_TransformVertexBufferSet,
					command.TransformOffset,
					command.TransformInstanceCount
				);
			}
			for (auto& dcOverride : command.OverrideCommands)
			{
				Renderer::RenderMesh(
					commandBuffer,
					command.Pipeline,
					dcOverride.OverrideMaterial,
					command.Mesh->GetVertexBuffer(),
					command.Mesh->GetIndexBuffer(),
					dcOverride.Transform
				);
			}
		}
	}
	void GeometryPass::submitAnimatedMeshes(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer)
	{
		for (auto& [key, command] : queue.AnimatedMeshDrawCommands)
		{
			Renderer::BindPipeline(
				commandBuffer,
				command.Pipeline,
				m_SceneRenderer->m_UniformBufferSet,
				m_SceneRenderer->m_StorageBufferSet,
				command.MaterialAsset->GetMaterial()
			);
			if (command.Count > 0)
			{
				Renderer::RenderMesh(
					commandBuffer,
					command.Pipeline,
					command.OverrideMaterial,
					command.Mesh->GetVertexBuffer(),
					command.Mesh->GetIndexBuffer(),
					{ glm::mat4(1.0f), command.BoneTransformsIndex },
					m_TransformVertexBufferSet,
					command.TransformOffset,
					command.TransformInstanceCount
				);
			}
			for (auto& dcOverride : command.OverrideCommands)
			{
				Renderer::RenderMesh(
					commandBuffer,
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
	}
	void GeometryPass::submitInstancedMeshes(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer)
	{
		for (auto& [key, command] : queue.InstanceMeshDrawCommands)
		{
			Renderer::BindPipeline(
				commandBuffer,
				command.Pipeline,
				m_SceneRenderer->m_UniformBufferSet,
				nullptr,
				command.MaterialAsset->GetMaterial()
			);

			Renderer::RenderMesh(
				commandBuffer,
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
	}
	void GeometryPass::submit2D(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer, const glm::mat4& viewMatrix)
	{
		XYZ_PROFILE_FUNC("GeometryPass2D::Submit");

		m_Renderer2D->BeginScene(viewMatrix);

		for (auto& [key, command] : queue.SpriteDrawCommands)
		{
			Ref<Pipeline> pipeline = m_PipelineCache.PreparePipeline(command.MaterialAsset, m_SceneRenderer->m_GeometryRenderPass);
			for (const auto& data : command.SpriteData)
				m_Renderer2D->SubmitQuad(data.Transform, data.TexCoords, data.TextureIndex, data.Color);

			Renderer::BindPipeline(commandBuffer, pipeline, m_SceneRenderer->m_UniformBufferSet, nullptr, command.MaterialAsset->GetMaterial());
			m_Renderer2D->FlushQuads(pipeline, command.MaterialInstance, true);
		}

		for (auto& [key, command] : queue.BillboardDrawCommands)
		{
			Ref<Pipeline> pipeline = m_PipelineCache.PreparePipeline(command.MaterialAsset, m_SceneRenderer->m_GeometryRenderPass);
			for (const auto& data : command.BillboardData)
				m_Renderer2D->SubmitQuadBillboard(data.Position, data.Size, data.TexCoords, data.TextureIndex, data.Color);

			Renderer::BindPipeline(commandBuffer, pipeline, m_SceneRenderer->m_UniformBufferSet, nullptr, command.MaterialAsset->GetMaterial());
			m_Renderer2D->FlushQuads(pipeline, command.MaterialInstance, true);
		}
		m_Renderer2D->EndScene();
	}

	void GeometryPass::submitStaticMeshesDepth(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer)
	{
		Renderer::BindPipeline(
			commandBuffer,
			m_DepthPipeline3DStatic.Pipeline,
			m_SceneRenderer->m_UniformBufferSet,
			nullptr,
			m_DepthPipeline3DStatic.Material
		);

		for (auto& [key, command] : queue.MeshDrawCommands)
		{
			Renderer::RenderMesh(
				commandBuffer,
				m_DepthPipeline3DStatic.Pipeline,
				m_DepthPipeline3DStatic.MaterialInstance,
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
					commandBuffer,
					m_DepthPipeline3DStatic.Pipeline,
					m_DepthPipeline3DStatic.MaterialInstance,
					command.Mesh->GetVertexBuffer(),
					command.Mesh->GetIndexBuffer(),
					dcOverride.Transform
				);
			}
		}
	}
	void GeometryPass::submitAnimatedMeshesDepth(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer)
	{
		Renderer::BindPipeline(
			commandBuffer,
			m_DepthPipeline3DAnimated.Pipeline,
			m_SceneRenderer->m_UniformBufferSet,
			m_SceneRenderer->m_StorageBufferSet,
			m_DepthPipeline3DAnimated.Material
		);
		for (auto& [key, command] : queue.AnimatedMeshDrawCommands)
		{		
			Renderer::RenderMesh(
				commandBuffer,
				m_DepthPipeline3DAnimated.Pipeline,
				m_DepthPipeline3DAnimated.MaterialInstance,
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
					commandBuffer,
					m_DepthPipeline3DAnimated.Pipeline,
					m_DepthPipeline3DAnimated.MaterialInstance,
					command.Mesh->GetVertexBuffer(),
					command.Mesh->GetIndexBuffer(),
					{ dcOverride.Transform, dcOverride.BoneTransformsIndex },
					m_TransformVertexBufferSet,
					command.TransformOffset,
					command.TransformInstanceCount
				);
			}
		}
	}
	void GeometryPass::submitInstancedMeshesDepth(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer)
	{
		Renderer::BindPipeline(
			commandBuffer,
			m_DepthPipelineInstanced.Pipeline,
			m_SceneRenderer->m_UniformBufferSet,
			nullptr,
			m_DepthPipelineInstanced.Material
		);
		for (auto& [key, command] : queue.InstanceMeshDrawCommands)
		{
			Renderer::RenderMesh(
				commandBuffer,
				m_DepthPipelineInstanced.Pipeline,
				m_DepthPipelineInstanced.MaterialInstance,
				command.Mesh->GetVertexBuffer(),
				command.Mesh->GetIndexBuffer(),
				{ command.Transform, 0 },
				m_InstanceVertexBufferSet,
				command.InstanceOffset,
				command.InstanceCount
			);
		}
	}

	void GeometryPass::submit2DDepth(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer, const glm::mat4& viewMatrix)
	{
		XYZ_PROFILE_FUNC("GeometryPass2D::Submit");

		m_Renderer2D->BeginScene(viewMatrix);
	
		Renderer::BindPipeline(
			commandBuffer, 
			m_DepthPipeline2D.Pipeline, 
			m_SceneRenderer->m_UniformBufferSet,
			nullptr, 
			m_DepthPipeline2D.Material
		);
		for (auto& [key, command] : queue.SpriteDrawCommands)
		{
			for (const auto& data : command.SpriteData)
				m_Renderer2D->SubmitQuad(data.Transform, data.TexCoords, data.TextureIndex, data.Color);

			m_Renderer2D->FlushQuads(m_DepthPipeline2D.Pipeline, command.MaterialInstance, false);
		}
		for (auto& [key, command] : queue.BillboardDrawCommands)
		{	
			for (const auto& data : command.BillboardData)
				m_Renderer2D->SubmitQuadBillboard(data.Position, data.Size, data.TexCoords, data.TextureIndex, data.Color);

			m_Renderer2D->FlushQuads(m_DepthPipeline2D.Pipeline, command.MaterialInstance, false);
		}
		m_Renderer2D->EndScene(false);
	}

	void GeometryPass::prepareStaticDrawCommands(GeometryRenderQueue& queue, size_t& overrideCount, uint32_t& transformsCount)
	{	
		for (auto& [key, dc] : queue.MeshDrawCommands)
		{
			dc.Pipeline = m_PipelineCache.PreparePipeline(dc.MaterialAsset, m_SceneRenderer->m_GeometryRenderPass);
			dc.TransformOffset = transformsCount * sizeof(GeometryRenderQueue::TransformData);
			overrideCount += dc.OverrideCommands.size();
			for (const auto& transform : dc.TransformData)
			{
				m_SceneRenderer->m_TransformData[transformsCount] = transform;
				transformsCount++;
			}
		}
	}
	void GeometryPass::prepareComputeCommands(GeometryRenderQueue& queue)
	{
		for (auto& [key, dc] : queue.ComputeCommands)
		{
			dc.Pipeline = m_PipelineCache.PrepareComputePipeline(dc.MaterialCompute);
			for (auto& cmd : dc.ComputeCommands)
			{
				for (auto& data : cmd.AllocationData)
				{
					if (data.ComputeDataSize == 0) // Size of data required for computation is zero, no update required
						continue;

					m_SceneRenderer->m_StorageBufferSet->Update(
						cmd.ComputeData.data() + data.ComputeDataOffset, // Data required for computation
						data.ComputeDataSize,	// Sizeof data required for computation
						data.Allocation.GetOffset(),
						data.Allocation.GetBinding(),
						data.Allocation.GetSet()
					);
				}
			}
		}
	}

	void GeometryPass::prepareIndirectCommands(GeometryRenderQueue& queue)
	{
		for (auto& [key, dc] : queue.IndirectDrawCommands)
		{
			dc.Pipeline = m_PipelineCache.PreparePipeline(dc.MaterialAsset, m_SceneRenderer->m_GeometryRenderPass);
			
		}
	}
	

	void GeometryPass::prepareAnimatedDrawCommands(GeometryRenderQueue& queue, size_t& overrideCount, uint32_t& transformsCount, uint32_t& boneTransformCount)
	{
		for (auto& [key, dc] : queue.AnimatedMeshDrawCommands)
		{
			dc.Pipeline = m_PipelineCache.PreparePipeline(dc.MaterialAsset, m_SceneRenderer->m_GeometryRenderPass);
			dc.TransformOffset = transformsCount * sizeof(SceneRenderer::TransformData);
			dc.BoneTransformsIndex = boneTransformCount;
			overrideCount += dc.OverrideCommands.size();
			for (const auto& transform : dc.TransformData)
			{
				m_SceneRenderer->m_TransformData[transformsCount] = transform;
				transformsCount++;
			}
			for (const auto& bones : dc.BoneData)
			{
				const size_t offset = boneTransformCount * bones.size();
				memcpy(&m_SceneRenderer->m_BoneTransformSSBO.Data[offset], bones.data(), sizeof(GeometryRenderQueue::BoneTransforms));
				boneTransformCount++;
			}
			for (auto& overrideDc : dc.OverrideCommands)
			{
				const auto& bones = overrideDc.BoneTransforms;
				const size_t offset = boneTransformCount * bones.size();
				memcpy(&m_SceneRenderer->m_BoneTransformSSBO.Data[offset], bones.data(), sizeof(GeometryRenderQueue::BoneTransforms));
				overrideDc.BoneTransformsIndex = boneTransformCount;
				boneTransformCount++;
			}
		}
	}

	void GeometryPass::postDepthPass(const Ref<RenderCommandBuffer>& commandBuffer)
	{
		Renderer::Submit([cb = commandBuffer, image = m_SceneRenderer->m_DepthRenderPass->GetSpecification().TargetFramebuffer->GetDepthImage().As<VulkanImage2D>()]()
			{
				VkImageMemoryBarrier imageMemoryBarrier = {};
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				imageMemoryBarrier.image = image->GetImageInfo().Image;
				imageMemoryBarrier.subresourceRange = { image->GetImageViewAspectFlags() , 0, image->GetSpecification().Mips, 0, 1};
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				const VkCommandBuffer vulkanCommandBuffer = (const VkCommandBuffer)cb.As<VulkanPrimaryRenderCommandBuffer>()->CommandBufferHandle(Renderer::GetCurrentFrame());

				vkCmdPipelineBarrier(
					vulkanCommandBuffer,
					VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &imageMemoryBarrier);
			});
	}

	void GeometryPass::prepareInstancedDrawCommands(GeometryRenderQueue& queue, uint32_t& instanceOffset)
	{
		for (auto& [key, dc] : queue.InstanceMeshDrawCommands)
		{
			dc.Pipeline = m_PipelineCache.PreparePipeline(dc.MaterialAsset, m_SceneRenderer->m_GeometryRenderPass);
			dc.InstanceOffset = instanceOffset;
			memcpy(&m_SceneRenderer->m_InstanceData.data()[instanceOffset], dc.InstanceData.data(), dc.InstanceData.size());
			instanceOffset += dc.InstanceData.size();
		}
	}

	void GeometryPass::prepare2DDrawCommands(GeometryRenderQueue& queue)
	{
		for (auto& [key, command] : queue.SpriteDrawCommands)
		{
			for (uint32_t i = 0; i < command.TextureCount; ++i)
				command.MaterialAsset->GetMaterial()->SetImageArray("u_Texture", command.Textures[i]->GetImage(), i);
			for (uint32_t i = command.TextureCount; i < Renderer2D::GetMaxTextures(); ++i)
				command.MaterialAsset->GetMaterial()->SetImageArray("u_Texture", m_WhiteTexture->GetImage(), i);
		}
		for (auto& [key, command] : queue.BillboardDrawCommands)
		{
			for (uint32_t i = 0; i < command.TextureCount; ++i)
				command.MaterialAsset->GetMaterial()->SetImageArray("u_Texture", command.Textures[i]->GetImage(), i);
			for (uint32_t i = command.TextureCount; i < Renderer2D::GetMaxTextures(); ++i)
				command.MaterialAsset->GetMaterial()->SetImageArray("u_Texture", m_WhiteTexture->GetImage(), i);
		}
	}

	void GeometryPass::createDepthResources()
	{
		// 3D Anim
		{
			Ref<MaterialAsset> depthMaterialAsset = Renderer::GetDefaultResources().RendererAssets.at("Depth3DMaterialAnim").As<MaterialAsset>();
			m_DepthPipeline3DAnimated.Shader = depthMaterialAsset->GetShader();
			m_DepthPipeline3DAnimated.Material = Material::Create(depthMaterialAsset->GetShader());
			m_DepthPipeline3DAnimated.MaterialInstance = m_DepthPipeline3DAnimated.Material->CreateMaterialInstance();

			PipelineSpecification spec;
			spec.RenderPass = m_SceneRenderer->m_DepthRenderPass;
			spec.Shader = m_DepthPipeline3DAnimated.Shader;
			spec.Topology = PrimitiveTopology::Triangles;
			spec.DepthTest = true;
			spec.DepthWrite = true;
			m_DepthPipeline3DAnimated.Pipeline = Pipeline::Create(spec);
		}

		// 3D Static
		{
			Ref<MaterialAsset> depthMaterialAsset = Renderer::GetDefaultResources().RendererAssets.at("Depth3DMaterial").As<MaterialAsset>();
			m_DepthPipeline3DStatic.Shader = depthMaterialAsset->GetShader();
			m_DepthPipeline3DStatic.Material = Material::Create(depthMaterialAsset->GetShader());
			m_DepthPipeline3DStatic.MaterialInstance = m_DepthPipeline3DStatic.Material->CreateMaterialInstance();

			PipelineSpecification spec;
			spec.RenderPass = m_SceneRenderer->m_DepthRenderPass;
			spec.Shader = m_DepthPipeline3DStatic.Shader;
			spec.Topology = PrimitiveTopology::Triangles;
			spec.DepthTest = true;
			spec.DepthWrite = true;
			m_DepthPipeline3DStatic.Pipeline = Pipeline::Create(spec);
		}

		// 2D
		{
			Ref<MaterialAsset> depthMaterialAsset = Renderer::GetDefaultResources().RendererAssets.at("Depth2DMaterial").As<MaterialAsset>();
			m_DepthPipeline2D.Shader = depthMaterialAsset->GetShader();
			m_DepthPipeline2D.Material = Material::Create(depthMaterialAsset->GetShader());
			m_DepthPipeline2D.MaterialInstance = m_DepthPipeline2D.Material->CreateMaterialInstance();

			PipelineSpecification spec;
			spec.RenderPass = m_SceneRenderer->m_DepthRenderPass;
			spec.Shader = m_DepthPipeline2D.Shader;
			spec.Topology = PrimitiveTopology::Triangles;
			spec.DepthTest = true;
			spec.DepthWrite = true;
			m_DepthPipeline2D.Pipeline = Pipeline::Create(spec);
		}

		// Instanced
		{
			Ref<MaterialAsset> depthMaterialAsset = Renderer::GetDefaultResources().RendererAssets.at("DepthInstancedMaterial").As<MaterialAsset>();
			m_DepthPipelineInstanced.Shader = depthMaterialAsset->GetShader();
			m_DepthPipelineInstanced.Material = Material::Create(depthMaterialAsset->GetShader());
			m_DepthPipelineInstanced.MaterialInstance = m_DepthPipelineInstanced.Material->CreateMaterialInstance();

			PipelineSpecification spec;
			spec.RenderPass = m_SceneRenderer->m_DepthRenderPass;
			spec.Shader = m_DepthPipelineInstanced.Shader;
			spec.Topology = PrimitiveTopology::Triangles;
			spec.DepthTest = true;
			spec.DepthWrite = true;
			m_DepthPipelineInstanced.Pipeline = Pipeline::Create(spec);
		}
	}
}