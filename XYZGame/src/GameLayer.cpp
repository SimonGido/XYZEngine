#include "GameLayer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <random>

namespace XYZ {

	struct GPUTimeQueries
	{
		uint32_t GPUTime = 0;
		uint32_t Renderer2DPassQuery = 0;

		static constexpr uint32_t Count() { return sizeof(GPUTimeQueries) / sizeof(uint32_t); }
	};

	static Ref<RenderPass> CreateRenderPass()
	{
		FramebufferSpecification compFramebufferSpec;
		compFramebufferSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
		compFramebufferSpec.SwapChainTarget = true;
		compFramebufferSpec.ClearOnLoad = false;

		compFramebufferSpec.Attachments = { ImageFormat::RGBA };
		Ref<Framebuffer> framebuffer = Framebuffer::Create(compFramebufferSpec);

		RenderPassSpecification renderPassSpec;
		renderPassSpec.TargetFramebuffer = framebuffer;
		return RenderPass::Create(renderPassSpec);;
	}

	static Ref<Pipeline> CreatePipeline(const Ref<Shader>& shader, const Ref<RenderPass>& renderPass)
	{
		PipelineSpecification specification;
		specification.Shader = shader;
		specification.Layouts = shader->GetLayouts();
		specification.RenderPass = renderPass;
		specification.Topology = PrimitiveTopology::Triangles;
		specification.DepthTest = true;
		return Pipeline::Create(specification);
	}

	GameLayer::GameLayer()
		:
		m_Camera(30.0f, 1.778f, 0.1f, 1000.0f)
	{
	}

	GameLayer::~GameLayer()
	{
		
	}

	void GameLayer::OnAttach()
	{
		Application::Get().GetImGuiLayer()->EnableDockspace(false);
		const uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
		const uint32_t windowHeight = Application::Get().GetWindow().GetHeight();

		
		m_Scene = Ref<Scene>::Create("Game");
		m_Scene->SetViewportSize(windowWidth, windowHeight);
		m_SceneRenderer = Ref<SceneRenderer>::Create(m_Scene, SceneRendererSpecification{ true });
		m_Camera.SetViewportSize((float)windowWidth, (float)windowHeight);
	
		
		m_TestQuadEntity = m_Scene->CreateEntity("TestQuad");

		m_TestQuadEntity.EmplaceComponent<SpriteRenderer>(
			Renderer::GetDefaultResources().DefaultQuadMaterial,
			Ref<SubTexture>::Create(Renderer::GetDefaultResources().WhiteTexture),
			glm::vec4(1.0f),
			0
		);

		m_TestQuadEntity.EmplaceComponent<PointLight2D>(glm::vec3(1.0f), 1.0f, 1.0f);
		
		size_t count = 0;
		Ref<Mesh> mesh = marchingMesh(count);

		Ref<ShaderAsset> meshShaderAsset = AssetManager::GetAsset<ShaderAsset>("Resources/Shaders/MeshShader.shader");
		auto material = Ref<MaterialAsset>::Create(meshShaderAsset);
		material->SetTexture("u_Texture", Renderer::GetDefaultResources().WhiteTexture);
		m_TestQuadEntity.EmplaceComponent<MeshComponent>(mesh, material);
		Renderer::WaitAndRenderAll();
	}

	void GameLayer::OnDetach()
	{
	}
	void GameLayer::OnUpdate(Timestep ts)
	{
		m_Camera.OnUpdate(ts);

		m_Scene->OnUpdate(ts);
		m_Scene->OnRenderEditor(m_SceneRenderer, m_Camera.GetViewProjection(), m_Camera.GetViewMatrix(), m_Camera.GetPosition());
	}

	void GameLayer::OnEvent(Event& event)
	{
		m_Camera.OnEvent(event);
	}

	void GameLayer::OnImGuiRender()
	{
		m_SceneRenderer->OnImGuiRender();

		if (ImGui::Begin("Marching Cubes"))
		{
			if (ImGui::BeginTable("##MarchingCubesTable", 2, ImGuiTableFlags_SizingStretchProp))
			{
				UI::ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
				UI::ScopedColorStack color(true,
					ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f },
					ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f },
					ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f }
				);
				const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * GImGui->Font->Scale * 2.0f;

				UI::TableRow("Min",
					[]() { ImGui::Text("Min"); },
					[&]() { UI::ScopedTableColumnAutoWidth scoped(3, lineHeight);
							if (UI::Vec3Control({ "X", "Y", "Z" }, m_Min))
							{
								size_t count = 0;
								auto mesh = marchingMesh(count);
								if (count)
								{
									m_TestQuadEntity.GetComponent<MeshComponent>().Mesh = mesh;
								}
							}
					}
				);

