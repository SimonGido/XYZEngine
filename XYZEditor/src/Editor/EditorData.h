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
			
			enum Sprites
			{
				MoveIcon = 32,
				RotateIcon,
				ScaleIcon,
				CursorIcon,
				MediaPlayIcon,
				AnimationIcon = MediaPlayIcon + 3 + 1,
				SceneIcon,
				JpgIcon,
				PngIcon,
				MediaNextIcon,
				ArrowIcon = MediaNextIcon + 3 + 1,
				MeshIcon,
				TextureIcon,
				StopIcon,
				MediaBeginningIcon,
				FolderIcon = MediaBeginningIcon + 3 + 1,
				ShaderIcon,
				MaterialIcon,
				ScriptIcon,
				MediaPauseIcon,
				NumIcons
			};
			enum Colors
			{
				BoundingBox,
				Collider2D,
				IconColor,
				IconHoverColor,
				IconClickColor,
				DisabledColor,
				ContainerSelectedItem,
				NumColors
			};
			glm::vec4 Color[NumColors];
		};
		using ED = EditorData;
	}
}