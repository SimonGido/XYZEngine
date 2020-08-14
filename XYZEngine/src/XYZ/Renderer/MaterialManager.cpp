#include "stdafx.h"
#include "MaterialManager.h"



namespace XYZ {

	Ref<Material> MaterialManager::GetMaterial(int16_t id)
	{
		return m_Materials[id];
	}

	MaterialManager::MaterialManager(token)
	{
		for (int16_t id = 0; id < sc_MaxNumberOfMaterials; ++id)
		{
			m_AvailableIDs.push(id);
		}
	}
	int16_t MaterialManager::RegisterMaterial(const Ref<Material>& material)
	{
		XYZ_ASSERT(m_MaterialsInExistence < sc_MaxNumberOfMaterials, "Too many materials in existence");
		int16_t id = m_AvailableIDs.front();
		m_AvailableIDs.pop();
		m_MaterialsInExistence++;
		m_Materials[id] = material;
		return id;
	}
	void MaterialManager::RemoveMaterial(int16_t id)
	{
		if (m_Materials.find(id) != m_Materials.end())
		{
			m_AvailableIDs.push(id);
			m_Materials.erase(id);
			m_MaterialsInExistence--;
		}
		else
			XYZ_LOG_ERR(false,"Attempting to remove not registered material");
	}
}