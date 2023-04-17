#include "stdafx.h"
#include "VulkanRaytracingGeometry.h"

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

	VulkanRaytracingGeometry::VulkanRaytracingGeometry(const RaytracingGeometrySpecification& specification)
		:
		m_Specification(specification)
	{
		m_VertexFormat = VertexFormatToVK(specification.Format);
	}
	void VulkanRaytracingGeometry::RT_invalidate()
	{
	}
}