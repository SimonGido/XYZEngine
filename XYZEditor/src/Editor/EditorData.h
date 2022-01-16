#pragma once
#include "XYZ/Renderer/SpriteSheet.h"

namespace XYZ {
	namespace Editor {
		class EditorData
		{
		public:
			static void Init();
			static void Shutdown();

			inline static Ref<Texture2D>   IconsTexture;
			inline static Ref<SpriteSheet> IconsSpriteSheet;
		};
	}
}