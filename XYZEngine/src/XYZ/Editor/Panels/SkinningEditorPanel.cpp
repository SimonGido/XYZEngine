#include "stdafx.h"
#include "SkinningEditorPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"

#include "XYZ/Utils/Math/Math.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <tpp_interface.hpp>

namespace XYZ {

    static uint32_t s_NextBone = 1;

    static uint16_t ToggleBit(uint16_t flags, uint16_t flag)
    {
        uint16_t result = flag;
        if (IS_SET(flags, flag))
            result = 0;
        return result;
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

        float h = (float)index * (360.0f / (float)MAX_BONES);
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


    SkinningEditorPanel::SkinningEditorPanel(uint32_t panelID)
        :
        m_PanelID(panelID),
        m_BonePool(15 * sizeof(SkinningEditorPanel::PreviewBone)),
        m_Colors{
            glm::vec4(0.0f, 0.7f, 0.8f, 1.0f),
            glm::vec4(0.8f, 0.8f, 0.8f, 0.5f),
            glm::vec4(0.9f, 0.9f, 0.9f, 1.0f),
            glm::vec4(0.9f, 0.9f, 0.3f, 1.0f)
        },
        m_PointRadius(5.0f)
    {
        for (uint32_t i = 0; i < NumCategories; ++i)
            m_CategoriesOpen[i] = false;

        InGui::ImageWindow(m_PanelID, "Skinning Editor", glm::vec2(0.0f), glm::vec2(200.0f), m_Context);
        InGui::End();

        const InGuiWindow& window = InGui::GetWindow(m_PanelID);
        auto flags = window.Flags;
        flags &= ~InGuiWindowFlags::EventBlocking;
        InGui::SetWindowFlags(m_PanelID, flags);

        m_Shader = Shader::Create("Assets/Shaders/SkinningEditor.glsl");

        m_ViewportSize = window.Size;
        m_MousePosition = glm::vec2(0.0f);

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

        for (uint32_t i = 0; i < MAX_BONES; ++i)
            m_ColorIDs[i] = i;
        std::shuffle(&m_ColorIDs[0], &m_ColorIDs[MAX_BONES - 1], std::default_random_engine(0));
    }
    void SkinningEditorPanel::SetContext(Ref<SubTexture> context)
    {
        m_Context = context;
        m_ContextSize.x = (float)m_Context->GetTexture()->GetWidth();
        m_ContextSize.y = (float)m_Context->GetTexture()->GetHeight();

        rebuildRenderBuffers();
    }
    void SkinningEditorPanel::OnUpdate(Timestep ts)
    {
        const InGuiWindow& window = InGui::GetWindow(m_PanelID);
        if (IS_SET(window.Flags, InGuiWindowFlags::Hoovered))
        {
            m_MousePosition = getMouseWindowSpace();
            m_FoundVertex = findVertex(m_MousePosition);
            m_FoundTriangle = findTriangle(m_MousePosition);
            m_FoundBone = findBone(m_MousePosition);

            if (IS_SET(m_Flags, CreateBone))
            {
                previewBoneCreate();
            }
            else if (IS_SET(m_Flags, EditBone))
            {
                handleBoneEdit();
            }
            else if (IS_SET(m_Flags, EditVertex))
            {
                handleVertexEdit();
            }
            else if (IS_SET(m_Flags, WeightBrush))
            {
                handleWeightsBrush();
            }
        }
        if (IS_SET(m_Flags, PreviewPose))
        {
            updateBoneHierarchy();
            updateVertexBuffer();
        }
        renderAll();
    }
    void SkinningEditorPanel::OnInGuiRender()
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
                        m_Flags = ToggleBit(m_Flags, PreviewPose);
                        initializePose();
                        updateVertexBuffer();
                    }
                    InGui::Separator();
                    if (IS_SET(InGui::Button("Create Bone", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
                    {
                        m_Flags = ToggleBit(m_Flags, CreateBone);
                    }
                    InGui::Separator();
                    if (IS_SET(InGui::Button("Edit Bone", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
                    {
                        uint16_t previewPose = m_Flags & PreviewPose;
                        m_Flags = ToggleBit(m_Flags, EditBone);
                        m_Flags |= previewPose;
                    }
                    InGui::Separator();
                    if (IS_SET(InGui::Button("Delete Bone", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
                    {
                        m_Flags = ToggleBit(m_Flags, DeleteBone);
                    }
                }
                InGui::Separator();
                if (InGui::BeginGroup("Geometry", size, m_CategoriesOpen[Geometry]))
                {
                    if (IS_SET(InGui::Button("Create Vertex", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
                    {
                        m_Flags = ToggleBit(m_Flags, CreateVertex);
                    }
                    InGui::Separator();
                    if (IS_SET(InGui::Button("Edit Vertex", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
                    {
                        m_Flags = ToggleBit(m_Flags, EditVertex);
                    }
                    InGui::Separator();
                    if (IS_SET(InGui::Button("Delete Vertex", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
                    {           
                        m_Flags = ToggleBit(m_Flags, DeleteVertex);
                    }
                    InGui::Separator();
                    if (IS_SET(InGui::Button("Delete Triangle", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
                    {
                        m_Flags = ToggleBit(m_Flags, DeleteTriangle);
                    }
                    InGui::Separator();
                    if (IS_SET(InGui::Button("Clear", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
                    {
                        clear();
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
                    if (IS_SET(InGui::Button("Weight Brush", glm::vec2(50.0f, 25.0f)), InGuiReturnType::Clicked))
                    {
                        m_Flags = ToggleBit(m_Flags, WeightBrush);
                    }
                }
                InGui::Separator();

                if (!IS_SET(InGui::GetWindow(m_PanelID).Flags, InGuiWindowFlags::Hoovered))
                {
                }
            }
            InGui::End();
        }
    }
    void SkinningEditorPanel::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        if (IS_SET(InGui::GetWindow(m_PanelID).Flags, InGuiWindowFlags::Hoovered))
        {
            dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&SkinningEditorPanel::onMouseButtonPress, this));
            dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&SkinningEditorPanel::onMouseButtonRelease, this));
            dispatcher.Dispatch<MouseScrollEvent>(Hook(&SkinningEditorPanel::onMouseScroll, this));
        }
    }
    bool SkinningEditorPanel::onMouseButtonPress(MouseButtonPressEvent& event)
    {
        if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
        {
            if (IS_SET(m_Flags, CreateBone))
            {
                handleBoneCreate();
            }
            else if (IS_SET(m_Flags, CreateVertex) && !m_Triangulated)
            {
                m_SelectedVertex = nullptr;
                m_Vertices.push_back({ m_MousePosition.x, m_MousePosition.y });
                return true;
            }
            else if (IS_SET(m_Flags, DeleteVertex) && !m_Triangulated)
            {
                eraseVertexAtPosition(m_MousePosition);
                return true;
            }
            else if (IS_SET(m_Flags, DeleteTriangle))
            {
                eraseTriangleAtPosition(m_MousePosition);
                return true;
            }
            if (!IS_SET(m_Flags, WeightBrush))
            {
                m_SelectedBone = m_FoundBone;
                m_SelectedVertex = m_FoundVertex;
                m_SelectedTriangle = m_FoundTriangle;
            }
            else
            {
                if (m_FoundBone)
                    m_SelectedBone = m_FoundBone;
            }
        }
        else if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_MIDDLE))
        {
            m_SelectedBone = nullptr;
            m_SelectedVertex = nullptr;
            m_SelectedTriangle = nullptr;
        }
        return false;
    }
    bool SkinningEditorPanel::onMouseButtonRelease(MouseButtonReleaseEvent& event)
    {
        return false;
    }
    bool SkinningEditorPanel::onMouseScroll(MouseScrollEvent& event)
    {
        if (IS_SET(m_Flags, WeightBrush))
        {
            m_WeightBrushRadius -= event.GetOffsetY();
        }
        return false;
    }
    void SkinningEditorPanel::clear()
    {
        m_Flags = 0;
        m_Triangulated = false;
        m_Triangles.clear();
        m_Vertices.clear();
        m_PreviewVertices.clear();
        for (auto bone : m_Bones)
            m_BonePool.Deallocate<PreviewBone>(bone);
       
        m_Bones.clear();
        m_FoundBone = nullptr;
        m_FoundVertex = nullptr;
        m_FoundTriangle = nullptr;
        m_SelectedBone = nullptr;
        m_SelectedTriangle = nullptr;
        m_SelectedVertex = nullptr;
    }
    void SkinningEditorPanel::triangulate()
    {
        if (m_Vertices.size() < 3)
            return;

        m_FoundTriangle = nullptr;
        m_FoundVertex = nullptr;

        std::vector<tpp::Delaunay::Point> points;
        for (auto& p : m_Vertices)
        {
            points.push_back({ p.X, p.Y });
        }
        tpp::Delaunay generator(points);
        generator.setMinAngle(30.5f);
        generator.setMaxArea(12000.5f);
        generator.Triangulate(true);

        m_Triangles.clear();
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
            if (!trianglesHaveIndex((uint32_t)keypointIdx1))
            {
                GetTriangulationPt(points, keypointIdx1, sp1, x, y);
                if (m_Vertices.size() <= keypointIdx1)
                    m_Vertices.resize((size_t)keypointIdx1 + 1);
                m_Vertices[keypointIdx1] = { (float)x, (float)y };
            }
            if (!trianglesHaveIndex((uint32_t)keypointIdx2))
            {
                GetTriangulationPt(points, keypointIdx2, sp2, x, y);
                if (m_Vertices.size() <= keypointIdx2)
                    m_Vertices.resize((size_t)keypointIdx2 + 1);
                m_Vertices[keypointIdx2] = { (float)x, (float)y };
            }
            if (!trianglesHaveIndex((uint32_t)keypointIdx3))
            {
                GetTriangulationPt(points, keypointIdx3, sp3, x, y);
                if (m_Vertices.size() <= keypointIdx3)
                    m_Vertices.resize((size_t)keypointIdx3 + 1);
                m_Vertices[keypointIdx3] = { (float)x, (float)y };
            }
            m_Triangles.push_back({
                (uint32_t)keypointIdx1,
                (uint32_t)keypointIdx2,
                (uint32_t)keypointIdx3
                });
        }
        m_Triangulated = true;
        rebuildRenderBuffers();
    }
    void SkinningEditorPanel::initializePose()
    {
        m_BoneHierarchy.Traverse([](void* parent, void* child) -> bool {

            PreviewBone* childBone = static_cast<PreviewBone*>(child);
            childBone->PreviewTransform = glm::translate(glm::vec3(childBone->Start, 0.0f));

            return false;
        });
    }
    void SkinningEditorPanel::updateBoneHierarchy()
    {
        m_BoneHierarchy.Traverse([](void* parent, void* child) -> bool {

            PreviewBone* childBone = static_cast<PreviewBone*>(child);
            if (parent)
            {
                PreviewBone* parentBone = static_cast<PreviewBone*>(parent);
                childBone->PreviewFinalTransform = parentBone->PreviewFinalTransform * childBone->PreviewTransform;
            }
            else
            {
                childBone->PreviewFinalTransform = childBone->PreviewTransform;
            }
            return false;
       });
    }
    void SkinningEditorPanel::updateVertexBuffer()
    {
        m_PreviewVertices.clear();
        m_PreviewVertices.reserve(m_Vertices.size());
        for (auto& vertex : m_Vertices)
        {        
            glm::vec2 finalPos = glm::vec2(vertex.X, vertex.Y);
            if (IS_SET(m_Flags, PreviewPose))
            {
                BoneVertex vertexLocalToBone = vertex;
                auto posLocalToBone = getPositionLocalToBone(vertexLocalToBone);
                vertexLocalToBone.X = posLocalToBone.x;
                vertexLocalToBone.Y = posLocalToBone.y;
                finalPos = getPositionFromBones(vertexLocalToBone);
            }
            glm::vec3 finalColor = vertex.Color;
            if (IS_SET(m_Flags, WeightBrush))
            {
                finalColor = getColorFromBoneWeights(vertex);
            }
            m_PreviewVertices.push_back({
                finalColor,
                glm::vec3(finalPos, 1.0f),
                CalculateTexCoord(glm::vec2(vertex.X, vertex.Y),m_ContextSize),
                VertexBoneData()
             });
        }
        m_VertexBuffer->Update(m_PreviewVertices.data(), m_PreviewVertices.size() * sizeof(PreviewVertex));
    }
    void SkinningEditorPanel::rebuildRenderBuffers()
    {
        m_PreviewVertices.clear();
        if (m_Vertices.size())
        {
            m_PreviewVertices.reserve(m_Vertices.size());
            for (auto& vertex : m_Vertices)
            {
                VertexBoneData data;
                uint32_t counter = 0;
                for (auto bone : m_Bones)
                {
                    VertexBoneData data;
                    for (uint32_t i = 0; i < 4; ++i)
                    {
                        if (bone->ID == vertex.Data.IDs[i])
                        {
                            data.IDs[i] = counter;
                            data.Weights[i] = vertex.Data.Weights[i];
                        }
                    }
                    counter++;
                }

                m_PreviewVertices.push_back({
                    glm::vec3(1.0f),
                    glm::vec3(vertex.X, vertex.Y, 0.0f),
                    CalculateTexCoord(glm::vec2(vertex.X, vertex.Y),m_ContextSize),
                    data
                   });
            }
        }
        else
        {
            m_PreviewVertices.reserve(4);
            m_PreviewVertices.push_back({
                glm::vec3(1.0f),
                glm::vec3(-m_ContextSize.x / 2.0f, -m_ContextSize.y / 2.0f, 0.0f),
                glm::vec2(0.0f)
                });
            m_PreviewVertices.push_back({
                glm::vec3(1.0f),
                glm::vec3(m_ContextSize.x / 2.0f, -m_ContextSize.y / 2.0f, 0.0f),
                glm::vec2(1.0f,0.0f)
                });
            m_PreviewVertices.push_back({
                glm::vec3(1.0f),
                glm::vec3(m_ContextSize.x / 2.0f,  m_ContextSize.y / 2.0f, 0.0f),
                glm::vec2(1.0f)
                });
            m_PreviewVertices.push_back({
                glm::vec3(1.0f),
                glm::vec3(-m_ContextSize.x / 2.0f, m_ContextSize.y / 2.0f, 0.0f),
                glm::vec2(0.0f, 1.0f)
                });
        }

        m_VertexArray = VertexArray::Create();
        m_VertexBuffer = VertexBuffer::Create(m_PreviewVertices.data(), m_PreviewVertices.size() * sizeof(PreviewVertex), BufferUsage::Dynamic);
        m_VertexBuffer->SetLayout({
            {0, ShaderDataComponent::Float3, "a_Color"},
            {1, ShaderDataComponent::Float3, "a_Position"},
            {2, ShaderDataComponent::Float2, "a_TexCoord"},
            {3, ShaderDataComponent::Int4,	 "a_BoneIDs"},
            {4, ShaderDataComponent::Float4, "a_Weights"},
            });
        m_VertexArray->AddVertexBuffer(m_VertexBuffer);
        if (m_Triangles.size())
        {
            Ref<IndexBuffer> ibo = IndexBuffer::Create((uint32_t*)m_Triangles.data(), m_Triangles.size() * 3);
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
    void SkinningEditorPanel::eraseEmptyPoints()
    {
        std::vector<uint32_t> erasedPoints;
        for (uint32_t i = 0; i < m_Vertices.size(); ++i)
        {
            if (!trianglesHaveIndex(i))
                erasedPoints.push_back(i);
        }
        for (int32_t i = erasedPoints.size() - 1; i >= 0; --i)
        {
            m_Vertices.erase(m_Vertices.begin() + erasedPoints[i]);
            for (auto& triange : m_Triangles)
            {
                if (triange.First >= erasedPoints[i])
                    triange.First--;
                if (triange.Second >= erasedPoints[i])
                    triange.Second--;
                if (triange.Third >= erasedPoints[i])
                    triange.Third--;
            }
        }
    }
    void SkinningEditorPanel::eraseVertexAtPosition(const glm::vec2& pos)
    {
        uint32_t counter = 0;
        for (auto& vertex : m_Vertices)
        {
            if (glm::distance(pos, glm::vec2(vertex.X, vertex.Y)) < m_PointRadius)
            {
                m_Vertices.erase(m_Vertices.begin() + counter);
                m_SelectedVertex = nullptr;
                return;
            }
            counter++;
        }
    }
    void SkinningEditorPanel::eraseTriangleAtPosition(const glm::vec2& pos)
    {
        uint32_t counter = 0;
        for (auto& triangle : m_Triangles)
        {
            BoneVertex& first = m_Vertices[triangle.First];
            BoneVertex& second = m_Vertices[triangle.Second];
            BoneVertex& third = m_Vertices[triangle.Third];
            if (Math::PointInTriangle(pos, glm::vec2(first.X, first.Y), glm::vec2(second.X, second.Y), glm::vec2(third.X, third.Y)))
            {
                m_SelectedTriangle = nullptr;
                m_Triangles.erase(m_Triangles.begin() + counter);
                eraseEmptyPoints();
                rebuildRenderBuffers();
                return;
            }
            counter++;
        }
    }
    void SkinningEditorPanel::decomposeBone(PreviewBone* bone, glm::vec2& start, glm::vec2& end, glm::vec2& normal)
    {
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(bone->PreviewFinalTransform, scale, rotation, translation, skew, perspective);

        start = glm::vec2(translation.x, translation.y);
        glm::vec4 tmpEnd = glm::toMat4(rotation) * glm::vec4(bone->End, 0.0f, 1.0f);
        end = start + glm::vec2(tmpEnd.x, tmpEnd.y);
        glm::vec2 dir = glm::normalize(end - start);     
        normal = { -dir.y, dir.x };
    }
    void SkinningEditorPanel::previewBoneCreate()
    {
        glm::vec2 newBoneStart = m_MousePosition;
        glm::vec4 color = glm::vec4(RandomColor(m_ColorIDs[s_NextBone]), 1.0f);

        if (m_NewBoneData.Creating)
        {
            glm::vec2 dir = glm::normalize(m_MousePosition - m_NewBoneData.Start);
            glm::vec2 normal = { -dir.y, dir.x };
            Renderer2D::SubmitCircle(glm::vec3(m_NewBoneData.Start, 0.0f), m_PointRadius, 20, glm::vec4(color.x, color.y, color.z, 0.2f));
            Renderer2D::SubmitCircle(glm::vec3(m_MousePosition, 0.0f), m_PointRadius, 20, glm::vec4(color.x, color.y, color.z, 0.2f));
            Renderer2D::SubmitLine(glm::vec3(m_NewBoneData.Start + normal * m_PointRadius, 0.0f), glm::vec3(m_MousePosition, 0.0f), glm::vec4(color.x, color.y, color.z, 0.2f));
            Renderer2D::SubmitLine(glm::vec3(m_NewBoneData.Start - normal * m_PointRadius, 0.0f), glm::vec3(m_MousePosition, 0.0f), glm::vec4(color.x, color.y, color.z, 0.2f));
        }
        else if (m_FoundBone)
        {
            glm::vec2 end = m_FoundBone->WorldStart + m_FoundBone->End;
            if (glm::distance(m_MousePosition, end) < m_PointRadius)
            {
                Renderer2D::SubmitCircle(glm::vec3(end, 0.0f), m_PointRadius, 20, color);
            }
        }
        else if (m_SelectedBone)
        {                  
            glm::vec2 dir = glm::normalize(m_MousePosition - m_SelectedBone->WorldStart);
            glm::vec2 normal = { -dir.y, dir.x };
            Renderer2D::SubmitCircle(glm::vec3(m_MousePosition, 0.0f), m_PointRadius, 20, color);
            Renderer2D::SubmitLine(glm::vec3(m_MousePosition + normal * m_PointRadius, 0.0f), glm::vec3(m_SelectedBone->WorldStart, 0.0f), glm::vec4(color.x, color.y, color.z, 0.2f));
            Renderer2D::SubmitLine(glm::vec3(m_MousePosition - normal * m_PointRadius, 0.0f), glm::vec3(m_SelectedBone->WorldStart, 0.0f), glm::vec4(color.x, color.y, color.z, 0.2f));
        }       
        else
        {
            Renderer2D::SubmitCircle(glm::vec3(m_MousePosition, 0.0f), m_PointRadius, 20, color);
        }
    }
    void SkinningEditorPanel::handleBoneCreate()
    {
        if (m_FoundBone)
        {
            m_SelectedBone = m_FoundBone;
            // It is not the end of the bone so do not start new bone
            if (glm::distance(m_MousePosition, m_SelectedBone->WorldStart + m_SelectedBone->End) > m_PointRadius)
                return;
        }
        if (!m_NewBoneData.Creating)
        {       
            m_NewBoneData.Creating = true;
            m_NewBoneData.Parent = m_SelectedBone;
            m_NewBoneData.Start = m_MousePosition;
        }
        else
        {  
            PreviewBone* newBone = m_BonePool.Allocate<PreviewBone>();
            newBone->Color = RandomColor(m_ColorIDs[s_NextBone++]);
            newBone->Start = m_NewBoneData.Start;
            m_Bones.push_back(newBone);
            if (m_NewBoneData.Parent)
            {
                newBone->ID = m_BoneHierarchy.Insert(newBone, m_NewBoneData.Parent->ID);
                newBone->End = m_MousePosition - newBone->Start;
                newBone->Start -= m_NewBoneData.Parent->WorldStart;
            }
            else
            {
                newBone->ID = m_BoneHierarchy.Insert(newBone);
                newBone->End = m_MousePosition - newBone->Start;
            }
            m_NewBoneData.Creating = false;
            m_NewBoneData.Parent = nullptr;
        }
    }
    void SkinningEditorPanel::handleBoneEdit()
    {
        if (m_SelectedBone)
        {
            if (Input::IsMouseButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
            {
                if (IS_SET(m_Flags, PreviewPose))
                {
                    if (glm::distance(m_MousePosition, m_SelectedBone->WorldStart) < m_PointRadius)
                    {
                        m_SelectedBone->Start = m_MousePosition;
                        m_SelectedBone->WorldStart = m_MousePosition;
                        if (auto parent = m_BoneHierarchy.GetParentData(m_SelectedBone->ID))
                        {
                            PreviewBone* parentBone = static_cast<PreviewBone*>(parent);
                            m_SelectedBone->Start -= parentBone->WorldStart;
                            m_SelectedBone->WorldStart = parentBone->WorldStart + m_SelectedBone->Start;
                        }
                        initializePose();
                    }
                    else
                    {
                        glm::mat4 translation = glm::translate(glm::vec3(m_SelectedBone->Start, 0.0f));
                        glm::vec2 endTmp = m_SelectedBone->WorldStart + m_SelectedBone->End;
                        glm::vec2 origDir = glm::normalize(endTmp - m_SelectedBone->WorldStart);
                        glm::vec2 dir = glm::normalize(m_MousePosition - m_SelectedBone->WorldStart);
                                        
                        float angle = glm::atan(dir.y, dir.x) - glm::atan(origDir.y, origDir.x);
                        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
                        m_SelectedBone->PreviewTransform = translation * rotation;                       
                    }
                }
                else
                {
                    m_SelectedBone->Start = m_MousePosition;
                    if (auto parent = m_BoneHierarchy.GetParentData(m_SelectedBone->ID))
                    {
                        PreviewBone* parentBone = static_cast<PreviewBone*>(parent);
                        m_SelectedBone->Start -= parentBone->WorldStart;
                    }
                }
                updateVertexBuffer();
            }
        }
    }
    void SkinningEditorPanel::handleVertexEdit()
    {
        if (m_SelectedVertex)
        {
            if (Input::IsMouseButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
            {
                m_SelectedVertex->X = m_MousePosition.x;
                m_SelectedVertex->Y = m_MousePosition.y;
                
                updateVertexBuffer();
            }
        }
    }
    void SkinningEditorPanel::handleWeightsBrush()
    {
        Renderer2D::SubmitCircle(glm::vec3(m_MousePosition, 0.0f), m_WeightBrushRadius, 30, glm::vec4(1.0f, 0.8f, 1.0f, 1.0f));
        std::vector<BoneVertex*> vertices;
        findVerticesInRadius(m_MousePosition, m_WeightBrushRadius, vertices);

        if (m_SelectedBone)
        {
            float val = 0.0f;
            if (Input::IsMouseButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
                val = 1.0f;
            else if (Input::IsMouseButtonPressed(MouseCode::MOUSE_BUTTON_LEFT))
                val = -1.0f;
            else
                return;
           
            for (auto vertex : vertices)
            {
                float dist = glm::distance(glm::vec2(vertex->X, vertex->Y), m_MousePosition);
                bool exists = false;
                for (uint32_t i = 0; i < 4; ++i)
                {
                    if (vertex->Data.IDs[i] == m_SelectedBone->ID)
                    {
                        exists = true;
                        vertex->Data.Weights[i] += val * (1.0f - (dist / m_WeightBrushRadius)) * m_WeightBrushStrength;
                        if (vertex->Data.Weights[i] > 1.0f)
                        {
                            vertex->Data.Weights[i] = 1.0f;
                        }
                        else if (vertex->Data.Weights[i] < 0.0f)
                        {
                            vertex->Data.Weights[i] = 0.0f;
                            vertex->Data.IDs[i] = -1;
                        }
                    }
                }
                // Bone was not found, so add new id
                if (!exists)
                {
                    for (uint32_t i = 0; i < 4; ++i)
                    {
                        if (vertex->Data.IDs[i] == -1)
                        {
                            vertex->Data.IDs[i] = m_SelectedBone->ID;
                            vertex->Data.Weights[i] += val * (1.0f - (dist / m_WeightBrushRadius)) * m_WeightBrushStrength;
                            if (vertex->Data.Weights[i] > 1.0f)
                                vertex->Data.Weights[i] = 1.0f;
                            else if (vertex->Data.Weights[i] < 0.0f)
                                vertex->Data.Weights[i] = 0.0f;
                            break;
                        }
                    }
                }
            }
            updateVertexBuffer();
        }
    }
    bool SkinningEditorPanel::trianglesHaveIndex(uint32_t index) const
    {
        for (auto& triangle : m_Triangles)
        {
            if (triangle.First == index || triangle.Second == index || triangle.Third == index)
                return true;
        }
        return false;
    }

    glm::vec2 SkinningEditorPanel::calculateTexCoord(const glm::vec2& pos) const
    {
        glm::vec2 position = pos + m_ContextSize / 2.0f;
        return glm::vec2(position.x / m_ContextSize.x, position.y / m_ContextSize.y);
    }
    glm::vec2 SkinningEditorPanel::getPositionLocalToBone(const BoneVertex& vertex) 
    {
        bool hasBone = false;
        glm::mat4 boneTransform = glm::mat4(0.0f);
        for (uint32_t i = 0; i < 4; ++i)
        {
            if (vertex.Data.IDs[i] != -1)
            {
                PreviewBone* bone = static_cast<PreviewBone*>(m_BoneHierarchy.GetData(vertex.Data.IDs[i]));
                boneTransform += glm::translate(glm::vec3(bone->Start, 0.0f)) * vertex.Data.Weights[i];
                hasBone = true;
            }
        }
        if (!hasBone)
            return { vertex.X, vertex.Y };
        return glm::inverse(boneTransform) * glm::vec4(vertex.X, vertex.Y, 0.0f, 1.0f);
    }
    glm::vec3 SkinningEditorPanel::getColorFromBoneWeights(const BoneVertex& vertex) 
    {
        bool hasBone = false;
        glm::vec3 color = glm::vec3(1.0f);
        for (uint32_t i = 0; i < 4; ++i)
        {
            if (vertex.Data.IDs[i] != -1)
            {
                PreviewBone* bone = static_cast<PreviewBone*>(m_BoneHierarchy.GetData(vertex.Data.IDs[i]));
                color *= bone->Color * vertex.Data.Weights[i];
                hasBone = true;
            }
        }
        if (!hasBone)
            return vertex.Color;
        return color;
    }
    glm::vec2 SkinningEditorPanel::getPositionFromBones(const BoneVertex& vertex)
    {
        bool hasBone = false;
        glm::mat4 boneTransform = glm::mat4(0.0f);
        for (uint32_t i = 0; i < 4; ++i)
        {
            if (vertex.Data.IDs[i] != -1)
            {
                PreviewBone* bone = static_cast<PreviewBone*>(m_BoneHierarchy.GetData(vertex.Data.IDs[i]));
                boneTransform += bone->PreviewFinalTransform * vertex.Data.Weights[i];
                hasBone = true;
            }
        }
        if (!hasBone)
            return { vertex.X, vertex.Y };
        return boneTransform * glm::vec4(vertex.X, vertex.Y, 0.0f, 1.0f);
    }
    glm::vec2 SkinningEditorPanel::getMouseWindowSpace() const
    {
        const InGuiWindow& window = InGui::GetWindow(m_PanelID);
        auto [mx, my] = getMouseViewportSpace();
        mx *= window.Size.x / 2.0f;
        my *= window.Size.y / 2.0f;
        return { mx , my };
    }
    std::pair<float, float> SkinningEditorPanel::getMouseViewportSpace() const
    {
        auto [mx, my] = Input::GetMousePosition();
        auto& window = InGui::GetWindow(m_PanelID);
        mx -= window.Position.x;
        my -= window.Position.y;

        auto viewportWidth = window.Size.x;
        auto viewportHeight = window.Size.y;

        return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
    }
 
    void SkinningEditorPanel::renderAll()
    {
        m_Framebuffer->Bind(); 
        Renderer::SetClearColor(m_Framebuffer->GetSpecification().ClearColor);
        Renderer::Clear();;

        const InGuiWindow& window = InGui::GetWindow(m_PanelID);
        glm::mat4 projection = glm::ortho(-window.Size.x / 2.0f, window.Size.x / 2.0f, -window.Size.y / 2.0f, window.Size.y / 2.0f);
        renderMesh(projection);
        renderPreviews(projection);
        m_Framebuffer->Unbind();
    }
    void SkinningEditorPanel::renderMesh(const glm::mat4& viewProjection)
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
    void SkinningEditorPanel::renderPreviews(const glm::mat4& viewProjection)
    {
        Renderer2D::BeginScene(viewProjection);
        for (auto& triangle : m_Triangles)
            renderTriangle(triangle, m_Colors[TriangleColor]);

        if (m_FoundTriangle)
            renderTriangle(*m_FoundTriangle, m_Colors[TriangleHighlightColor]);
        
        if (IS_SET(m_Flags, CreateBone))
            previewBoneCreate();

        if (IS_SET(m_Flags, Flags::PreviewPose))
        {
            for (auto& vertex : m_PreviewVertices)
                Renderer2D::SubmitCircle(glm::vec3(vertex.Position.x, vertex.Position.y, 0.0f), m_PointRadius, 20, m_Colors[VertexColor]);
            m_BoneHierarchy.Traverse([&](void* parent, void* child) -> bool {

                PreviewBone* childBone = static_cast<PreviewBone*>(child);
                if (parent)
                    renderBoneRelation(static_cast<PreviewBone*>(parent), childBone);
                glm::vec2 start, end, normal;
                decomposeBone(childBone, start, end, normal);
                renderBone(m_PointRadius, start, end, normal, glm::vec4(childBone->Color, 1.0f));
                if (m_SelectedBone && m_SelectedBone->ID == childBone->ID)
                    renderBone(m_PointRadius * 1.2f, start, end, normal * 1.2f, m_Colors[BoneHighlightColor]);
                return false;
                });         
        }
        else
        {
            for (auto& vertex : m_Vertices)
                Renderer2D::SubmitCircle(glm::vec3(vertex.X, vertex.Y, 0.0f), m_PointRadius, 20, m_Colors[VertexColor]);
            m_BoneHierarchy.Traverse([&](void* parent, void* child) -> bool {

                PreviewBone* childBone = static_cast<PreviewBone*>(child);
                childBone->WorldStart = childBone->Start;
                glm::vec2 end = childBone->Start + childBone->End;
                if (parent)
                {
                    PreviewBone* parentBone = static_cast<PreviewBone*>(parent);
                    childBone->WorldStart += parentBone->WorldStart;
                    end += parentBone->WorldStart;

                    renderBoneRelation(parentBone, childBone);
                }      
                glm::vec2 dir = glm::normalize(end - childBone->WorldStart);
                glm::vec2 normal = { -dir.y, dir.x };
                renderBone(m_PointRadius, childBone->WorldStart, end, normal, glm::vec4(childBone->Color, 1.0f));
                if (m_SelectedBone && m_SelectedBone->ID == childBone->ID)
                    renderBone(m_PointRadius * 1.2f, childBone->WorldStart, end, normal * 1.2f, m_Colors[BoneHighlightColor]);
                return false;
                });         
        }

        Renderer2D::FlushLines();
        Renderer2D::EndScene();
        Renderer2D::EndScene();
    }
    void SkinningEditorPanel::renderBoneRelation(PreviewBone* parent, PreviewBone* child)
    {
        glm::vec2 start, end;
        if (IS_SET(m_Flags, PreviewPose))
        {
            glm::vec2 parentStart, parentEnd, parentNormal;
            decomposeBone(parent, parentStart, parentEnd, parentNormal);

            glm::vec2 childStart, childEnd, childNormal;
            decomposeBone(child, childStart, childEnd, childNormal);
            start = parentStart;
            end = childStart;
        }
        else
        {
            end = parent->WorldStart + child->Start;
            start = parent->WorldStart;         
        }
        glm::vec2 dir = glm::normalize(end - start);
        glm::vec2 normal = { -dir.y, dir.x };
        Renderer2D::SubmitLine(glm::vec3(start + normal * m_PointRadius, 0.0f), glm::vec3(end, 0.0f), glm::vec4(child->Color, 0.2f));
        Renderer2D::SubmitLine(glm::vec3(start - normal * m_PointRadius, 0.0f), glm::vec3(end, 0.0f), glm::vec4(child->Color, 0.2f));
    }
    void SkinningEditorPanel::renderTriangle(const Triangle& triangle, const glm::vec4& color)
    {
        if (IS_SET(m_Flags, PreviewPose))
        {
            PreviewVertex& first = m_PreviewVertices[triangle.First];
            PreviewVertex& second = m_PreviewVertices[triangle.Second];
            PreviewVertex& third = m_PreviewVertices[triangle.Third];

            Renderer2D::SubmitLine(glm::vec3(first.Position.x, first.Position.y, 0.0f), glm::vec3(second.Position.x, second.Position.y, 0.0f), color);
            Renderer2D::SubmitLine(glm::vec3(second.Position.x, second.Position.y, 0.0f), glm::vec3(third.Position.x, third.Position.y, 0.0f), color);
            Renderer2D::SubmitLine(glm::vec3(third.Position.x, third.Position.y, 0.0f), glm::vec3(first.Position.x, first.Position.y, 0.0f), color);
        }
        else
        {
            BoneVertex& first = m_Vertices[triangle.First];
            BoneVertex& second = m_Vertices[triangle.Second];
            BoneVertex& third = m_Vertices[triangle.Third];

            Renderer2D::SubmitLine(glm::vec3(first.X, first.Y, 0.0f), glm::vec3(second.X, second.Y, 0.0f), color);
            Renderer2D::SubmitLine(glm::vec3(second.X, second.Y, 0.0f), glm::vec3(third.X, third.Y, 0.0f), color);
            Renderer2D::SubmitLine(glm::vec3(third.X, third.Y, 0.0f), glm::vec3(first.X, first.Y, 0.0f), color);
        }
    }

    void SkinningEditorPanel::renderBone(float radius, const glm::vec2& start, const glm::vec2& end, const glm::vec2& normal, const glm::vec4& color)
    {
        Renderer2D::SubmitCircle(glm::vec3(start, 0.0f), radius, 20, color);
        Renderer2D::SubmitCircle(glm::vec3(end, 0.0f), radius, 20, color);
        Renderer2D::SubmitLine(glm::vec3(start + (normal * radius),0.0f), glm::vec3(end, 0.0f), color);
        Renderer2D::SubmitLine(glm::vec3(start - (normal * radius),0.0f), glm::vec3(end, 0.0f), color);
    }

   
    SkinningEditorPanel::Triangle* SkinningEditorPanel::findTriangle(const glm::vec2& pos)
    {
        Triangle triangle;
        for (auto& triangle : m_Triangles)
        {
            BoneVertex& first = m_Vertices[triangle.First];
            BoneVertex& second = m_Vertices[triangle.Second];
            BoneVertex& third = m_Vertices[triangle.Third];
            if (Math::PointInTriangle(pos, glm::vec2(first.X, first.Y), glm::vec2(second.X, second.Y), glm::vec2(third.X, third.Y)))
            {
                return &triangle;
            }
        }
        return nullptr;
    }
    SkinningEditorPanel::BoneVertex* SkinningEditorPanel::findVertex(const glm::vec2& pos)
    {
        for (auto& vertex : m_Vertices)
        {
            if (glm::distance(pos, glm::vec2(vertex.X, vertex.Y)) < m_PointRadius)
            {
                return &vertex;
            }
        }
    }
    SkinningEditorPanel::PreviewBone* SkinningEditorPanel::findBone(const glm::vec2& pos)
    {
        PreviewBone* bone = nullptr;
        m_BoneHierarchy.Traverse([&](void* parent, void* child) -> bool {

            PreviewBone* childBone = static_cast<PreviewBone*>(child);
            glm::vec2 start;
            glm::vec2 end;
            glm::vec2 normal;
            if (IS_SET(m_Flags, Flags::PreviewPose))
            {
                decomposeBone(childBone, start, end, normal);
            }
            else
            {
                start = childBone->WorldStart;
                end = start + childBone->End;
                glm::vec2 dir = glm::normalize(end - start);
                normal = { -dir.y, dir.x };        
            }
            glm::vec2 v1 = start + (normal * m_PointRadius);
            glm::vec2 v2 = start - (normal * m_PointRadius);
            
            if (glm::distance(pos, start) < m_PointRadius 
             || glm::distance(pos, end) < m_PointRadius 
             || Math::PointInTriangle(pos, v1, v2, end))
            {
                bone = childBone;
                return true;
            }

            return false;
        });
        return bone;
    }

    void SkinningEditorPanel::findVerticesInRadius(const glm::vec2& pos, float radius, std::vector<BoneVertex*>& vertices)
    {
        for (auto& vertex : m_Vertices)
        {
            if (glm::distance(glm::vec2(vertex.X, vertex.Y), pos) < radius)
            {
                vertices.push_back(&vertex);
            }
        }
    }
}