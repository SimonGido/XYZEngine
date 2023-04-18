#pragma once
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"
#include "VulkanBuffer.h"

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
		glm::mat4		  Transform;
	};


	class VulkanRaytracingBLAS : public RefCount
	{
	public:
		VulkanRaytracingBLAS(const std::vector<RaytracingGeometrySpecification>& specifications);

	private:
		void createTransformBuffer();

		void RT_convertRaytracingGeometryToVK();


	private:
		Ref<VulkanBuffer> m_AccelerationBuffer;
		Ref<VulkanBuffer> m_TransformBuffer;

		std::vector<RaytracingGeometrySpecification> m_Geometry;
		std::vector<VkAccelerationStructureGeometryKHR> m_VKGeometry;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR> m_VKOffsets;
	};
}