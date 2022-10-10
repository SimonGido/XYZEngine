#pragma once

#include "SceneEntity.h"
#include "Scene.h"

#include "XYZ/Utils/Math/Ray.h"


namespace XYZ {
	class SceneIntersection
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