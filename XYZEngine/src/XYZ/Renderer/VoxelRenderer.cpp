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
		result.Min = glm::vec3(0.0f);
		result.Max = glm::vec3(width, height, depth) * voxelSize;

		result = result.TransformAABB(transform);
		return result;
	}


	VoxelRenderer::VoxelRenderer()
		:
		m_ModelsOctree(AABB(glm::vec3(-1024), glm::vec3(1024)), 4)
	{
		m_CommandBuffer = PrimaryRenderCommandBuffer::Create(0, "VoxelRenderer");
		m_CommandBuffer->CreateTimestampQueries(GPUTimeQueries::Count());

		const uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;
		m_UniformBufferSet = UniformBufferSet::Create(framesInFlight);
		m_UniformBufferSet->Create(sizeof(UBVoxelScene), UBVoxelScene::Set, UBVoxelScene::Binding);
		uint32_t size = sizeof(UBVoxelScene);
		m_StorageBufferSet = StorageBufferSet::Create(framesInFlight);
		m_StorageBufferSet->Create(SSBOVoxels::MaxVoxels, SSBOVoxels::Set, SSBOVoxels::Binding, false, true);
		m_StorageBufferSet->Create(sizeof(SSBOVoxelModels), SSBOVoxelModels::Set, SSBOVoxelModels::Binding);
		m_StorageBufferSet->Create(SSBOColors::MaxSize, SSBOColors::Set, SSBOColors::Binding, false, true);
		m_StorageBufferSet->Create(sizeof(SSBOVoxelAccelerationGrids), SSBOVoxelAccelerationGrids::Set, SSBOVoxelAccelerationGrids::Binding, false, true);
		m_StorageBufferSet->Create(SSBOVoxelComputeData::MaxSize, SSBOVoxelComputeData::Set, SSBOVoxelComputeData::Binding);
		m_StorageBufferSet->Create(sizeof(SSBOOCtree), SSBOOCtree::Set, SSBOOCtree::Binding);

		m_VoxelStorageAllocator = Ref<StorageBufferAllocator>::Create(SSBOVoxels::MaxVoxels, SSBOVoxels::Binding, SSBOVoxels::Set);
		m_ColorStorageAllocator = Ref<StorageBufferAllocator>::Create(SSBOColors::MaxSize, SSBOColors::Binding, SSBOColors::Set);
		m_AccelerationGridsAllocator = Ref<StorageBufferAllocator>::Create(sizeof(SSBOVoxelAccelerationGrids), SSBOVoxelAccelerationGrids::Binding, SSBOVoxelAccelerationGrids::Set);
		m_ComputeStorageAllocator = Ref<StorageBufferAllocator>::Create(SSBOVoxelComputeData::MaxSize, SSBOVoxelComputeData::Binding, SSBOVoxelComputeData::Set);
		
		memset(m_SSBOAccelerationGrids.AccelerationGridCells, 0, sizeof(SSBOVoxelAccelerationGrids));

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
		
		m_RenderModelsSorted.clear();
		m_RenderModels.clear();
		m_VoxelMeshBuckets.clear();
		m_EffectCommands.clear();

		m_Statistics = {};

		updateViewportSize();
		updateUniformBufferSet();
		m_ModelsOctree.Clear();

	}
	void VoxelRenderer::EndScene()
	{
		prepareModels();

		m_CommandBuffer->Begin();
		m_GPUTimeQueries.GPUTime = m_CommandBuffer->BeginTimestampQuery();

	
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
		const auto& submeshes = mesh->GetSubmeshes();
		for (const auto& instance : mesh->GetInstances())
		{
			const glm::mat4 instanceTransform = transform * instance.Transform;
			const VoxelSubmesh& submesh = submeshes[instance.SubmeshIndex];

		
			submitSubmesh(mesh, submesh, instanceTransform, instance.SubmeshIndex);
		}	
	}

	void VoxelRenderer::SubmitMesh(const Ref<VoxelMesh>& mesh, const glm::mat4& transform, const uint32_t* keyFrames)
	{
		const auto& submeshes = mesh->GetSubmeshes();
		uint32_t index = 0;
		for (const auto& instance : mesh->GetInstances())
		{
			const uint32_t submeshIndex = instance.ModelAnimation.SubmeshIndices[keyFrames[index]];
			const VoxelSubmesh& submesh = submeshes[submeshIndex];
			const glm::mat4 instanceTransform = transform * instance.Transform;

			submitSubmesh(mesh, submesh, instanceTransform, submeshIndex);
			index++;
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
			if (ImGui::Button("Reload Shader"))
			{
				Ref<Shader> shader = Shader::Create("Resources/Shaders/Voxel/RaymarchShader.glsl");
				m_RaymarchMaterial = Material::Create(shader);

				m_RaymarchMaterial->SetImage("o_Image", m_OutputTexture->GetImage());
				m_RaymarchMaterial->SetImage("o_DepthImage", m_DepthTexture->GetImage());
				PipelineComputeSpecification spec;
				spec.Shader = shader;

				m_RaymarchPipeline = PipelineCompute::Create(spec);
			}

			ImGui::DragFloat3("Light Direction", glm::value_ptr(m_UBVoxelScene.DirectionalLight.Direction), 0.1f);
			ImGui::DragFloat3("Light Color", glm::value_ptr(m_UBVoxelScene.DirectionalLight.Radiance), 0.1f);
			ImGui::DragFloat("Light Multiplier", &m_UBVoxelScene.DirectionalLight.Multiplier, 0.1f);
			ImGui::NewLine();

			ImGui::Checkbox("Top Grid", &m_UseAccelerationGrid);
			ImGui::Checkbox("Octree", &m_UseOctree);
			ImGui::Checkbox("Show Octree", &m_ShowOctree);
			ImGui::Checkbox("Show AABB", &m_ShowAABB);
			ImGui::NewLine();

			if (ImGui::BeginTable("##Statistics", 2, ImGuiTableFlags_SizingFixedFit))
			{
				const uint32_t voxelBufferUsage = 100.0f * static_cast<float>(m_VoxelStorageAllocator->GetAllocatedSize()) / m_VoxelStorageAllocator->GetSize();
				const uint32_t colorBufferUsage = 100.0f * static_cast<float>(m_ColorStorageAllocator->GetAllocatedSize()) / m_ColorStorageAllocator->GetSize();
				const uint32_t AccelerationGridBufferUsage = 100.0f * static_cast<float>(m_AccelerationGridsAllocator->GetAllocatedSize()) / m_AccelerationGridsAllocator->GetSize();

				UI::TextTableRow("%s", "Mesh Allocations:", "%u", static_cast<uint32_t>(m_LastFrameMeshAllocations.size()));
				UI::TextTableRow("%s", "Model Count:", "%u", m_Statistics.ModelCount);

				UI::TextTableRow("%s", "Voxel Buffer Usage:", "%u%%", voxelBufferUsage);
				UI::TextTableRow("%s", "Color Buffer Usage:", "%u%%", colorBufferUsage);
				UI::TextTableRow("%s", "Top Grid Buffer Usage:", "%u%%", AccelerationGridBufferUsage);


				const uint32_t frameIndex = Renderer::GetCurrentFrame();
				float gpuTime = m_CommandBuffer->GetExecutionGPUTime(frameIndex, m_GPUTimeQueries.GPUTime);

				UI::TextTableRow("%s", "GPU Time:", "%.3fms", gpuTime);
				
				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	
	void VoxelRenderer::updateVoxelModelsSSBO(uint32_t AccelerationGridCount)
	{
		// Update ssbos
		const uint32_t voxelModelsUpdateSize =
			sizeof(SSBOVoxelModels::NumModels)
			+ sizeof(SSBOVoxelModels::Padding)
			+ m_SSBOVoxelModels.NumModels * sizeof(VoxelModel);


		const uint32_t AccelerationGridDataOffset =
			sizeof(SSBOVoxelModels::NumModels)
			+ sizeof(SSBOVoxelModels::Padding)
			+ SSBOVoxelModels::MaxModels * sizeof(VoxelModel);


		const uint32_t AccelerationGridUpdateSize = AccelerationGridCount * sizeof(VoxelAccelerationGrid);
		void* AccelerationGridData = (uint8_t*)&m_SSBOVoxelModels + AccelerationGridDataOffset;

		m_StorageBufferSet->Update((void*)&m_SSBOVoxelModels, voxelModelsUpdateSize, 0, SSBOVoxelModels::Binding, SSBOVoxelModels::Set);
		m_StorageBufferSet->Update(AccelerationGridData, AccelerationGridUpdateSize, AccelerationGridDataOffset, SSBOVoxelModels::Binding, SSBOVoxelModels::Set);
	}

	void VoxelRenderer::updateOctreeSSBO()
	{
		// Update octree
		uint32_t nodeCount = 0;
		uint32_t dataCount = 0;
		const glm::vec3 cameraPosition(glm::vec3(m_UBVoxelScene.CameraPosition));
		for (auto& octreeNode : m_ModelsOctree.GetNodes())
		{
			auto& node = m_SSBOOctree.Nodes[nodeCount];
			memcpy(node.Children, octreeNode.Children, 8 * sizeof(uint32_t));
			node.IsLeaf = octreeNode.IsLeaf;
			node.Max = glm::vec4(octreeNode.BoundingBox.Max, 1.0f);
			node.Min = glm::vec4(octreeNode.BoundingBox.Min, 1.0f);
			node.DataStart = dataCount;
			for (const auto& data : octreeNode.Data)
			{
				m_SSBOOctree.ModelIndices[dataCount++] = data.Data;
			}
			node.DataEnd = dataCount;
			nodeCount++;
		}
		m_SSBOOctree.NodeCount = nodeCount;

		const uint32_t firstUpdateSize =
			sizeof(SSBOOCtree::NodeCount)
			+ sizeof(SSBOOCtree::Padding)
			+ sizeof(VoxelModelOctreeNode) * nodeCount;

		const uint32_t secondUpdateSize = sizeof(uint32_t) * dataCount;
		const uint32_t secondUpdateOffset =
			sizeof(SSBOOCtree::NodeCount)
			+ sizeof(SSBOOCtree::Padding)
			+ sizeof(SSBOOCtree::Nodes);

		m_StorageBufferSet->Update(&m_SSBOOctree, firstUpdateSize, 0, SSBOOCtree::Binding, SSBOOCtree::Set);
		void* secondUpdateData = &reinterpret_cast<uint8_t*>(&m_SSBOOctree)[secondUpdateOffset];
		m_StorageBufferSet->Update(secondUpdateData, secondUpdateSize, secondUpdateOffset, SSBOOCtree::Binding, SSBOOCtree::Set);
	}

	void VoxelRenderer::submitSubmesh(const Ref<VoxelMesh>& mesh, const VoxelSubmesh& submesh, const glm::mat4& transform, uint32_t submeshIndex)
	{		
		const uint32_t voxelCount = static_cast<uint32_t>(submesh.ColorIndices.size());

		auto& renderModel = m_RenderModels.emplace_back();
		renderModel.BoundingBox =  VoxelModelToAABB(transform, submesh.Width, submesh.Height, submesh.Depth, submesh.VoxelSize);
		renderModel.SubmeshIndex = submeshIndex;
		renderModel.Transform = transform;
		renderModel.Mesh = mesh;

		m_Statistics.ModelCount++;
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

		Bool32 useAccelerationGrid = m_UseAccelerationGrid;
		Bool32 useOctree = m_UseOctree;
		Bool32 showOctree = m_ShowOctree;
		Bool32 showAABB = m_ShowAABB;

		Renderer::DispatchCompute(
			m_RaymarchPipeline,
			nullptr,
			m_WorkGroups.x, m_WorkGroups.y, 1,
			PushConstBuffer{ useAccelerationGrid, useOctree, showOctree, showAABB }
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
			m_WorkGroups.x, m_WorkGroups.y, 1
		);
		
		Renderer::EndPipelineCompute(m_SSGIPipeline);
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
	


	void VoxelRenderer::prepareModels()
	{
		for (auto& renderModel : m_RenderModels)
			m_RenderModelsSorted.push_back(&renderModel);
	
		// Sort by distance from camera
		const glm::vec3 cameraPosition(m_UBVoxelScene.CameraPosition);
		std::sort(m_RenderModelsSorted.begin(), m_RenderModelsSorted.end(), [&](const VoxelRenderModel* a, const VoxelRenderModel* b) {
			return a->BoundingBox.Distance(cameraPosition) < b->BoundingBox.Distance(cameraPosition);
		});

		// Try insert into octree if pass frustum culling
		int32_t modelIndex = 0;
		for (auto model : m_RenderModelsSorted)
		{
			m_ModelsOctree.InsertData(model->BoundingBox, modelIndex);
			auto& allocation = m_VoxelMeshBuckets[model->Mesh->GetHandle()];
			model->ModelIndex = modelIndex;
			allocation.Mesh = model->Mesh;
			allocation.Models.push_back(model);
			modelIndex++;
		}

		// Pass it to ssbo data
		const uint32_t colorSize = SSBOColors::ColorPalleteSize;
		uint32_t AccelerationGridCount = 0;
		for (auto& [key, meshAllocation] : m_VoxelMeshBuckets)
		{
			if (meshAllocation.Models.empty())
				continue;

			MeshAllocation& meshAlloc = createMeshAllocation(meshAllocation.Mesh);

			for (const auto& cmdModel : meshAllocation.Models)
			{
				const VoxelSubmesh& submesh = cmdModel->Mesh->GetSubmeshes()[cmdModel->SubmeshIndex];
				VoxelModel& model = m_SSBOVoxelModels.Models[cmdModel->ModelIndex];
				model.ColorIndex = meshAlloc.ColorAllocation.GetOffset() / colorSize;
				model.VoxelOffset = meshAlloc.SubmeshOffsets[cmdModel->SubmeshIndex];
				model.InverseTransform = glm::inverse(cmdModel->Transform);
				model.Width = submesh.Width;
				model.Height = submesh.Height;
				model.Depth = submesh.Depth;
				model.VoxelSize = submesh.VoxelSize;
				model.AccelerationGridIndex = -1;
				m_SSBOVoxelModels.NumModels++;
			}

			// Store mesh top grid
			if (meshAllocation.Mesh->HasAccelerationGrid())
			{
				auto& AccelerationGrid = meshAllocation.Mesh->GetAccelerationGrid();
				m_SSBOVoxelModels.AccelerationGrids[AccelerationGridCount].CellsOffset = meshAlloc.AccelerationGridAllocation.GetOffset();
				m_SSBOVoxelModels.AccelerationGrids[AccelerationGridCount].Width = AccelerationGrid.Width;
				m_SSBOVoxelModels.AccelerationGrids[AccelerationGridCount].Height = AccelerationGrid.Height;
				m_SSBOVoxelModels.AccelerationGrids[AccelerationGridCount].Depth = AccelerationGrid.Depth;
				m_SSBOVoxelModels.AccelerationGrids[AccelerationGridCount].Size = AccelerationGrid.Size;
				for (const auto& cmdModel : meshAllocation.Models)
				{
					VoxelModel& model = m_SSBOVoxelModels.Models[cmdModel->ModelIndex];
					model.AccelerationGridIndex = AccelerationGridCount;
				}
				AccelerationGridCount++;
			}
		}
		updateVoxelModelsSSBO(AccelerationGridCount);
		updateOctreeSSBO();
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
		const uint32_t colorSize = SSBOColors::ColorPalleteSize;
		
		const bool reallocated =
			m_VoxelStorageAllocator->Allocate(meshSize, allocation.VoxelAllocation)
			|| m_ColorStorageAllocator->Allocate(colorSize, allocation.ColorAllocation);


		if (reallocated || mesh->NeedUpdate())
		{
			// It is safe to allocate top grid only here because of multithread generation
			if (mesh->HasAccelerationGrid())
			{
				auto& accelerationGrid = mesh->GetAccelerationGrid();
				const uint32_t accelerationGridsSize = accelerationGrid.Cells.size();
				m_AccelerationGridsAllocator->Allocate(accelerationGridsSize, allocation.AccelerationGridAllocation);
				uint32_t cellOffset = allocation.AccelerationGridAllocation.GetOffset();
				
				// Copy acceleration grid
				for (auto count : accelerationGrid.Cells)
					m_SSBOAccelerationGrids.AccelerationGridCells[cellOffset++] = count > 0 ? 1 : 0;
			}
			// Update acceleration grid SSBO
			void* accelerationGridData = &m_SSBOAccelerationGrids.AccelerationGridCells[allocation.AccelerationGridAllocation.GetOffset()];
			m_StorageBufferSet->Update(accelerationGridData, allocation.AccelerationGridAllocation.GetSize(), allocation.AccelerationGridAllocation.GetOffset(), SSBOVoxelAccelerationGrids::Binding, SSBOVoxelAccelerationGrids::Set);
			

			// Copy voxels
			uint32_t submeshIndex = 0;
			uint32_t offset = allocation.VoxelAllocation.GetOffset();
			allocation.SubmeshOffsets.resize(submeshes.size());
			for (auto& submesh : submeshes)
			{
				allocation.SubmeshOffsets[submeshIndex] = offset;
				const uint32_t voxelCount = static_cast<uint32_t>(submesh.ColorIndices.size());
				m_StorageBufferSet->Update(submesh.ColorIndices.data(), submesh.ColorIndices.size(), offset, SSBOVoxels::Binding, SSBOVoxels::Set);			
				offset += voxelCount;
				submeshIndex++;
			}
			// Update color pallete SSBO
			m_StorageBufferSet->Update(mesh->GetColorPallete().data(), allocation.ColorAllocation.GetSize(), allocation.ColorAllocation.GetOffset(), SSBOColors::Binding, SSBOColors::Set);					
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

				m_StorageBufferSet->Update(updateVoxelData, voxelCount, offset, SSBOVoxels::Binding, SSBOVoxels::Set);
			}
		}
	}
}
