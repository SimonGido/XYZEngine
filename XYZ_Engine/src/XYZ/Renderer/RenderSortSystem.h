#pragma once
#include "XYZ/Physics/Transform.h"
#include "RenderComponent.h"

#include "XYZ/Utils/DataStructures/Tree.h"

namespace XYZ {
	class RenderSortSystem
	{
	public:
		void PushRenderData(const RenderComponent2D* renderable, const Transform2D* transform);
		void SubmitToRenderer();


	private:
		struct Data
		{
			const RenderComponent2D* Renderable;
			const Transform2D* Transform;
		};


		struct OpaqueComparator
		{
			bool operator()(const Data& a, const Data& b) const
			{
				if (a.Renderable->Material->GetSortKey() == b.Renderable->Material->GetSortKey())
				{
					return a.Renderable->Layer < b.Renderable->Layer;
				}
				a.Renderable->Material->GetSortKey() < b.Renderable->Material->GetSortKey();
			}
		};

		struct TransparentComparator
		{
			bool operator()(const Data& a, const Data& b) const
			{
				if (a.Renderable->Layer == b.Renderable->Layer)
				{
					return a.Renderable->Material->GetSortKey() < b.Renderable->Material->GetSortKey();
				}
				return a.Renderable->Layer > b.Renderable->Layer;
			}
		};



		std::vector<Data> m_Opaque;
		std::vector<Data> m_Transparent;
	};
}