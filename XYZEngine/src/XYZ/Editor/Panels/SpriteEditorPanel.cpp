#include "stdafx.h"
#include "SpriteEditorPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Utils/Math/Math.h"

#include <tpp_interface.hpp>

namespace XYZ {

	static void GetTriangulationPt(const std::vector<tpp::Delaunay::Point>& points, int keyPointIdx, const tpp::Delaunay::Point& sPoint, double& x, double& y)
	{
		if (keyPointIdx >= points.size())
		{
			x = sPoint[0]; // added Steiner point, it's data copied to sPoint
			y = sPoint[1];
		}
		else
		{
			// point from original data
			x = sPoint[0];
			y = sPoint[1];
		}
	}

	static void CircleOfLines(InGuiMesh& mesh, const glm::vec3& pos, float radius, size_t sides, const glm::vec4& color)
	{
		float step = 360 / sides;
		for (int a = step; a < 360 + step; a += step)
		{
			float before = glm::radians((float)(a - step));
			float heading = glm::radians((float)a);

			glm::vec3 p0 = glm::vec3(pos.x + std::cos(before) * radius, pos.y + std::sin(before) * radius, pos.z);
			glm::vec3 p1 = glm::vec3(pos.x + std::cos(heading) * radius, pos.y + std::sin(heading) * radius, pos.z);

			mesh.Lines.push_back({ color, p0, p1 });
		}
	}

