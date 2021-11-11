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
        static std::vector<VkExtensionProperties> GetSupportedExtensions()
        {
            uint32_t extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> extensions(extensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
            return extensions;
        }
	}
 
    static const std::vector<VkExtensionProperties> s_SupportedExtensions       = Utils::GetSupportedExtensions();
    static const std::vector<VkLayerProperties>     s_SupportedValidationLayers = Utils::GetSupportedValidationLayer();

    bool VulkanValidation::CheckExtensionSupport(const char* extensionName)
    {
        for (const auto& supportedExt : s_SupportedExtensions)
        {
            if (strcmp(supportedExt.extensionName, extensionName) == 0)
                return true;
        }
        return false;
    }

  
    bool VulkanValidation::CheckLayerSupport(const char* layerName)
    {
        for (const auto& supportedLayer : s_SupportedValidationLayers)
        {
            if (strcmp(layerName, supportedLayer.layerName) == 0)
                return true;
        }
        return false;
    }

    bool VulkanValidation::AddValidationLayer(const char* layer, std::vector<const char*>& layers)
    {
        if (CheckLayerSupport(layer))
        {
            layers.push_back(layer);
            return true;
        }
        return false;
    }

    bool VulkanValidation::AddExtension(const char* extension, std::vector<const char*>& extensions)
    {
        if (CheckExtensionSupport(extension))
        {
            extensions.push_back(extension);
            return true;
        }
        return false;
    }
}