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

	class VulkanAccelerationStructure
	{
	public:
		VulkanAccelerationStructure(const RaytracingGeometrySpecification& specification);


	private:
		void RT_invalidate();


	private:
		RaytracingGeometrySpecification m_Specification;
	};
}