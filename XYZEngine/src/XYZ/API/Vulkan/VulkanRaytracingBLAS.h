#pragma once
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"

namespace XYZ {
	
	enum class VertexFormat
	{
		None,
		Float16,
		Float32
	};

	struct RaytracingGeometrySpecification
	{
		Ref<IndexBuffer>  IndexBuffer;
		Ref<VertexBuffer> VertexBuffer;
		uint32_t		  VertexStride;
		VertexFormat	  Format;
	};


	class VulkanRaytracingBLAS
	{
	public:


	private:
		void RT_convertRaytracingGeometryToVK();
		void RT_buildBlas();

	private:
		std::vector<RaytracingGeometrySpecification> m_Geometry;
		std::vector<VkAccelerationStructureGeometryKHR> m_VKGeometry;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR> m_VKOffsets;
	};
}