#pragma once
#include "Material.h"
#include "XYZ/Core/Singleton.h"

#include <queue>

namespace XYZ {

	/**
	* @class MaterialManager temporary class
	*/
	class MaterialManager : public Singleton<MaterialManager>
	{
		friend class Material;
	public:
		MaterialManager(token);
		/**
		* Store material in unordered_map ,give it unique id
		* @param[in] material
		* @return 16 bit unique id
		*/
		int16_t RegisterMaterial(std::shared_ptr<Material> material);


		/**
		* Remove material from unordered_map, push it's id to the queue of available ids
		* @param[in] id
		*/
		void  RemoveMaterial(int16_t id);




		/**
		* @param[in] id		Id of the material
		* @return shared_ptr to the Material
		*/
		std::shared_ptr<Material> GetMaterial(int16_t id);

	private:
		int16_t m_MaterialsInExistence = 0;
		std::queue<int16_t> m_AvailableIDs{};

		// Temporary untill we have asset manager
		std::unordered_map<int16_t, std::shared_ptr<Material>> m_Materials;

		static constexpr int16_t sc_MaxNumberOfMaterials = 2 ^ (sizeof(int16_t) * 8);
	};
}