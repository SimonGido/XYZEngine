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
		~VulkanRaytracingBLAS();

	private:
		void createTransformBuffer();

		void RT_convertRaytracingGeometryToVK();
		void RT_createBLAS();
		void RT_createTLAS();

	private:
		VulkanBuffer m_BLASBuffer;
		VulkanBuffer m_TransformBuffer;

		VkAccelerationStructureKHR m_BLAS;

		std::vector<uint32_t> m_TriangleCounts;
		std::vector<RaytracingGeometrySpecification> m_Geometry;
		std::vector<VkAccelerationStructureGeometryKHR> m_VKGeometry;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR> m_VKOffsets;
	};
}