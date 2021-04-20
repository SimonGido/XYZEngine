#include "stdafx.h"
#include "SkinningEditor.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"
#include "PreviewRenderer.h"

#include <glm/gtx/transform.hpp>



namespace XYZ {
    namespace Editor {

        static uint32_t s_NextBone = 1;

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
            m_WeightBrushStrength(0.0f),
            m_HighlightColor(glm::vec4(0.8f, 0.9f, 1.0f, 1.0f)),
            m_UIHighlightColor(glm::vec4(1.9f, 1.8f, 2.0f, 1.0f)),
            m_Flags(0),
            m_BoneEditFlags(0)
        {
            m_Shader = Shader::Create("Assets/Shaders/SkinningEditor.glsl");
            for (uint32_t i = 0; i < NumCategories; ++i)
                m_CategoriesOpen[i] = false;

            std::vector<IGHierarchyElement> elements;
            elements.push_back({ IGElementType::ImageWindow, {} });
            PushUI(elements[0].Children);
            auto [poolHandle, handleCount] = IG::AllocateUI(elements);
            m_PoolHandle = poolHandle;
            SetupUI();
            m_Window = &IG::GetUI<IGImageWindow>(m_PoolHandle, 0);
            m_Window->Label = "Skinning Editor";
            m_Tree = &IG::GetUI<IGTree>(m_PoolHandle, 33);
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
            m_RenderSubTexture = Ref<SubTexture>::Create(m_RenderTexture, glm::vec2(0.0f, 0.0f));
            m_Window->SubTexture = m_RenderSubTexture;
            
            rebuildRenderBuffers();
            m_Window->ResizeCallback = [&](const glm::vec2& size) {
                m_Framebuffer->Resize((uint32_t)size.x, (uint32_t)size.y);
                m_Camera.ProjectionMatrix = glm::ortho(
                    -m_Window->Size.x / 2.0f, m_Window->Size.x / 2.0f,
                    -m_Window->Size.y / 2.0f, m_Window->Size.y / 2.0f
                );
                m_Camera.UpdateViewProjection();
            };

            for (uint32_t i = 0; i < MaxBones; ++i)
                m_ColorIDs[i] = i;
            std::shuffle(&m_ColorIDs[0], &m_ColorIDs[MaxBones - 1], std::default_random_engine(0));
        }
        void SkinningEditor::SetContext(Ref<SubTexture> context)
        {
            m_Context = context;
            m_ContextSize.x = (float)m_Context->GetTexture()->GetWidth();
            m_ContextSize.y = (float)m_Context->GetTexture()->GetHeight();
            rebuildRenderBuffers();
        }
        void SkinningEditor::OnUpdate(Timestep ts)
        {
            if (IS_SET(m_Window->Flags, IGWindow::Hoovered))
            {
                m_MousePosition = getMouseWindowSpace();
                auto [subMesh, triangle] = findTriangle(m_MousePosition);
                m_FoundSubmesh = subMesh;
                m_FoundTriangle = triangle;
                m_FoundVertex = findVertex(m_MousePosition).second;
                m_FoundBone = findBone(m_MousePosition);
                m_Camera.Update(ts);
                
                if (IS_SET(m_Flags, EditBone))
                {
                    handleBoneEdit();
                }
                else if (IS_SET(m_Flags, EditVertex))
                {
                    handleVertexEdit();
                }
                else if (IS_SET(m_Flags, WeightBrush))
                {
                    m_WeightBrushStrength = IG::GetUI<IGSlider>(m_PoolHandle, 31).Value / sc_WeightBrushDivisor;
                    handleWeightsBrush();
                }
             
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
            
            updateBoneHierarchy();
            if (IS_SET(m_Flags, PreviewPose))
                updateRenderBuffers();
            updateUIColor();
            renderAll();
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
        void SkinningEditor::PushUI(std::vector<IGHierarchyElement>& elements)
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
                {IGElementType::Button, {}},  // Weigths brush
                {IGElementType::Separator, {}},
                {IGElementType::Slider, {}}
                } });
            elements.push_back({ IGElementType::Separator, {} });
            elements.push_back({ IGElementType::Tree, {} });
		}
        void SkinningEditor::SetupUI()
        {
            IG::ForEach<IGSeparator>(m_PoolHandle, [](IGSeparator& separator) {
                separator.Flags |= IGSeparator::AdjustToRoot;
                });
            IG::ForEach<IGButton>(m_PoolHandle, [](IGButton& button) {
                button.Size.y = 30.0f;
                });

            auto& boneGroup = IG::GetUI<IGGroup>(m_PoolHandle, 1);
            boneGroup.Style.Layout.SpacingY = 0.0f;
            boneGroup.AdjustToParent = false;
            boneGroup.Size.x = 150.0f;
            boneGroup.Label = "Bone";
            IG::GetUI<IGButton>(m_PoolHandle, 3).Label = "Preview Pose";
            IG::GetUI<IGButton>(m_PoolHandle, 5).Label = "Create Bone";
            IG::GetUI<IGButton>(m_PoolHandle, 7).Label = "Edit Bone";
            IG::GetUI<IGButton>(m_PoolHandle, 9).Label = "Delete Bone";

            auto& vertexGroup = IG::GetUI<IGGroup>(m_PoolHandle, 11);
            vertexGroup.Style.Layout.SpacingY = 0.0f;
            vertexGroup.AdjustToParent = false;
            vertexGroup.Size.x = 150.0f;
            vertexGroup.Label = "Vertex";
            IG::GetUI<IGButton>(m_PoolHandle, 13).Label = "Create Submesh";
            IG::GetUI<IGButton>(m_PoolHandle, 15).Label = "Create Vertex";
            IG::GetUI<IGButton>(m_PoolHandle, 17).Label = "Edit Vertex";
            IG::GetUI<IGButton>(m_PoolHandle, 19).Label = "Delete Vertex";
            IG::GetUI<IGButton>(m_PoolHandle, 21).Label = "Delete Triangle";
            IG::GetUI<IGButton>(m_PoolHandle, 23).Label = "Clear";
            IG::GetUI<IGButton>(m_PoolHandle, 25).Label = "Triangulate";
            
            auto& weightGroup = IG::GetUI<IGGroup>(m_PoolHandle, 27);
            weightGroup.Style.Layout.SpacingY = 0.0f;
            weightGroup.AdjustToParent = false;
            weightGroup.Size.x = 150.0f;
            weightGroup.Label = "Weight";
            IG::GetUI<IGButton>(m_PoolHandle, 29).Label = "Weight Brush";
            IG::GetUI<IGSlider>(m_PoolHandle, 31).Label = "Brush Strength";
        }
        bool SkinningEditor::onMouseButtonPress(MouseButtonPressEvent& event)
        {
            if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_LEFT))
            {          
                handleSelection();
            }
            else if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
            {
                if (IS_SET(m_Flags, CreateBone))
                {
                    createBone();
                    m_BoneEditFlags = PreviewBone::End;
                    if (Input::IsKeyPressed(KeyCode::KEY_LEFT_CONTROL))
                        m_Flags |= EditBone;
                    else
                        m_Flags = EditBone;
                }
                else if (IS_SET(m_Flags, DeleteBone))
                {
                    if (m_SelectedBone) eraseBone(m_SelectedBone);
                    m_FoundBone = nullptr;
                    m_SelectedBone = nullptr;
                }
                else if (IS_SET(m_Flags, EditBone))
                {
                    if (m_SelectedBone)
                        m_BoneEditFlags = findEditBoneFlag(*m_SelectedBone);
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
            else if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_MIDDLE))
            {
                m_SelectedBone = nullptr;
                m_SelectedVertex = nullptr;
                m_SelectedTriangle = nullptr;
                m_SelectedSubmesh = nullptr;
            }
            return false;
        }
        bool SkinningEditor::onMouseButtonRelease(MouseButtonReleaseEvent& event)
        {
            if (event.IsButtonReleased(MouseCode::MOUSE_BUTTON_RIGHT))
            {
                m_BoneEditFlags = 0;
            }
            return false;
        }
        bool SkinningEditor::onMouseScroll(MouseScrollEvent& event)
        {
            if (IS_SET(m_Flags, WeightBrush))
            {
                m_WeightBrushRadius -= event.GetOffsetY();
            }
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
            Renderer2D::BeginScene(m_Camera.ViewProjectionMatrix);
            PreviewRenderer::RenderSkinnedMesh(m_Mesh, IS_SET(m_Flags, PreviewPose));
            PreviewRenderer::RenderHierarchy(m_BoneHierarchy, IS_SET(m_Flags, PreviewPose));
            Renderer2D::FlushLines();

            Renderer::SetLineThickness(4.0f);
            renderSelection();

            if (IS_SET(m_Flags, WeightBrush))
                Renderer2D::SubmitCircle(glm::vec3(m_MousePosition, 0.0f), m_WeightBrushRadius, 20);

            Renderer2D::FlushLines();

            Renderer2D::EndScene();
            Renderer::SetLineThickness(2.0f);
            m_Framebuffer->Unbind();
        }

        void SkinningEditor::renderSelection()
        {
            if (!IS_SET(m_Flags, PreviewPose))
            {
                if (m_FoundSubmesh)
                {
                    if (m_FoundTriangle)
                        PreviewRenderer::RenderTriangle(*m_FoundSubmesh, *m_FoundTriangle, m_HighlightColor);
                    if (m_FoundVertex)
                        Renderer2D::SubmitCircle(glm::vec3(m_FoundVertex->Position, 0.0f), SkinnedMesh::PointRadius, 20, m_HighlightColor);

                    if (m_SelectedTriangle)
                        PreviewRenderer::RenderTriangle(*m_FoundSubmesh, *m_SelectedTriangle, m_HighlightColor);
                    if (m_SelectedVertex)
                        Renderer2D::SubmitCircle(glm::vec3(m_SelectedVertex->Position, 0.0f), SkinnedMesh::PointRadius, 20, m_HighlightColor);
                }
            }
            if (m_FoundBone)
                PreviewRenderer::RenderBone(*m_FoundBone, m_HighlightColor, PreviewBone::PointRadius, IS_SET(m_Flags, PreviewPose));   
            if (m_SelectedBone)
                PreviewRenderer::RenderBone(*m_SelectedBone, m_HighlightColor, PreviewBone::PointRadius, IS_SET(m_Flags, PreviewPose));
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
        void SkinningEditor::updateUIColor()
        {
            if (IS_SET(m_Flags, PreviewPose))
                IG::GetUI<IGButton>(m_PoolHandle, 3).Color = m_UIHighlightColor;
            if (IS_SET(m_Flags, CreateBone))
                IG::GetUI<IGButton>(m_PoolHandle, 5).Color = m_UIHighlightColor;
            if (IS_SET(m_Flags, EditBone))
                IG::GetUI<IGButton>(m_PoolHandle, 7).Color = m_UIHighlightColor;
            if (IS_SET(m_Flags, DeleteBone))
                IG::GetUI<IGButton>(m_PoolHandle, 9).Color = m_UIHighlightColor;
          

            if (IS_SET(m_Flags, CreateVertex))
                IG::GetUI<IGButton>(m_PoolHandle, 15).Color = m_UIHighlightColor;
            if (IS_SET(m_Flags, EditVertex))
                IG::GetUI<IGButton>(m_PoolHandle, 17).Color = m_UIHighlightColor;
            if (IS_SET(m_Flags, DeleteVertex))
                IG::GetUI<IGButton>(m_PoolHandle, 19).Color = m_UIHighlightColor;
            if (IS_SET(m_Flags, DeleteTriangle))
                IG::GetUI<IGButton>(m_PoolHandle, 21).Color = m_UIHighlightColor;
            
            if (IS_SET(m_Flags, WeightBrush))
                IG::GetUI<IGButton>(m_PoolHandle, 29).Color = m_UIHighlightColor;
        }
        void SkinningEditor::clear()
        {
            m_Flags = 0;
            m_Mesh.Submeshes.clear();
            m_Mesh.PreviewVertices.clear();
            m_Tree->Clear();
            m_BoneHierarchy.Clear();
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
                if (m_Mesh.Submeshes.empty()) 
                    m_Mesh.Submeshes.push_back({});
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
            if (IS_SET(m_Flags, PreviewPose))
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
            else
            {
                m_BoneHierarchy.Traverse([](void* parent, void* child) -> bool {

                    PreviewBone* childBone = static_cast<PreviewBone*>(child);
                    childBone->WorldPosition = childBone->LocalPosition;
                    if (parent)
                    {
                        PreviewBone* parentBone = static_cast<PreviewBone*>(parent);
                        childBone->WorldPosition += parentBone->WorldPosition;
                    }
                    return false;
                });
            }
        }
        void SkinningEditor::updateRenderBuffers()
        {
            if (m_Triangulated)
            {
                m_Mesh.PreviewVertices.clear();
                m_Mesh.BuildPreviewVertices(m_BoneHierarchy, m_ContextSize, IS_SET(m_Flags, PreviewPose), IS_SET(m_Flags, WeightBrush));
                m_VertexBuffer->Update(m_Mesh.PreviewVertices.data(), m_Mesh.PreviewVertices.size() * sizeof(PreviewVertex));
            }
        }
        void SkinningEditor::rebuildRenderBuffers()
        {
            m_Mesh.PreviewVertices.clear();
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

        void SkinningEditor::createBone()
        {
            PreviewBone* newBone = m_BonePool.Allocate<PreviewBone>();
            m_Bones.push_back(newBone);
          
            char buffer[20];
            sprintf(buffer, "bone_%u", s_NextBone);
            newBone->Name = buffer;
            newBone->Color = Helper::RandomColor(m_ColorIDs[s_NextBone++]);        
            newBone->WorldPosition = m_MousePosition;

            if (m_SelectedBone)
            {
                newBone->ID = m_BoneHierarchy.Insert(newBone, m_SelectedBone->ID);
                m_Tree->AddItem(newBone->ID, m_SelectedBone->ID, IGTreeItem(newBone->Name));             
                newBone->LocalPosition = m_MousePosition - m_SelectedBone->WorldPosition;
            }
            else
            {
                newBone->ID = m_BoneHierarchy.Insert(newBone);
                m_Tree->AddItem(newBone->ID, IGTreeItem(newBone->Name));
                newBone->LocalPosition = m_MousePosition;
            }

            m_SelectedBone = newBone;
        }


        void SkinningEditor::handleSelection()
        {
            // Tool that can modify triangle/vertex is enabled
            if (m_Flags > DeleteBone && m_Flags < WeightBrush)
            {
                m_SelectedSubmesh = m_FoundSubmesh;
                if (m_SelectedVertex != m_FoundVertex)
                    m_SelectedVertex = m_FoundVertex;
                else
                    m_SelectedVertex = nullptr;

                if (m_SelectedTriangle != m_FoundTriangle)
                    m_SelectedTriangle = m_FoundTriangle;
                else
                    m_SelectedTriangle = nullptr;
            }
            else
            {
                m_SelectedSubmesh  = nullptr;
                m_SelectedVertex   = nullptr;
                m_SelectedTriangle = nullptr;
            }

            // Tool that can modify bone is enabled
            if (m_Flags < CreateVertex || m_Flags > DeleteTriangle)
            {
                if (m_SelectedBone != m_FoundBone)
                    m_SelectedBone = m_FoundBone;
                else
                    m_SelectedBone = nullptr;
            }
            else
            {
                
                m_SelectedBone     = nullptr;
            }
        }

        void SkinningEditor::handleBoneEdit()
        {
            if (m_SelectedBone)
            {
                if (IS_SET(m_Flags, PreviewPose))
                {
                    if (IS_SET(m_BoneEditFlags, PreviewBone::Start))
                    {
                        glm::mat4 parentSpace = glm::mat4(1.0f);
                        if (auto parent = m_BoneHierarchy.GetParentData(m_SelectedBone->ID))
                            parentSpace = static_cast<PreviewBone*>(parent)->WorldTransform;
                        m_SelectedBone->Translate(m_MousePosition, parentSpace);
                    }
                    else if (IS_SET(m_BoneEditFlags, (PreviewBone::Body | PreviewBone::End)))
                    {
                        m_SelectedBone->Rotate(m_MousePosition);
                    }
                }
                else
                {
                    if (IS_SET(m_BoneEditFlags, (PreviewBone::Start | PreviewBone::Body)))
                    {
                        m_SelectedBone->LocalPosition = m_MousePosition;
                        m_SelectedBone->WorldPosition = m_MousePosition;
                        if (auto parent = m_BoneHierarchy.GetParentData(m_SelectedBone->ID))
                        {
                            PreviewBone* parentBone = static_cast<PreviewBone*>(parent);
                            m_SelectedBone->LocalPosition -= parentBone->WorldPosition;
                        }
                    }
                    else if (IS_SET(m_BoneEditFlags, PreviewBone::End))
                    {
                        m_SelectedBone->Direction = glm::normalize(m_MousePosition - m_SelectedBone->WorldPosition);
                        m_SelectedBone->Length = glm::distance(m_MousePosition, m_SelectedBone->WorldPosition);
                    }
                }
            }
        }

        void SkinningEditor::handleVertexEdit()
        {
            if (m_SelectedVertex)
            {
                m_SelectedVertex->Position.x = m_MousePosition.x;
                m_SelectedVertex->Position.y = m_MousePosition.y;
                updateRenderBuffers();
            }
        }

        void SkinningEditor::handleWeightsBrush()
        {
            if (!Input::IsMouseButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
                return;

            auto clamp = [](BoneVertex* vertex, uint32_t index) {
                if (vertex->Data.Weights[index] > 1.0f)
                {
                    vertex->Data.Weights[index] = 1.0f;
                }
                else if (vertex->Data.Weights[index] < 0.0f)
                {
                    vertex->Data.Weights[index] = 0.0f;
                    vertex->Data.IDs[index] = -1;
                }
            };
            
            std::vector<BoneVertex*> vertices = std::move(findVerticesInRadius(m_MousePosition, m_WeightBrushRadius));
            if (m_SelectedBone)
            {
                float val = 1.0f;
                if (Input::IsKeyPressed(KeyCode::KEY_LEFT_CONTROL))
                    val = -1.0f;

                for (auto vertex : vertices)
                {
                    float dist = glm::distance(vertex->Position, m_MousePosition);
                    bool exists = false;
                    for (uint32_t i = 0; i < BoneData::sc_MaxBonesPerVertex; ++i)
                    {
                        if (vertex->Data.IDs[i] == m_SelectedBone->ID)
                        {
                            exists = true;
                            vertex->Data.Weights[i] += val * (1.0f - (dist / m_WeightBrushRadius)) * m_WeightBrushStrength;
                            clamp(vertex, i);
                        }
                    }
                    if (!exists)
                    {
                        for (uint32_t i = 0; i < BoneData::sc_MaxBonesPerVertex; ++i)
                        {
                            if (vertex->Data.IDs[i] == -1)
                            {
                                vertex->Data.IDs[i] = m_SelectedBone->ID;
                                vertex->Data.Weights[i] += val * (1.0f - (dist / m_WeightBrushRadius)) * m_WeightBrushStrength;
                                clamp(vertex, i);
                                break;
                            }
                        }
                    }
                }
                updateRenderBuffers();
            }          
        }

        uint8_t SkinningEditor::findEditBoneFlag(const PreviewBone& bone)
        {
            uint8_t flags = 0;
            if (m_SelectedBone->Collide(m_MousePosition, PreviewBone::Body, IS_SET(m_Flags, PreviewPose)))
                flags = PreviewBone::Body;
            if (m_SelectedBone->Collide(m_MousePosition, PreviewBone::Start, IS_SET(m_Flags, PreviewPose)))
                flags = PreviewBone::Start;
            if (m_SelectedBone->Collide(m_MousePosition, PreviewBone::End, IS_SET(m_Flags, PreviewPose)))
                flags = PreviewBone::End;

            return flags;
        }

        std::vector<BoneVertex*> SkinningEditor::findVerticesInRadius(const glm::vec2& pos, float radius)
        {
            std::vector<BoneVertex*> vertices;
            for (auto& subMesh : m_Mesh.Submeshes)
            {
                for (auto& vertex : subMesh.Vertices)
                {
                    if (glm::distance(vertex.Position, pos) < radius)
                    {
                        vertices.push_back(&vertex);
                    }
                }
            }
            return vertices;
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
                if (childBone->Collide(pos, (PreviewBone::Start | PreviewBone::End | PreviewBone::Body), IS_SET(m_Flags, PreviewPose)))
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


