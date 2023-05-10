#include "stdafx.h"
#include "VoxelRenderer.h"

#include "Renderer.h"

#include "XYZ/API/Vulkan/VulkanPipelineCompute.h"
#include "XYZ/API/Vulkan/VulkanStorageBufferSet.h"

#include "XYZ/Utils/Math/Math.h"
#include "XYZ/Utils/Math/AABB.h"
#include "XYZ/Utils/Random.h"

#include "XYZ/ImGui/ImGui.h"


#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace XYZ {

#define TILE_SIZE 16

	
	static AABB VoxelModelToAABB(const glm::mat4& transform, uint32_t width, uint32_t height, uint32_t depth, float voxelSize)
	{
		AABB result;
		glm::vec3 min = glm::vec3(0.0f);
		glm::vec3 max = glm::vec3(width, height, depth) * voxelSize;

		result = result.TransformAABB(transform);
		return result;
	}


	VoxelRenderer::VoxelRenderer()
	{
		m_CommandBuffer = PrimaryRenderCommandBuffer::Create(0, "VoxelRenderer");
		m_CommandBuffer->CreateTimestampQueries(GPUTimeQueries::Count());

		const uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;
		m_UniformBufferSet = UniformBufferSet::Create(framesInFlight);
		m_UniformBufferSet->Create(sizeof(UBVoxelScene), UBVoxelScene::Set, UBVoxelScene::Binding);
		uint32_t size = sizeof(UBVoxelScene);
		m_StorageBufferSet = StorageBufferSet::Create(framesInFlight);
		m_StorageBufferSet->Create(sizeof(SSBOVoxels), SSBOVoxels::Set, SSBOVoxels::Binding);
		m_StorageBufferSet->Create(sizeof(SSBOVoxelModels), SSBOVoxelModels::Set, SSBOVoxelModels::Binding);
		m_StorageBufferSet->Create(sizeof(SSBOColors), SSBOColors::Set, SSBOColors::Binding);
		m_StorageBufferSet->Create(sizeof(SSBOVoxelTopGrids), SSBOVoxelTopGrids::Set, SSBOVoxelTopGrids::Binding);
		m_StorageBufferSet->Create(SSBOVoxelComputeData::MaxSize, SSBOVoxelComputeData::Set, SSBOVoxelComputeData::Binding);

		m_VoxelStorageAllocator = Ref<StorageBufferAllocator>::Create(sizeof(SSBOVoxels), SSBOVoxels::Binding, SSBOVoxels::Set);
		m_ColorStorageAllocator = Ref<StorageBufferAllocator>::Create(sizeof(SSBOColors), SSBOColors::Binding, SSBOColors::Set);
		m_TopGridsAllocator = Ref<StorageBufferAllocator>::Create(sizeof(SSBOVoxelTopGrids), SSBOVoxelTopGrids::Binding, SSBOVoxelTopGrids::Set);
		m_ComputeStorageAllocator = Ref<StorageBufferAllocator>::Create(SSBOVoxelComputeData::MaxSize, SSBOVoxelComputeData::Binding, SSBOVoxelComputeData::Set);
		
		memset(m_SSBOTopGrids.TopGridCells, 0, sizeof(SSBOVoxelTopGrids));

		TextureProperties props;
		props.Storage = true;
		props.SamplerWrap = TextureWrap::Clamp;
		m_OutputTexture = Texture2D::Create(ImageFormat::RGBA16F, 1280, 720, nullptr, props);
		m_DepthTexture = Texture2D::Create(ImageFormat::RED32F, 1280, 720, nullptr, props);
		m_SSGITexture = Texture2D::Create(ImageFormat::RGBA16F, 1280, 720, nullptr, props);
		createDefaultPipelines();


		m_UBVoxelScene.DirectionalLight.Direction = { -0.2f, -1.4f, -1.5f };
		m_UBVoxelScene.DirectionalLight.Radiance = glm::vec3(1.0f);
		m_UBVoxelScene.DirectionalLight.Multiplier = 1.0f;
		m_WorkGroups = { 32, 32 };
	}
	void VoxelRenderer::BeginScene(const VoxelRendererCamera& camera)
	{
		m_UBVoxelScene.InverseProjection = glm::inverse(camera.Projection);
		m_UBVoxelScene.InverseView = glm::inverse(camera.ViewMatrix);
		m_UBVoxelScene.CameraPosition = glm::vec4(camera.CameraPosition, 1.0f);
		m_UBVoxelScene.ViewportSize.x = m_ViewportSize.x;
		m_UBVoxelScene.ViewportSize.y = m_ViewportSize.y;
		m_SSBOVoxelModels.NumModels = 0;
		
		m_DrawCommands.clear();
		m_EffectCommands.clear();

		m_Frustum = camera.Frustum;
		m_Statistics = {};

		updateViewportSize();
		updateUniformBufferSet();

	}
	void VoxelRenderer::EndScene()
	{
		prepareDrawCommands();

		m_CommandBuffer->Begin();
		m_GPUTimeQueries.GPUTime = m_CommandBuffer->BeginTimestampQuery();

		waterPass();

		effectPass();
		clearPass();
		renderPass();
		if (m_UseSSGI)
			ssgiPass();

		m_CommandBuffer->EndTimestampQuery(m_GPUTimeQueries.GPUTime);

		m_CommandBuffer->End();
		m_CommandBuffer->Submit();

		m_LastFrameMeshAllocations = std::move(m_MeshAllocations);
	}
	void VoxelRenderer::SetViewportSize(uint32_t width, uint32_t height)
	{
		if (m_ViewportSize.x != width || m_ViewportSize.y != height)
		{
			m_ViewportSize = glm::ivec2(width, height);
			m_ViewportSizeChanged = true;
		}
	}

	void VoxelRenderer::SubmitMesh(const Ref<VoxelMesh>& mesh, const glm::mat4& transform)
	{
		auto& drawCommand = m_DrawCommands[mesh->GetHandle()];
		const auto& submeshes = mesh->GetSubmeshes();
		drawCommand.Mesh = mesh;
		for (const auto& instance : mesh->GetInstances())
		{
			const glm::mat4 instanceTransform = transform * instance.Transform;
			const VoxelSubmesh& submesh = submeshes[instance.SubmeshIndex];

			if (cullSubmesh(submesh, instanceTransform))
			{
				m_Statistics.CulledModels++;
				continue;
			}

			submitSubmesh(submesh, drawCommand, instanceTransform, instance.SubmeshIndex);
		}	
	}

	void VoxelRenderer::SubmitMesh(const Ref<VoxelMesh>& mesh, const glm::mat4& transform, const uint32_t* keyFrames)
	{
		auto& drawCommand = m_DrawCommands[mesh->GetHandle()];
		const auto& submeshes = mesh->GetSubmeshes();
		drawCommand.Mesh = mesh;
		uint32_t index = 0;
		for (const auto& instance : mesh->GetInstances())
		{
			const uint32_t submeshIndex = instance.ModelAnimation.SubmeshIndices[keyFrames[index]];
			const VoxelSubmesh& submesh = submeshes[submeshIndex];
			const glm::mat4 instanceTransform = transform * instance.Transform;

			if (cullSubmesh(submesh, instanceTransform))
			{
				m_Statistics.CulledModels++;
				continue;
			}

			submitSubmesh(submesh, drawCommand, instanceTransform, submeshIndex);
			index++;
		}	
	}

	void VoxelRenderer::SubmitMesh(const Ref<VoxelMesh>& mesh, const glm::mat4& transform,bool cull)
	{
		auto& drawCommand = m_DrawCommands[mesh->GetHandle()];
		const auto& submeshes = mesh->GetSubmeshes();
		drawCommand.Mesh = mesh;
		for (const auto& instance : mesh->GetInstances())
		{
			const glm::mat4 instanceTransform = transform * instance.Transform;
			const VoxelSubmesh& submesh = submeshes[instance.SubmeshIndex];
	
			if (cull && cullSubmesh(submesh, instanceTransform))
			{
				m_Statistics.CulledModels++;
				continue;
			}
			submitSubmesh(submesh, drawCommand, instanceTransform, instance.SubmeshIndex);
		}
	}

	Ref<Image2D> VoxelRenderer::GetFinalPassImage() const
	{
		if (m_UseSSGI)
			return m_SSGITexture->GetImage();
		return m_OutputTexture->GetImage();
	}

	bool VoxelRenderer::CreateComputeAllocation(uint32_t size, StorageBufferAllocation& allocation)
	{
		return m_ComputeStorageAllocator->Allocate(size, allocation);
	}

	void VoxelRenderer::SubmitComputeData(const void* data, uint32_t size, uint32_t offset, const StorageBufferAllocation& allocation, bool allFrames)
	{
		XYZ_ASSERT(offset + size <= allocation.GetSize(), "");
		if (allFrames)
			m_StorageBufferSet->UpdateEachFrame(data, size, allocation.GetOffset() + offset, allocation.GetBinding(), allocation.GetSet());
		else
			m_StorageBufferSet->Update(data, size, allocation.GetOffset() + offset, allocation.GetBinding(), allocation.GetSet());
	}
	
	void VoxelRenderer::SubmitEffect(const Ref<MaterialAsset>& material, const glm::ivec3& workGroups, const PushConstBuffer& constants)
	{
		auto& effectCommand = m_EffectCommands[material->GetHandle()];
		effectCommand.Material = material;
		auto& invocation = effectCommand.Invocations.emplace_back();
		invocation.WorkGroups = workGroups;
		invocation.Constants = constants;
	}

	void VoxelRenderer::OnImGuiRender()
	{
		if (ImGui::Begin("Voxel Renderer"))
		{
			ImGui::DragFloat3("Light Direction", glm::value_ptr(m_UBVoxelScene.DirectionalLight.Direction), 0.1f);
			ImGui::DragFloat3("Light Color", glm::value_ptr(m_UBVoxelScene.DirectionalLight.Radiance), 0.1f);
			ImGui::DragFloat("Light Multiplier", &m_UBVoxelScene.DirectionalLight.Multiplier, 0.1f);
			
			ImGui::NewLine();
			ImGui::DragInt("SSGI SampleCount", (int*) &m_SSGIValues.SampleCount, 1, 0, 20);
			ImGui::DragFloat("SSGI IndirectAmount", &m_SSGIValues.IndirectAmount);
			ImGui::DragFloat("SSGI NoiseAmount", &m_SSGIValues.NoiseAmount);
			ImGui::Checkbox("SSGI Noise", (bool*)&m_SSGIValues.Noise);
			ImGui::Checkbox("SSGI", &m_UseSSGI);
			ImGui::NewLine();

			ImGui::DragInt("Water Frames Delay", (int*)&m_WaterFramesDelay);
			ImGui::Checkbox("Water", &m_Water);
			ImGui::Checkbox("Top Grid", &m_UseTopGrid);
			ImGui::NewLine();

			if (ImGui::BeginTable("##Statistics", 2, ImGuiTableFlags_SizingFixedFit))
			{
				const uint32_t voxelBufferUsage = 100 * m_VoxelStorageAllocator->GetAllocatedSize() / m_VoxelStorageAllocator->GetSize();
				const uint32_t colorBufferUsage = 100 * m_ColorStorageAllocator->GetAllocatedSize() / m_ColorStorageAllocator->GetSize();

				UI::TextTableRow("%s", "Mesh Allocations:", "%u", static_cast<uint32_t>(m_LastFrameMeshAllocations.size()));
				UI::TextTableRow("%s", "Update Allocations:", "%u", static_cast<uint32_t>(m_UpdatedAllocations.size()));
				UI::TextTableRow("%s", "Model Count:", "%u", m_Statistics.ModelCount);
				UI::TextTableRow("%s", "Culled Models:", "%u", m_Statistics.CulledModels);
				UI::TextTableRow("%s", "Voxel Buffer Usage:", "%u%%", voxelBufferUsage);
				UI::TextTableRow("%s", "Color Buffer Usage:", "%u%%", colorBufferUsage);

				const uint32_t frameIndex = Renderer::GetCurrentFrame();
				float gpuTime = m_CommandBuffer->GetExecutionGPUTime(frameIndex, m_GPUTimeQueries.GPUTime);

				UI::TextTableRow("%s", "GPU Time:", "%.3fms", gpuTime);
				
				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	
	void VoxelRenderer::submitSubmesh(const VoxelSubmesh& submesh, VoxelDrawCommand& drawCommand, const glm::mat4& transform, uint32_t submeshIndex)
	{
		const AABB aabb = VoxelModelToAABB(transform, submesh.Width, submesh.Height, submesh.Depth, submesh.VoxelSize);

		m_Statistics.ModelCount++;

		const glm::vec3 aabbMax = glm::vec3(submesh.Width, submesh.Height, submesh.Depth) * submesh.VoxelSize;

		glm::vec3 centerTranslation = -glm::vec3(
			submesh.Width / 2  * submesh.VoxelSize,
			submesh.Height / 2 * submesh.VoxelSize,
			submesh.Depth / 2  * submesh.VoxelSize
		);
		const uint32_t voxelCount = static_cast<uint32_t>(submesh.ColorIndices.size());

		VoxelCommandModel& cmdModel = drawCommand.Models.emplace_back();
		cmdModel.SubmeshIndex = submeshIndex;
		cmdModel.ModelIndex = m_SSBOVoxelModels.NumModels;

		VoxelModel& model = m_SSBOVoxelModels.Models[m_SSBOVoxelModels.NumModels];
		model.InverseTransform = glm::inverse(transform * glm::translate(glm::mat4(1.0f), centerTranslation));

		model.MaxTraverses = submesh.MaxTraverses;
		
		model.Width = submesh.Width;
		model.Height = submesh.Height;
		model.Depth = submesh.Depth;
		model.VoxelSize = submesh.VoxelSize;

		m_SSBOVoxelModels.NumModels++;
	}

	void VoxelRenderer::clearPass()
	{
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

		
		Renderer::BeginPipelineCompute(
			m_CommandBuffer,
			m_ClearPipeline,
			m_UniformBufferSet,
			m_StorageBufferSet,
			m_ClearMaterial
		);
		Renderer::DispatchCompute(
			m_ClearPipeline,
			nullptr,
			m_WorkGroups.x, m_WorkGroups.y, 1,
			PushConstBuffer
			{
				glm::vec4(0.3, 0.2, 0.7, 1.0),
				std::numeric_limits<float>::max()
			}
		);
		imageBarrier(m_ClearPipeline, m_OutputTexture->GetImage());
		imageBarrier(m_ClearPipeline, m_DepthTexture->GetImage());
		Renderer::EndPipelineCompute(m_ClearPipeline);
	}

	void VoxelRenderer::effectPass()
	{
		auto ssboBarrier = [](Ref<VulkanPipelineCompute> pipeline, Ref<VulkanStorageBufferSet> storageBufferSet) {

			Renderer::Submit([pipeline, storageBufferSet]() {
				uint32_t frameIndex = Renderer::GetCurrentFrame();
				auto storageBuffer = storageBufferSet->Get(SSBOVoxels::Binding, SSBOVoxels::Set, frameIndex).As<VulkanStorageBuffer>();
				VkBufferMemoryBarrier bufferBarrier = {};
				bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
				bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Access mask for the source stage
				bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;  // Access mask for the destination stage
				bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferBarrier.buffer = storageBuffer->GetVulkanBuffer();  // The SSBO buffer
				bufferBarrier.offset = 0;                // Offset in the buffer
				bufferBarrier.size = VK_WHOLE_SIZE;      // Size of the buffer
				vkCmdPipelineBarrier(
					pipeline->GetActiveCommandBuffer(),                   // The command buffer
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,  // Source pipeline stage
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,  // Destination pipeline stage
					0,                               // Dependency flags
					0, nullptr,                      // Memory barriers (global memory barriers)
					1, &bufferBarrier,               // Buffer memory barriers
					0, nullptr                       // Image memory barriers
				);
				});
		};


		for (auto& [key, effect] : m_EffectCommands)
		{
			Ref<PipelineCompute> pipeline = getEffectPipeline(effect.Material);
			Renderer::BeginPipelineCompute(
				m_CommandBuffer,
				pipeline,
				m_UniformBufferSet,
				m_StorageBufferSet,
				effect.Material->GetMaterial()
			);

			for (auto& invoc : effect.Invocations)
			{
				Renderer::DispatchCompute(
					pipeline,
					nullptr,
					invoc.WorkGroups.x, invoc.WorkGroups.y, invoc.WorkGroups.z,
					invoc.Constants
				);
				ssboBarrier(pipeline, m_StorageBufferSet);
			}

			Renderer::EndPipelineCompute(pipeline);
		}
	}



	void VoxelRenderer::renderPass()
	{
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


		Renderer::BeginPipelineCompute(
			m_CommandBuffer,
			m_RaymarchPipeline,
			m_UniformBufferSet,
			m_StorageBufferSet,
			m_RaymarchMaterial
		);

		Bool32 useTopGrid = m_UseTopGrid;
		Renderer::DispatchCompute(
			m_RaymarchPipeline,
			nullptr,
			m_WorkGroups.x, m_WorkGroups.y, 1,
			PushConstBuffer{ useTopGrid }
		);


		imageBarrier(m_RaymarchPipeline, m_OutputTexture->GetImage());
		imageBarrier(m_RaymarchPipeline, m_DepthTexture->GetImage());

		Renderer::EndPipelineCompute(m_RaymarchPipeline);		
	}
	void VoxelRenderer::ssgiPass()
	{
		Renderer::BeginPipelineCompute(
			m_CommandBuffer,
			m_SSGIPipeline,
			m_UniformBufferSet,
			nullptr,
			m_SSGIMaterial
		);
		
		Renderer::DispatchCompute(
			m_SSGIPipeline,
			nullptr,
			m_WorkGroups.x, m_WorkGroups.y, 1,
			PushConstBuffer
			{
				m_SSGIValues
			}
		);
		
		Renderer::EndPipelineCompute(m_SSGIPipeline);
	}


	static uint32_t reqDispatchCounter = 0;
	static int randSeed = 0;

	void VoxelRenderer::waterPass()
	{
		auto ssboBarrier = [](Ref<VulkanPipelineCompute> pipeline, Ref<VulkanStorageBufferSet> storageBufferSet) {

			Renderer::Submit([pipeline, storageBufferSet]() {
				uint32_t frameIndex = Renderer::GetCurrentFrame();
				auto storageBuffer = storageBufferSet->Get(SSBOVoxels::Binding, SSBOVoxels::Set, frameIndex).As<VulkanStorageBuffer>();
				VkBufferMemoryBarrier bufferBarrier = {};
				bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
				bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Access mask for the source stage
				bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;  // Access mask for the destination stage
				bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferBarrier.buffer = storageBuffer->GetVulkanBuffer();  // The SSBO buffer
				bufferBarrier.offset = 0;                // Offset in the buffer
				bufferBarrier.size = VK_WHOLE_SIZE;      // Size of the buffer
				vkCmdPipelineBarrier(
					pipeline->GetActiveCommandBuffer(),                   // The command buffer
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,  // Source pipeline stage
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,  // Destination pipeline stage
					0,                               // Dependency flags
					0, nullptr,                      // Memory barriers (global memory barriers)
					1, &bufferBarrier,               // Buffer memory barriers
					0, nullptr                       // Image memory barriers
				);
			});
		};

		
		bool shouldwater = m_WaterFramesCounter >= m_WaterFramesDelay;
		if (shouldwater && reqDispatchCounter == 0 && m_Water)
		{
			reqDispatchCounter = 3;
			m_WaterFramesCounter = 0;
			randSeed = RandomNumber(0u, 1u);
		}

		if (reqDispatchCounter > 0)
		{
			Renderer::BeginPipelineCompute(
				m_CommandBuffer,
				m_WaterPipeline,
				m_UniformBufferSet,
				m_StorageBufferSet,
				m_WaterMaterial
			);
			const glm::ivec3 workGroups = {
				std::ceil(512.0f / 3.0f / TILE_SIZE), 
				std::ceil(512.0f / 3.0f / TILE_SIZE),
				400 / 2 / 4 // submesh height / 2 / local_size_z
			};
			for (uint32_t j = 0; j < 2; ++j)
			{
				for (uint32_t i = 0; i < 9; ++i)
				{
					Renderer::DispatchCompute(
						m_WaterPipeline,
						nullptr,
						workGroups.x, workGroups.y, workGroups.z,
						PushConstBuffer
						{
							randSeed, 0u, 1u, i, j
						}
					);
					ssboBarrier(m_WaterPipeline, m_StorageBufferSet);
				}
			}
			ssboBarrier(m_WaterPipeline, m_StorageBufferSet);
			Renderer::EndPipelineCompute(m_WaterPipeline);
			reqDispatchCounter--;
		}
		m_WaterFramesCounter++;
	}
	void VoxelRenderer::updateViewportSize()
	{
		if (m_ViewportSizeChanged)
		{
			m_ViewportSizeChanged = false;
			TextureProperties props;
			props.Storage = true;
			props.SamplerWrap = TextureWrap::Clamp;
			m_OutputTexture = Texture2D::Create(ImageFormat::RGBA16F, m_ViewportSize.x, m_ViewportSize.y, nullptr, props);
			m_DepthTexture = Texture2D::Create(ImageFormat::RED32F, m_ViewportSize.x, m_ViewportSize.y, nullptr, props);
			m_SSGITexture = Texture2D::Create(ImageFormat::RGBA16F, m_ViewportSize.x, m_ViewportSize.y, nullptr, props);
			
			m_RaymarchMaterial->SetImage("o_Image", m_OutputTexture->GetImage());
			m_RaymarchMaterial->SetImage("o_DepthImage", m_DepthTexture->GetImage());

			m_ClearMaterial->SetImage("o_Image", m_OutputTexture->GetImage());
			m_ClearMaterial->SetImage("o_DepthImage", m_DepthTexture->GetImage());
		
			m_SSGIMaterial->SetImage("u_Image", m_OutputTexture->GetImage());
			m_SSGIMaterial->SetImage("u_DepthImage", m_DepthTexture->GetImage());
			m_SSGIMaterial->SetImage("o_SSGIImage", m_SSGITexture->GetImage());

			m_UBVoxelScene.ViewportSize.x = m_ViewportSize.x;
			m_UBVoxelScene.ViewportSize.y = m_ViewportSize.y;

			m_WorkGroups = {
				(m_ViewportSize.x + m_ViewportSize.x % TILE_SIZE) / TILE_SIZE,
				(m_ViewportSize.y + m_ViewportSize.y % TILE_SIZE) / TILE_SIZE
			};
		}
	}
	void VoxelRenderer::updateUniformBufferSet()
	{
		Ref<VoxelRenderer> instance = this;
		Renderer::Submit([instance]() mutable {

			const uint32_t currentFrame = Renderer::GetCurrentFrame();
			instance->m_UniformBufferSet->Get(UBVoxelScene::Binding, UBVoxelScene::Set, currentFrame)->RT_Update(&instance->m_UBVoxelScene, sizeof(UBVoxelScene), 0);

		});
	}
	
	bool VoxelRenderer::cullSubmesh(const VoxelSubmesh& submesh, const glm::mat4& transform) const
	{
		const AABB aabb = VoxelModelToAABB(transform, submesh.Width, submesh.Height, submesh.Depth, submesh.VoxelSize);
		return !aabb.InsideFrustum(m_Frustum);

	}
	void VoxelRenderer::prepareDrawCommands()
	{
		const uint32_t colorSize = static_cast<uint32_t>(sizeof(SSBOColors::ColorPallete[0]));
		Ref<VoxelRenderer> instance = this;

		uint32_t topGridCount = 0;
		for (auto& [key, drawCommand] : m_DrawCommands)
		{
			if (drawCommand.Models.empty())
				continue;

			MeshAllocation& meshAlloc = createMeshAllocation(drawCommand.Mesh);
			
			for (const auto& cmdModel : drawCommand.Models)
			{
				VoxelModel& model = m_SSBOVoxelModels.Models[cmdModel.ModelIndex];
				model.ColorIndex = meshAlloc.ColorAllocation.GetOffset() / colorSize;
				model.VoxelOffset = meshAlloc.SubmeshOffsets[cmdModel.SubmeshIndex];
				model.TopGridIndex = -1;
			}

			// Store mesh top grid
			if (drawCommand.Mesh->HasTopGrid())
			{
				auto& topGrid = drawCommand.Mesh->GetTopGrid();
				m_SSBOVoxelModels.TopGrids[topGridCount].MaxTraverses = topGrid.MaxTraverses;
				m_SSBOVoxelModels.TopGrids[topGridCount].CellsOffset = meshAlloc.TopGridAllocation.GetOffset();
				m_SSBOVoxelModels.TopGrids[topGridCount].Width = topGrid.Width;
				m_SSBOVoxelModels.TopGrids[topGridCount].Height = topGrid.Height;
				m_SSBOVoxelModels.TopGrids[topGridCount].Depth = topGrid.Depth;
				m_SSBOVoxelModels.TopGrids[topGridCount].Size = topGrid.Size;
				for (const auto& cmdModel : drawCommand.Models)
				{
					VoxelModel& model = m_SSBOVoxelModels.Models[cmdModel.ModelIndex];
					model.TopGridIndex = topGridCount;
				}
				topGridCount++;
			}				
		}

		for (const auto& updated : m_UpdatedAllocations)
		{
			void* voxelData = &m_SSBOVoxels.Voxels[updated.VoxelAllocation.GetOffset()];
			void* colorData = &m_SSBOColors.ColorPallete[updated.ColorAllocation.GetOffset() / colorSize];
			void* topGridData = &m_SSBOTopGrids.TopGridCells[updated.TopGridAllocation.GetOffset()];

			m_StorageBufferSet->UpdateEachFrame(voxelData, updated.VoxelAllocation.GetSize(), updated.VoxelAllocation.GetOffset(), SSBOVoxels::Binding, SSBOVoxels::Set);
			m_StorageBufferSet->UpdateEachFrame(colorData, updated.ColorAllocation.GetSize(), updated.ColorAllocation.GetOffset(), SSBOColors::Binding, SSBOColors::Set);
			m_StorageBufferSet->UpdateEachFrame(topGridData, updated.TopGridAllocation.GetSize(), updated.TopGridAllocation.GetOffset(), SSBOVoxelTopGrids::Binding, SSBOVoxelTopGrids::Set);
		}
		for (const auto& updated : m_UpdatedSuballocations)
		{
			void* voxelData = &m_SSBOVoxels.Voxels[updated.Offset];
			m_StorageBufferSet->UpdateEachFrame(voxelData, updated.Size, updated.Offset, SSBOVoxels::Binding, SSBOVoxels::Set);
		}
		m_UpdatedAllocations.clear();
		m_UpdatedSuballocations.clear();

		const uint32_t voxelModelsUpdateSize =
				sizeof(SSBOVoxelModels::NumModels)
			+	sizeof(SSBOVoxelModels::Padding)
			+	m_SSBOVoxelModels.NumModels * sizeof(VoxelModel);


		const uint32_t topGridDataOffset =
				sizeof(SSBOVoxelModels::NumModels)
			+	sizeof(SSBOVoxelModels::Padding)
			+	SSBOVoxelModels::MaxModels * sizeof(VoxelModel);


		const uint32_t topGridUpdateSize = topGridCount * sizeof(VoxelTopGrid);

		void* topGridData = (uint8_t*)&m_SSBOVoxelModels + topGridDataOffset;

		m_StorageBufferSet->Update((void*)&m_SSBOVoxelModels, voxelModelsUpdateSize, 0, SSBOVoxelModels::Binding, SSBOVoxelModels::Set);	
		m_StorageBufferSet->Update(topGridData, topGridUpdateSize, topGridDataOffset, SSBOVoxelModels::Binding, SSBOVoxelModels::Set);
	}
	void VoxelRenderer::createDefaultPipelines()
	{
		Ref<Shader> shader = Shader::Create("Resources/Shaders/Voxel/RaymarchShader.glsl");
		m_RaymarchMaterial = Material::Create(shader);

		m_RaymarchMaterial->SetImage("o_Image", m_OutputTexture->GetImage());
		m_RaymarchMaterial->SetImage("o_DepthImage", m_DepthTexture->GetImage());
		PipelineComputeSpecification spec;
		spec.Shader = shader;
	
		m_RaymarchPipeline = PipelineCompute::Create(spec);


		Ref<Shader> clearShader = Shader::Create("Resources/Shaders/Voxel/ImageClearShader.glsl");
		m_ClearMaterial = Material::Create(clearShader);
		m_ClearMaterial->SetImage("o_Image", m_OutputTexture->GetImage());
		m_ClearMaterial->SetImage("o_DepthImage", m_DepthTexture->GetImage());

		spec.Shader = clearShader;
		m_ClearPipeline = PipelineCompute::Create(spec);


		Ref<Shader> ssgiShader = Shader::Create("Resources/Shaders/Voxel/SSGI.glsl");
		m_SSGIMaterial = Material::Create(ssgiShader);
		m_SSGIMaterial->SetImage("u_Image", m_OutputTexture->GetImage());
		m_SSGIMaterial->SetImage("u_DepthImage", m_DepthTexture->GetImage());
		m_SSGIMaterial->SetImage("o_SSGIImage", m_SSGITexture->GetImage());

		spec.Shader = ssgiShader;
		m_SSGIPipeline = PipelineCompute::Create(spec);


		Ref<Shader> waterShader = Shader::Create("Resources/Shaders/Voxel/Water.glsl");
		m_WaterMaterial = Material::Create(waterShader);
		spec.Shader = waterShader;
		m_WaterPipeline = PipelineCompute::Create(spec);
	}

	Ref<PipelineCompute> VoxelRenderer::getEffectPipeline(const Ref<MaterialAsset>& material)
	{
		AssetHandle handle = material->GetHandle();
		auto it = m_EffectPipelines.find(handle);
		if (it != m_EffectPipelines.end())
			return it->second;

		PipelineComputeSpecification spec;
		spec.Shader = material->GetShader();
		spec.Specialization = material->GetSpecialization();

		Ref<PipelineCompute> result = PipelineCompute::Create(spec);
		m_EffectPipelines[handle] = result;

		return result;
	}

	VoxelRenderer::MeshAllocation& VoxelRenderer::createMeshAllocation(const Ref<VoxelMesh>& mesh)
	{
		const AssetHandle& meshHandle = mesh->GetHandle();
		const auto& submeshes = mesh->GetSubmeshes();
		const uint32_t meshSize = mesh->GetNumVoxels() * sizeof(uint8_t);

		// Check if we have cached allocation from previous frame
		auto lastFrame = m_LastFrameMeshAllocations.find(meshHandle);
		if (lastFrame != m_LastFrameMeshAllocations.end())
		{
			// Reuse allocation from previous frame
			m_MeshAllocations[meshHandle] = std::move(lastFrame->second);
			m_LastFrameMeshAllocations.erase(lastFrame);
		}

		MeshAllocation& meshAlloc = m_MeshAllocations[meshHandle];
		reallocateVoxels(mesh, meshAlloc);
		return meshAlloc;
	}
	void VoxelRenderer::reallocateVoxels(const Ref<VoxelMesh>& mesh, MeshAllocation& allocation)
	{
		const auto& submeshes = mesh->GetSubmeshes();

		const uint32_t meshSize = mesh->GetNumVoxels() * sizeof(uint8_t);
		const uint32_t colorSize = static_cast<uint32_t>(sizeof(SSBOColors::ColorPallete[0]));
		const uint32_t topGridsSize = mesh->GetTopGrid().VoxelCount.size();

		const bool reallocated =
			m_VoxelStorageAllocator->Allocate(meshSize, allocation.VoxelAllocation)
			|| m_ColorStorageAllocator->Allocate(colorSize, allocation.ColorAllocation)
			|| m_TopGridsAllocator->Allocate(topGridsSize, allocation.TopGridAllocation);


		if (reallocated || mesh->NeedUpdate())
		{
			allocation.SubmeshOffsets.resize(submeshes.size());

			// Copy voxels
			uint32_t submeshIndex = 0;
			uint32_t offset = allocation.VoxelAllocation.GetOffset();
			for (auto& submesh : submeshes)
			{
				allocation.SubmeshOffsets[submeshIndex] = offset;
				const uint32_t voxelCount = static_cast<uint32_t>(submesh.ColorIndices.size());
				memcpy(&m_SSBOVoxels.Voxels[offset], submesh.ColorIndices.data(), voxelCount * sizeof(uint8_t));
				offset += voxelCount;
				submeshIndex++;
			}

			// Copy color pallete
			const uint32_t colorPalleteIndex = allocation.ColorAllocation.GetOffset() / colorSize;
			memcpy(m_SSBOColors.ColorPallete[colorPalleteIndex], mesh->GetColorPallete().data(), colorSize);

			// Copy top grid
			uint32_t cellOffset = allocation.TopGridAllocation.GetOffset();
			for (auto voxelCount : mesh->GetTopGrid().VoxelCount)
				m_SSBOTopGrids.TopGridCells[cellOffset++] = voxelCount == 0 ? 0 : 1;

			m_UpdatedAllocations.push_back({
				allocation.VoxelAllocation,
				allocation.TopGridAllocation,
				allocation.ColorAllocation
				});
		}
		else
		{
			auto ranges = mesh->DirtySubmeshes();
			for (auto& [submeshIndex, range] : ranges)
			{
				const uint32_t offset = allocation.SubmeshOffsets[submeshIndex] + range.Start;
				const uint32_t voxelCount = range.End - range.Start;
				const VoxelSubmesh& submesh = mesh->GetSubmeshes()[submeshIndex];
				const uint8_t* updateVoxelData = &submesh.ColorIndices.data()[range.Start];

				memcpy(&m_SSBOVoxels.Voxels[offset], updateVoxelData, voxelCount * sizeof(uint8_t));
				m_UpdatedSuballocations.push_back({ offset, voxelCount });
			}
		}
	}
}
