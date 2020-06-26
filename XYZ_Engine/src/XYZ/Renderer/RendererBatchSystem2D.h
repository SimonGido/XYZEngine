#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Physics/PhysicsComponent.h"
#include "Renderable2D.h"
#include "SortingLayer.h"


#include <queue>

namespace XYZ {
	

	/**
	* @class RendererBatchSystem2D
	* @brief represents system , that groups ( batches ) renderables dependent on the material and z distance from the camera
	*/
	class RendererBatchSystem2D : public System
	{
	public:
		/**
		* Construct a batch system
		*/
		RendererBatchSystem2D();

		/**
		* Add entity to the Opaque/Transparent group dependent on the sort key in the material of the entity
		* @param[in] enity 
		*/
		virtual void Add(Entity entity) override;

		/**
		* Remove entity
		* @param[in] entity
		*/
		virtual void Remove(Entity entity) override;

		/**
		* Check if system contains entity
		* @param[in] entity
		*/
		virtual bool Contains(Entity entity) override;

		/**
		* If entity is updated, if the sort key of the entity has changed, remove it from the current group,
		* and insert it to the new group dependent on the key
		* @param[in] entity
		*/
		virtual void EntityUpdated(Entity entity) override;

		/**
		* Build commands and submit them to the Renderer2D
		*/
		void SubmitToRenderer();
	


	private:
	

		struct Component : public System::Component
		{
			Renderable2D* Renderable;
			Transform2D* Transform;
		};

		std::vector<Component> m_OpaqueComponents;
		std::vector<Component> m_TransparentComponents;

		struct OpaqueComparator
		{
			bool operator()(const Component& a, const Component& b) const
			{
				int sortLayerA = a.Renderable->SortLayerID;
				int sortLayerB = b.Renderable->SortLayerID;

				float valueA = (float)SortingLayer::Get().GetOrderValueByID(sortLayerA) + a.Transform->GetPosition().z;
				float valueB = (float)SortingLayer::Get().GetOrderValueByID(sortLayerB) + b.Transform->GetPosition().z;


				if (a.Renderable->Material->GetSortKey() == b.Renderable->Material->GetSortKey())
				{
					return valueA < valueB;
				}
				a.Renderable->Material->GetSortKey() < b.Renderable->Material->GetSortKey();
			}
		};

		struct TransparentComparator
		{
			bool operator()(const Component& a, const Component& b) const
			{
				int sortLayerA = a.Renderable->SortLayerID;
				int sortLayerB = b.Renderable->SortLayerID;

				float valueA = (float)SortingLayer::Get().GetOrderValueByID(sortLayerA) + a.Transform->GetPosition().z;
				float valueB = (float)SortingLayer::Get().GetOrderValueByID(sortLayerB) + b.Transform->GetPosition().z;


				if (fabs(valueA - valueB) <= std::numeric_limits<float>::epsilon())
				{
					a.Renderable->Material->GetSortKey() < b.Renderable->Material->GetSortKey();
				}
				return valueA > valueB;
			}
		};

	private:	

		// Maybe useless
		template <typename Comparator>
		void InsertionSort(std::vector<Component>& vec)
		{
			Comparator cmp;
			for (int i = 1; i < vec.size(); ++i)
			{
				auto el = vec[i];
				int j = i - 1;

				while (j >= 0 && cmp(el, vec[j]))
				{
					vec[j + 1] = vec[j];
					j = j - 1;
				}
				vec[j + 1] = el;
			}
		}
	};
}