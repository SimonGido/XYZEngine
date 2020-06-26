#pragma once
#include "XYZ/Utils/DataStructures/FreeList.h"


namespace XYZ {

	using SortingLayerID = int;

	/*! @class SortingLayer
	*	@brief Takes care of sorting layer IDs
	*/
	class SortingLayer
	{
	public:
		SortingLayer(const SortingLayer&) = delete;
		void CreateLayer(const std::string& name,int order);
		void DeleteLayer(const std::string& name);

		SortingLayerID GetOrderValueByName(const std::string& name);
		SortingLayerID GetOrderValueByID(SortingLayerID id);

		int GetNumberOfLayers() { return m_Layers.Range(); }

		static SortingLayer& Get() { return s_Instance; }
	private:
		SortingLayer();
		// Currently ID is also sorting value , might change in future
		FreeList<int> m_Layers;
		std::unordered_map<std::string, SortingLayerID> m_LookUp;

		static SortingLayer s_Instance;
	};
}