#include "stdafx.h"
#include "SortingLayer.h"

namespace XYZ {
	SortingLayer::SortingLayer(token)
	{
		CreateLayer("Default");
	}

	void SortingLayer::CreateLayer(const std::string& name)
	{
		XYZ_ASSERT(m_Layers.size() <= sc_MaxNumberOfLayers, "Maximum number of layers exceeded");
		m_Layers.push_back(Layer{ name });
		m_Next++;
	}
	void SortingLayer::DeleteLayer(const std::string& name)
	{
		for (size_t i = 0; i < m_Layers.size(); ++i)
		{
			if (m_Layers[i].Name == name)
			{
				m_Layers.erase(m_Layers.begin() + i);
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

	void SortingLayer::SetOrderOfLayer(const std::string& name, size_t order)
	{
		XYZ_ASSERT(order >= 0 && order <= sc_MaxNumberOfLayers, "Attempting to set order of sorting layer out of boundaries");
		for (size_t i = 0; i < m_Layers.size(); ++i)
		{
			if (m_Layers[i].Name == name)
			{
				move(m_Layers, i, order);
				return;
			}
		}
	}
	SortLayerID SortingLayer::GetOrderValue(const std::string& name) const
	{
		for (size_t i = 0; i < m_Layers.size(); ++i)
		{
			if (m_Layers[i].Name == name)
			{
				return (1 << static_cast<int>(i));
			}
		}
		XYZ_ASSERT(false, "Trying to access not existing sorting layer");
	}
	
}