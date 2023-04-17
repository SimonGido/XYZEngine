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

	static VkDeviceAddress GetBufferAddress(VkDevice device, VkBuffer buffer)
	{
		VkBufferDeviceAddressInfo info = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
		info.buffer = buffer;
		return vkGetBufferDeviceAddress(device, &info);
	}

	void VulkanRaytracingBLAS::RT_convertRaytracingGeometryToVK()
	{
		m_VKGeometry.clear();
		m_VKOffsets.clear();

		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		VkDeviceSize asTotalSize{ 0 };     // Memory size of all allocated BLAS
		uint32_t     nbCompactions{ 0 };   // Nb of BLAS requesting compaction
		VkDeviceSize maxScratchSize{ 0 };  // Largest scratch size

		for (auto& geometry : m_Geometry)
		{
			Ref<VulkanVertexBuffer> vkVertexBuffer = geometry.VertexBuffer.As<VulkanVertexBuffer>();
			Ref<VulkanIndexBuffer> vkIndexBuffer = geometry.IndexBuffer.As<VulkanIndexBuffer>();

			VkDeviceAddress vertexAddress = GetBufferAddress(device, vkVertexBuffer->GetVulkanBuffer());
			VkDeviceAddress indexAddress = GetBufferAddress(device, vkIndexBuffer->GetVulkanBuffer());

			VkAccelerationStructureGeometryTrianglesDataKHR triangles{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR };
			triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;  // vec3 vertex position data.
			triangles.vertexData.deviceAddress = vertexAddress;
			triangles.vertexStride = geometry.VertexStride;
			
			triangles.indexType = vkIndexBuffer->GetVulkanIndexType();
			triangles.indexData.deviceAddress = indexAddress;
			// Indicate identity transform by setting transformData to null device pointer.
			//triangles.transformData = {};
			triangles.maxVertex = vkVertexBuffer->GetUseSize() / geometry.VertexStride;


			auto& vkGeometry = m_VKGeometry.emplace_back();
			vkGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
			vkGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
			vkGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
			vkGeometry.geometry.triangles = triangles;

			VkAccelerationStructureBuildRangeInfoKHR offset;
			offset.firstVertex = 0;
			offset.primitiveCount = vkIndexBuffer->GetUseCount() / 3;
			offset.primitiveOffset = 0;
			offset.transformOffset = 0;

			m_VKOffsets.push_back(offset);
		}

	}
	void VulkanRaytracingBLAS::RT_buildBlas()
	{
		VkDeviceSize asTotalSize{ 0 };     // Memory size of all allocated BLAS
		uint32_t     nbCompactions{ 0 };   // Nb of BLAS requesting compaction
		VkDeviceSize maxScratchSize{ 0 };  // Largest scratch size
	
	}
}