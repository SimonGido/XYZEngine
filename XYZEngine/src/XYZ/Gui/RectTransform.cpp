#include "RectTransform.h"

#include "CanvasRenderer.h"
#include "Text.h"

namespace XYZ {
	static bool OnQuadRectTransformResized(ComponentResizedEvent& event)
	{
		auto entity = event.GetEntity();
		constexpr size_t quadVertexCount = 4;
		auto& transform = entity.GetComponent<RectTransform>();
		auto& renderer = entity.GetComponent<CanvasRenderer>();
		auto& texCoord = renderer.SubTexture->GetTexCoords();

		glm::vec2 texCoords[quadVertexCount] = {
			{texCoord.x,texCoord.y},
			{texCoord.z,texCoord.y},
			{texCoord.z,texCoord.w},
			{texCoord.x,texCoord.w}
		};
		glm::vec3 vertices[quadVertexCount] = {
			{  -transform.Size.x / 2.0f,  -transform.Size.y / 2.0f, 0.0f},
			{   transform.Size.x / 2.0f,  -transform.Size.y / 2.0f, 0.0f},
			{   transform.Size.x / 2.0f,   transform.Size.y / 2.0f, 0.0f},
			{  -transform.Size.x / 2.0f,   transform.Size.y / 2.0f, 0.0f}
		};
		for (size_t i = 0; i < quadVertexCount; ++i)
			renderer.Mesh.Vertices.push_back(Vertex{ renderer.Color, vertices[i], texCoords[i] });
		return false;
	}

	static bool OnTextRectTransformResized(ComponentResizedEvent& event)
	{
		auto& entity = event.GetEntity();
		auto& transform = entity.GetComponent<RectTransform>();
		auto& renderer = entity.GetComponent<CanvasRenderer>();
		auto& text = entity.GetComponent<Text>();

		size_t oldMeshSize = renderer.Mesh.Vertices.size();
		int32_t height = 0.0f;
		int32_t xCursor = 0.0f;
		int32_t yCursor = 0.0f;
		uint32_t counter = 0;
		for (auto c : text.Source)
		{
			auto& character = text.Font->GetCharacter(c);
			if (xCursor + character.XAdvance >= transform.Size.x)
				break;

			glm::vec2 charSize = {
				character.X1Coord - character.X0Coord,
				character.Y1Coord - character.Y0Coord
			};
			if (height < character.Y1Coord - character.Y0Coord)
				height = character.Y1Coord - character.Y0Coord;

			glm::vec2 charOffset = { character.XOffset, charSize.y - character.YOffset };
			glm::vec2 charPosition = { xCursor + charOffset.x, yCursor - charOffset.y };
			glm::vec4 charTexCoord = {
				(float)character.X0Coord / (float)text.Font->GetWidth(), (float)character.Y0Coord / (float)text.Font->GetHeight(),
				(float)character.X1Coord / (float)text.Font->GetWidth(), (float)character.Y1Coord / (float)text.Font->GetHeight()
			};

			glm::vec3 quads[4] = {
				{ charPosition.x,			   charPosition.y , 0.0f },
				{ charPosition.x + charSize.x, charPosition.y, 0.0f, },
				{ charPosition.x + charSize.x, charPosition.y + charSize.y, 0.0f, },
				{ charPosition.x,			   charPosition.y + charSize.y, 0.0f, },
			};
			glm::vec2 texCoords[4] = {
				 {charTexCoord.x, charTexCoord.w},
				 {charTexCoord.z, charTexCoord.w},
				 {charTexCoord.z, charTexCoord.y},
				 {charTexCoord.x, charTexCoord.y},
			};
			for (uint32_t i = 0; i < 4; ++i)
				renderer.Mesh.Vertices.push_back(Vertex{ text.Color, quads[i], texCoords[i] });

			xCursor += character.XAdvance;
			counter++;
		}
		if (text.Alignment == TextAlignment::Center)
		{
			int32_t xOffset = xCursor / 2;
			int32_t yOffset = height / 2;
			for (size_t i = oldMeshSize; i < renderer.Mesh.Vertices.size(); ++i)
			{
				renderer.Mesh.Vertices[i].Position.x -= xOffset;
				renderer.Mesh.Vertices[i].Position.y -= yOffset;
			}
		}
		return false;
	}

	RectTransform::RectTransform(const glm::vec3& position, const glm::vec2& size)
		: WorldPosition(position), Position(position), Size(size)
	{
		RegisterCallback<ComponentResizedEvent>(Hook(&RectTransform::onResize, this));
	}

	bool RectTransform::onResize(ComponentResizedEvent& event)
	{
		auto entity = event.GetEntity();
		if (entity.HasComponent<CanvasRenderer>())
		{
			entity.GetComponent<CanvasRenderer>().Mesh.Vertices.clear();
			if (entity.HasComponent<Text>())
			{
				OnTextRectTransformResized(event);
			}
			else
			{
				OnQuadRectTransformResized(event);
			}
			return true;
		}
		return false;
	}
}