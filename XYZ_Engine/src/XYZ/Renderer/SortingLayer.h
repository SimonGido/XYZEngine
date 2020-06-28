#pragma once

#include "XYZ/Core/Singleton.h"


namespace XYZ {

	using SortLayerID = int32_t;


	/*! @class SortingLayer
	*	@brief Takes care of sorting layer IDs
	*/
	class SortingLayer : public Singleton<SortingLayer>
	{
	public:
		SortingLayer(token);

		void CreateLayer(const std::string& name);
		void DeleteLayer(const std::string& name);
		void SetOrderOfLayer(const std::string& name,size_t order);

		SortLayerID GetOrderValue(const std::string& name) const;

		size_t GetNumberOfLayers() { return m_Layers.size(); }	
	private:	
		
		struct Layer
		{
			std::string Name;
		};

		int32_t m_Next = 0;
		std::vector<Layer> m_Layers;

		static constexpr int32_t sc_MaxNumberOfLayers = 31;

	};
}