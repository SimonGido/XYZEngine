#include "stdafx.h"
#include "InGuiConfig.h"

namespace XYZ {
	InGuiConfig::InGuiConfig()
		:
		WindowPadding(5.0f),
		MenuItemSize(100.0f,25.0f),
		TabLabelOffset(10.0f),
		WindowFrameThickness(5.0f),
		PanelHeight(25.0f),
		TreeNodeOffset(25.0f),
		TabBarHeight(25.0f),
		MenuBarHeight(25.0f),
		LabelOffset(5.0f)
	{
		Material = Ref<XYZ::Material>::Create(Shader::Create("Assets/Shaders/InGui/InGuiShader.glsl"));
		Material->Set("u_Color", glm::vec4(1.0f));

		LineShader = Shader::Create("Assets/Shaders/InGui/InGuiLineShader.glsl");

		for (size_t i = 0; i < NumColors; i += 2)
		{
			Colors[i] = glm::vec4(1.0f);
			Colors[i + 1] = glm::vec4( 1.4f, 1.9f, 2.1, 1.0f );
		}
		Colors[MenuDefault] = glm::vec4(1.0f, 1.0f, 1.0f, 0.8f);
		Colors[TextDefault] = glm::vec4(1.0f);
		Colors[TextHighlight] = glm::vec4(1.0f, 0.7f, 1.0f, 1.0f);
		Colors[LineColor] = glm::vec4(1.0f);
		Colors[TabHoover] = glm::vec4(2.0f, 2.5f, 2.8f, 1.0f);
		Colors[WindowFrameColor] = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
	}
}