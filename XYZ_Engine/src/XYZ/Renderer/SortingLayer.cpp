#include "stdafx.h"
#include "SortingLayer.h"

namespace XYZ {
	SortingLayer SortingLayer::s_Instance;

	void SortingLayer::CreateLayer(const std::string& name, int order)
	{
		m_LookUp[name] = m_Layers.Insert(order);
	}
	void SortingLayer::DeleteLayer(const std::string& name)
	{
		XYZ_ASSERT(m_LookUp.find(name) != m_LookUp.end(), "Sorting layer does not exist");
		int index = m_LookUp[name];
		m_Layers.Erase(index);
		m_LookUp.erase(name);
	}
	SortingLayerID SortingLayer::GetOrderValueByName(const std::string& name)
	{
		XYZ_ASSERT(m_LookUp.find(name) != m_LookUp.end(), "Sorting layer does not exist");
		return m_LookUp[name];
	}
	SortingLayerID SortingLayer::GetOrderValueByID(SortingLayerID id)
	{
		return m_Layers[id];
	}
	SortingLayer::SortingLayer()
	{
		m_LookUp["default"] = m_Layers.Insert(0);
	}
}