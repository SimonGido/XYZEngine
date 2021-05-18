#include "stdafx.h"
#include "InGuiConfig.h"

namespace XYZ {
	InGuiConfig::InGuiConfig()
		:
		WindowPadding(5.0f),
		MenuItemSize(100.0f,25.0f),
		PanelHeight(25.0f),
		MenuBarHeight(25.0f),
		LabelOffset(5.0f)
	{
		Material = Ref<XYZ::Material>::Create(Shader::Create("Assets/Shaders/InGui/InGuiShader.glsl"));
		Material->Set("u_Color", glm::vec4(1.0f));

		LineShader = Shader::Create("Assets/Shaders/InGui/InGuiLineShader.glsl");

		Colors[DefaultColor] = glm::vec4{ 1.0f };
		for (size_t i = 1; i < NumColors; i++)
		{
			Colors[i] = glm::vec4( 1.4f, 1.9f, 2.1, 1.0f );
		}
	}
}