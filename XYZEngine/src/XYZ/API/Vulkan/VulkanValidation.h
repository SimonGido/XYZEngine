#pragma once


namespace XYZ {

	class VulkanValidation
	{
	public:
		static bool CheckLayerSupport(const char* layerName);
		static bool CheckExtensionSupport(const char* extensionName);
		
		static bool AddValidationLayer(const char* layer, std::vector<const char*>& layers);
		static bool AddExtension(const char* extension, std::vector<const char*>& extensions);
	};
}