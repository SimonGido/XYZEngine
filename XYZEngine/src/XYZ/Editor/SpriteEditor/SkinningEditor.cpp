#include "stdafx.h"
#include "SkinningEditor.h"

#include "XYZ/Core/Input.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {
	namespace Editor {
        namespace Helper {
            void GenerateQuad(std::vector<PreviewVertex>& vertices, const glm::vec2& size)
            {
                vertices.push_back({
                    glm::vec3(0.0f),
                    glm::vec3(-size.x / 2.0f, -size.y / 2.0f, 0.0f),
                    glm::vec2(0.0f)
                    });
                vertices.push_back({
                    glm::vec3(0.0f),
                    glm::vec3(size.x / 2.0f, -size.y / 2.0f, 0.0f),
                    glm::vec2(1.0f,0.0f)
                    });
                vertices.push_back({
                    glm::vec3(0.0f),
                    glm::vec3(size.x / 2.0f,  size.y / 2.0f, 0.0f),
                    glm::vec2(1.0f)
                    });
                vertices.push_back({
                    glm::vec3(0.0f),
                    glm::vec3(-size.x / 2.0f, size.y / 2.0f, 0.0f),
                    glm::vec2(0.0f, 1.0f)
                    });
            }
            static glm::vec3 HSVtoRGB(float H, float S, float V)
            {
                XYZ_ASSERT(!(H > 360 || H < 0 || S > 100 || S < 0 || V > 100 || V < 0), "");

                float s = S / 100;
                float v = V / 100;
                float C = s * v;
                float X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
                float m = v - C;
                float r, g, b;
                if (H >= 0 && H < 60)
                    r = C, g = X, b = 0;
                else if (H >= 60 && H < 120)
                    r = X, g = C, b = 0;
                else if (H >= 120 && H < 180)
                    r = 0, g = C, b = X;
                else if (H >= 180 && H < 240)
                    r = 0, g = X, b = C;
                else if (H >= 240 && H < 300)
                    r = X, g = 0, b = C;
                else
                    r = C, g = 0, b = X;

                float R = (r + m);
                float G = (g + m);
                float B = (b + m);

                return { R, G, B };
            }
            static glm::vec3 RandomColor(uint32_t index)
            {
                std::random_device dev;
                std::mt19937 rng(dev());
                std::uniform_real_distribution<float> sDist(90.0f, 100.0f);
                std::uniform_real_distribution<float> vDist(50.0f, 100.0f);

                float h = (float)index * (360.0f / (float)SkinningEditor::MaxBones);
                //float s = sDist(rng);
                //float v = vDist(rng);
                float s = 80.0f;
                float v = 60.0f;
                return HSVtoRGB(h, s, v);
            }

            glm::vec2 CalculateTexCoord(const glm::vec2& pos, const glm::vec2& size)
            {
                glm::vec2 position = pos + size / 2.0f;
                return glm::vec2(position.x / size.x, position.y / size.y);
            }
        }
        SkinningEditor::SkinningEditor()
            :
            m_ContextSize(glm::vec2(0.0f)),
            m_Window(nullptr),
            m_Tree(nullptr),
            m_BonePool(15 * sizeof(PreviewBone)),
            m_SelectedSubmesh(nullptr),
            m_SelectedVertex(nullptr),
            m_SelectedTriangle(nullptr),
            m_SelectedBone(nullptr),
            m_WeightBrushRadius(15.0f),
            m_WeightBrushStrength(0.1f),
            m_Flags(0)
        {
            m_Shader = Shader::Create("Assets/Shaders/SkinningEditor.glsl");
            for (uint32_t i = 0; i < NumCategories; ++i)
                m_CategoriesOpen[i] = false;

            std::vector<IGHierarchyElement> elements;
            elements.push_back({ IGElementType::ImageWindow, {} });
            SetupUI(elements[0].Children);
            auto [poolHandle, handleCount] = IG::AllocateUI(elements);
            m_PoolHandle = poolHandle;
            m_Window = &IG::GetUI<IGImageWindow>(m_PoolHandle, 0);
            m_Window->Label = "Skinning Editor";
            m_Tree = &IG::GetUI<IGTree>(m_PoolHandle, 31);
            m_ViewportSize = m_Window->Size;

            FramebufferSpecs specs;
            specs.Width = (uint32_t)m_ViewportSize.x;
            specs.Height = (uint32_t)m_ViewportSize.y;
            specs.ClearColor = { 0.1f,0.1f,0.1f,1.0f };
            specs.Attachments = {
                FramebufferTextureSpecs(FramebufferTextureFormat::RGBA16F),
                FramebufferTextureSpecs(FramebufferTextureFormat::DEPTH24STENCIL8)
            };
            m_Framebuffer = Framebuffer::Create(specs);
            m_RenderTexture = RenderTexture::Create(m_Framebuffer);
            m_RenderSubTexture = Ref<SubTexture>::Create(m_RenderTexture, glm::vec4(0.0f, 1.0f, 1.0f, 0.0f));
            m_Window->SubTexture = m_RenderSubTexture;
            m_Window->ResizeCallback = [&](const glm::vec2& size) {
                m_Framebuffer->Resize((uint32_t)size.x, (uint32_t)size.y);
                m_Camera.ProjectionMatrix  = glm::ortho(
                    -m_Window->Size.x / 2.0f, m_Window->Size.x / 2.0f, 
                    -m_Window->Size.y / 2.0f, m_Window->Size.y / 2.0f
                );
                m_Camera.UpdateViewProjection();
            };

            rebuildRenderBuffers();
        }
        void SkinningEditor::SetContext(Ref<SubTexture> context)
        {
            m_Context = context;
            m_ContextSize.x = (float)m_Context->GetTexture()->GetWidth();
            m_ContextSize.y = (float)m_Context->GetTexture()->GetHeight();
        }
        void SkinningEditor::OnUpdate(Timestep ts)
		{
            if (m_Window)
            m_MousePosition = getMouseWindowSpace();
		}

        void SkinningEditor::OnEvent(Event& event)
        {
            EventDispatcher dispatcher(event);
            if (IS_SET(m_Window->Flags, IGWindow::Hoovered))
            {
                dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&SkinningEditor::onMouseButtonPress, this));
                dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&SkinningEditor::onMouseButtonRelease, this));
                dispatcher.Dispatch<MouseScrollEvent>(Hook(&SkinningEditor::onMouseScroll, this));
                dispatcher.Dispatch<KeyPressedEvent>(Hook(&SkinningEditor::onKeyPress, this));
            }
        }
        void SkinningEditor::SetupUI(std::vector<IGHierarchyElement>& elements)
		{
            elements.push_back({IGElementType::Group, { // Bones
                {IGElementType::Separator, {}},
                {IGElementType::Button, {}}, // Preview pose
                {IGElementType::Separator, {}},
                {IGElementType::Button, {}}, // Create bone
                {IGElementType::Separator, {}},
                {IGElementType::Button, {}}, // Edit bone
                {IGElementType::Separator, {}},
                {IGElementType::Button, {}}  // Delete bone
                }});
            elements.push_back({ IGElementType::Separator, {} });
            elements.push_back({ IGElementType::Group, { // Vertices
                {IGElementType::Separator, {}},
                {IGElementType::Button, {}},  // Create submesh
                {IGElementType::Separator, {}},
                {IGElementType::Button, {}},  // Create vertex
                {IGElementType::Separator, {}},
                {IGElementType::Button, {}},  // Edit vertex
                {IGElementType::Separator, {}},
                {IGElementType::Button, {}},  // Delete vertex
                {IGElementType::Separator, {}},
                {IGElementType::Button, {}},  // Delete triangle
                {IGElementType::Separator, {}},
                {IGElementType::Button, {}},  // Clear
                {IGElementType::Separator, {}},
                {IGElementType::Button, {}}   // Triangulate
                } });

            elements.push_back({ IGElementType::Separator, {} });
            elements.push_back({ IGElementType::Group, { // Weights
                {IGElementType::Separator, {}},
                {IGElementType::Button, {}}  // Weigths brush
                } });
            elements.push_back({ IGElementType::Separator, {} });
            elements.push_back({ IGElementType::Tree, {} });
		}
        bool SkinningEditor::onMouseButtonPress(MouseButtonPressEvent& event)
        {
            if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
            {
                if (IS_SET(m_Flags, CreateBone))
                {
                    // Create bone
                }
                else if (IS_SET(m_Flags, DeleteBone))
                {
                    // Delete bone
                }
                else if (IS_SET(m_Flags, EditBone))
                {
                    // Edit bone
                }
                else if (IS_SET(m_Flags, CreateVertex))
                {
                    // Create vertex
                }
                else if (IS_SET(m_Flags, DeleteVertex))
                {
                    // Delete vertex
                }
                else if (IS_SET(m_Flags, DeleteTriangle))
                {
                    // Delete triangle
                }
            }
            return false;
        }
        bool SkinningEditor::onMouseButtonRelease(MouseButtonReleaseEvent& event)
        {
            return false;
        }
        bool SkinningEditor::onMouseScroll(MouseScrollEvent& event)
        {
            return false;
        }
        bool SkinningEditor::onKeyPress(KeyPressedEvent& event)
        {
            return false;
        }
        void SkinningEditor::updateRenderBuffers()
        {
            buildPreviewVertices();
            m_VertexBuffer->Update(m_PreviewVertices.data(), m_PreviewVertices.size() * sizeof(PreviewVertex));
        }
        void SkinningEditor::rebuildRenderBuffers()
        {
            std::vector<uint32_t> indices;
            uint32_t offset = 0;
            for (auto& subMesh : m_Mesh.Submeshes)
            {
                for (auto& triangle : subMesh.Triangles)
                {
                    indices.push_back(triangle.First + offset);
                    indices.push_back(triangle.Second + offset);
                    indices.push_back(triangle.Third + offset);
                }
                offset += subMesh.Vertices.size();
            }
            if (indices.empty())
            {
                Helper::GenerateQuad(m_PreviewVertices, m_ContextSize);
                indices = { 0,1,2,2,3,0 };
            }
            buildPreviewVertices();

            m_VertexArray = VertexArray::Create();
            m_VertexBuffer = VertexBuffer::Create(m_PreviewVertices.data(), m_PreviewVertices.size() * sizeof(PreviewVertex), BufferUsage::Dynamic);
            m_VertexBuffer->SetLayout({
                {0, ShaderDataComponent::Float3, "a_Color"},
                {1, ShaderDataComponent::Float3, "a_Position"},
                {2, ShaderDataComponent::Float2, "a_TexCoord"}
               });
            m_VertexArray->AddVertexBuffer(m_VertexBuffer);

            Ref<IndexBuffer> ibo = IndexBuffer::Create(indices.data(), indices.size());
            m_VertexArray->SetIndexBuffer(ibo);
        }
        void SkinningEditor::buildPreviewVertices()
        {
            m_PreviewVertices.clear();
            for (auto& subMesh : m_Mesh.Submeshes)
            {
                for (auto& vertex : subMesh.Vertices)
                {
                    m_PreviewVertices.push_back({
                        glm::vec3(0.0f),
                        glm::vec3(vertex.Position.x, vertex.Position.y, 0.0f),
                        Helper::CalculateTexCoord(vertex.Position,m_ContextSize)
                   });
                }
            }
        }
        glm::vec2 SkinningEditor::getMouseWindowSpace() const
        {
            auto [mx, my] = getMouseViewportSpace();
            mx *= m_Window->Size.x / 2.0f;
            my *= m_Window->Size.y / 2.0f;
            mx += m_Camera.Position.x;
            my += m_Camera.Position.y;
            return { mx , my };
        }

        std::pair<float, float> SkinningEditor::getMouseViewportSpace() const
        {
            auto [mx, my] = Input::GetMousePosition();
            glm::vec2 position = m_Window->GetAbsolutePosition();
            mx -= position.x;
            my -= position.y;

            auto viewportWidth =  m_Window->Size.x;
            auto viewportHeight = m_Window->Size.y;

            return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
        }

        std::pair<Submesh*, BoneVertex*> SkinningEditor::findVertex(const glm::vec2& pos)
        {
            for (auto& subMesh : m_Mesh.Submeshes)
            {
                if (BoneVertex* vertex = subMesh.FindVertex(pos, SkinnedMesh::PointRadius))
                    return { &subMesh, vertex };
            }
            return std::pair<Submesh*, BoneVertex*>(nullptr, nullptr);
        }
        std::pair<Submesh*, Triangle*> SkinningEditor::findTriangle(const glm::vec2& pos)
        {
            for (auto& subMesh : m_Mesh.Submeshes)
            {
                if (Triangle* triangle = subMesh.FindTriangle(pos, SkinnedMesh::PointRadius))
                    return { &subMesh, triangle };
            }
            return std::pair<Submesh*, Triangle*>();
        }
        PreviewBone* SkinningEditor::findBone(const glm::vec2& pos) const
        {
            PreviewBone* bone = nullptr;
            m_BoneHierarchy.Traverse([&](void* parent, void* child) -> bool {
                PreviewBone* childBone = static_cast<PreviewBone*>(child);
                if (childBone->Collide(pos, (PreviewBone::Start | PreviewBone::End | PreviewBone::Body)))
                {
                    bone = childBone;
                    return true;
                }
                return false;
            });
            return bone;
        }
	}
}


