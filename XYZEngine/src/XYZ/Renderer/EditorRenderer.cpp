#include "stdafx.h"
#include "EditorRenderer.h"

#include "Renderer2D.h"
#include "Renderer.h"


namespace XYZ {
	struct EditorRendererData
	{
		struct EditorSpriteDrawCommand
		{
			EditorSpriteRenderer* Sprite;
			TransformComponent* Transform;
		};
		struct EditorAABBDrawCommand
		{
			glm::vec3 Min;
			glm::vec3 Max;
			glm::vec4 Color;
		};
		struct EditorLineDrawCommand
		{
			glm::vec3 P0;
			glm::vec3 P1;
			glm::vec4 Color;
		};

		std::vector<EditorSpriteDrawCommand> EditorSpriteDrawList;
		std::vector<EditorAABBDrawCommand>   EditorAABBDrawList;
		std::vector<EditorLineDrawCommand>   EditorLineDrawList;

		Ref<RenderPass> Pass;
		glm::mat4 ViewProjectionMatrix;
		glm::vec3 ViewPosition;
	};

	static EditorRendererData s_Data;

	void EditorRenderer::BeginPass(const Ref<RenderPass>& pass, const glm::mat4& viewProjection, const glm::vec3& viewPos)
	{
		XYZ_ASSERT(!s_Data.Pass.Raw(), "Forgot to call end pass");
		s_Data.Pass = pass;
		s_Data.ViewProjectionMatrix = viewProjection;
		s_Data.ViewPosition = viewPos;
	}

	void EditorRenderer::EndPass()
	{
		Renderer::BeginRenderPass(s_Data.Pass, false);
		Renderer2D::BeginScene(s_Data.ViewProjectionMatrix, s_Data.ViewPosition);

		for (auto& dc : s_Data.EditorSpriteDrawList)
		{
			Renderer2D::SetMaterial(dc.Sprite->Material);
			uint32_t textureID = Renderer2D::SetTexture(dc.Sprite->SubTexture->GetTexture());
			Renderer2D::SubmitQuad(dc.Transform->WorldTransform, dc.Sprite->SubTexture->GetTexCoords(), textureID, dc.Sprite->Color);
		}
		for (auto& dc : s_Data.EditorAABBDrawList)
		{
			glm::vec3 topLeft = { dc.Min.x, dc.Max.y, dc.Min.z };
			glm::vec3 topRight = { dc.Max.x, dc.Max.y, dc.Min.z };
			glm::vec3 bottomLeft = { dc.Min.x, dc.Min.y, dc.Min.z };
			glm::vec3 bottomRight = { dc.Max.x, dc.Min.y, dc.Min.z };

			Renderer2D::SubmitLine(topLeft, topRight, dc.Color);
			Renderer2D::SubmitLine(topRight, bottomRight, dc.Color);
			Renderer2D::SubmitLine(bottomRight, bottomLeft, dc.Color);
			Renderer2D::SubmitLine(bottomLeft, topLeft, dc.Color);
		}
		for (auto& dc : s_Data.EditorLineDrawList)
		{
			Renderer2D::SubmitLine(dc.P0, dc.P1, dc.Color);
		}
		Renderer2D::Flush();
		Renderer2D::FlushLines();

		s_Data.EditorSpriteDrawList.clear();
		s_Data.EditorAABBDrawList.clear();
		s_Data.EditorLineDrawList.clear();

		Renderer::EndRenderPass();
		s_Data.Pass.Reset();
	}

	void EditorRenderer::SubmitEditorSprite(EditorSpriteRenderer* sprite, TransformComponent* transform)
	{
		s_Data.EditorSpriteDrawList.push_back({ sprite,transform });
	}

	void EditorRenderer::SubmitEditorAABB(TransformComponent* transform, const glm::vec4& color)
	{
		auto [translation, rotation, scale] = transform->GetWorldComponents();
		glm::vec3 bottomLeft = { translation.x - scale.x / 2,translation.y - scale.y / 2, translation.z };
		glm::vec3 topRight = { translation.x + scale.x / 2,translation.y + scale.y / 2, translation.z };
		s_Data.EditorAABBDrawList.push_back({ bottomLeft, topRight , color });
	}

	void EditorRenderer::SubmitEditorAABB(const glm::vec3& min, const glm::vec3& max, const glm::vec4& color)
	{
		s_Data.EditorAABBDrawList.push_back({ min, max , color });
	}

	void EditorRenderer::SubmitEditorLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
	{
		s_Data.EditorLineDrawList.push_back({ p0, p1, color });
	}

	void EditorRenderer::SubmitEditorCircle(const glm::vec3& pos, float radius, uint32_t sides, const glm::vec4& color)
	{
		float step = 360 / sides;
		for (int a = step; a < 360 + step; a += step)
		{
			float before = glm::radians((float)(a - step));
			float heading = glm::radians((float)a);

			s_Data.EditorLineDrawList.push_back({
				glm::vec3(pos.x + std::cos(before) * radius, pos.y + std::sin(before) * radius, pos.z),
				glm::vec3(pos.x + std::cos(heading) * radius, pos.y + std::sin(heading) * radius, pos.z),
				color
				});
		}
	}
}