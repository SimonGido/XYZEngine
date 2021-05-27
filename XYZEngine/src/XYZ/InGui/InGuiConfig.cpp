#include "stdafx.h"
#include "InGuiConfig.h"

namespace XYZ {
	InGuiConfig::InGuiConfig()
		:
		WindowPadding(5.0f),
		MenuItemSize(100.0f, 25.0f),
		DockspaceNodeSize(75.0f, 50.0f),
		DockspaceNodeOffset(20.0f),
		TabLabelOffset(10.0f),
		WindowFrameThickness(5.0f),
		PanelHeight(25.0f),
		TreeNodeOffset(25.0f),
		TabBarHeight(25.0f),
		MenuBarHeight(25.0f),
		LabelOffset(5.0f),
		ResizeThreshhold(10.0f)
	{
		Material = Ref<XYZ::Material>::Create(Shader::Create("Assets/Shaders/InGui/InGuiShader.glsl"));
		Material->Set("u_Color", glm::vec4(1.0f));

		LineShader = Shader::Create("Assets/Shaders/InGui/InGuiLineShader.glsl");

		for (size_t i = 1; i < NumColors; i += 2)
		{
			Colors[i] = glm::vec4( 0.3f, 0.5f, 0.8f, 1.0f );
		}

		Colors[ButtonColor]				= glm::vec4(0.2f, 0.3f, 0.6f, 1.0f);
		Colors[CheckboxColor]			= glm::vec4(0.2f, 0.2f, 0.4f, 1.0f);
		Colors[SliderColor]				= glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
		Colors[SliderHighlight]			= glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
		Colors[SliderHandleColor]		= glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
		Colors[WindowColor]				= glm::vec4(0.3f, 0.4f, 0.5f, 0.7f);
		Colors[WindowPanelColor]		= glm::vec4(0.1f, 0.1f, 0.15f, 1.0f);
		Colors[MenuColor]				= glm::vec4(0.1f, 0.1f, 0.1f, 0.4f);
		Colors[TabColor]				= glm::vec4(0.1f, 0.1f, 0.15f, 1.0f);
		Colors[InputColor]				= glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
		Colors[ImageColor]				= glm::vec4(1.0f);
		Colors[TextColor]				= glm::vec4(1.0f);
		Colors[TextHighlight]			= glm::vec4(0.8f, 0.7f, 1.0f, 1.0f);
		Colors[WindowFrameColor]		= glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
		Colors[DockspaceNodeColor]		= glm::vec4(0.1f, 0.3f, 0.5f, 0.5f);
	}
}