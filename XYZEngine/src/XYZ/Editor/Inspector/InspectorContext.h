#pragma once
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture.h"

namespace XYZ {
	namespace Editor {

		class InspectorContext
		{
		public:
			virtual ~InspectorContext() = default;

			virtual void OnImGuiRender(Ref<Renderer2D> renderer) = 0;
		};
	}
}