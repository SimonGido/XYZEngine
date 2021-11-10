#include "stdafx.h"
#include "VulkanValidation.h"


#include <vulkan/vulkan.h>

namespace XYZ {
	namespace Utils {

        static std::vector<VkLayerProperties> GetSupportedValidationLayer()
        {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            return availableLayers;
        }

	}
    bool VulkanValidation::AddLayer(std::string layerName)
    {
        return false;
    }
}