#pragma once
#include <glm/glm.hpp>

#include "XYZ/Renderer/SpriteSheet.h"

namespace XYZ {
	namespace Editor {
		
		struct EditorData
		{
			void Init();
			void Shutdown();

			Ref<Texture2D>   IconsTexture;
			Ref<SpriteSheet> IconsSpriteSheet;

			enum Colors
			{
				BoundingBox,
				Collider2D,
				IconColor,
				IconHoverColor,
				IconClickColor,
				DisabledColor,
				NumColors
			};
			glm::vec4 Color[NumColors];
		};
		using ED = EditorData;
	}
}