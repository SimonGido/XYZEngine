#include "stdafx.h"
#include "EditorData.h"


namespace XYZ {
	namespace Editor {
		void EditorData::Init()
		{
			IconsTexture	 = Texture2D::Create("Resources/Editor/icons.png");
			IconsSpriteSheet = Ref<SpriteSheet>::Create(IconsTexture, 8, 8);
			
			
			Color[BoundingBox]    = glm::vec4(1.0f);
			Color[Collider2D]	  = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			Color[IconColor]	  = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
			Color[IconHoverColor] = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
			Color[IconClickColor] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			Color[DisabledColor]  = glm::vec4(0.5f, 0.5f, 0.5f, 0.3f);
			Color[ContainerSelectedItem] = glm::vec4(0.1f, 0.4f, 0.5f, 1.0f);
		}

		void EditorData::Shutdown()
		{
			IconsTexture.Reset();
			IconsSpriteSheet.Reset();
		}
	
	}
}