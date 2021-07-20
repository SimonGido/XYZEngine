#include "GameLayer.h"

#include <imgui/imgui.h>

namespace XYZ {

	static void SubmitGridToRenderer(uint32_t width, uint32_t height, const glm::vec2& cellSize, const glm::vec2& center, const glm::vec4& color)
	{
		glm::vec2 leftBottom = center - glm::vec2(cellSize.x * width, cellSize.y * height) / 2.0f;
		float lineWidth = width * cellSize.x;
		float lineHeight = height * cellSize.y;
		for (uint32_t i = 0; i < width + 1; ++i)
		{
			float offset = cellSize.x * i;
			glm::vec3 p0 = glm::vec3(leftBottom.x + offset, leftBottom.y, 0.0f);
			glm::vec3 p1 = glm::vec3(p0.x, p0.y + lineHeight, 0.0f);
			Renderer2D::SubmitLine(p0, p1, color);
		}
		for (uint32_t i = 0; i < height + 1; ++i)
		{
			float offset = cellSize.y * i;
			glm::vec3 p0 = glm::vec3(leftBottom.x, leftBottom.y + offset, 0.0f);
			glm::vec3 p1 = glm::vec3(p0.x + lineWidth, p0.y, 0.0f);
			Renderer2D::SubmitLine(p0, p1, color);
		}
	}

	static glm::vec3 CellPosition(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const glm::vec2& cellSize, const glm::vec2& center)
	{
		glm::vec2 leftBottom = center - glm::vec2(cellSize.x * width, cellSize.y * height) / 2.0f;
		return glm::vec3(
			leftBottom.x + (x * cellSize.x) + (cellSize.x / 2.0f), 
			leftBottom.y + (y * cellSize.y) + (cellSize.y / 2.0f), 
			0.0f
		);
	}

	static std::pair<uint32_t, uint32_t> WorldPositionToGridCoords(const glm::vec2& pos, uint32_t width, uint32_t height, const glm::vec2& cellSize, const glm::vec2& center)
	{
		glm::vec2 leftBottom = center - glm::vec2(cellSize.x * width, cellSize.y * height) / 2.0f;
		glm::vec2 coords = pos - leftBottom;
		
		coords.x = std::clamp(coords.x, 0.0f, (float)width);
		coords.y = std::clamp(coords.y, 0.0f, (float)height);

		return { 
			(uint32_t)(std::floor(coords.x / cellSize.x)),
			(uint32_t)(std::floor(coords.y / cellSize.y))
		};
	}

	static glm::vec2 GetMouseViewportPosition(const OrthographicCameraController& controller)
	{
		auto [x, y] = Input::GetMousePosition();
		auto width = Application::Get().GetWindow().GetWidth();
		auto height = Application::Get().GetWindow().GetHeight();

		auto bounds = controller.GetBounds();
		auto pos = controller.GetCamera().GetPosition();
		x = (x / width) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
		y = bounds.GetHeight() * 0.5f - (y / height) * bounds.GetHeight();
		return {pos.x +  x, pos.y + y };
	}
	GameLayer::GameLayer()
		:
		m_Timestep(0.0f),
		m_MousePosition(0.0f),
		m_CameraController(16.0f / 9.0f),
		m_Grid(512, 512)
	{
	}

	GameLayer::~GameLayer()
	{
	}

	void GameLayer::OnAttach()
	{
		auto& app = Application::Get();
		app.GetImGuiLayer()->EnableDockspace(false);
		app.GetImGuiLayer()->BlockEvents(false);
	}

	void GameLayer::OnDetach()
	{
	}

	void GameLayer::OnUpdate(Timestep ts)
	{
		m_Timestep = ts;
		m_CameraController.OnUpdate(ts);

		Renderer::Clear();
		Renderer::SetClearColor({ 0.1f,0.1f,0.1f,1.0f });
		Renderer2D::BeginScene(m_CameraController.GetCamera().GetViewProjectionMatrix(), m_CameraController.GetCamera().GetPosition());
		Renderer2D::SubmitQuad(glm::vec3(0.0f),glm::vec2(1.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
		
		SubmitGridToRenderer(512, 512, glm::vec2(0.1f), glm::vec2(0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		Renderer2D::SubmitCircle(glm::vec3(m_MousePosition, 0.0f), 0.1f, 10);
		if (Input::IsMouseButtonPressed(MouseCode::MOUSE_BUTTON_LEFT))
		{
			auto [xCoord, yCoord] = WorldPositionToGridCoords(m_MousePosition, 512, 512, glm::vec2(0.1f), glm::vec2(0.0f));
			m_Grid.GetParticle(xCoord, yCoord).m_Active = true;
		}
		else if(Input::IsMouseButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
		{
			auto [xCoord, yCoord] = WorldPositionToGridCoords(m_MousePosition, 512, 512, glm::vec2(0.1f), glm::vec2(0.0f));
			m_Grid.GetParticle(xCoord, yCoord).m_Active = true;
			m_Grid.GetParticle(xCoord, yCoord).m_IsWater = true;
		}
		for (uint32_t x = 0; x < m_Grid.GetWidth(); ++x)
		{
			for (uint32_t y = 0; y < m_Grid.GetHeight(); ++y)
			{
				auto& particle = m_Grid.GetParticle(x, y);
				if (particle.m_Active)
				{				
					glm::vec4 color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
					if (particle.m_IsWater)
						color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
					Renderer2D::SubmitQuad(
						CellPosition(x, y, 512, 512, glm::vec2(0.1f), glm::vec2(0.0f)),
						glm::vec2(0.1f),
						color
					);
					particle.Update(x, y, m_Grid);
				}
			}
		}

		Renderer2D::Flush();
		Renderer2D::FlushLines();

		m_RendererStats = Renderer2D::GetStats();
		Renderer2D::EndScene();
	}

	void GameLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&GameLayer::onWindowResize, this));
		
		m_CameraController.OnEvent(event);
	}

	void GameLayer::OnImGuiRender()
	{
		if (ImGui::Begin("Debug"))
		{
			ImGui::Text("Performance: ");
			ImGui::SameLine();
			ImGui::Text("%f s", m_Timestep);

			ImGui::Text("Quad Drawcalls: ");
			ImGui::SameLine();
			ImGui::Text("%u", m_RendererStats.DrawCalls);

			ImGui::Text("Line Drawcalls: ");
			ImGui::SameLine();
			ImGui::Text("%u", m_RendererStats.LineDrawCalls);

			m_MousePosition = GetMouseViewportPosition(m_CameraController);
		}
		ImGui::End();
	}

	bool GameLayer::onWindowResize(WindowResizeEvent& event)
	{
		return false;
	}
}