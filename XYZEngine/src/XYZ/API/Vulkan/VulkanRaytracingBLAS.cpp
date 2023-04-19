#include "stdafx.h"
#include "VulkanRaytracingBLAS.h"

#include "VulkanContext.h"

namespace XYZ {

	static VkFormat VertexFormatToVK(VertexFormat format)
	{
		switch (format)
		{
		case XYZ::VertexFormat::None:
			XYZ_ASSERT(false, "Invalid format");
			return VK_FORMAT_UNDEFINED;
		case XYZ::VertexFormat::Float16:
			return VK_FORMAT_R16G16B16_SFLOAT;
		case XYZ::VertexFormat::Float32:
			return VK_FORMAT_R32G32B32_SFLOAT;
		default:
			break;
		}
	}

	static VkTransformMatrixKHR GlmMat4ToVK(const glm::mat4& matrix)
	{
		VkTransformMatrixKHR result;
		for (uint32_t i = 0; i < 3; ++i)
			memcpy(result.matrix[i], &matrix[i], 4 * sizeof(float));
		
		return result;
	}

	static VkDeviceAddress GetBufferAddress(VkDevice device, VkBuffer buffer)
	{
		VkBufferDeviceAddressInfo info = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
		info.buffer = buffer;
		return vkGetBufferDeviceAddress(device, &info);
	}

	VulkanRaytracingBLAS::VulkanRaytracingBLAS(const std::vector<RaytracingGeometrySpecification>& specifications)
		:
		m_Geometry(specifications)
	{
		
		Ref<VulkanRaytracingBLAS> instance = this;
		Renderer::Submit([instance]() mutable {
			instance->createTransformBuffer();
			instance->RT_convertRaytracingGeometryToVK();
			instance->RT_createBLAS();
		});
	}

	VulkanRaytracingBLAS::~VulkanRaytracingBLAS()
	{
		VkAccelerationStructureKHR blas = m_BLAS;
		Renderer::SubmitResource([blas]() {

			auto device = VulkanContext::GetCurrentDevice();
			VkDevice vkDevice = device->GetVulkanDevice();
			vkDestroyAccelerationStructureKHR(vkDevice, blas, nullptr);
		});
	}

	void VulkanRaytracingBLAS::createTransformBuffer()
	{
		VkTransformMatrixKHR* transforms = new VkTransformMatrixKHR[m_Geometry.size()];
		for (uint32_t i = 0; i < m_Geometry.size(); ++i)
		{
			transforms[i] = GlmMat4ToVK(m_Geometry[i].Transform);
		}
		m_TransformBuffer.RT_Create(
			&transforms,
			m_Geometry.size() * sizeof(VkTransformMatrixKHR),
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
		);
	}

