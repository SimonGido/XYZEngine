#include "stdafx.h"
#include "VoxelRenderer.h"

#include "Renderer.h"

#include "XYZ/API/Vulkan/VulkanPipelineCompute.h"
#include "XYZ/Utils/Math/Math.h"
#include "XYZ/Utils/Math/AABB.h"

#include <glm/gtc/type_ptr.hpp>


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace XYZ {
	static std::random_device s_RandomDev; // obtain a random number from hardware
	static std::mt19937 s_RandomGen(s_RandomDev());

	static uint32_t RandomColor()
	{
		// seed the generator
		std::uniform_int_distribution<> distr(0, 255); // define the range

		uint32_t result = 0;
		result |= static_cast<uint8_t>(distr(s_RandomGen));		  // R
		result |= static_cast<uint8_t>(distr(s_RandomGen)) << 8;  // G
		result |= static_cast<uint8_t>(distr(s_RandomGen)) << 16; // B
		result |= static_cast<uint8_t>(distr(s_RandomGen)) << 24; // A

		return result;
	}
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
		
		m_StorageAllocator = Ref<StorageBufferAllocator>::Create(sizeof(SSBOVoxels), SSBOVoxels::Binding, SSBOVoxels::Set);

		TextureProperties props;
		props.Storage = true;
		props.SamplerWrap = TextureWrap::Clamp;
		m_OutputTexture = Texture2D::Create(ImageFormat::RGBA32F, 1280, 720, nullptr, props);
		m_DepthTexture = Texture2D::Create(ImageFormat::RED32F, 1280, 720, nullptr, props);
		createRaymarchPipeline();

		m_UBVoxelScene.LightDirection = { -0.2f, -1.4f, -1.5f, 1.0f };
		m_UBVoxelScene.LightColor = glm::vec4(1.0f);


		for (uint32_t i = 0; i < 256; i++)
		{
			m_SSBOVoxelModels.Colors[i] = RandomColor();
		}
	}
	void VoxelRenderer::BeginScene(const VoxelRendererCamera& camera)
	{
		m_UBVoxelScene.InverseProjection = glm::inverse(camera.Projection);
		m_UBVoxelScene.InverseView = glm::inverse(camera.ViewMatrix);
		m_UBVoxelScene.CameraPosition = glm::vec4(camera.CameraPosition, 1.0f);
		
		m_UBVoxelScene.ViewportSize.x = m_ViewportSize.x;
		m_UBVoxelScene.ViewportSize.y = m_ViewportSize.y;
		m_DrawCommands.clear();
		m_Frustum = camera.Frustum;

		updateViewportSize();
		updateUniformBufferSet();

	}
	void VoxelRenderer::EndScene()
	{
		updateStorageBufferSet();
		render();

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
	void VoxelRenderer::SetColors(const std::array<uint32_t, 256>& colors)
	{
		memcpy(m_SSBOVoxelModels.Colors, colors.data(), sizeof(SSBOVoxelModels::Colors));
	}
	void VoxelRenderer::SetColors(const std::array<VoxelColor, 256>& colors)
	{
		memcpy(m_SSBOVoxelModels.Colors, colors.data(), sizeof(SSBOVoxelModels::Colors));
	}

	void VoxelRenderer::SubmitMesh(const Ref<VoxelMesh>& mesh, const glm::mat4& transform, float voxelSize)
	{
		auto& drawCommand = m_DrawCommands[mesh->GetHandle()];
		const auto& submeshes = mesh->GetSubmeshes();
		drawCommand.Mesh = mesh;
		for (const auto& instance : mesh->GetInstances())
		{
			const glm::mat4 instanceTransform = transform * instance.Transform;

			const VoxelSubmesh& submesh = submeshes[instance.SubmeshIndex];
			submitSubmesh(submesh, drawCommand, transform * instance.Transform, voxelSize, instance.SubmeshIndex);
		}	
	}

	void VoxelRenderer::SubmitMesh(const Ref<VoxelMesh>& mesh, const glm::mat4& transform, const uint32_t* keyFrames, float voxelSize)
	{
		auto& drawCommand = m_DrawCommands[mesh->GetHandle()];
		const auto& submeshes = mesh->GetSubmeshes();
		drawCommand.Mesh = mesh;
		uint32_t index = 0;
		for (const auto& instance : mesh->GetInstances())
		{
			const uint32_t submeshIndex = instance.ModelAnimation.SubmeshIndices[keyFrames[index]];

			const VoxelSubmesh& submesh = submeshes[submeshIndex];
			submitSubmesh(submesh, drawCommand, transform * instance.Transform, voxelSize, submeshIndex);

			index++;
		}	
	}


	
	void VoxelRenderer::OnImGuiRender()
	{
		if (ImGui::Begin("Voxel Renderer"))
		{
			ImGui::DragFloat3("Light Direction", glm::value_ptr(m_UBVoxelScene.LightDirection), 0.1f);
			ImGui::DragFloat3("Light Color", glm::value_ptr(m_UBVoxelScene.LightColor), 0.1f);

			ImGui::DragInt("Max Traverses", (int*)&m_UBVoxelScene.MaxTraverses, 1, 0, 1024);

			ImGui::Text("Mesh Allocations: %d", static_cast<uint32_t>(m_LastFrameMeshAllocations.size()));
			ImGui::Text("Update Allocations: %d", static_cast<uint32_t>(m_UpdatedAllocations.size()));


			const uint32_t frameIndex = Renderer::GetCurrentFrame();
			float gpuTime = m_CommandBuffer->GetExecutionGPUTime(frameIndex, m_GPUTimeQueries.GPUTime);
			ImGui::Text("GPU Time: %.3fms", gpuTime);
		}
		ImGui::End();
	}

	bool VoxelRenderer::submitSubmesh(const VoxelSubmesh& submesh, VoxelDrawCommand& drawCommand, const glm::mat4& transform, float voxelSize, uint32_t submeshIndex)
	{
		const AABB aabb = VoxelModelToAABB(transform, submesh.Width, submesh.Height, submesh.Depth, voxelSize);
		if (!aabb.InsideFrustum(m_Frustum))
			return false;

		const glm::vec3 aabbMax = glm::vec3(submesh.Width, submesh.Height, submesh.Depth) * voxelSize;

		glm::vec3 centerTranslation = -glm::vec3(
			submesh.Width / 2 * voxelSize,
			submesh.Height / 2 * voxelSize,
			submesh.Depth / 2 * voxelSize
		);
		const uint32_t voxelCount = static_cast<uint32_t>(submesh.ColorIndices.size());

		VoxelCommandModel& cmdModel = drawCommand.Models.emplace_back();
		cmdModel.SubmeshIndex = submeshIndex;

		VoxelModel& model = cmdModel.Model;

		model.Transform = transform * glm::translate(glm::mat4(1.0f), centerTranslation);
		const glm::mat4 inverseTransform = glm::inverse(model.Transform);
		model.InverseModelView = inverseTransform * m_UBVoxelScene.InverseView;
		model.RayOrigin = inverseTransform * m_UBVoxelScene.CameraPosition;

		model.Width = submesh.Width;
		model.Height = submesh.Height;
		model.Depth = submesh.Depth;
		model.VoxelSize = voxelSize;

		model.OriginInside = Math::PointInBox(model.RayOrigin, glm::vec3(0.0f), aabbMax);
		return true;
	}

	void VoxelRenderer::clear()
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
			32, 32, 1,
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

	void VoxelRenderer::render()
	{
		m_CommandBuffer->Begin();		
		m_GPUTimeQueries.GPUTime = m_CommandBuffer->BeginTimestampQuery();
		clear();
		Renderer::BeginPipelineCompute(
			m_CommandBuffer,
			m_RaymarchPipeline,
			m_UniformBufferSet,
			m_StorageBufferSet,
			m_RaymarchMaterial
		);

		for (const auto& [key, drawCommand] : m_DrawCommands)
		{
			if (drawCommand.Models.empty())
				continue;

			Renderer::DispatchCompute(
				m_RaymarchPipeline,
				nullptr,
				32, 32, 1
			);
		}
		Renderer::EndPipelineCompute(m_RaymarchPipeline);
		m_CommandBuffer->EndTimestampQuery(m_GPUTimeQueries.GPUTime);

		m_CommandBuffer->End();
		m_CommandBuffer->Submit();
	}
	void VoxelRenderer::updateViewportSize()
	{
		if (m_ViewportSizeChanged)
		{
			m_ViewportSizeChanged = false;
			TextureProperties props;
			props.Storage = true;
			props.SamplerWrap = TextureWrap::Clamp;
			m_OutputTexture = Texture2D::Create(ImageFormat::RGBA32F, m_ViewportSize.x, m_ViewportSize.y, nullptr, props);
			m_DepthTexture == Texture2D::Create(ImageFormat::RED32F, m_ViewportSize.x, m_ViewportSize.y, nullptr, props);
			m_RaymarchMaterial->SetImage("o_Image", m_OutputTexture->GetImage());
			m_RaymarchMaterial->SetImage("o_DepthImage", m_DepthTexture->GetImage());

			m_ClearMaterial->SetImage("o_Image", m_OutputTexture->GetImage());
			m_ClearMaterial->SetImage("o_DepthImage", m_DepthTexture->GetImage());
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
	void VoxelRenderer::updateStorageBufferSet()
	{
		Ref<VoxelRenderer> instance = this;
		uint32_t modelCount = 0;
		for (auto& [key, drawCommand] : m_DrawCommands)
		{
			if (drawCommand.Models.empty())
				continue;

			MeshAllocation& meshAlloc = createMeshAllocation(drawCommand.Mesh);
			for (auto& cmdModel : drawCommand.Models)
			{
				cmdModel.Model.VoxelOffset = meshAlloc.SubmeshOffsets[cmdModel.SubmeshIndex];
				m_SSBOVoxelModels.Models[modelCount++] = cmdModel.Model;
			}
		}

		m_SSBOVoxelModels.NumModels = modelCount;

		const uint32_t voxelModelsUpdateSize = 
			  sizeof(SSBOVoxelModels::Colors) 
			+ sizeof(SSBOVoxelModels::NumModels) 
			+ sizeof(SSBOVoxelModels::Padding) 
			+ modelCount * sizeof(VoxelModel);
		
		for (const auto& updated : m_UpdatedAllocations)
		{
			const auto& alloc = updated.Allocation;
			void* updateData = &m_SSBOVoxels.Voxels[alloc.GetOffset()];
			m_StorageBufferSet->UpdateEachFrame(updateData, alloc.GetSize(), alloc.GetOffset(), SSBOVoxels::Binding, SSBOVoxels::Set);		
		}
		m_UpdatedAllocations.clear();
		m_StorageBufferSet->Update((void*)&m_SSBOVoxelModels, voxelModelsUpdateSize, 0, SSBOVoxelModels::Binding, SSBOVoxelModels::Set);
	}
	void VoxelRenderer::createRaymarchPipeline()
	{
		Ref<Shader> shader = Shader::Create("Resources/Shaders/RaymarchShader.glsl");
		m_RaymarchMaterial = Material::Create(shader);

		m_RaymarchMaterial->SetImage("o_Image", m_OutputTexture->GetImage());
		m_RaymarchMaterial->SetImage("o_DepthImage", m_DepthTexture->GetImage());

		PipelineComputeSpecification spec;
		spec.Shader = shader;
	
		m_RaymarchPipeline = PipelineCompute::Create(spec);


		Ref<Shader> clearShader = Shader::Create("Resources/Shaders/ImageClearShader.glsl");
		m_ClearMaterial = Material::Create(clearShader);
		m_ClearMaterial->SetImage("o_Image", m_OutputTexture->GetImage());
		m_ClearMaterial->SetImage("o_DepthImage", m_DepthTexture->GetImage());

		spec.Shader = clearShader;
		m_ClearPipeline = PipelineCompute::Create(spec);
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
		if (m_StorageAllocator->Allocate(meshSize, meshAlloc.Allocation) || mesh->NeedUpdate())
		{		
			m_UpdatedAllocations.push_back({ meshAlloc.Allocation, meshHandle });
			meshAlloc.SubmeshOffsets.resize(submeshes.size());
			uint32_t submeshIndex = 0;
			uint32_t offset = meshAlloc.Allocation.GetOffset();
			for (auto& submesh : submeshes)
			{
				meshAlloc.SubmeshOffsets[submeshIndex] = offset;
				const uint32_t voxelCount = static_cast<uint32_t>(submesh.ColorIndices.size());
				memcpy(&m_SSBOVoxels.Voxels[offset], submesh.ColorIndices.data(), voxelCount * sizeof(uint8_t));
				offset += voxelCount;
				submeshIndex++;
			}
		}
		return meshAlloc;
	}
}
