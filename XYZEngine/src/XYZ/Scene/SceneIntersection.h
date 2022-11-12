#pragma once

#include "SceneEntity.h"
#include "Scene.h"
#include "Prefab.h"

#include "XYZ/Utils/Math/Ray.h"


namespace XYZ {
	class XYZ_API SceneIntersection
	{
	public:
		struct HitData
		{
			SceneEntity Entity;
			float		Distance;
		};

		static std::deque<HitData> Intersect(const Ray& ray, Ref<Scene> scene);
	};
}