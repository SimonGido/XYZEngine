#include "stdafx.h"
#include "InGuiMeshFactory.h"

namespace XYZ {

	namespace Helper {

		static void CenterText(IGMesh& mesh, size_t startQuad, const glm::vec2& pos, const glm::vec2& size, float textLength, IGTextCenter center)
		{
			switch (center)
			{
			case XYZ::IGTextCenter::Left:
				for (size_t i = startQuad; i < mesh.Quads.size(); ++i)
				{
					mesh.Quads[i].Position.y += std::floor((size.y / 2.0f) + (size.y / 2.0f));
				}
				break;
			case XYZ::IGTextCenter::Right:
				//for (size_t i = startQuad; i < mesh.Quads.size(); ++i)
				//{
				//	mesh.Quads[i].Position.x += 
				//	mesh.Quads[i].Position.y += std::floor((size.y / 2.0f) + (size.y / 2.0f));
				//}
				break;
			case XYZ::IGTextCenter::Middle:
				for (size_t i = startQuad; i < mesh.Quads.size(); ++i)
				{
					mesh.Quads[i].Position.x += std::floor((size.x / 2.0f) - (size.x / 2.0f));
					mesh.Quads[i].Position.y += std::floor((size.y / 2.0f) + (size.y / 2.0f));
				}
				break;
			case XYZ::IGTextCenter::None:
				break;
			default:
				break;
			}
		}

		static glm::vec2 GenerateTextMesh(
			const char* source,
			const Ref<Font>& font,
			const glm::vec4& color,
			const glm::vec2& pos,
			const glm::vec2& size,
			IGMesh& mesh,
			uint32_t textureID,
			uint32_t maxCount,
			uint32_t scissorIndex
		)
		{
			if (!source)
				return { 0.0f, 0.0f };

			float width = 0.0f;
			float xCursor = 0.0f;
			float yCursor = 0.0f;

			uint32_t counter = 0;
			while (source[counter] != '\0' && counter < maxCount)
			{
				auto& character = font->GetCharacter(source[counter]);
				if (source[counter] == '\n')
				{
					width = xCursor;
					yCursor += font->GetLineHeight();
					xCursor = 0.0f;
					counter++;
					continue;
				}

				if (xCursor + (float)character.XAdvance >= size.x)
					break;

				glm::vec2 charSize = {
					character.X1Coord - character.X0Coord,
					character.Y1Coord - character.Y0Coord
				};

				glm::vec2 charOffset = { character.XOffset, character.YOffset };
				glm::vec2 charPosition = { pos.x + xCursor + charOffset.x, pos.y + yCursor - charOffset.y };
				glm::vec4 charTexCoord = {
					(float)(character.X0Coord) / (float)(font->GetWidth()), (float)(character.Y0Coord) / (float)(font->GetHeight()),
					(float)(character.X1Coord) / (float)(font->GetWidth()), (float)(character.Y1Coord) / (float)(font->GetHeight())
				};		

				mesh.Quads.push_back({ color, charTexCoord, glm::vec3{charPosition, 0.0f}, charSize, textureID, scissorIndex });

				xCursor += character.XAdvance;
				counter++;
			}
			if (width < xCursor)
				width = xCursor;
			return { width, yCursor + font->GetLineHeight() };
		}

		static void GenerateFrame(IGMesh& mesh, const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		{
			IGLine line;
			line.Color = color;

			line.P0 = glm::vec3(position, 0.0f);
			line.P1 = glm::vec3(position.x + size.x, position.y, 0.0f);
			mesh.Lines.push_back(line);

			line.P0 = glm::vec3(position.x + size.x, position.y, 0.0f);
			line.P1 = glm::vec3(position.x + size.x, position.y + size.y, 0.0f);
			mesh.Lines.push_back(line);

			line.P0 = glm::vec3(position.x + size.x, position.y + size.y, 0.0f);
			line.P1 = glm::vec3(position.x, position.y + size.y, 0.0f);
			mesh.Lines.push_back(line);

			line.P0 = glm::vec3(position.x, position.y + size.y, 0.0f);
			line.P1 = glm::vec3(position, 0.0f);
			mesh.Lines.push_back(line);
		}
	}

	template<>
	glm::vec2 IGMeshFactory::GenerateUI<IGElementType::Window>(const char* label, const glm::vec4& labelColor, const IGMeshFactoryData& data)
	{
		IGWindow* window = static_cast<IGWindow*>(data.Element);
		if (!IS_SET(window->Flags, IGWindow::Collapsed))
		{
			if (window->Style.RenderFrame)
				Helper::GenerateFrame(*data.Mesh, window->AbsolutePosition, window->Size, window->FrameColor);
		
			data.Mesh->Quads.push_back({
					window->Color,
					data.RenderData->SubTextures[data.SubTextureIndex]->GetTexCoords(),
					{window->AbsolutePosition, 0.0f},
					window->Size,
					IGRenderData::TextureID
				});
		}
		data.Mesh->Quads.push_back({
				window->Color,
				data.RenderData->SubTextures[IGRenderData::Button]->GetTexCoords(),
				{window->AbsolutePosition, 0.0f},
				{window->Size.x, IGWindow::PanelHeight },
				IGRenderData::TextureID
			});
		data.Mesh->Quads.push_back({
				window->Color,
				data.RenderData->SubTextures[IGRenderData::MinimizeButton]->GetTexCoords(),
				{window->AbsolutePosition.x + window->Size.x - IGWindow::PanelHeight, window->AbsolutePosition.y, 0.0f},
				{IGWindow::PanelHeight, IGWindow::PanelHeight },
				IGRenderData::TextureID
			});


		size_t oldQuadCount = data.Mesh->Quads.size();
		glm::vec2 textPosition = { std::floor(window->AbsolutePosition.x), std::floor(window->AbsolutePosition.y) };
		glm::vec2 textSize = { window->Size.x, window->Size.y };
		glm::vec2 genTextSize = Helper::GenerateTextMesh(
			label, data.RenderData->Font, labelColor,
			textPosition, textSize, *data.Mesh, IGRenderData::FontTextureID, 1000, 0
		);
		Helper::CenterText(*data.Mesh, oldQuadCount, window->Position, 
			{ window->Size.x, IGWindow::PanelHeight }, 
			genTextSize.x, window->Style.LabelCenter
		);
		return window->Size;
	}
}