	void VulkanRaytracingBLAS::RT_convertRaytracingGeometryToVK()
	{
		auto device = VulkanContext::GetCurrentDevice();
		VkDevice vkDevice = device->GetVulkanDevice();

		VkDeviceOrHostAddressConstKHR transformAddress{};
		transformAddress.deviceAddress = GetBufferAddress(vkDevice, m_TransformBuffer.GetVulkanBuffer());

		VkDeviceSize asTotalSize{ 0 };     // Memory size of all allocated BLAS
		uint32_t     nbCompactions{ 0 };   // Nb of BLAS requesting compaction
		VkDeviceSize maxScratchSize{ 0 };  // Largest scratch size

		uint32_t index = 0;
		m_VKGeometry.resize(m_Geometry.size());
		m_VKOffsets.resize(m_Geometry.size());
		m_TriangleCounts.resize(m_Geometry.size());



		for (auto& geometry : m_Geometry)
		{
			Ref<VulkanVertexBuffer> vkVertexBuffer = geometry.VertexBuffer.As<VulkanVertexBuffer>();
			Ref<VulkanIndexBuffer> vkIndexBuffer = geometry.IndexBuffer.As<VulkanIndexBuffer>();

			VkDeviceAddress vertexAddress = GetBufferAddress(vkDevice, vkVertexBuffer->GetVulkanBuffer());
			VkDeviceAddress indexAddress = GetBufferAddress(vkDevice, vkIndexBuffer->GetVulkanBuffer());

		
			VkAccelerationStructureGeometryKHR& vkGeometry = m_VKGeometry[index];
			vkGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
			vkGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
			vkGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
			vkGeometry.pNext = nullptr;
			

			VkAccelerationStructureGeometryTrianglesDataKHR& triangles = vkGeometry.geometry.triangles;
			triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR ;
			triangles.vertexFormat = VertexFormatToVK(geometry.Format);
			triangles.vertexData.deviceAddress = vertexAddress;
			triangles.vertexStride = geometry.VertexStride;
			triangles.pNext = nullptr;

			triangles.indexType = vkIndexBuffer->GetVulkanIndexType();
			triangles.indexData.deviceAddress = indexAddress;

			triangles.transformData = transformAddress;
			triangles.maxVertex = vkVertexBuffer->GetUseSize() / geometry.VertexStride;

			
			VkAccelerationStructureBuildRangeInfoKHR& offset = m_VKOffsets[index];
			offset.firstVertex = 0;
			offset.primitiveCount = vkIndexBuffer->GetUseCount() / 3;
			offset.primitiveOffset = 0;
			offset.transformOffset = index * sizeof(VkTransformMatrixKHR);

			m_TriangleCounts[index] = vkIndexBuffer->GetUseCount() / 3;
			index++;
		}
	}
	void VulkanRaytracingBLAS::RT_createBLAS()
	{
		auto device = VulkanContext::GetCurrentDevice();
		VkDevice vkDevice = device->GetVulkanDevice();

		VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo;
		accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationBuildGeometryInfo.geometryCount = static_cast<uint32_t>(m_VKGeometry.size());
		accelerationBuildGeometryInfo.pGeometries = m_VKGeometry.data();
		accelerationBuildGeometryInfo.ppGeometries = nullptr;
		
		accelerationBuildGeometryInfo.pNext = nullptr;
		accelerationBuildGeometryInfo.srcAccelerationStructure = nullptr;
		accelerationBuildGeometryInfo.dstAccelerationStructure = nullptr;

		VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo;
		accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		accelerationStructureBuildSizesInfo.pNext = nullptr;


		PFN_vkGetAccelerationStructureBuildSizesKHR pfnGetAccelerationStructureBuildSizesKHR;
		pfnGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(vkDevice, "vkGetAccelerationStructureBuildSizesKHR"));


		pfnGetAccelerationStructureBuildSizesKHR(
			vkDevice,
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&accelerationBuildGeometryInfo,
			m_TriangleCounts.data(),
			&accelerationStructureBuildSizesInfo
		);

		m_BLASBuffer.RT_Create(accelerationStructureBuildSizesInfo.accelerationStructureSize,
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

		VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
		accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		accelerationStructureCreateInfo.buffer = m_BLASBuffer.GetVulkanBuffer();
		accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
		accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		accelerationStructureCreateInfo.pNext = nullptr;


		PFN_vkCreateAccelerationStructureKHR pfnCreateAccelerationStructureKHR;
		pfnCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(vkDevice, "vkCreateAccelerationStructureKHR"));

		VK_CHECK_RESULT(pfnCreateAccelerationStructureKHR(vkDevice, &accelerationStructureCreateInfo, nullptr, &m_BLAS));

		VulkanBuffer scratchBuffer;
		scratchBuffer.RT_Create(accelerationStructureBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

		accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		accelerationBuildGeometryInfo.dstAccelerationStructure = m_BLAS;
		accelerationBuildGeometryInfo.scratchData.deviceAddress = GetBufferAddress(vkDevice, scratchBuffer.GetVulkanBuffer());

		const VkAccelerationStructureBuildRangeInfoKHR* const buildRanges = m_VKOffsets.data();

		VkCommandBuffer commandBuffer = device->GetCommandBuffer(true);
		vkCmdBuildAccelerationStructuresKHR(
			commandBuffer,
			1,
			&accelerationBuildGeometryInfo,
			&buildRanges
		);
		
		device->FlushCommandBuffer(commandBuffer);
	}
}