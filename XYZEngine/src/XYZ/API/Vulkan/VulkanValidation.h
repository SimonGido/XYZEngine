#pragma once


namespace XYZ {

	class VulkanValidation
	{
	public:
		bool AddLayer(std::string layerName);


	private:
		std::vector<std::string> m_ValidationLayers;
	};
}