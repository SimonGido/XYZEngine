#include "stdafx.h"
#include "SpriteEditorPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"

#include "XYZ/Utils/Math/Math.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>


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
		m_PanelID(panelID),
		m_BonePool(15 * sizeof(SpriteEditorPanel::Bone))
	{
		InGui::ImageWindow(m_PanelID, "Sprite Editor", glm::vec2(0.0f), glm::vec2(200.0f), m_Context);
		InGui::End();


		const InGuiWindow& window = InGui::GetWindow(m_PanelID);
		auto flags = window.Flags;
		flags &= ~InGuiWindowFlags::EventBlocking;
		InGui::SetWindowFlags(m_PanelID, flags);

		m_CategoriesOpen[Bones] = false;
		m_CategoriesOpen[Geometry] = false;
		m_CategoriesOpen[Weights] = false;

		m_Material = Ref<Material>::Create(Shader::Create("Assets/Shaders/Test.glsl"));

		m_ViewportSize = window.Size;
		FramebufferSpecs specs;
		specs.Width = (uint32_t)window.Size.x;
		specs.Height = (uint32_t)window.Size.y;
		specs.ClearColor = { 0.1f,0.1f,0.1f,1.0f };
		specs.Attachments = {
			FramebufferTextureSpecs(FramebufferTextureFormat::RGBA16F),
			FramebufferTextureSpecs(FramebufferTextureFormat::DEPTH24STENCIL8)
		};
		m_Framebuffer = Framebuffer::Create(specs);
		m_RenderTexture = RenderTexture::Create(m_Framebuffer);
		m_RenderSubTexture = Ref<SubTexture>::Create(m_RenderTexture, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		rebuildRenderBuffers();
	}
	void SpriteEditorPanel::SetContext(Ref<SubTexture> context)
	{
		m_Context = context;
		m_ContextSize.x = (float)m_Context->GetTexture()->GetWidth();
		m_ContextSize.y = (float)m_Context->GetTexture()->GetHeight();

		m_Material->ClearTextures();
		m_Material->Set("u_Texture", m_Context->GetTexture(), 0);
		rebuildRenderBuffers();
	}
	void SpriteEditorPanel::OnUpdate(Timestep ts)
	{
		const InGuiWindow& window = InGui::GetWindow(m_PanelID);
		if (IS_SET(window.Flags, InGuiWindowFlags::Hoovered))
		{
			auto [mx, my] = getMouseViewportSpace();
			mx *= window.Size.x / 2.0f;
			my *= window.Size.y / 2.0f;
			m_Triangle = findTriangle({ mx, my });
			m_BoneID = findBone({ mx, my });

			
			if (m_CreatedBone)
			{
				if (auto parent = m_BoneHierarchy.GetParentData(m_CreatedBone->ID))
				{
					Bone* parentBone = static_cast<Bone*>(parent);
					mx -= parentBone->End.x + parentBone->WorldStart.x;
					my -= parentBone->End.y + parentBone->WorldStart.y;
				}
				m_CreatedBone->End = { mx - m_CreatedBone->Start.x, my - m_CreatedBone->Start.y };
			}
			else if (m_EditedBone)
			{
				if (IS_SET(m_Flags, PreviewPose))
				{
					m_EditedBone->PreviewTransform = m_EditedBone->PreviewTransform * glm::rotate(0.01f, glm::vec3(0.0f, 0.0f, 1.0f));
				}
			}
			else if (m_EditedVertex)
			{
				(*m_EditedVertex).X = mx;
				(*m_EditedVertex).Y = my;
				updateVertexBuffer();
			}		
		}

		m_Framebuffer->Bind();
		Renderer::SetClearColor(m_Framebuffer->GetSpecification().ClearColor);
		Renderer::Clear();;

		glm::mat4 projection = glm::ortho(-window.Size.x / 2.0f, window.Size.x / 2.0f, -window.Size.y / 2.0f, window.Size.y / 2.0f);
		m_Material->Set("u_ViewProjectionMatrix", projection);
		m_Material->Bind();
		m_VertexArray->Bind();
		Renderer::DrawIndexed(PrimitiveType::Triangles, m_VertexArray->GetIndexBuffer()->GetCount());


		Renderer2D::BeginScene(projection);

		if (IS_SET(m_Flags, Flags::PreviewPose))
		{
			m_BoneHierarchy.Traverse([](void* parent, void* child) -> bool {

				Bone* childBone = static_cast<Bone*>(child);
				if (parent)
				{
					Bone* parentBone = static_cast<Bone*>(parent);
					childBone->PreviewFinalTransform = parentBone->PreviewFinalTransform * childBone->PreviewTransform;
				}
				else
				{
					childBone->PreviewFinalTransform = childBone->PreviewTransform;
				}
				return false;
			});

			m_BoneHierarchy.Traverse([&](void* parent, void* child) -> bool {

				Bone* childBone = static_cast<Bone*>(child);
				glm::vec4 color = glm::vec4(0.2f, 0.0f, 1.0f, 1.0f);
				if (childBone->ID == m_BoneID)
					color = glm::vec4(0.8f, 0.3f, 1.0f, 1.0f);
		
				glm::vec3 scale;
				glm::quat rotation;
				glm::vec3 translation;
				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(childBone->PreviewFinalTransform, scale, rotation, translation, skew, perspective);
				Renderer2D::SubmitCircle(translation, sc_PointRadius, 20, color);
				
				glm::vec4 endPoint = glm::toMat4(rotation) * glm::vec4(childBone->End, 0.0f, 1.0f);
				endPoint += glm::vec4(translation, 0.0f);
				Renderer2D::SubmitLine(translation, glm::vec3(endPoint.x, endPoint.y, 0.0f), glm::vec4(0.7f, 0.4f, 1.0f, 1.0f));

				Renderer2D::SubmitCircle(endPoint, sc_PointRadius, 20, color);

				return false;
			});
		}
		else
		{
			m_BoneHierarchy.Traverse([&](void* parent, void* child) -> bool {

				Bone* childBone = static_cast<Bone*>(child);
				glm::vec4 color = glm::vec4(0.2f, 0.0f, 1.0f, 1.0f);
				if (childBone->ID == m_BoneID)
					color = glm::vec4(0.8f, 0.3f, 1.0f, 1.0f);

				childBone->WorldStart = childBone->Start;
				glm::vec2 end = childBone->Start + childBone->End;
				if (parent)
				{
					Bone* parentBone = static_cast<Bone*>(parent);
					glm::vec2 parentBoneEnd = parentBone->WorldStart + parentBone->End;
					childBone->WorldStart += parentBoneEnd;
					end += parentBoneEnd;
				}

				Renderer2D::SubmitCircle(glm::vec3(childBone->WorldStart, 0.0f), sc_PointRadius, 20, color);
				Renderer2D::SubmitLine(glm::vec3(childBone->WorldStart, 0.0f), glm::vec3(end, 0.0f), glm::vec4(0.7f, 0.4f, 1.0f, 1.0f));
				Renderer2D::SubmitCircle(glm::vec3(end, 0.0f), sc_PointRadius, 20, glm::vec4(0.2f, 0.0f, 1.0f, 1.0f));

				return false;
				});
		}
		for (auto& vertex : m_Vertices)
		{
			Renderer2D::SubmitCircle(glm::vec3(vertex.X, vertex.Y, 0.0f), sc_PointRadius, 20, sc_VertexColor);
		}
		for (size_t i = 2; i < m_Indices.size(); i += 3)
		{
			Triangle triangle{
				m_Indices[i - 2],
				m_Indices[i - 1],
				m_Indices[i]
			};
			showTriangle(triangle, sc_TriangleColor);
		}
		if (m_TriangleFound)
		{
			showTriangle(m_Triangle, sc_TriangleHighlightColor);
		}
		Renderer2D::FlushLines();
		Renderer2D::EndScene();

		Renderer2D::EndScene();
		m_Framebuffer->Unbind();
	}

	void SpriteEditorPanel::OnInGuiRender()
	{
		if (m_Context.Raw())
		{
			if (InGui::ImageWindow(m_PanelID, "Sprite Editor", glm::vec2(0.0f), glm::vec2(200.0f), m_RenderSubTexture))
			{
				glm::vec2 size = { 150.0f, InGuiWindow::PanelHeight };
				if (InGui::BeginGroup("Bones", size, m_CategoriesOpen[Bones]))
				{
					if (IS_SET(InGui::Button("Preview Pose", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
					{
						if (IS_SET(m_Flags, EditBone))
							m_Flags |= PreviewPose;
						else
							m_Flags = PreviewPose;
						initializePose();
					}
					if (IS_SET(InGui::Button("Create Bone", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
					{
						m_Flags = CreateBone;
					}
					InGui::Separator();
					if (IS_SET(InGui::Button("Edit Bone", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
					{
						if (IS_SET(m_Flags, PreviewPose))
							m_Flags |= EditBone;
						else
							m_Flags = EditBone;
					}
					InGui::Separator();
					if (IS_SET(InGui::Button("Delete Bone", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
					{
						m_Flags = DeleteBone;
					}
				}
				InGui::Separator();
				if (InGui::BeginGroup("Geometry", size, m_CategoriesOpen[Geometry]))
				{
					if (IS_SET(InGui::Button("Create Vertex", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
					{
						m_Flags = CreateVertex;
					}
					InGui::Separator();
					if (IS_SET(InGui::Button("Edit Vertex", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
					{
						m_Flags = EditVertex;
					}
					InGui::Separator();
					if (IS_SET(InGui::Button("Delete Vertex", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
					{
						m_Flags = DeleteVertex;
					}
					InGui::Separator();
					if (IS_SET(InGui::Button("Delete Triangle", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
					{
						m_Flags = DeleteTriangle;
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

				
				if (!IS_SET(InGui::GetWindow(m_PanelID).Flags, InGuiWindowFlags::Hoovered))
				{
					m_EditedVertex = nullptr;
				}
			}
			InGui::End();
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
				auto [mx, my] = getMouseViewportSpace();
				glm::vec2 windowSize = InGui::GetWindow(m_PanelID).Size;
				mx *= windowSize.x / 2.0f;
				my *= windowSize.y / 2.0f;

				// Bone
				if (IS_SET(m_Flags, Flags::CreateBone))
				{
					Bone* bone = m_BonePool.Allocate<Bone>();
					if (Input::IsKeyPressed(KeyCode::KEY_LEFT_CONTROL) && m_LastCreatedBone)
					{
						bone->ID = m_BoneHierarchy.Insert(bone, m_LastCreatedBone->ID);
						bone->Start = glm::vec2(0.0f);
					}
					else
					{
						bone->ID = m_BoneHierarchy.Insert(bone);
						bone->Start = { mx, my };
					}					
					m_CreatedBone = bone;
				}
				else if (IS_SET(m_Flags, Flags::EditBone))
				{
					m_BoneID = findBone({ mx, my });
					if (m_FoundBone)
						m_EditedBone = static_cast<Bone*>(m_BoneHierarchy.GetData(m_BoneID));
				}
				
				// Vertex
				else if (IS_SET(m_Flags, Flags::CreateVertex))
				{
					m_Vertices.push_back({ mx, my });
					return true;
				}
				else if (IS_SET(m_Flags, Flags::EditVertex))
				{
					uint32_t counter = 0;
					for (auto& vertex : m_Vertices)
					{
						if (glm::distance(glm::vec2(mx, my), glm::vec2(vertex.X, vertex.Y)) < sc_PointRadius)
						{
							m_EditedVertex = &vertex;
							return true;
						}
						counter++;
					}
				}
				else if (IS_SET(m_Flags, Flags::DeleteVertex) && !m_Triangulated)
				{
					uint32_t counter = 0;
					for (auto& vertex : m_Vertices)
					{
						if (glm::distance(glm::vec2(mx, my), glm::vec2(vertex.X, vertex.Y)) < sc_PointRadius)
						{
							m_Vertices.erase(m_Vertices.begin() + counter);
							return true;
						}
						counter++;
					}
				}
				else if (IS_SET(m_Flags, Flags::DeleteTriangle) && m_Triangulated && m_TriangleFound)
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
			m_EditedBone = nullptr;
			m_LastCreatedBone = m_CreatedBone;
			m_CreatedBone = nullptr;
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
		std::vector<PreviewVertex> vertices;
		if (m_Vertices.size())
		{
			vertices.reserve(m_Vertices.size());
			for (auto& vertex : m_Vertices)
			{
				vertices.push_back({
					glm::vec3(1.0f),
					glm::vec3(vertex.X, vertex.Y, 0.0f),
					calculateTexCoord(glm::vec2(vertex.X, vertex.Y))
					});
			}
		}
		else
		{
			vertices.reserve(4);
			vertices.push_back({
				glm::vec3(1.0f),
				glm::vec3(-m_ContextSize.x / 2.0f, -m_ContextSize.y / 2.0f, 0.0f),
				glm::vec2(0.0f)
				});
			vertices.push_back({
				glm::vec3(1.0f),
				glm::vec3(m_ContextSize.x / 2.0f, -m_ContextSize.y / 2.0f, 0.0f),
				glm::vec2(1.0f,0.0f)
				});
			vertices.push_back({
				glm::vec3(1.0f),
				glm::vec3(m_ContextSize.x / 2.0f,  m_ContextSize.y / 2.0f, 0.0f),
				glm::vec2(1.0f)
				});
			vertices.push_back({
				glm::vec3(1.0f),
				glm::vec3(-m_ContextSize.x / 2.0f, m_ContextSize.y / 2.0f, 0.0f),
				glm::vec2(0.0f, 1.0f)
				});
		}

		m_VertexArray = VertexArray::Create();
		m_VertexBuffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(PreviewVertex), BufferUsage::Dynamic);
		m_VertexBuffer->SetLayout({
			{0, ShaderDataComponent::Float3, "a_Color"},
			{1, ShaderDataComponent::Float3, "a_Position"},
			{2, ShaderDataComponent::Float2, "a_TexCoord"},
			});
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		if (m_Indices.size())
		{
			Ref<IndexBuffer> ibo = IndexBuffer::Create(m_Indices.data(), m_Indices.size());
			m_VertexArray->SetIndexBuffer(ibo);
		}
		else
		{
			uint32_t indices[6] = {
				0,1,2,2,3,0
			};
			Ref<IndexBuffer> ibo = IndexBuffer::Create(indices, 6);
			m_VertexArray->SetIndexBuffer(ibo);
		}

	}
	void SpriteEditorPanel::updateVertexBuffer()
	{
		std::vector<PreviewVertex> vertices;
		vertices.reserve(m_Vertices.size());
		for (auto& vertex : m_Vertices)
		{
			vertices.push_back({
				glm::vec3(1.0f),
				glm::vec3(vertex.X,  vertex.Y,0.0f),
				calculateTexCoord(glm::vec2(vertex.X, vertex.Y))
				});
		}
		m_VertexBuffer->Update(vertices.data(), vertices.size() * sizeof(PreviewVertex));
	}
	glm::vec2 SpriteEditorPanel::calculateTexCoord(const glm::vec2& pos)
	{
		glm::vec2 position = pos + m_ContextSize / 2.0f;
		return glm::vec2(position.x / m_ContextSize.x , position.y / m_ContextSize.y);
	}
	void SpriteEditorPanel::initializePose()
	{
		m_BoneHierarchy.Traverse([](void* parent, void* child) -> bool {

			Bone* childBone = static_cast<Bone*>(child);
			if (parent)
			{
				Bone* parentBone = static_cast<Bone*>(parent);
				childBone->PreviewTransform = glm::translate(glm::vec3(parentBone->End, 0.0f));
			}
			else
			{
				childBone->PreviewTransform = glm::translate(glm::vec3(childBone->Start, 0.0f));
			}
			return false;
		});
	}
	void SpriteEditorPanel::triangulate()
	{
		if (m_Vertices.size() < 3)
			return;
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
	void SpriteEditorPanel::showTriangle(const Triangle& triangle, const glm::vec4& color)
	{
		Vertex& first = m_Vertices[triangle.First];
		Vertex& second = m_Vertices[triangle.Second];
		Vertex& third = m_Vertices[triangle.Third];

		Renderer2D::SubmitLine(glm::vec3(first.X, first.Y, 0.0f), glm::vec3(second.X, second.Y, 0.0f), color);
		Renderer2D::SubmitLine(glm::vec3(second.X, second.Y, 0.0f), glm::vec3(third.X, third.Y, 0.0f), color);
		Renderer2D::SubmitLine(glm::vec3(third.X, third.Y, 0.0f), glm::vec3(first.X, first.Y, 0.0f), color);
	}
	SpriteEditorPanel::Triangle SpriteEditorPanel::findTriangle(const glm::vec2& pos)
	{
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
			if (Math::PointInTriangle(pos, glm::vec2(first.X, first.Y), glm::vec2(second.X, second.Y), glm::vec2(third.X, third.Y)))
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
	int32_t SpriteEditorPanel::findBone(const glm::vec2& pos)
	{
		int32_t boneId = -1;
		m_FoundBone = false;
		m_BoneHierarchy.Traverse([&](void* parent, void* child) -> bool {

			Bone* childBone = static_cast<Bone*>(child);
			if (IS_SET(m_Flags, Flags::PreviewPose))
			{
				glm::vec3 scale;
				glm::quat rotation;
				glm::vec3 translation;
				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(childBone->PreviewFinalTransform, scale, rotation, translation, skew, perspective);

				if (glm::distance(pos, glm::vec2(translation.x, translation.y)) < sc_PointRadius)
				{
					boneId = childBone->ID;
					m_FoundBone = true;
					return true;
				}			
			}	
			else
			{
				if (glm::distance(pos, childBone->WorldStart) < sc_PointRadius)
				{
					boneId = childBone->ID;
					m_FoundBone = true;
					return true;
				}
			}
			return false;
		});
		return boneId;
	}
	std::pair<float, float> SpriteEditorPanel::getMouseViewportSpace() const
	{
		auto [mx, my] = Input::GetMousePosition();
		auto& window = InGui::GetWindow(m_PanelID);
		mx -= window.Position.x;
		my -= window.Position.y;

		auto viewportWidth = window.Size.x;
		auto viewportHeight = window.Size.y;

		return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
	}
}