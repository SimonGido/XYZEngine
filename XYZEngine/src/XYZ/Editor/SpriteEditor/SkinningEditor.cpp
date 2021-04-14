#include "stdafx.h"
#include "SkinningEditor.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"
#include "PreviewRenderer.h"

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
        static uint16_t ToggleBit(uint16_t flags, uint16_t flag)
        {
            uint16_t result = flag;
            if (IS_SET(flags, flag))
                result = 0;
            return result;
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
            m_FoundSubmesh(nullptr),
            m_FoundBone(nullptr),
            m_FoundTriangle(nullptr),
            m_FoundVertex(nullptr),
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
                m_Camera.ProjectionMatrix = glm::ortho(
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
            if (IS_SET(m_Window->Flags, IGWindow::Hoovered))
            {
                m_MousePosition = getMouseWindowSpace();
                m_FoundVertex = findVertex(m_MousePosition).second;
                m_FoundTriangle = findTriangle(m_MousePosition).second;
                m_FoundBone = findBone(m_MousePosition);
                // Bone group
                if (IG::GetUI<IGButton>(m_PoolHandle, 3).Is(IGReturnType::Clicked)) // Preview Pose
                {
                    m_Flags = ToggleBit(m_Flags, PreviewPose);
                    initializePose();
                    updateRenderBuffers();
                }
                else if (IG::GetUI<IGButton>(m_PoolHandle, 5).Is(IGReturnType::Clicked)) // Create bone
                {
                    m_Flags = ToggleBit(m_Flags, CreateBone);
                }
                else if (IG::GetUI<IGButton>(m_PoolHandle, 7).Is(IGReturnType::Clicked)) // Edit bone
                {
                    uint16_t previewPose = m_Flags & PreviewPose;
                    m_Flags = ToggleBit(m_Flags, EditBone);
                    m_Flags |= previewPose;
                }
                else if (IG::GetUI<IGButton>(m_PoolHandle, 9).Is(IGReturnType::Clicked)) // Edit bone
                {
                    m_Flags = ToggleBit(m_Flags, DeleteBone);
                }
                else if (IG::GetUI<IGButton>(m_PoolHandle, 9).Is(IGReturnType::Clicked)) // Delete bone
                {
                    m_Flags = ToggleBit(m_Flags, DeleteBone);
                }
                // Vertex group
                if (IG::GetUI<IGButton>(m_PoolHandle, 13).Is(IGReturnType::Clicked)) // Create Submesh
                {
                    m_Mesh.Submeshes.push_back({});
                }
                else if (IG::GetUI<IGButton>(m_PoolHandle, 15).Is(IGReturnType::Clicked)) // Create Vertex
                {
                    m_Flags = ToggleBit(m_Flags, CreateVertex);
                }
                else if (IG::GetUI<IGButton>(m_PoolHandle, 17).Is(IGReturnType::Clicked)) // Edit Vertex
                {
                    m_Flags = ToggleBit(m_Flags, EditVertex);
                }
                else if (IG::GetUI<IGButton>(m_PoolHandle, 19).Is(IGReturnType::Clicked)) // Delete Vertex
                {
                    m_Flags = ToggleBit(m_Flags, DeleteVertex);
                }
                else if (IG::GetUI<IGButton>(m_PoolHandle, 21).Is(IGReturnType::Clicked)) // Delete Triangle
                {
                    m_Flags = ToggleBit(m_Flags, DeleteTriangle);
                }
                else if (IG::GetUI<IGButton>(m_PoolHandle, 23).Is(IGReturnType::Clicked)) // Clear
                {
                    clear();
                }
                else if (IG::GetUI<IGButton>(m_PoolHandle, 25).Is(IGReturnType::Clicked)) // Triangulate
                {
                    if (!m_Triangulated)
                    {
                        m_Mesh.Triangulate();
                        rebuildRenderBuffers();
                        m_Triangulated = true;
                    }
                    else
                    {
                        XYZ_LOG_WARN("Mesh was already triangulated");
                    }
                }
                // Weight group
                if (IG::GetUI<IGButton>(m_PoolHandle, 29).Is(IGReturnType::Clicked))
                {
                    m_Flags = ToggleBit(m_Flags, WeightBrush);
                }
            }
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
                    if (m_FoundBone) eraseBone(m_FoundBone);
                    m_FoundBone = nullptr;
                    m_SelectedBone = nullptr;
                }
                else if (IS_SET(m_Flags, EditBone))
                {
                    // Edit bone
                }
                else if (IS_SET(m_Flags, CreateVertex))
                {
                    createVertex(m_MousePosition);
                    m_SelectedVertex = nullptr;
                }
                else if (IS_SET(m_Flags, DeleteVertex))
                {
                    m_Mesh.EraseVertexAtPosition(m_MousePosition);
                    m_SelectedVertex = nullptr;
                }
                else if (IS_SET(m_Flags, DeleteTriangle))
                {
                    m_Mesh.EraseTriangleAtPosition(m_MousePosition);
                    m_SelectedVertex = nullptr;
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
        void SkinningEditor::renderAll()
        {
            m_Framebuffer->Bind(); 
            Renderer::SetClearColor(m_Framebuffer->GetSpecification().ClearColor);
            Renderer::Clear();

            renderPreviewMesh(m_Camera.ViewProjectionMatrix);
            PreviewRenderer::RenderSkinnedMesh(m_Mesh, IS_SET(m_Flags, PreviewPose));
            PreviewRenderer::RenderHierarchy(m_BoneHierarchy, IS_SET(m_Flags, PreviewPose));

            m_Framebuffer->Unbind();
        }
      
        void SkinningEditor::renderPreviewMesh(const glm::mat4& viewProjection)
        {
            m_Shader->Bind();
            m_Shader->SetMat4("u_ViewProjectionMatrix", viewProjection);
            if (IS_SET(m_Flags, WeightBrush))
                m_Shader->SetInt("u_ColorEnabled", 1);
            else
                m_Shader->SetInt("u_ColorEnabled", 0);

            m_Shader->SetMat4("u_Transform", glm::mat4(1.0f));
            m_Context->GetTexture()->Bind();

            m_VertexArray->Bind();
            Renderer::DrawIndexed(PrimitiveType::Triangles, m_VertexArray->GetIndexBuffer()->GetCount());

        }
        void SkinningEditor::clear()
        {
            m_Flags = 0;
            m_Mesh.Submeshes.clear();
            m_Mesh.PreviewVertices.clear();
            for (auto bone : m_Bones)
                m_BonePool.Deallocate<PreviewBone>(bone);

            m_Bones.clear();
            m_FoundBone = nullptr;
            m_FoundVertex = nullptr;
            m_FoundTriangle = nullptr;
            m_SelectedBone = nullptr;
            m_SelectedTriangle = nullptr;
            m_SelectedVertex = nullptr;
            m_Triangulated = false;
            rebuildRenderBuffers();
        }
        void SkinningEditor::eraseBone(PreviewBone* bone)
        {
            for (size_t i = 0; i < m_Bones.size(); ++i)
            {
                if (m_Bones[i]->ID == bone->ID)
                {
                    m_Bones.erase(m_Bones.begin() + i);
                    break;
                }
            }
            m_BoneHierarchy.TraverseNodeChildren(bone->ID, [&](void* parent, void* child)->bool {

                PreviewBone* childBone = static_cast<PreviewBone*>(child);           
                for (size_t i = 0; i < m_Bones.size(); ++i)
                {
                    if (m_Bones[i]->ID == childBone->ID)
                    {
                        m_Bones.erase(m_Bones.begin() + i);
                        break;
                    }
                }
                m_BonePool.Deallocate<PreviewBone>(childBone);
                return false;
                });

            m_BoneHierarchy.Remove(bone->ID);
            m_Tree->RemoveItem(bone->ID);
            m_BonePool.Deallocate<PreviewBone>(bone);
        }
        void SkinningEditor::createVertex(const glm::vec2& pos)
        {
            m_SelectedVertex = nullptr;
            m_FoundVertex = nullptr;
            if (!m_Triangulated)
            {
                if (m_Mesh.Submeshes.empty()) m_Mesh.Submeshes.push_back({});
                m_Mesh.Submeshes.back().Vertices.push_back({ m_MousePosition });
            }
            else
            {
                XYZ_LOG_WARN("Mesh was already triangulated, can not add new vertices");
            }
        }
        void SkinningEditor::initializePose()
        {
            m_BoneHierarchy.Traverse([](void* parent, void* child) -> bool {

                PreviewBone* childBone = static_cast<PreviewBone*>(child);
                childBone->LocalTransform = glm::translate(glm::vec3(childBone->LocalPosition, 0.0f));

                return false;
            });
        }
        void SkinningEditor::updateBoneHierarchy()
        {
            m_BoneHierarchy.Traverse([](void* parent, void* child) -> bool {

                PreviewBone* childBone = static_cast<PreviewBone*>(child);
                if (parent)
                {
                    PreviewBone* parentBone = static_cast<PreviewBone*>(parent);
                    childBone->WorldTransform = parentBone->WorldTransform * childBone->LocalTransform;
                }
                else
                {
                    childBone->WorldTransform = childBone->LocalTransform;
                }
                return false;
           });
        }
        void SkinningEditor::updateRenderBuffers()
        {
            m_Mesh.BuildPreviewVertices(m_BoneHierarchy, m_ContextSize, IS_SET(m_Flags, PreviewPose), IS_SET(m_Flags, WeightBrush));
            m_VertexBuffer->Update(m_Mesh.PreviewVertices.data(), m_Mesh.PreviewVertices.size() * sizeof(PreviewVertex));
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
                Helper::GenerateQuad(m_Mesh.PreviewVertices, m_ContextSize);
                indices = { 0,1,2,2,3,0 };
            }
            
            m_Mesh.BuildPreviewVertices(m_BoneHierarchy, m_ContextSize, IS_SET(m_Flags, PreviewPose), IS_SET(m_Flags, WeightBrush));
            m_VertexArray = VertexArray::Create();
            m_VertexBuffer = VertexBuffer::Create(m_Mesh.PreviewVertices.data(), m_Mesh.PreviewVertices.size() * sizeof(PreviewVertex), BufferUsage::Dynamic);
            m_VertexBuffer->SetLayout({
                {0, ShaderDataComponent::Float3, "a_Color"},
                {1, ShaderDataComponent::Float3, "a_Position"},
                {2, ShaderDataComponent::Float2, "a_TexCoord"}
               });
            m_VertexArray->AddVertexBuffer(m_VertexBuffer);

            Ref<IndexBuffer> ibo = IndexBuffer::Create(indices.data(), indices.size());
            m_VertexArray->SetIndexBuffer(ibo);
        }

        void SkinningEditor::handleBoneEdit()
        {
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


