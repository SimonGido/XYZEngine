#pragma once



namespace XYZ {

	using SortLayerID = uint32_t;


	/*! @class SortingLayer
	*	@brief Takes care of sorting layer IDs
	*/
	class SortLayer
	{
	public:
		static void CreateLayer(const std::string& name);
		static void DeleteLayer(const std::string& name);
		static void SetOrderOfLayer(const std::string& name, size_t order);

		static SortLayerID GetOrderValue(const std::string& name);

		static size_t GetNumberOfLayers() { return s_Layers.size(); }	
	private:	

		static std::vector<std::string> s_Layers;

		static constexpr int32_t sc_MaxNumberOfLayers = 31;

	};
}