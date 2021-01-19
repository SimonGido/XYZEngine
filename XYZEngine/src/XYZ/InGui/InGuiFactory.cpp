#include "stdafx.h"
#include "InGuiFactory.h"

namespace XYZ {


	void InGuiFactory::GenerateWindow(const char* text, InGuiWindow& window, const glm::vec4& color, const InGuiRenderData& renderData)
	{
		window.Mesh.Quads.clear();
		window.Mesh.Lines.clear();
		window.Mesh.Quads.push_back(
			{
				color,
				renderData.SubTexture[InGuiRenderData::WINDOW]->GetTexCoords(),
				{window.Position, 0.0f},
				window.Size,
				InGuiRenderData::TextureID
			});
	}
}