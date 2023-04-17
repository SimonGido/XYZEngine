#pragma once
#include "XYZ/Core/Ref/Ref.h"

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

	class RaytracingGeometry : public RefCount
	{

	};

	class VulkanRaytracingGeometry
	{
	public:
		VulkanRaytracingGeometry(const RaytracingGeometrySpecification& specification);


	private:
		void RT_invalidate();

	private:
		RaytracingGeometrySpecification m_Specification;
		VkFormat						m_VertexFormat;
	};
}