	SpriteEditorPanel::SpriteEditorPanel(uint32_t panelID)
		:
		m_PanelID(panelID)
	{
		InGui::Begin(m_PanelID, "Sprite Editor", glm::vec2(0.0f), glm::vec2(200.0f));
		InGui::End();

		auto flags = InGui::GetWindow(m_PanelID).Flags;
		flags &= ~InGuiWindowFlags::EventBlocking;
		InGui::SetWindowFlags(m_PanelID, flags);
	}
	void SpriteEditorPanel::SetContext(Ref<SubTexture> context)
	{
		m_Context = context;
		m_Size.x = (float)m_Context->GetTexture()->GetWidth();
		m_Size.y = (float)m_Context->GetTexture()->GetHeight();
	}
	void SpriteEditorPanel::OnInGuiRender()
	{
		if (InGui::Begin(m_PanelID, "Sprite Editor", glm::vec2(0.0f), glm::vec2(200.0f)))
		{
			if (m_Context.Raw())
			{
				glm::vec2 windowSize = InGui::GetWindow(m_PanelID).Size;
				glm::vec2 windowPosition = InGui::GetWindow(m_PanelID).Position;
				glm::vec2 position = (windowSize / 2.0f) - (m_Size / 2.0f);
				auto& layout = InGui::GetWindow(m_PanelID).Layout;

				// TODO scale it enough to support 4k ?
				InGui::BeginScrollableArea(windowSize - glm::vec2(2.0f * layout.RightPadding, 0.0f), m_ScrollOffset, 100.0f, 10.0f);
				glm::vec2 nextPos = InGui::GetPositionOfNext();
				InGui::SetPositionOfNext(windowPosition + position);
				InGui::Image(m_Size, m_Context);

				auto [mx, my] = Input::GetMousePosition();
				m_Triangle = findTriangle({ mx, my });

				for (auto& point : m_Points)
				{
					glm::vec2 relativePos = {
						windowPosition.x + (windowSize.x / 2.0f) + point.X,
						windowPosition.y + (windowSize.y / 2.0f) + point.Y
					};
					CircleOfLines(
						InGui::GetWindowScrollableOverlayMesh(m_PanelID), glm::vec3(relativePos.x, relativePos.y, 0.0f),
						sc_PointRadius, 20, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)
					);
				}
				auto& mesh = InGui::GetWindowScrollableOverlayMesh(m_PanelID);
				glm::vec2 offset = {
						windowPosition.x + (windowSize.x / 2.0f),
						windowPosition.y + (windowSize.y / 2.0f)
				};

				for (size_t i = 2; i < m_Indices.size(); i += 3)
				{
					Triangle triangle{
						m_Indices[i - 2],
						m_Indices[i - 1],
						m_Indices[i]
					};	
					showTriangle(mesh, triangle, offset, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
				}
				if (m_TriangleFound)
				{
					showTriangle(mesh, m_Triangle, offset, glm::vec4(0.6f, 0.3f, 1.0f, 1.0f));
				}

				InGui::Separator();
				InGui::SetPositionOfNext(nextPos);
				if (IS_SET(InGui::Button("Triangulate", glm::vec2(70.0f, 50.0f)), InGuiReturnType::Clicked))
				{			
					triangulate();
				}
				InGui::Separator();
				if (IS_SET(InGui::Button("Clear", glm::vec2(70.0f, 50.0f)), InGuiReturnType::Clicked))
				{
					m_Indices.clear();
					m_Points.clear();
					m_Triangulated = false;
				}
				if (m_MovedPoint)
				{
					auto [mx, my] = Input::GetMousePosition();
					glm::vec2 relativePos = glm::vec2(mx, my) - windowPosition - (windowSize / 2.0f);

					(*m_MovedPoint).X = relativePos.x;
					(*m_MovedPoint).Y = relativePos.y;
				}
				if (!IS_SET(InGui::GetWindow(m_PanelID).Flags, InGuiWindowFlags::Hoovered))
				{
					m_MovedPoint = nullptr;
				}
				InGui::EndScrollableArea();

			}
		}
		InGui::End();
	}
	void SpriteEditorPanel::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&SpriteEditorPanel::onMouseButtonPress, this));
		dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&SpriteEditorPanel::onMouseButtonRelease, this));
		dispatcher.Dispatch<MouseScrollEvent>(Hook(&SpriteEditorPanel::onMouseScroll, this));
	}
	bool SpriteEditorPanel::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
		{
			if (IS_SET(InGui::GetWindow(m_PanelID).Flags, InGuiWindowFlags::Hoovered))
			{
				auto [mx, my] = Input::GetMousePosition();
				glm::vec2 windowPosition = InGui::GetWindow(m_PanelID).Position;
				glm::vec2 windowSize = InGui::GetWindow(m_PanelID).Size;
				glm::vec2 relativePos = glm::vec2( mx, my ) - windowPosition - (windowSize / 2.0f);
				
				m_Points.push_back({ relativePos.x, relativePos.y });
				return true;
			}
		}
		else if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_LEFT))
		{
			auto [mx, my] = Input::GetMousePosition();
			glm::vec2 windowPosition = InGui::GetWindow(m_PanelID).Position;
			glm::vec2 windowSize = InGui::GetWindow(m_PanelID).Size;
			glm::vec2 relativePos = glm::vec2(mx, my) - windowPosition - (windowSize / 2.0f);
			uint32_t counter = 0;
			for (auto& point : m_Points)
			{
				glm::vec2 relativePos = {
					windowPosition.x + (windowSize.x / 2.0f) + point.X,
					windowPosition.y + (windowSize.y / 2.0f) + point.Y
				};
				if (glm::distance(glm::vec2(mx, my), relativePos) < sc_PointRadius)
				{
					m_MovedPoint = &point;
					return true;
				}
				counter++;
			}
		}
		else if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_MIDDLE))
		{
			if (IS_SET(InGui::GetWindow(m_PanelID).Flags, InGuiWindowFlags::Hoovered))
			{
				auto [mx, my] = Input::GetMousePosition();
				glm::vec2 windowPosition = InGui::GetWindow(m_PanelID).Position;
				glm::vec2 windowSize = InGui::GetWindow(m_PanelID).Size;
				glm::vec2 relativePos = glm::vec2(mx, my) - windowPosition - (windowSize / 2.0f);
				uint32_t counter = 0;

				if (!m_Triangulated)
				{
					for (auto& point : m_Points)
					{
						glm::vec2 relativePos = {
							windowPosition.x + (windowSize.x / 2.0f) + point.X,
							windowPosition.y + (windowSize.y / 2.0f) + point.Y
						};
						if (glm::distance(glm::vec2(mx, my), relativePos) < sc_PointRadius)
						{
							m_Points.erase(m_Points.begin() + counter);
							return true;
						}
						counter++;
					}
				}
				else if (m_TriangleFound)
				{
					for (size_t i = 2; i < m_Indices.size(); i += 3)
					{
						if (m_Triangle.First == m_Indices[i - 2]
							&& m_Triangle.Second == m_Indices[i - 1]
							&& m_Triangle.Third == m_Indices[i])
						{
							m_Indices.erase(m_Indices.begin() + i - 2, m_Indices.begin() + i + 1);
							break;
						}
					}
					m_TriangleFound = false;
				}
				return true;
			}
		}
		return false;
	}
	bool SpriteEditorPanel::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		if (event.IsButtonReleased(MouseCode::MOUSE_BUTTON_LEFT))
		{
			m_MovedPoint = nullptr;
		}
		return false;
	}
	bool SpriteEditorPanel::onMouseScroll(MouseScrollEvent& event)
	{
		if (IS_SET(InGui::GetWindow(m_PanelID).Flags, InGuiWindowFlags::Hoovered))
		{
			//float yAllowedSize = InGui::GetWindow(m_PanelID).Size.y
			//	- InGui::GetWindow(m_PanelID).Layout.TopPadding
			//	- InGui::GetWindow(m_PanelID).Layout.BottomPadding;
			
			//if (m_Size.y * (m_Scale + event.GetOffsetY() * 0.1f) < yAllowedSize)
			{
				m_Scale += event.GetOffsetY() * 0.25f;
			}
		}
		return false;
	}
	void SpriteEditorPanel::triangulate()
	{
		std::vector<tpp::Delaunay::Point> points;
		for (auto& p : m_Points)
		{
			points.push_back({ p.X, p.Y });
		}
		tpp::Delaunay generator(points);
		generator.setMinAngle(30.5f);
		generator.setMaxArea(12000.5f);
		generator.Triangulate(true);

		m_Indices.clear();
		m_Points.clear();
		for (tpp::Delaunay::fIterator fit = generator.fbegin(); fit != generator.fend(); ++fit)
		{
			tpp::Delaunay::Point sp1;
			tpp::Delaunay::Point sp2;
			tpp::Delaunay::Point sp3;

			int keypointIdx1 = generator.Org(fit, &sp1);
			int keypointIdx2 = generator.Dest(fit, &sp2);
			int keypointIdx3 = generator.Apex(fit, &sp3);

			double x = 0.0f, y = 0.0f;
			if (std::find(m_Indices.begin(), m_Indices.end(), (uint32_t)keypointIdx1) == m_Indices.end())
			{
				GetTriangulationPt(points, keypointIdx1, sp1, x, y);
				if (m_Points.size() <= keypointIdx1)
					m_Points.resize((size_t)keypointIdx1 + 1);
				m_Points[keypointIdx1] = { (float)x, (float)y };
			}
			if (std::find(m_Indices.begin(), m_Indices.end(), (uint32_t)keypointIdx2) == m_Indices.end())
			{
				GetTriangulationPt(points, keypointIdx2, sp2, x, y);
				if (m_Points.size() <= keypointIdx2)
					m_Points.resize((size_t)keypointIdx2 + 1);
				m_Points[keypointIdx2] = { (float)x, (float)y };
			}
			if (std::find(m_Indices.begin(), m_Indices.end(), (uint32_t)keypointIdx3) == m_Indices.end())
			{
				GetTriangulationPt(points, keypointIdx3, sp3, x, y);
				if (m_Points.size() <= keypointIdx3)
					m_Points.resize((size_t)keypointIdx3 + 1);
				m_Points[keypointIdx3] = { (float)x, (float)y };
			}


			m_Indices.push_back((uint32_t)keypointIdx1);
			m_Indices.push_back((uint32_t)keypointIdx2);
			m_Indices.push_back((uint32_t)keypointIdx3);
		}
		m_Triangulated = true;
	}
	void SpriteEditorPanel::showTriangle(InGuiMesh& mesh, const Triangle& triangle, const glm::vec2& offset, const glm::vec4& color)
	{
		Point& first = m_Points[triangle.First];
		Point& second = m_Points[triangle.Second];
		Point& third = m_Points[triangle.Third];

		glm::vec2 firstPos = {
			offset.x + first.X,
			offset.y + first.Y
		};
		glm::vec2 secondPos = {
			offset.x + second.X,
			offset.y + second.Y
		};
		glm::vec2 thirdPos = {
			offset.x + third.X,
			offset.y + third.Y
		};

		mesh.Lines.push_back({
			color,
			glm::vec3(firstPos.x, firstPos.y, 0.0f),
			glm::vec3(secondPos.x, secondPos.y, 0.0f)
			});


		mesh.Lines.push_back({
			color,
			glm::vec3(secondPos.x, secondPos.y, 0.0f),
			glm::vec3(thirdPos.x, thirdPos.y, 0.0f)
			});

		mesh.Lines.push_back({
			color,
			glm::vec3(thirdPos.x, thirdPos.y, 0.0f),
			glm::vec3(firstPos.x, firstPos.y, 0.0f)
			});
	}
	SpriteEditorPanel::Triangle SpriteEditorPanel::findTriangle(const glm::vec2& pos)
	{
		glm::vec2 windowSize = InGui::GetWindow(m_PanelID).Size;
		glm::vec2 windowPosition = InGui::GetWindow(m_PanelID).Position;
		glm::vec2 position = (windowSize / 2.0f) - (m_Size / 2.0f);

		Triangle triangle;
		m_TriangleFound = false;
		for (size_t i = 2; i < m_Indices.size(); i += 3)
		{
			uint32_t firstIndex = m_Indices[i - 2];
			uint32_t secondIndex = m_Indices[i - 1];
			uint32_t thirdIndex = m_Indices[i];

			Point& first = m_Points[firstIndex];
			Point& second = m_Points[secondIndex];
			Point& third = m_Points[thirdIndex];

			glm::vec2 firstPos = {
				windowPosition.x + (windowSize.x / 2.0f) + first.X,
				windowPosition.y + (windowSize.y / 2.0f) + first.Y
			};
			glm::vec2 secondPos = {
				windowPosition.x + (windowSize.x / 2.0f) + second.X,
				windowPosition.y + (windowSize.y / 2.0f) + second.Y
			};
			glm::vec2 thirdPos = {
				windowPosition.x + (windowSize.x / 2.0f) + third.X,
				windowPosition.y + (windowSize.y / 2.0f) + third.Y
			};

			if (Math::PointInTriangle(pos, firstPos, secondPos, thirdPos))
			{
				triangle.First = firstIndex;
				triangle.Second = secondIndex;
				triangle.Third = thirdIndex;
				m_TriangleFound = true;
				return triangle;
			}
		}

		return triangle;
	}
}