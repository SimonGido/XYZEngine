#include "stdafx.h"
#include "Vulkan.h"

#include "VulkanContext.h"
//#include "VulkanDiagnostics.h"

namespace XYZ {
	namespace Utils {

		//static const char* StageToString(VkPipelineStageFlagBits stage)
		//{
		//	switch (stage)
		//	{
		//	case VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT: return "VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT";
		//	case VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT: return "VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT";
		//	}
		//	XYZ_ASSERT(false, "");
		//	return nullptr;
		//}
		//
		//void RetrieveDiagnosticCheckpoints()
		//{
		//	bool supported = VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->IsExtensionSupported(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME);
		//	if (!supported)
		//		return;
		//
		//
		//	{
		//		const uint32_t checkpointCount = 4;
		//		VkCheckpointDataNV data[checkpointCount];
		//		for (uint32_t i = 0; i < checkpointCount; i++)
		//			data[i].sType = VK_STRUCTURE_TYPE_CHECKPOINT_DATA_NV;
		//
		//		uint32_t retrievedCount = checkpointCount;
		//		vkGetQueueCheckpointDataNV(VulkanContext::GetCurrentDevice()->GetGraphicsQueue(), &retrievedCount, data);
		//		XYZ_ERROR("RetrieveDiagnosticCheckpoints (Graphics Queue):");
		//		for (uint32_t i = 0; i < retrievedCount; i++)
		//		{
		//			VulkanCheckpointData* checkpoint = (VulkanCheckpointData*)data[i].pCheckpointMarker;
		//			XYZ_ERROR("Checkpoint: {0} (stage: {1})", checkpoint->Data, StageToString(data[i].stage));
		//		}
		//	}
		//	{
		//		const uint32_t checkpointCount = 4;
		//		VkCheckpointDataNV data[checkpointCount];
		//		for (uint32_t i = 0; i < checkpointCount; i++)
		//			data[i].sType = VK_STRUCTURE_TYPE_CHECKPOINT_DATA_NV;
		//
		//		uint32_t retrievedCount = checkpointCount;
		//		vkGetQueueCheckpointDataNV(::XYZ::VulkanContext::GetCurrentDevice()->GetComputeQueue(), &retrievedCount, data);
		//		XYZ_ERROR("RetrieveDiagnosticCheckpoints (Compute Queue):");
		//		for (uint32_t i = 0; i < retrievedCount; i++)
		//		{
		//			VulkanCheckpointData* checkpoint = (VulkanCheckpointData*)data[i].pCheckpointMarker;
		//			XYZ_ERROR("Checkpoint: {0} (stage: {1})", checkpoint->Data, StageToString(data[i].stage));
		//		}
		//	}
		//	//__debugbreak();
		//}

	}
}