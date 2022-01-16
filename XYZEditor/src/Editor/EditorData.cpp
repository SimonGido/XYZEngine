#include "stdafx.h"
#include "EditorData.h"

namespace XYZ {
	namespace Editor {
		void EditorData::Init()
		{
			IconsTexture = Texture2D::Create("Resources/Editor/icons.png");
			IconsSpriteSheet = Ref<SpriteSheet>::Create(IconsTexture, 4, 4);
		}
		void EditorData::Shutdown()
		{
			IconsTexture.Reset();
			IconsSpriteSheet.Reset();
		}
	}
}