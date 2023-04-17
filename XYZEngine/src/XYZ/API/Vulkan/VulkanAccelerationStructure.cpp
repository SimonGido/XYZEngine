#include "stdafx.h"
#include "VulkanAccelerationStructure.h"

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

	VulkanAccelerationStructure::VulkanAccelerationStructure(const RaytracingGeometrySpecification& specification)
		:
		m_Specification(specification)
	{
	}
	void VulkanAccelerationStructure::RT_invalidate()
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		Ref<VulkanVertexBuffer> vkVertexBuffer = m_Specification.VertexBuffer.As<VulkanVertexBuffer>();
		Ref<VulkanIndexBuffer> vkIndexBuffer = m_Specification.IndexBuffer.As<VulkanIndexBuffer>();

		VkDeviceAddress vertexAddress = GetBufferAddress(device, vkVertexBuffer->GetVulkanBuffer());
		VkDeviceAddress indexAddress = GetBufferAddress(device, vkIndexBuffer->GetVulkanBuffer());

		VkAccelerationStructureGeometryTrianglesDataKHR triangles{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR };
		triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;  // vec3 vertex position data.
		triangles.vertexData.deviceAddress = vertexAddress;
		triangles.vertexStride = m_Specification.VertexStride;

		triangles.indexType = vkIndexBuffer->GetVulkanIndexType();
		triangles.indexData.deviceAddress = indexAddress;
		// Indicate identity transform by setting transformData to null device pointer.
		//triangles.transformData = {};
		triangles.maxVertex = vkVertexBuffer->GetUseSize() / m_Specification.VertexStride;


		VkAccelerationStructureGeometryKHR vkGeometry;
		vkGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		vkGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		vkGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		vkGeometry.geometry.triangles = triangles;

		VkAccelerationStructureBuildRangeInfoKHR offset;
		offset.firstVertex = 0;
		offset.primitiveCount = vkIndexBuffer->GetUseCount() / 3;
		offset.primitiveOffset = 0;
		offset.transformOffset = 0;

		
	}
}