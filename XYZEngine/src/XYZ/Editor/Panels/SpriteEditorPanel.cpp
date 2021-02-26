#include "stdafx.h"
#include "SpriteEditorPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Utils/Math/Math.h"


#include <glm/gtx/transform.hpp>
#include <tpp_interface.hpp>

namespace XYZ {

	struct PreviewVertex
	{
		glm::vec3 Color;
		glm::vec3 Position;
		glm::vec2 TexCoord;
	};

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

		m_CategoriesOpen[Bones] = false;
		m_CategoriesOpen[Geometry] = false;
		m_CategoriesOpen[Weights] = false;

		m_Material = Ref<Material>::Create(Shader::Create("Assets/Shaders/Test.glsl"));
	}
	void SpriteEditorPanel::SetContext(Ref<SubTexture> context)
	{
		m_Context = context;
		m_ContextSize.x = (float)m_Context->GetTexture()->GetWidth();
		m_ContextSize.y = (float)m_Context->GetTexture()->GetHeight();

		m_Material->ClearTextures();
		m_Material->Set("u_Texture", m_Context->GetTexture(), 0);
	}
	void SpriteEditorPanel::OnInGuiRender()
	{
		if (InGui::Begin(m_PanelID, "Sprite Editor", glm::vec2(0.0f), glm::vec2(200.0f)))
		{
			if (m_Context.Raw())
			{
				glm::vec2 windowSize = InGui::GetWindow(m_PanelID).Size;
				glm::vec2 windowPosition = InGui::GetWindow(m_PanelID).Position;
				glm::vec2 position = (windowSize / 2.0f) - (m_ContextSize / 2.0f);
				auto& layout = InGui::GetWindow(m_PanelID).Layout;

				InGui::BeginScrollableArea(windowSize - glm::vec2(2.0f * layout.RightPadding, 0.0f), m_ScrollOffset, 100.0f, 10.0f);
				glm::vec2 nextPos = InGui::GetPositionOfNext();
				InGui::SetPositionOfNext(windowPosition + position);
				InGui::Image(m_ContextSize, m_Context);

				auto [mx, my] = Input::GetMousePosition();
				m_Triangle = findTriangle({ mx, my });
				auto& mesh = InGui::GetWindowScrollableOverlayMesh(m_PanelID);

				for (auto& vertex : m_Vertices)
				{
					glm::vec2 relativePos = {
						windowPosition.x + (windowSize.x / 2.0f) + vertex.X,
						windowPosition.y + (windowSize.y / 2.0f) + vertex.Y
					};
					CircleOfLines(
						mesh, glm::vec3(relativePos.x, relativePos.y, 0.0f),
						sc_PointRadius, 20, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)
					);
				}
				
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

				glm::vec2 size = { 150.0f, InGuiWindow::PanelHeight };
				if (InGui::BeginGroup("Bones", size, m_CategoriesOpen[Bones]))
				{

				}
				InGui::Separator();
				if (InGui::BeginGroup("Geometry", size, m_CategoriesOpen[Geometry]))
				{
					if (IS_SET(InGui::Button("Create Vertex", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
					{
						m_GeometryCategoryFlags = CreateVertex;
					}
					InGui::Separator();
					if (IS_SET(InGui::Button("Edit Vertex", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
					{
						m_GeometryCategoryFlags = EditVertex;
					}
					InGui::Separator();
					if (IS_SET(InGui::Button("Delete Vertex", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
					{
						m_GeometryCategoryFlags = DeleteVertex;
					}
					InGui::Separator();
					if (IS_SET(InGui::Button("Delete Triangle", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
					{
						m_GeometryCategoryFlags = DeleteTriangle;
					}
					InGui::Separator();
					if (IS_SET(InGui::Button("Clear", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
					{
						m_Indices.clear();
						m_Vertices.clear();
						m_Triangulated = false;
					}	
					InGui::Separator();
					if (IS_SET(InGui::Button("Triangulate", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
					{
						triangulate();
					}
				}
				InGui::Separator();
				if (InGui::BeginGroup("Weights", size, m_CategoriesOpen[Weights]))
				{

				}
				InGui::Separator();
				
				if (m_EditedVertex)
				{
					auto [mx, my] = Input::GetMousePosition();
					glm::vec2 relativePos = glm::vec2(mx, my) - windowPosition - (windowSize / 2.0f);

					(*m_EditedVertex).X = relativePos.x;
					(*m_EditedVertex).Y = relativePos.y;
					updateVertexBuffer();
				}
				if (!IS_SET(InGui::GetWindow(m_PanelID).Flags, InGuiWindowFlags::Hoovered))
				{
					m_EditedVertex = nullptr;
				}
				InGui::EndScrollableArea();			
			}
		}
		InGui::End();

		if (m_Triangulated)
		{
			float w = (float)Input::GetWindowSize().first;
			float h = (float)Input::GetWindowSize().second;

			Renderer::SetActiveQueue(RenderQueueType::Overlay);
			m_Material->Set("u_ViewProjectionMatrix", glm::ortho(0.0f, w, h, 0.0f));
			m_Material->Bind();
			m_VertexArray->Bind();
			Renderer::DrawIndexed(PrimitiveType::Triangles, m_VertexArray->GetIndexBuffer()->GetCount());
			Renderer::SetActiveQueue(RenderQueueType::Default);
		}
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
				glm::vec2 relativePos = glm::vec2(mx, my) - windowPosition - (windowSize / 2.0f);
				
				if (IS_SET(m_GeometryCategoryFlags, GeometryCategoryFlags::CreateVertex))
				{
					glm::vec2 pos = glm::vec2(mx, my) - windowPosition - (windowSize / 2.0f);
					m_Vertices.push_back({ pos.x, pos.y });
					return true;
				}
				else if (IS_SET(m_GeometryCategoryFlags, GeometryCategoryFlags::EditVertex))
				{
					uint32_t counter = 0;
					for (auto& vertex : m_Vertices)
					{
						glm::vec2 pos = {
							windowPosition.x + (windowSize.x / 2.0f) + vertex.X,
							windowPosition.y + (windowSize.y / 2.0f) + vertex.Y
						};
						if (glm::distance(glm::vec2(mx, my), pos) < sc_PointRadius)
						{
							m_EditedVertex = &vertex;
							return true;
						}
						counter++;
					}
				}
				else if (IS_SET(m_GeometryCategoryFlags, GeometryCategoryFlags::DeleteVertex) && !m_Triangulated)
				{
					uint32_t counter = 0;
					for (auto& point : m_Vertices)
					{
						glm::vec2 pos = {
							windowPosition.x + (windowSize.x / 2.0f) + point.X,
							windowPosition.y + (windowSize.y / 2.0f) + point.Y
						};
						if (glm::distance(glm::vec2(mx, my), pos) < sc_PointRadius)
						{
							m_Vertices.erase(m_Vertices.begin() + counter);
							return true;
						}
						counter++;
					}
				}
				else if (IS_SET(m_GeometryCategoryFlags, GeometryCategoryFlags::DeleteTriangle) && m_Triangulated && m_TriangleFound)
				{
					for (size_t i = 2; i < m_Indices.size(); i += 3)
					{
						if (m_Triangle.First == m_Indices[i - 2]
							&& m_Triangle.Second == m_Indices[i - 1]
							&& m_Triangle.Third == m_Indices[i])
						{
							m_Indices.erase(m_Indices.begin() + i - 2, m_Indices.begin() + i + 1);
							eraseEmptyPoints();
							rebuildRenderBuffers();
							m_TriangleFound = false;
							return true;
						}
					}
					
				}
			}
		}
		return false;
	}
	bool SpriteEditorPanel::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		if (event.IsButtonReleased(MouseCode::MOUSE_BUTTON_RIGHT))
		{
			m_EditedVertex = nullptr;
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
	void SpriteEditorPanel::rebuildRenderBuffers()
	{
		if (m_Vertices.size())
		{
			auto [mx, my] = Input::GetMousePosition();
			glm::vec2 windowPosition = InGui::GetWindow(m_PanelID).Position;
			glm::vec2 windowSize = InGui::GetWindow(m_PanelID).Size;
			glm::vec2 relativePos = glm::vec2(mx, my) - windowPosition - (windowSize / 2.0f);

			std::vector<PreviewVertex> vertices;
			vertices.reserve(m_Vertices.size());
			for (auto& vertex : m_Vertices)
			{
				glm::vec2 pos = {
					windowPosition.x + (windowSize.x / 2.0f) + vertex.X,
					windowPosition.y + (windowSize.y / 2.0f) + vertex.Y
				};
				vertices.push_back({
					glm::vec3(1.0f),
					glm::vec3(pos,-0.7f),
					calculateTexCoord(pos)
					});

				std::cout << calculateTexCoord(pos).x << " " << calculateTexCoord(pos).y << std::endl;
			}
			m_VertexArray = VertexArray::Create();
			m_VertexBuffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(PreviewVertex), BufferUsage::Dynamic);
			m_VertexBuffer->SetLayout({
				{0, ShaderDataComponent::Float3, "a_Color"},
				{1, ShaderDataComponent::Float3, "a_Position"},
				{2, ShaderDataComponent::Float2, "a_TexCoord"},
				});
			m_VertexArray->AddVertexBuffer(m_VertexBuffer);
			Ref<IndexBuffer> ibo = IndexBuffer::Create(m_Indices.data(), m_Indices.size());
			m_VertexArray->SetIndexBuffer(ibo);
		}
	}
	void SpriteEditorPanel::updateVertexBuffer()
	{
		glm::vec2 windowSize = InGui::GetWindow(m_PanelID).Size;
		glm::vec2 windowPosition = InGui::GetWindow(m_PanelID).Position;

		std::vector<PreviewVertex> vertices;
		vertices.reserve(m_Vertices.size());
		for (auto& vertex : m_Vertices)
		{
			glm::vec2 pos = {
			   windowPosition.x + (windowSize.x / 2.0f) + vertex.X,
			   windowPosition.y + (windowSize.y / 2.0f) + vertex.Y
			};
			vertices.push_back({
				glm::vec3(1.0f),
				glm::vec3(pos.x,pos.y,0.0f),
				calculateTexCoord(pos) 
				});
		}
		m_VertexBuffer->Update(vertices.data(), vertices.size() * sizeof(PreviewVertex));
	}
	glm::vec2 SpriteEditorPanel::calculateTexCoord(const glm::vec2& pos)
	{
		glm::vec2 windowSize = InGui::GetWindow(m_PanelID).Size;
		glm::vec2 windowPosition = InGui::GetWindow(m_PanelID).Position;
		glm::vec2 position = (windowSize / 2.0f) - (m_ContextSize / 2.0f);

		glm::vec2 bottomLeftPoint = windowPosition + position;
		glm::vec2 topRightPoint = windowPosition + position + glm::vec2(m_ContextSize.x, m_ContextSize.y);

		glm::vec2 diff = topRightPoint - pos;
		return glm::vec2(diff.x / m_ContextSize.x, diff.y / m_ContextSize.y);
	}
	void SpriteEditorPanel::triangulate()
	{
		std::vector<tpp::Delaunay::Point> points;
		for (auto& p : m_Vertices)
		{
			points.push_back({ p.X, p.Y });
		}
		tpp::Delaunay generator(points);
		generator.setMinAngle(30.5f);
		generator.setMaxArea(12000.5f);
		generator.Triangulate(true);

		m_Indices.clear();
		m_Vertices.clear();
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
				if (m_Vertices.size() <= keypointIdx1)
					m_Vertices.resize((size_t)keypointIdx1 + 1);
				m_Vertices[keypointIdx1] = { (float)x, (float)y };
			}
			if (std::find(m_Indices.begin(), m_Indices.end(), (uint32_t)keypointIdx2) == m_Indices.end())
			{
				GetTriangulationPt(points, keypointIdx2, sp2, x, y);
				if (m_Vertices.size() <= keypointIdx2)
					m_Vertices.resize((size_t)keypointIdx2 + 1);
				m_Vertices[keypointIdx2] = { (float)x, (float)y };
			}
			if (std::find(m_Indices.begin(), m_Indices.end(), (uint32_t)keypointIdx3) == m_Indices.end())
			{
				GetTriangulationPt(points, keypointIdx3, sp3, x, y);
				if (m_Vertices.size() <= keypointIdx3)
					m_Vertices.resize((size_t)keypointIdx3 + 1);
				m_Vertices[keypointIdx3] = { (float)x, (float)y };
			}


			m_Indices.push_back((uint32_t)keypointIdx1);
			m_Indices.push_back((uint32_t)keypointIdx2);
			m_Indices.push_back((uint32_t)keypointIdx3);
		}
		m_Triangulated = true;
		rebuildRenderBuffers();
	}
	void SpriteEditorPanel::eraseEmptyPoints()
	{
		std::vector<uint32_t> erasedPoints;
		for (uint32_t i = 0; i < m_Vertices.size(); ++i)
		{
			auto it = std::find(m_Indices.begin(), m_Indices.end(), i);
			if (it == m_Indices.end())	
				erasedPoints.push_back(i);
		}
		for (int32_t i = erasedPoints.size() - 1; i >= 0; --i)
		{
			m_Vertices.erase(m_Vertices.begin() + erasedPoints[i]);
			for (auto& index : m_Indices)
			{
				if (index >= erasedPoints[i])
					index--;
			}
		}
	}
	void SpriteEditorPanel::showTriangle(InGuiMesh& mesh, const Triangle& triangle, const glm::vec2& offset, const glm::vec4& color)
	{
		Vertex& first = m_Vertices[triangle.First];
		Vertex& second = m_Vertices[triangle.Second];
		Vertex& third = m_Vertices[triangle.Third];

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
		glm::vec2 position = (windowSize / 2.0f) - (m_ContextSize / 2.0f);

		Triangle triangle;
		m_TriangleFound = false;
		for (size_t i = 2; i < m_Indices.size(); i += 3)
		{
			uint32_t firstIndex = m_Indices[i - 2];
			uint32_t secondIndex = m_Indices[i - 1];
			uint32_t thirdIndex = m_Indices[i];

			Vertex& first = m_Vertices[firstIndex];
			Vertex& second = m_Vertices[secondIndex];
			Vertex& third = m_Vertices[thirdIndex];

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