#include "stdafx.h"
#include "SortLayer.h"

namespace XYZ {

	std::vector<std::string> SortLayer::s_Layers;

	void SortLayer::CreateLayer(const std::string& name)
	{
		XYZ_ASSERT(s_Layers.size() <= sc_MaxNumberOfLayers, "Maximum number of layers exceeded");
		s_Layers.push_back( name );
	}
	void SortLayer::DeleteLayer(const std::string& name)
	{
		for (size_t i = 0; i < s_Layers.size(); ++i)
		{
			if (s_Layers[i] == name)
			{
				s_Layers.erase(s_Layers.begin() + i);
				return;
			}
		}
		XYZ_ASSERT(false, "Trying to remove not existing sorting layer");
	}

	template <typename t> void move(std::vector<t>& v, size_t oldIndex, size_t newIndex)
	{
		if (oldIndex > newIndex)
			std::rotate(v.rend() - oldIndex - 1, v.rend() - oldIndex, v.rend() - newIndex);
		else
			std::rotate(v.begin() + oldIndex, v.begin() + oldIndex + 1, v.begin() + newIndex + 1);
	}

	void SortLayer::SetOrderOfLayer(const std::string& name, size_t order)
	{
		XYZ_ASSERT(order >= 0 && order <= sc_MaxNumberOfLayers, "Attempting to set order of sorting layer out of boundaries");
		for (size_t i = 0; i < s_Layers.size(); ++i)
		{
			if (s_Layers[i] == name)
			{
				move(s_Layers, i, order);
				return;
			}
		}
	}
	SortLayerID SortLayer::GetOrderValue(const std::string& name)
	{
		for (size_t i = 0; i < s_Layers.size(); ++i)
		{
			if (s_Layers[i] == name)
			{
				return (1 << static_cast<int>(i));
			}
		}
		XYZ_ASSERT(false, "Trying to access not existing sorting layer");
	}
	
}