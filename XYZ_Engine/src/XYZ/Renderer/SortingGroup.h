#pragma once
#include "Renderable2D.h"
#include "VertexArray.h"

namespace XYZ {

	typedef std::map<std::shared_ptr<Material>, std::vector<int>> RenderablesGroup;

	/**
	* @class SortingGroup
	* @brief Sort renderables dependent on specfied Comparator 
	* @tparam T		Comparator
	*/
	template <typename T>
	class SortingGroup
	{
	public:
		/**
		* Insert renderable to the vector , keeps it sorted
		* @param[in] renderable		Pointer to the renderable
		*/
		void AddRenderable(int index)
		{
			auto storage = ECSManager::Get()->GetComponentStorage<Renderable2D>();
			auto material = (*storage)[index].material;
			auto it = std::lower_bound(m_Renderables[material].begin(), m_Renderables[material].end(), index, T());
			m_Renderables[material].insert(it, index);	
		}
		/**
		* Remove renderable from the vector, in linear time
		* @param[in] renderable
		*/
		void RemoveRenderable(int index)
		{
			auto storage = ECSManager::Get()->GetComponentStorage<Renderable2D>();
			auto material = (*storage)[index].material;


			auto it = std::find(m_Renderables[material].begin(), m_Renderables[material].end(), index);
			if (it == m_Renderables[material].end() || (*it) != index)
				XYZ_ASSERT(false, "Attempting to remove not existing renderable");

			m_Renderables[material].erase(it);
		}

		/**
		* @return renderables group
		*/
		const RenderablesGroup& GetRenderables() { return m_Renderables; }


	private:
		RenderablesGroup m_Renderables;
	};

}