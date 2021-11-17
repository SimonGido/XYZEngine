#include "stdafx.h"
#include "EditorRenderer.h"

#include "Renderer2D.h"
#include "Renderer.h"


namespace XYZ {

	void EditorRenderer::BeginPass(const Ref<RenderPass>& pass, const glm::mat4& viewProjection, const glm::vec3& viewPos)
	{
		XYZ_ASSERT(!m_Pass.Raw(), "Forgot to call end pass");
		m_Pass = pass;
		m_ViewProjectionMatrix = viewProjection;
		m_ViewPosition = viewPos;
	}

	void EditorRenderer::EndPass(Ref<Renderer2D> renderer2D, bool clear)
	{
		//Renderer::BeginRenderPass(m_Pass, clear);
		renderer2D->BeginScene();

		for (auto& dc : m_EditorSpriteDrawList)
		{
			renderer2D->SetMaterial(dc.Material);
			const uint32_t textureID = renderer2D->SetTexture(dc.SubTexture->GetTexture());
			renderer2D->SubmitQuad(dc.Transform, dc.SubTexture->GetTexCoords(), textureID, dc.Color);
		}
		for (auto& dc : m_EditorAABBDrawList)
		{
			glm::vec3 topLeft = { dc.Min.x, dc.Max.y, dc.Min.z };
			glm::vec3 topRight = { dc.Max.x, dc.Max.y, dc.Min.z };
			glm::vec3 bottomLeft = { dc.Min.x, dc.Min.y, dc.Min.z };
			glm::vec3 bottomRight = { dc.Max.x, dc.Min.y, dc.Min.z };

			renderer2D->SubmitLine(topLeft, topRight, dc.Color);
			renderer2D->SubmitLine(topRight, bottomRight, dc.Color);
			renderer2D->SubmitLine(bottomRight, bottomLeft, dc.Color);
			renderer2D->SubmitLine(bottomLeft, topLeft, dc.Color);
		}
		for (auto& dc : m_EditorLineDrawList)
		{
			renderer2D->SubmitLine(dc.P0, dc.P1, dc.Color);
		}
		renderer2D->Flush();
		renderer2D->FlushLines();

		m_EditorSpriteDrawList.clear();
		m_EditorAABBDrawList.clear();
		m_EditorLineDrawList.clear();

		//Renderer::EndRenderPass();
		m_Pass.Reset();
	}

	void EditorRenderer::SubmitEditorSprite(const Ref<Material>& material, const Ref<SubTexture>& subTexture, const glm::vec4& color, const glm::mat4& transform)
	{
		m_EditorSpriteDrawList.push_back({ material, subTexture, color,transform });
	}

	void EditorRenderer::SubmitEditorAABB(const glm::vec3& min, const glm::vec3& max, const glm::vec4& color)
	{
		m_EditorAABBDrawList.push_back({ min, max , color });
	}

	void EditorRenderer::SubmitEditorLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
	{
		m_EditorLineDrawList.push_back({ p0, p1, color });
	}

	void EditorRenderer::SubmitEditorCircle(const glm::vec3& pos, float radius, uint32_t sides, const glm::vec4& color)
	{
		const int step = 360 / sides;
		for (int a = step; a < 360 + step; a += step)
		{
			const float before = glm::radians((float)(a - step));
			const float heading = glm::radians((float)a);

			m_EditorLineDrawList.push_back({
				glm::vec3(pos.x + std::cos(before) * radius, pos.y + std::sin(before) * radius, pos.z),
				glm::vec3(pos.x + std::cos(heading) * radius, pos.y + std::sin(heading) * radius, pos.z),
				color
				});
		}
	}
	void EditorRenderer::SubmitEditorLineQuad(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color)
	{
		const glm::vec3 p[4] = {
			pos,
			pos + glm::vec3(size.x, 0.0f, 0.0f),
			pos + glm::vec3(size.x, size.y, 0.0f),
			pos + glm::vec3(0.0f,size.y, 0.0f)
		};
		m_EditorLineDrawList.push_back({ p[0], p[1], color });
		m_EditorLineDrawList.push_back({ p[1], p[2], color });
		m_EditorLineDrawList.push_back({ p[2], p[3], color });
		m_EditorLineDrawList.push_back({ p[3], p[0], color });
	}
}