				UI::TableRow("Max",
					[]() { ImGui::Text("Max"); },
					[&]() { UI::ScopedTableColumnAutoWidth scoped(3, lineHeight);
							if (UI::Vec3Control({ "X", "Y", "Z" }, m_Max))
							{
								size_t count = 0;
								auto mesh = marchingMesh(count);
								if (count)
								{
									m_TestQuadEntity.GetComponent<MeshComponent>().Mesh = mesh;
								}
							}
					}
				);
				UI::TableRow("CellsX",
					[]() { ImGui::Text("CellsX"); },
					[&]() {
						UI::ScopedWidth w(100.0f);
						if (ImGui::InputInt("##SortLayer", (int*)&m_NumCellsX))
						{
							if (m_NumCellsX != 0)
							{
								size_t count = 0;
								auto mesh = marchingMesh(count);
								if (count)
								{
									m_TestQuadEntity.GetComponent<MeshComponent>().Mesh = mesh;
								}
							}
						}
					}
				);
				UI::TableRow("CellsY",
					[]() { ImGui::Text("CellsY"); },
					[&]() { 
						UI::ScopedWidth w(100.0f); 
						if (ImGui::InputInt("##SortLayer", (int*)&m_NumCellsY))
						{
							if (m_NumCellsY != 0)
							{
								size_t count = 0;
								auto mesh = marchingMesh(count);
								if (count)
								{
									m_TestQuadEntity.GetComponent<MeshComponent>().Mesh = mesh;
								}
							}
						}
					}
				);
				UI::TableRow("CellsZ",
					[]() { ImGui::Text("CellsZ"); },
					[&]() { 
						UI::ScopedWidth w(100.0f); 
						if (ImGui::InputInt("##SortLayer", (int*)&m_NumCellsZ))
						{
							if (m_NumCellsZ != 0)
							{
								size_t count = 0;
								auto mesh = marchingMesh(count);
								if (count)
								{
									m_TestQuadEntity.GetComponent<MeshComponent>().Mesh = mesh;
								}
							}
						}
					}
				);
				UI::TableRow("IsoLevel",
					[]() { ImGui::Text("IsoLevel"); },
					[&]() {
						UI::ScopedWidth w(100.0f);
						if (ImGui::DragFloat("##IsoLevelDrag", &m_IsoLevel, 0.05f))
						{			
							size_t count = 0;
							auto mesh = marchingMesh(count);
							if (count)
							{
								m_TestQuadEntity.GetComponent<MeshComponent>().Mesh = mesh;
							}					
						}
					}
				);
				ImGui::EndTable();
			}
			ImGui::End();
		}
	}

	bool GameLayer::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		return false;
	}
	bool GameLayer::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		return false;
	}
	bool GameLayer::onWindowResize(WindowResizeEvent& event)
	{
		return false;
	}

	bool GameLayer::onKeyPress(KeyPressedEvent& event)
	{
		return false;
	}

	bool GameLayer::onKeyRelease(KeyReleasedEvent& event)
	{
		return false;
	}

	void GameLayer::displayStats()
	{
		if (ImGui::Begin("Stats"))
		{
			const auto& stats = Renderer::GetStats();
			ImGui::Text("Draw Arrays: %d", stats.DrawArraysCount);
			ImGui::Text("Draw Indexed: %d", stats.DrawIndexedCount);
			ImGui::Text("Draw Instanced: %d", stats.DrawInstancedCount);
			ImGui::Text("Draw Fullscreen: %d", stats.DrawFullscreenCount);
			ImGui::Text("Draw Indirect: %d", stats.DrawIndirectCount);
			ImGui::Text("Commands Count: %d", stats.CommandsCount);
		}
		ImGui::End();
	}
	Ref<Mesh> GameLayer::marchingMesh(size_t& count)
	{
		std::vector<Triangle> triangles;
		MarchingCubes::PerlinPolygonize((double)m_IsoLevel, m_Min, m_Max, m_NumCellsX, m_NumCellsY, m_NumCellsZ, triangles);

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		uint32_t counter = 0;
		for (const auto& tr : triangles)
		{
			vertices.push_back({ tr[0], glm::vec2(0.0f) });
			vertices.push_back({ tr[1], glm::vec2(0.0f) });
			vertices.push_back({ tr[2], glm::vec2(0.0f) });
			indices.push_back(counter++);
			indices.push_back(counter++);
			indices.push_back(counter++);
		}
		count = indices.size();
		Ref<MeshSource> meshSource = Ref<MeshSource>::Create(vertices, indices);
		Ref<Mesh> mesh = Ref<Mesh>::Create(meshSource);
		return mesh;
	}
}