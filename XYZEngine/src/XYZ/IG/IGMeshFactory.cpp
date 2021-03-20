#include "stdafx.h"

#include "IGMeshFactory.h"
#include "IGUIElements.h"


namespace XYZ {

	namespace Helper {

		static void CenterText(IGMesh& mesh, size_t startQuad, const glm::vec2& pos, const glm::vec2& size, const glm::vec2& textSize, IGTextCenter center)
		{
			switch (center)
			{
			case XYZ::IGTextCenter::Left:
				for (size_t i = startQuad; i < mesh.Quads.size(); ++i)
				{
					mesh.Quads[i].Position.y += std::floor((size.y / 2.0f) + (textSize.y / 2.0f));
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
					mesh.Quads[i].Position.x += std::floor((size.x / 2.0f) - (textSize.x / 2.0f));
					mesh.Quads[i].Position.y += std::floor((size.y / 2.0f) + (textSize.y / 2.0f));
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

		static void GenerateQuad(IGMesh& mesh, const glm::vec4& color, const glm::vec2& size, const glm::vec2& position, Ref<SubTexture> subTexture, uint32_t textureID, uint32_t scissorIndex)
		{
			const glm::vec4& oldTex = subTexture->GetTexCoords();
			glm::vec4 texCoords = {
				oldTex.x, oldTex.w, oldTex.z, oldTex.y
			};
			mesh.Quads.push_back(
				{
					color,
					texCoords,
				{position, 0.0f},
				size,
				textureID,
				scissorIndex
				});
		}

		static glm::vec2 GetMaxSize(const glm::vec2& firstMinPosition, const glm::vec2& firstMaxPosition, const glm::vec2& secondMinPosition, const glm::vec2& secondMaxPosition)
		{
			glm::vec2 minPosition = firstMinPosition;
			if (minPosition.x > secondMinPosition.x)
				minPosition.x = secondMinPosition.x;
			if (minPosition.y > secondMinPosition.y)
				minPosition.y = secondMinPosition.y;

			glm::vec2 maxPosition = firstMaxPosition;
			if (maxPosition.x < secondMaxPosition.x)
				maxPosition.x = secondMaxPosition.x;
			if (maxPosition.y < secondMaxPosition.y)
				maxPosition.y = secondMaxPosition.y;

			return maxPosition - minPosition;
		}

		static glm::vec2 GenerateLabeledQuad(const char* label, const glm::vec4& labelColor, const IGMeshFactoryData& data)
		{
			if (!data.RenderData->Rebuild)
				return data.Element->Size;

			glm::vec2 absolutePosition = data.Element->GetAbsolutePosition();
			Helper::GenerateQuad(
				*data.Mesh, data.Element->Color,
				data.Element->Size, absolutePosition,
				data.RenderData->SubTextures[data.SubTextureIndex],
				IGRenderData::TextureID,
				data.ScissorIndex
			);

			size_t oldQuadCount = data.Mesh->Quads.size();
			glm::vec2 textPosition = { std::floor(absolutePosition.x + data.Element->Size.x), std::floor(absolutePosition.y) };
			glm::vec2 textSize = { data.Element->Size.x, data.Element->Size.y };

			glm::vec2 genTextSize = Helper::GenerateTextMesh(
				label, data.RenderData->Font, labelColor,
				textPosition, textSize, *data.Mesh, IGRenderData::FontTextureID, 1000, data.ScissorIndex
			);

			Helper::CenterText(*data.Mesh, oldQuadCount,
				data.Element->Position, data.Element->Size,
				genTextSize, data.Element->Style.LabelCenter
			);

			return Helper::GetMaxSize(
				absolutePosition,
				absolutePosition + data.Element->Size,
				textPosition - glm::vec2(0.0f, genTextSize.y),
				textPosition + glm::vec2(genTextSize.x, 0.0f)
			);
		}

		static glm::vec2 GenerateLabeledQuad(const char* label,
			const glm::vec4& labelColor,
			const glm::vec4& color,
			const glm::vec2& position,
			const glm::vec2& size,
			const glm::vec2& textSize,
			uint32_t subTextureIndex,
			uint32_t scissorIndex,
			IGMesh* mesh,
			IGRenderData* renderData,
			IGTextCenter center)
		{
			if (!renderData->Rebuild)
				return size;

			Helper::GenerateQuad(
				*mesh, color,
				size, position,
				renderData->SubTextures[subTextureIndex],
				IGRenderData::TextureID,
				scissorIndex
			);

			size_t oldQuadCount = mesh->Quads.size();
			glm::vec2 textPosition = { std::floor(position.x + size.x), std::floor(position.y) };

			glm::vec2 genTextSize = Helper::GenerateTextMesh(
				label, renderData->Font, labelColor,
				textPosition, textSize, *mesh, IGRenderData::FontTextureID, 1000, scissorIndex
			);

			Helper::CenterText(*mesh, oldQuadCount,
				position, size,
				genTextSize, center
			);

			return Helper::GetMaxSize(
				position,
				position + size,
				textPosition - glm::vec2(0.0f, genTextSize.y),
				textPosition + glm::vec2(genTextSize.x, 0.0f)
			);
		}
	}

	template<>
	glm::vec2 IGMeshFactory::GenerateUI<IGWindow>(const char* label, const glm::vec4& labelColor, const IGMeshFactoryData& data)
	{
		if (!data.RenderData->Rebuild)
			return data.Element->Size;

		IGWindow* window = static_cast<IGWindow*>(data.Element);

		glm::vec2 absolutePosition = window->GetAbsolutePosition();
		if (!IS_SET(window->Flags, IGWindow::Collapsed))
		{
			if (window->Style.RenderFrame)
				Helper::GenerateFrame(*data.Mesh, absolutePosition, window->Size, window->FrameColor);

			data.Mesh->Quads.push_back({
				window->Color,
				data.RenderData->SubTextures[data.SubTextureIndex]->GetTexCoords(),
				{absolutePosition, 0.0f},
				window->Size,
				IGRenderData::TextureID
				});
		}
		data.Mesh->Quads.push_back({
			window->Color,
			data.RenderData->SubTextures[IGRenderData::Button]->GetTexCoords(),
			{absolutePosition, 0.0f},
			{window->Size.x, IGWindow::PanelHeight },
			IGRenderData::TextureID
			});
		data.Mesh->Quads.push_back({
			window->Color,
			data.RenderData->SubTextures[IGRenderData::MinimizeButton]->GetTexCoords(),
			{absolutePosition.x + window->Size.x - IGWindow::PanelHeight, absolutePosition.y, 0.0f},
			{IGWindow::PanelHeight, IGWindow::PanelHeight },
			IGRenderData::TextureID
			});


		size_t oldQuadCount = data.Mesh->Quads.size();
		glm::vec2 textPosition = { std::floor(window->Style.Layout.LeftPadding + absolutePosition.x), std::floor(absolutePosition.y) };
		glm::vec2 textSize = { window->Size.x, window->Size.y };
		glm::vec2 genTextSize = Helper::GenerateTextMesh(
			label, data.RenderData->Font, labelColor,
			textPosition, textSize, *data.Mesh, IGRenderData::FontTextureID, 1000, data.ScissorIndex
		);

		Helper::CenterText(*data.Mesh, oldQuadCount, window->Position,
			{ window->Size.x, IGWindow::PanelHeight },
			genTextSize, window->Style.LabelCenter
		);
		return window->Size;
	}


	template<>
	glm::vec2 IGMeshFactory::GenerateUI<IGCheckbox>(const char* label, const glm::vec4& labelColor, const IGMeshFactoryData& data)
	{
		return Helper::GenerateLabeledQuad(label, labelColor, data);
	}

	template<>
	glm::vec2 IGMeshFactory::GenerateUI<IGButton>(const char* label, const glm::vec4& labelColor, const IGMeshFactoryData& data)
	{
		return Helper::GenerateLabeledQuad(label, labelColor, data);
	}

	template<>
	glm::vec2 IGMeshFactory::GenerateUI<IGSlider>(const char* label, const glm::vec4& labelColor, const IGMeshFactoryData& data)
	{
		IGSlider* slider = static_cast<IGSlider*>(data.Element);

		glm::vec2 absolutePosition = data.Element->GetAbsolutePosition();
		glm::vec2 handleSize = glm::vec2(data.Element->Size.y, data.Element->Size.y);
		glm::vec2 handlePosition = absolutePosition + glm::vec2((slider->Size.x - slider->Size.y) * slider->Value, 0.0f);

		char value[6];
		int ret = snprintf(value, sizeof(value), "%f", slider->Value);

		glm::vec2 result = Helper::GenerateLabeledQuad(label, labelColor, data);
		Helper::GenerateQuad(*data.Mesh, slider->Color,
			handleSize, handlePosition,
			data.RenderData->SubTextures[IGRenderData::SliderHandle],
			IGRenderData::TextureID, data.ScissorIndex
		);

		size_t oldQuadCount = data.Mesh->Quads.size();
		glm::vec2 valueTextSize = Helper::GenerateTextMesh(value, data.RenderData->Font, labelColor,
			absolutePosition, data.Element->Size, *data.Mesh,
			IGRenderData::FontTextureID, 1000, data.ScissorIndex
		);
		Helper::CenterText(*data.Mesh, oldQuadCount, absolutePosition, slider->Size, valueTextSize, IGTextCenter::Middle);

		return result;
	}

	template<>
	glm::vec2 IGMeshFactory::GenerateUI<IGFloat>(const char* label, const glm::vec4& labelColor, const IGMeshFactoryData& data)
	{
		IGFloat* floatInput = static_cast<IGFloat*>(data.Element);
		glm::vec2 absolutePosition = data.Element->GetAbsolutePosition();
		glm::vec2 textPosition = absolutePosition + glm::vec2(floatInput->Style.Layout.LeftPadding, 0.0f);
		glm::vec2 textSize = data.Element->Size - glm::vec2(floatInput->Style.Layout.RightPadding + floatInput->Style.Layout.LeftPadding);

		glm::vec2 result = Helper::GenerateLabeledQuad(label, labelColor, data);

		size_t oldQuadCount = data.Mesh->Quads.size();
		glm::vec2 valueTextSize = Helper::GenerateTextMesh(
			floatInput->GetBuffer(), data.RenderData->Font, labelColor,
			textPosition, textSize, *data.Mesh,
			IGRenderData::FontTextureID, 1000, data.ScissorIndex
		);
		Helper::CenterText(*data.Mesh, oldQuadCount, absolutePosition, floatInput->Size, valueTextSize, IGTextCenter::Left);

		return result;
	}

	template<>
	glm::vec2 IGMeshFactory::GenerateUI<IGText>(const char* text, const glm::vec4& labelColor, const IGMeshFactoryData& data)
	{
		size_t oldQuadCount = data.Mesh->Quads.size();
		glm::vec2 absolutePosition = data.Element->GetAbsolutePosition();
		glm::vec2 textSize = Helper::GenerateTextMesh(text, data.RenderData->Font, labelColor,
			absolutePosition, data.Element->Size, *data.Mesh,
			IGRenderData::FontTextureID, 1000, data.ScissorIndex);

		Helper::CenterText(*data.Mesh, oldQuadCount, absolutePosition, data.Element->Size, textSize, IGTextCenter::Middle);
		return data.Element->Size;
	}

	template <>
	glm::vec2 IGMeshFactory::GenerateUI<IGTree>(const char* text, const glm::vec4& labelColor, const IGMeshFactoryData& data)
	{
		constexpr float nodeOffset = 25.0f;

		IGTree* tree = static_cast<IGTree*>(data.Element);
		glm::vec2 absolutePosition = tree->GetAbsolutePosition();
		glm::vec2 textSize = tree->Size;
		if (tree->Parent)
			textSize.x = tree->Parent->Size.x - tree->Parent->Style.Layout.LeftPadding - tree->Parent->Style.Layout.RightPadding;

		auto& nodes = tree->Hierarchy.GetFlatNodes();
		uint32_t currentDepth = 0;
		glm::vec2 offset = glm::vec2(0.0f);
		tree->Hierarchy.Traverse([&](void* parent, void* child) ->bool {

			IGTree::IGTreeItem* childItem = static_cast<IGTree::IGTreeItem*>(child);
			if (nodes[childItem->ID].Depth > currentDepth)
			{
				offset.x += nodeOffset;
				textSize.x -= nodeOffset;
			}
			while (nodes[childItem->ID].Depth < currentDepth)
			{
				offset.x -= nodeOffset;
				textSize.x += nodeOffset;
				currentDepth--;
			}
			bool open = true;
			if (parent)
			{
				IGTree::IGTreeItem* parentItem = static_cast<IGTree::IGTreeItem*>(parent);
				open = parentItem->Open;
			}
			if (open)
			{
				childItem->Position = absolutePosition + offset;
				glm::vec2 genSize = Helper::GenerateLabeledQuad(
					childItem->Label.c_str(), childItem->Color, childItem->Color,
					childItem->Position, data.Element->Size, textSize, data.SubTextureIndex,
					data.ScissorIndex, data.Mesh, data.RenderData, IGTextCenter::Left
				);
				offset.y += genSize.y;
			}
			else
				childItem->Open = false;


			currentDepth = nodes[childItem->ID].Depth;
			return false;
			});

		return offset;
	}
	template <>
	glm::vec2 IGMeshFactory::GenerateUI<IGGroup>(const char* label, const glm::vec4& labelColor, const IGMeshFactoryData& data)
	{
		IGGroup* group = static_cast<IGGroup*>(data.Element);
		glm::vec2 absolutePosition = group->GetAbsolutePosition();

		glm::vec2 result = Helper::GenerateLabeledQuad(
			nullptr, labelColor, group->Color,
			absolutePosition, { group->Size.x, IGGroup::PanelHeight }, group->Size, data.SubTextureIndex,
			data.ScissorIndex, data.Mesh, data.RenderData, IGTextCenter::Left
		);


		uint32_t subTextureIndex = IGRenderData::RightArrow;
		if (group->Open)
			subTextureIndex = IGRenderData::DownArrow;
		
		Helper::GenerateLabeledQuad(
			label, labelColor, group->Color,
			absolutePosition, { IGGroup::PanelHeight, IGGroup::PanelHeight }, group->Size, subTextureIndex,
			data.ScissorIndex, data.Mesh, data.RenderData, IGTextCenter::Left
		);

		return result;
	}
}