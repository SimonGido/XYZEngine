#include "stdafx.h"
#include "SkinningEditorPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"

#include "XYZ/Utils/Math/Math.h"
#include "XYZ/Asset/AssetManager.h"

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


    SkinningEditorPanel::SkinningEditorPanel()
        :
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

        std::initializer_list<IGHierarchyElement> types{
            {
                IGElementType::ImageWindow,
                {	
                    {IGElementType::Group, { // Bones
                        {IGElementType::Separator, {}},
                        {IGElementType::Button, {}}, // Preview pose
                        {IGElementType::Separator, {}},
                        {IGElementType::Button, {}}, // Create bone
                        {IGElementType::Separator, {}},
                        {IGElementType::Button, {}}, // Edit bone
                        {IGElementType::Separator, {}},
                        {IGElementType::Button, {}}  // Delete bone
                    }},
                    {IGElementType::Separator, {}}, 
                    {IGElementType::Group, { // Vertices
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
                    }},
                    {IGElementType::Separator, {}},
                    {IGElementType::Group, { // Weights
                        {IGElementType::Separator, {}},
                        {IGElementType::Button, {}}  // Weigths brush
                    }},
                    {IGElementType::Separator, {}}, // Bone hierarchy tree
                    {IGElementType::Tree, {}}
                }
            }   
        };

        auto [poolHandle, handleCount] = IG::AllocateUI(types);
        m_PoolHandle = poolHandle;
        m_HandleCount = handleCount;
        m_Window = &IG::GetUI<IGImageWindow>(m_PoolHandle, 0);
        m_Window->Label = "Skinning Editor";
        m_Tree = &IG::GetUI<IGTree>(m_PoolHandle, 31);

        m_ViewportSize = m_Window->Size;
        m_MousePosition = glm::vec2(0.0f);
        m_ViewProjection = glm::ortho(
            -m_Window->Size.x / 2.0f, m_Window->Size.x / 2.0f, 
            -m_Window->Size.y / 2.0f, m_Window->Size.y / 2.0f
        );

        m_Shader = Shader::Create("Assets/Shaders/SkinningEditor.glsl");

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

        rebuildRenderBuffers();
        m_Window->ResizeCallback = [&](const glm::vec2& size) {
            m_Framebuffer->Resize((uint32_t)size.x, (uint32_t)size.y);
            m_ViewProjection = glm::ortho(
                -m_Window->Size.x / 2.0f, m_Window->Size.x / 2.0f, 
                -m_Window->Size.y / 2.0f, m_Window->Size.y / 2.0f
            );
        };


        for (uint32_t i = 0; i < MAX_BONES; ++i)
            m_ColorIDs[i] = i;
        std::shuffle(&m_ColorIDs[0], &m_ColorIDs[MAX_BONES - 1], std::default_random_engine(0));


        setupUI();
    }
    void SkinningEditorPanel::SetContext(Ref<SubTexture> context)
    {
        m_Context = context;
        m_ContextSize.x = (float)m_Context->GetTexture()->GetWidth();
        m_ContextSize.y = (float)m_Context->GetTexture()->GetHeight();

        rebuildRenderBuffers();
    }
    void SkinningEditorPanel::OnUpdate()
    {
        if (IS_SET(m_Window->Flags, IGWindow::Hoovered))
        {
            m_MousePosition = getMouseWindowSpace();
            {
                auto [index, vertex] = findVertex(m_MousePosition);
                m_FoundVertex = vertex;
                m_FoundSubmeshIndex = index;
            }
            {
                auto [index, triangle] = findTriangle(m_MousePosition);
                m_FoundTriangle = triangle;
                m_FoundSubmeshIndex = index;
            }    
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
            
            // Bone group
            if (IG::GetUI<IGButton>(m_PoolHandle, 3).Is(IGReturnType::Clicked)) // Preview Pose
            {
                m_Flags = ToggleBit(m_Flags, PreviewPose);
                initializePose();
                updateVertexBuffer();
                Save();
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
                m_SubMeshes.push_back({});
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
                    for (auto& subMesh : m_SubMeshes)
                    {
                        triangulate(subMesh);
                    }
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
        if (IS_SET(m_Flags, PreviewPose))
        {
            updateBoneHierarchy();
            updateVertexBuffer();
        }
        renderAll();
    }
 
    void SkinningEditorPanel::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        if (IS_SET(m_Window->Flags, IGWindow::Hoovered))
        {
            dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&SkinningEditorPanel::onMouseButtonPress, this));
            dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&SkinningEditorPanel::onMouseButtonRelease, this));
            dispatcher.Dispatch<MouseScrollEvent>(Hook(&SkinningEditorPanel::onMouseScroll, this));
            dispatcher.Dispatch<KeyPressedEvent>(Hook(&SkinningEditorPanel::onKeyPress, this));
        }
    }
    void SkinningEditorPanel::Save()
    {
        Ref<Material> material = AssetManager::GetAsset<Material>(AssetManager::GetAssetHandle("Assets/Materials/Material.mat"));
       
        std::vector<XYZ::AnimatedVertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Bone> bones;
        Tree boneHierarchy;

        for (auto& vertex : m_PreviewVertices)
            vertices.push_back({ vertex.Position, vertex.TexCoord, vertex.BoneData });
        for (auto& subMesh : m_SubMeshes)
        {
            for (auto& triangle : subMesh.Triangles)
            {
                indices.push_back(triangle.First);
                indices.push_back(triangle.Second);
                indices.push_back(triangle.Third);
            }
        }
        
        uint32_t counter = 0;
        bones.resize(m_Bones.size());
        std::unordered_map<int32_t, uint32_t> boneMap;
        m_BoneHierarchy.Traverse([&](void* parent, void* child) -> bool {
            
            PreviewBone* childBone = static_cast<PreviewBone*>(child);
            bones[counter] = { 
                childBone->PreviewTransform,
            };      
            boneMap[childBone->ID] = counter;
            counter++;
            return false;
        });
        m_BoneHierarchy.Traverse([&](void* parent, void* child) -> bool {
            
            PreviewBone* childBone = static_cast<PreviewBone*>(child);
            uint32_t childBoneIndex = boneMap[childBone->ID];
            if (parent)
            {
                PreviewBone* parentBone = static_cast<PreviewBone*>(parent);
                uint32_t parentBoneIndex = boneMap[parentBone->ID];
                bones[childBoneIndex].ID = boneHierarchy.Insert(&bones[childBoneIndex], bones[parentBoneIndex].ID);
            }
            else
            {
                bones[childBoneIndex].ID = boneHierarchy.Insert(&bones[childBoneIndex]);
            }
            return false;
        });
        //AssetManager::GetAsset<SkeletalMesh>(AssetManager::GetAssetHandle("Assets/Meshes/SkeletalMesh.skm"));
        //AssetManager::CreateAsset<SkeletalMesh>("SkeletalMesh.skm", AssetType::SkeletalMesh, AssetManager::GetDirectoryHandle("Assets/Meshes"), vertices, indices, bones, boneHierarchy, material);
    }
    void SkinningEditorPanel::setupUI()
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
    }
    bool SkinningEditorPanel::onMouseButtonPress(MouseButtonPressEvent& event)
    {
        if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
        {
            if (IS_SET(m_Flags, CreateBone))
            {
                handleBoneCreate();
            }
            else if (IS_SET(m_Flags, DeleteBone))
            {
                if (m_FoundBone)
                    eraseBone(m_FoundBone);
                m_FoundBone = nullptr;
                m_SelectedBone = nullptr;
            }
            else if (IS_SET(m_Flags, EditBone))
            {
                setEditBone(m_FoundBone);
            }
            else if (IS_SET(m_Flags, CreateVertex))
            {
                m_SelectedVertex = nullptr;
                if (!m_Triangulated)
                {
                    if (m_SubMeshes.empty())
                        m_SubMeshes.push_back({});
                    m_SubMeshes.back().Vertices.push_back({ m_MousePosition.x, m_MousePosition.y });
                    return true;
                }
                else
                {
                    XYZ_LOG_WARN("Mesh was already triangulated, can not add new vertices");
                }
            }
            else if (IS_SET(m_Flags, DeleteVertex))
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
        if (event.IsButtonReleased(MouseCode::MOUSE_BUTTON_RIGHT))
        {
            setEditBone(nullptr);
        }
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
    bool SkinningEditorPanel::onKeyPress(KeyPressedEvent& event)
    {
        if (event.IsKeyPressed(KeyCode::KEY_DELETE))
        {
        
        }
        return false;
    }
    void SkinningEditorPanel::clear()
    {
        m_Flags = 0;
        m_SubMeshes.clear();
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
        m_Triangulated = false;
        rebuildRenderBuffers();
    }
    void SkinningEditorPanel::triangulate(Submesh& subMesh)
    {
        if (subMesh.Vertices.size() < 3)
            return;

        m_FoundTriangle = nullptr;
        m_FoundVertex = nullptr;

        std::vector<tpp::Delaunay::Point> points;
        for (auto& p : subMesh.Vertices)
        {
            points.push_back({ p.X, p.Y });
        }
        tpp::Delaunay generator(points);
        generator.setMinAngle(30.5f);
        generator.setMaxArea(12000.5f);
        generator.Triangulate(true);

        subMesh.Triangles.clear();
        subMesh.Vertices.clear();
        for (tpp::Delaunay::fIterator fit = generator.fbegin(); fit != generator.fend(); ++fit)
        {
            tpp::Delaunay::Point sp1;
            tpp::Delaunay::Point sp2;
            tpp::Delaunay::Point sp3;

            int keypointIdx1 = generator.Org(fit, &sp1);
            int keypointIdx2 = generator.Dest(fit, &sp2);
            int keypointIdx3 = generator.Apex(fit, &sp3);

            double x = 0.0f, y = 0.0f;
            if (!trianglesHaveIndex(subMesh, (uint32_t)keypointIdx1))
            {
                GetTriangulationPt(points, keypointIdx1, sp1, x, y);
                if (subMesh.Vertices.size() <= keypointIdx1)
                    subMesh.Vertices.resize((size_t)keypointIdx1 + 1);
                subMesh.Vertices[keypointIdx1] = { (float)x, (float)y };
            }
            if (!trianglesHaveIndex(subMesh, (uint32_t)keypointIdx2))
            {
                GetTriangulationPt(points, keypointIdx2, sp2, x, y);
                if (subMesh.Vertices.size() <= keypointIdx2)
                    subMesh.Vertices.resize((size_t)keypointIdx2 + 1);
                subMesh.Vertices[keypointIdx2] = { (float)x, (float)y };
            }
            if (!trianglesHaveIndex(subMesh, (uint32_t)keypointIdx3))
            {
                GetTriangulationPt(points, keypointIdx3, sp3, x, y);
                if (subMesh.Vertices.size() <= keypointIdx3)
                    subMesh.Vertices.resize((size_t)keypointIdx3 + 1);
                subMesh.Vertices[keypointIdx3] = { (float)x, (float)y };
            }
            subMesh.Triangles.push_back({
                (uint32_t)keypointIdx1,
                (uint32_t)keypointIdx2,
                (uint32_t)keypointIdx3
                });
        }
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
        for (auto& subMesh : m_SubMeshes)
        {
            for (auto& vertex : subMesh.Vertices)
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
        }
        m_VertexBuffer->Update(m_PreviewVertices.data(), m_PreviewVertices.size() * sizeof(PreviewVertex));
    }
    void SkinningEditorPanel::rebuildRenderBuffers()
    {
        m_PreviewVertices.clear();
        std::vector<uint32_t> indices;
        if (m_SubMeshes.size())
        {
            uint32_t offset = 0;
            for (auto& subMesh : m_SubMeshes)
            {          
                for (auto& triangle : subMesh.Triangles)
                {
                    indices.push_back(triangle.First + offset);
                    indices.push_back(triangle.Second + offset);
                    indices.push_back(triangle.Third + offset);
                }
                offset += subMesh.Vertices.size();
                for (auto& vertex : subMesh.Vertices)
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
                        glm::vec3(0.0f),
                        glm::vec3(vertex.X, vertex.Y, 0.0f),
                        CalculateTexCoord(glm::vec2(vertex.X, vertex.Y),m_ContextSize),
                        data
                        });
                }
            }
        }
        else
        {
            m_PreviewVertices.reserve(4);
            m_PreviewVertices.push_back({
                glm::vec3(0.0f),
                glm::vec3(-m_ContextSize.x / 2.0f, -m_ContextSize.y / 2.0f, 0.0f),
                glm::vec2(0.0f)
                });
            m_PreviewVertices.push_back({
                glm::vec3(0.0f),
                glm::vec3(m_ContextSize.x / 2.0f, -m_ContextSize.y / 2.0f, 0.0f),
                glm::vec2(1.0f,0.0f)
                });
            m_PreviewVertices.push_back({
                glm::vec3(0.0f),
                glm::vec3(m_ContextSize.x / 2.0f,  m_ContextSize.y / 2.0f, 0.0f),
                glm::vec2(1.0f)
                });
            m_PreviewVertices.push_back({
                glm::vec3(0.0f),
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
        if (indices.size())
        {
            Ref<IndexBuffer> ibo = IndexBuffer::Create(indices.data(), indices.size());
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
        for (auto& subMesh : m_SubMeshes)
        {
            std::vector<uint32_t> erasedPoints;
            for (uint32_t i = 0; i < subMesh.Vertices.size(); ++i)
            {
                if (!trianglesHaveIndex(subMesh, i))
                    erasedPoints.push_back(i);
            }
            for (int32_t i = erasedPoints.size() - 1; i >= 0; --i)
            {
                subMesh.Vertices.erase(subMesh.Vertices.begin() + erasedPoints[i]);
                for (auto& triange : subMesh.Triangles)
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
    }
    void SkinningEditorPanel::eraseVertexAtPosition(const glm::vec2& pos)
    {        
        for (auto& subMesh : m_SubMeshes)
        {
            uint32_t counter = 0;
            for (auto& vertex : subMesh.Vertices)
            {
                if (glm::distance(pos, glm::vec2(vertex.X, vertex.Y)) < m_PointRadius)
                {
                    subMesh.Vertices.erase(subMesh.Vertices.begin() + counter);
                    m_SelectedVertex = nullptr;
                    return;
                }
                counter++;
            }
        }
    }
    void SkinningEditorPanel::eraseTriangleAtPosition(const glm::vec2& pos)
    {     
        for (auto& subMesh : m_SubMeshes)
        {
            uint32_t counter = 0;
            for (auto& triangle : subMesh.Triangles)
            {
                BoneVertex& first =  subMesh.Vertices[triangle.First];
                BoneVertex& second = subMesh.Vertices[triangle.Second];
                BoneVertex& third =  subMesh.Vertices[triangle.Third];
                if (Math::PointInTriangle(pos, glm::vec2(first.X, first.Y), glm::vec2(second.X, second.Y), glm::vec2(third.X, third.Y)))
                {
                    m_SelectedTriangle = nullptr;
                    subMesh.Triangles.erase(subMesh.Triangles.begin() + counter);
                    eraseEmptyPoints();
                    rebuildRenderBuffers();
                    return;
                }
                counter++;
            }
        }
    }
    void SkinningEditorPanel::eraseBone(PreviewBone* bone)
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
    void SkinningEditorPanel::setEditBone(PreviewBone* bone)
    {
        m_EditBoneData.Bone = bone;
        m_EditBoneData.Rotate = false;
        if (IS_SET(m_Flags, PreviewPose) && bone)
        {
            float rot;
            glm::vec2 start, end, normal;
            decomposeBone(bone, start, end, rot, normal);
            if (glm::distance(m_MousePosition, start) > m_PointRadius * 2.0f)
                m_EditBoneData.Rotate = true;
        }
    }
    void SkinningEditorPanel::decomposeBone(PreviewBone* bone, glm::vec2& start, glm::vec2& end, float& rot, glm::vec2& normal, bool finalTransform)
    {
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        if (finalTransform)
            glm::decompose(bone->PreviewFinalTransform, scale, rotation, translation, skew, perspective);
        else
            glm::decompose(bone->PreviewTransform, scale, rotation, translation, skew, perspective);

        glm::vec3 euler = glm::eulerAngles(rotation);
        rot = euler.z;

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
            char buffer[20];
            sprintf(buffer, "bone_%u", s_NextBone);
            newBone->Name = buffer;

            newBone->Color = RandomColor(m_ColorIDs[s_NextBone++]);
            newBone->Start = m_NewBoneData.Start;
           
            m_Bones.push_back(newBone);
            if (m_NewBoneData.Parent)
            {
                newBone->ID = m_BoneHierarchy.Insert(newBone, m_NewBoneData.Parent->ID);
                m_Tree->AddItem(newBone->ID, m_NewBoneData.Parent->ID, IGTreeItem(newBone->Name));
  
                newBone->End = m_MousePosition - newBone->Start;
                newBone->Start -= m_NewBoneData.Parent->WorldStart;
            }
            else
            {
                newBone->ID = m_BoneHierarchy.Insert(newBone);
                m_Tree->AddItem(newBone->ID, IGTreeItem(newBone->Name));
                newBone->End = m_MousePosition - newBone->Start;
            }
            m_NewBoneData.Creating = false;
            m_NewBoneData.Parent = nullptr;
        }
    }
    void SkinningEditorPanel::handleBoneEdit()
    {
        if (m_EditBoneData.Bone)
        {        
            if (IS_SET(m_Flags, PreviewPose))
            {
                float rot;
                glm::vec2 start, end, normal;
                decomposeBone(m_SelectedBone, start, end, rot, normal);
                             
                if (m_EditBoneData.Rotate)
                {
                    glm::vec2 origDir = glm::normalize(end - start);
                    glm::vec2 dir = glm::normalize(m_MousePosition - start);
                    if (glm::distance(origDir, dir) > FLT_MIN)
                    {
                        float angle = glm::atan(dir.y, dir.x) - glm::atan(origDir.y, origDir.x);
                        m_SelectedBone->PreviewTransform = glm::rotate(m_SelectedBone->PreviewTransform, angle, glm::vec3(0.0f, 0.0f, 1.0f));
                    }
                }
                else
                {
                    glm::vec2 translation = m_MousePosition - start;
                    m_SelectedBone->PreviewTransform = glm::translate(m_SelectedBone->PreviewTransform, glm::vec3(translation, 0.0f));
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
    bool SkinningEditorPanel::trianglesHaveIndex(const Submesh& subMesh, uint32_t index) const
    {
        for (auto& triangle : subMesh.Triangles)
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
            return glm::vec3(0.0f);
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
        auto [mx, my] = getMouseViewportSpace();
        mx *= m_Window->Size.x / 2.0f;
        my *= m_Window->Size.y / 2.0f;
        return { mx , my };
    }
    std::pair<float, float> SkinningEditorPanel::getMouseViewportSpace() const
    {
        auto [mx, my] = Input::GetMousePosition();
        glm::vec2 position = m_Window->GetAbsolutePosition();
        mx -= position.x;
        my -= position.y;

        auto viewportWidth =  m_Window->Size.x;
        auto viewportHeight = m_Window->Size.y;

        return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
    }
 
    void SkinningEditorPanel::renderAll()
    {
        m_Framebuffer->Bind(); 
        Renderer::SetClearColor(m_Framebuffer->GetSpecification().ClearColor);
        Renderer::Clear();;

        renderMesh(m_ViewProjection);
        renderPreviews(m_ViewProjection);
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
        uint32_t counter = 0;
        uint32_t offset = 0;
        for (auto& subMesh : m_SubMeshes)
        {
            for (auto& triangle : subMesh.Triangles)
            {
                renderTriangle(counter, offset, triangle, m_Colors[TriangleColor]);
                if (m_FoundTriangle && triangle == *m_FoundTriangle)
                    renderTriangle(m_FoundSubmeshIndex, offset, *m_FoundTriangle, m_Colors[TriangleHighlightColor]);
            }
            counter++;
            offset += subMesh.Vertices.size();
        }
      
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

                float rot;
                glm::vec2 start, end, normal;
                decomposeBone(childBone, start, end, rot, normal);
                renderBone(m_PointRadius, start, end, normal, glm::vec4(childBone->Color, 1.0f));
                if (m_SelectedBone && m_SelectedBone->ID == childBone->ID)
                    renderBone(m_PointRadius * 1.2f, start, end, normal * 1.2f, m_Colors[BoneHighlightColor]);
                return false;
                });         
        }
        else
        {
            for (auto& subMesh : m_SubMeshes)
            {
                for (auto& vertex : subMesh.Vertices)
                    Renderer2D::SubmitCircle(glm::vec3(vertex.X, vertex.Y, 0.0f), m_PointRadius, 20, m_Colors[VertexColor]);
            }
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
            float rot;
            glm::vec2 parentStart, parentEnd, parentNormal;
            decomposeBone(parent, parentStart, parentEnd, rot, parentNormal);

            glm::vec2 childStart, childEnd, childNormal;
            decomposeBone(child, childStart, childEnd, rot, childNormal);
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
    void SkinningEditorPanel::renderTriangle(uint32_t subMeshIndex,uint32_t vertexOffset, const Triangle& triangle, const glm::vec4& color)
    {
        if (IS_SET(m_Flags, PreviewPose))
        {
            PreviewVertex& first = m_PreviewVertices[size_t(triangle.First + vertexOffset)];
            PreviewVertex& second = m_PreviewVertices[size_t(triangle.Second + vertexOffset)];
            PreviewVertex& third = m_PreviewVertices[size_t(triangle.Third + vertexOffset)];

            Renderer2D::SubmitLine(glm::vec3(first.Position.x, first.Position.y, 0.0f), glm::vec3(second.Position.x, second.Position.y, 0.0f), color);
            Renderer2D::SubmitLine(glm::vec3(second.Position.x, second.Position.y, 0.0f), glm::vec3(third.Position.x, third.Position.y, 0.0f), color);
            Renderer2D::SubmitLine(glm::vec3(third.Position.x, third.Position.y, 0.0f), glm::vec3(first.Position.x, first.Position.y, 0.0f), color);
        }
        else
        {
            BoneVertex& first =  m_SubMeshes[subMeshIndex].Vertices[triangle.First];
            BoneVertex& second = m_SubMeshes[subMeshIndex].Vertices[triangle.Second];
            BoneVertex& third =  m_SubMeshes[subMeshIndex].Vertices[triangle.Third];

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

   
    std::pair<uint32_t, SkinningEditorPanel::Triangle*> SkinningEditorPanel::findTriangle(const glm::vec2& pos)
    {
        for (size_t i = 0; i < m_SubMeshes.size(); ++i)
        {
            for (auto& triangle : m_SubMeshes[i].Triangles)
            {
                BoneVertex& first = m_SubMeshes[i].Vertices[triangle.First];
                BoneVertex& second = m_SubMeshes[i].Vertices[triangle.Second];
                BoneVertex& third = m_SubMeshes[i].Vertices[triangle.Third];
                if (Math::PointInTriangle(pos, glm::vec2(first.X, first.Y), glm::vec2(second.X, second.Y), glm::vec2(third.X, third.Y)))
                {
                    return { i, &triangle };
                }
            }
        }
        return { 0,nullptr };
    }
    std::pair<uint32_t, SkinningEditorPanel::BoneVertex*> SkinningEditorPanel::findVertex(const glm::vec2& pos)
    {
        for (size_t i = 0; i < m_SubMeshes.size(); ++i)
        {
            for (auto& vertex : m_SubMeshes[i].Vertices)
            {
                if (glm::distance(pos, glm::vec2(vertex.X, vertex.Y)) < m_PointRadius)
                {
                    return { i, &vertex };
                }
            }
        }
        return { 0,nullptr };
    }
    SkinningEditorPanel::PreviewBone* SkinningEditorPanel::findBone(const glm::vec2& pos)
    {
        PreviewBone* bone = nullptr;
        m_BoneHierarchy.Traverse([&](void* parent, void* child) -> bool {

            PreviewBone* childBone = static_cast<PreviewBone*>(child);
            float rot;
            glm::vec2 start, end, normal;
            if (IS_SET(m_Flags, Flags::PreviewPose))
            {
                decomposeBone(childBone, start, end, rot, normal);
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
        for (size_t i = 0; i < m_SubMeshes.size(); ++i)
        {
            for (auto& vertex : m_SubMeshes[i].Vertices)
            {
                if (glm::distance(glm::vec2(vertex.X, vertex.Y), pos) < radius)
                {
                    vertices.push_back(&vertex);
                }
            }
        }
    }
}