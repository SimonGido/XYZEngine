#include "stdafx.h"
#include "SkinningEditor.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Asset/AssetManager.h"
#include "PreviewRenderer.h"


#include "XYZ/BasicUI/BasicUILoader.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {
    namespace Editor {

        static uint32_t s_NextBone = 1;
        static uint32_t s_NextSubmesh = 1;
        static Ref<SkeletalMesh> s_SkeletalMesh;

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

                float h = (float)index * (360.0f / (float)SkinningEditor::sc_MaxBones);
                //float s = sDist(rng);
                //float v = vDist(rng);
                float s = 80.0f;
                float v = 60.0f;
                return HSVtoRGB(h, s, v);
            }

            static glm::vec2 CalculateTexCoord(const glm::vec2& pos, const glm::vec2& size)
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
        SkinningEditor::SkinningEditor(const std::string& filepath)
            :
            EditorUI(filepath),
            m_ContextSize(glm::vec2(0.0f)),
            m_Window(nullptr),
            m_BoneTree(nullptr),
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
            m_Shader = Shader::Create("Assets/Shaders/Editor/SkinningEditor.glsl");
            for (uint32_t i = 0; i < NumCategories; ++i)
                m_CategoriesOpen[i] = false;


            FramebufferSpecs specs;
            specs.Width = 10;
            specs.Height = 10;
            specs.ClearColor = { 0.1f,0.2f,0.2f,1.0f };
            specs.Attachments = {
                FramebufferTextureSpecs(FramebufferTextureFormat::RGBA16F),
                FramebufferTextureSpecs(FramebufferTextureFormat::DEPTH24STENCIL8)
            };
            m_Framebuffer = Framebuffer::Create(specs);
            m_RenderTexture = RenderTexture::Create(m_Framebuffer);
            m_RenderSubTexture = Ref<SubTexture>::Create(m_RenderTexture, glm::vec2(0.0f, 0.0f));

            m_Layout = { 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, {1}, true };  
            SetupUI();
            rebuildRenderBuffers();
          
            for (uint32_t i = 0; i < sc_MaxBones; ++i)
                m_ColorIDs[i] = i;
            std::shuffle(&m_ColorIDs[0], &m_ColorIDs[sc_MaxBones - 1], std::default_random_engine(0));
        }

        SkinningEditor::~SkinningEditor()
        {
        }
       
        void SkinningEditor::OnUpdate(Timestep ts)
        {
            bUIAllocator& allocator = bUI::GetAllocator(GetName());
            updateLayout(allocator);

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
                m_WeightBrushStrength = allocator.GetElement<bUISlider>("Brush Strength")->Value / sc_WeightBrushDivisor;
                handleWeightsBrush();
            }
            if (m_SelectedSubmesh)
            {
                bUIHierarchyItem& item = m_MeshTree->GetItem(m_SelectedSubmesh->ID);
                item.Color = glm::vec4(0.2f, 0.7f, 1.0f, 1.0f);     
                if (item.Open) // Editing
                    item.Color = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f);                          
            }
            else if (m_SelectedBone)
            {
                bUIHierarchyItem& item = m_BoneTree->GetItem(m_SelectedBone->ID);
                item.Color = glm::vec4(0.2f, 0.7f, 1.0f, 1.0f);     
                if (item.Open) // Editing
                    item.Color = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f);       
            }
            updateBoneHierarchy();
            if (IS_SET(m_Flags, PreviewPose))
                updateRenderBuffers();
            renderAll();
        }

        void SkinningEditor::OnReload()
        {
            SetupUI();
        }
       
        void SkinningEditor::SetupUI()
        {
            bUIAllocator& allocator = bUI::GetAllocator(GetName());

            m_Window = allocator.GetElement<bUIWindow>("Skinning Editor");
            m_PreviewWindow = allocator.GetElement<bUIWindow>("Skinning Preview");
            m_PreviewWindow->BlockEvents = false;
            m_Image = allocator.GetElement<bUIImage>("Skinning Editor Image");
            m_Image->BlockEvents = false;
            m_BoneTree = allocator.GetElement<bUITree>("Bones");
            m_BoneTree->OnSelect = [&](uint32_t id) {
                m_SelectedBone = static_cast<PreviewBone*>(m_BoneHierarchy.GetData(id));
            };

            m_MeshTree = allocator.GetElement<bUITree>("Submeshes");
            m_MeshTree->OnSelect = [&](uint32_t id) {
                for (Submesh& subMesh : m_Mesh.Submeshes)
                {
                    if (subMesh.ID == id)
                        m_SelectedSubmesh = &subMesh;
                }
            };
            m_ViewportSize = m_Image->Size;
            m_Camera.ProjectionMatrix = glm::ortho(
                -m_Image->Size.x / 2.0f, m_Image->Size.x / 2.0f,
                -m_Image->Size.y / 2.0f, m_Image->Size.y / 2.0f
            );
            m_Camera.UpdateViewProjection();

            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

            m_Image->ImageSubTexture = m_RenderSubTexture;

            m_PreviewWindow->OnResize = [&](const glm::vec2& size) {
                m_Framebuffer->Resize((uint32_t)m_Image->Size.x, (uint32_t)m_Image->Size.y);
                m_ViewportSize = m_Image->Size;
                m_Camera.ProjectionMatrix = glm::ortho(
                    -m_Image->Size.x / 2.0f, m_Image->Size.x / 2.0f,
                    -m_Image->Size.y / 2.0f, m_Image->Size.y / 2.0f
                );
                m_Camera.UpdateViewProjection();
            };

            bUIButton* clearButton = allocator.GetElement<bUIButton>("Clear");
            clearButton->Callbacks.push_back([&](bUICallbackType type, bUIElement& element) {
                if (type == bUICallbackType::Active)
                    clear();
            });

            bUIButton* saveButton = allocator.GetElement<bUIButton>("Save");
            saveButton->Callbacks.push_back([&](bUICallbackType type, bUIElement& element) {
                if (type == bUICallbackType::Active)
                    save();
            });
            setupBoneUI();
            setupVertexUI();
            setupWeightsUI();
        }

        void SkinningEditor::SetContext(const Ref<SubTexture>& context)
        {
            m_Context = context;
            m_ContextSize.x = (float)m_Context->GetTexture()->GetWidth();
            m_ContextSize.y = (float)m_Context->GetTexture()->GetHeight();
            rebuildRenderBuffers();
        }
   
        void SkinningEditor::OnEvent(Event& event)
        {
            EventDispatcher dispatcher(event);
 
            dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&SkinningEditor::onMouseButtonPress, this));
            dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&SkinningEditor::onMouseButtonRelease, this));
            dispatcher.Dispatch<MouseScrollEvent>(Hook(&SkinningEditor::onMouseScroll, this));
            dispatcher.Dispatch<KeyPressedEvent>(Hook(&SkinningEditor::onKeyPress, this));
            dispatcher.Dispatch<KeyTypedEvent>(Hook(&SkinningEditor::onKeyType, this));
        }
       
        void SkinningEditor::save()
        {
            Ref<Shader> shader = AssetManager::GetAsset<Shader>(AssetManager::GetAssetHandle("Assets/Shaders/SkeletalShader.glsl.shader"));
            Ref<Material> material = AssetManager::GetAsset<Material>(AssetManager::GetAssetHandle("Assets/Materials/SkeletalMaterial.mat"));
            Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(AssetManager::GetAssetHandle("Assets/Textures/SimpleChar.tex"));
            material->Set("u_Texture", texture);
            material->Set("u_Color", glm::vec4(1.0f));

            std::vector<XYZ::AnimatedVertex> vertices;
            std::vector<uint32_t> indices;
            std::vector<Bone> bones;
            Tree boneHierarchy(m_BoneHierarchy);

            uint32_t counter = 0;
            for (const auto& subMesh :  m_Mesh.Submeshes)
            {
                for (const auto& vertex : subMesh.VerticesLocalToBones)
                {
                    auto& previewVertex =  m_Mesh.PreviewVertices[counter];
                    VertexBoneData boneData;
                    for (size_t i = 0; i < BoneData::sc_MaxBonesPerVertex; ++i)
                    {
                        if (vertex.Data.IDs[i] != -1)
                        {
                            boneData.IDs[i] = vertex.Data.IDs[i];
                            boneData.Weights[i] = vertex.Data.Weights[i];
                        }
                    }          
                    vertices.push_back({ glm::vec3(vertex.Position, 0.0f), previewVertex.TexCoord, boneData });
                    counter++;
                }
            }
            counter = 0;
            for (auto& subMesh : m_Mesh.Submeshes)
            {
                for (auto& triangle : subMesh.Triangles)
                {
                    indices.push_back(triangle.First + counter);
                    indices.push_back(triangle.Second + counter);
                    indices.push_back(triangle.Third + counter);
                }
                counter += subMesh.GeneratedVertices.size();
            }

            counter = 0;
            bones.resize(m_Bones.size());
            for (auto& bone : m_Bones)
            {
                bones[counter].Transform = bone->LocalTransform;
                bones[counter].Name = bone->Name;
                bones[counter].ID = bone->ID;
                boneHierarchy.SetData(bones[counter].ID, &bones[counter]);
                counter++;
            }

            s_SkeletalMesh = AssetManager::GetAsset<SkeletalMesh>(AssetManager::GetAssetHandle("Assets/Meshes/SkeletalMesh.skm"));
            
            //AssetManager::CreateAsset<SkeletalMesh>(
            //    "CopySkeletalMesh.skm", 
            //    AssetType::SkeletalMesh, 
            //    AssetManager::GetDirectoryHandle("Assets/Meshes"), 
            //    mesh->GetVertices(), 
            //    mesh->GetIndicies(), 
            //    mesh->GetBones(), 
            //    mesh->GetBoneHierarchy(), 
            //    mesh->GetMaterial()
            //    );


            //AssetManager::CreateAsset<SkeletalMesh>("SkeletalMesh.skm", AssetType::SkeletalMesh, AssetManager::GetDirectoryHandle("Assets/Meshes"), vertices, indices, bones, boneHierarchy, material);
        }

 
        void SkinningEditor::setupBoneUI()
        {
            bUIAllocator& allocator = bUI::GetAllocator(GetName());
            
            bUICheckbox* previewPose = allocator.GetElement<bUICheckbox>("Preview Pose");
            previewPose->Callbacks.push_back([&](bUICallbackType type, bUIElement& element) {
                bUICheckbox& casted = static_cast<bUICheckbox&>(element);
                if (type == bUICallbackType::StateChange)
                {
                    if (casted.Checked)
                    {
                        m_Flags = ToggleBit(m_Flags, PreviewPose);
                        initializePose();
                        updateRenderBuffers();
                        int32_t id = casted.GetID();
                        bUI::ForEach<bUICheckbox>(GetName(), [id](bUICheckbox& checkbox) {
                            if (checkbox.GetID() != id)
                                checkbox.Checked = false;
                            });
                    }
                    else
                    {
                        m_Flags &= ~PreviewPose;
                    }
                }
             });


            bUICheckbox* createBone = allocator.GetElement<bUICheckbox>("Create Bone");
            createBone->Callbacks.push_back([&](bUICallbackType type, bUIElement& element) {
                bUICheckbox& casted = static_cast<bUICheckbox&>(element);
                if (type == bUICallbackType::StateChange)
                {
                    if (casted.Checked)
                    {
                        m_Flags = ToggleBit(m_Flags, CreateBone);
                        int32_t id = casted.GetID();
                        bUI::ForEach<bUICheckbox>(GetName(), [id](bUICheckbox& checkbox) {
                            if (checkbox.GetID() != id)
                                checkbox.Checked = false;
                        });
                    }
                    else
                    {
                        m_Flags &= ~CreateBone;
                    }
                }
               });
            bUICheckbox * editBone = allocator.GetElement<bUICheckbox>("Edit Bone");
            editBone->Callbacks.push_back([&](bUICallbackType type, bUIElement& element) {
                bUICheckbox& casted = static_cast<bUICheckbox&>(element);
                if (type == bUICallbackType::StateChange)
                {
                    if (casted.Checked)
                    {
                        uint16_t previewPose = m_Flags & PreviewPose;
                        m_Flags = ToggleBit(m_Flags, EditBone);
                        m_Flags |= previewPose;
                    }
                    else
                    {
                        m_Flags &= ~PreviewPose;
                    }
                }
            });

          
            bUICheckbox* deleteBone = allocator.GetElement<bUICheckbox>("Delete Bone");
            deleteBone->Callbacks.push_back([&](bUICallbackType type, bUIElement& element) {
                bUICheckbox& casted = static_cast<bUICheckbox&>(element);
                if (type == bUICallbackType::StateChange)
                {
                    if (casted.Checked)
                    {
                        m_Flags = ToggleBit(m_Flags, DeleteBone);
                        int32_t id = casted.GetID();
                        bUI::ForEach<bUICheckbox>(GetName(), [id](bUICheckbox& checkbox) {
                            if (checkbox.GetID() != id)
                                checkbox.Checked = false;
                        });
                    }
                    else
                    {
                        m_Flags &= ~DeleteBone;
                    }
                }
             });
        }
        void SkinningEditor::setupVertexUI()
        {
            bUIAllocator& allocator = bUI::GetAllocator(GetName());

            bUIButton* createSubMesh = allocator.GetElement<bUIButton>("Create Submesh");
            createSubMesh->Callbacks.push_back([&](bUICallbackType type, bUIElement& element) {
                if (type == bUICallbackType::Active)
                {
                    if (!m_Triangulated)
                        createSubmesh();
                }
                
            });


            bUICheckbox* createVertex = allocator.GetElement<bUICheckbox>("Create Vertex");
            createVertex->Callbacks.push_back([&](bUICallbackType type, bUIElement& element) {
                bUICheckbox& casted = static_cast<bUICheckbox&>(element);
                if (type == bUICallbackType::StateChange)
                {
                    if (casted.Checked)
                    {
                        m_Flags = ToggleBit(m_Flags, CreateVertex);
                        int32_t id = casted.GetID();
                        bUI::ForEach<bUICheckbox>(GetName(), [id](bUICheckbox& checkbox) {
                            if (checkbox.GetID() != id)
                                checkbox.Checked = false;
                            });
                    }
                    else
                    {
                        m_Flags &= ~CreateVertex;
                    }
                }
                });


            bUICheckbox* editVertex = allocator.GetElement<bUICheckbox>("Edit Vertex");
            editVertex->Callbacks.push_back([&](bUICallbackType type, bUIElement& element) {
                bUICheckbox& casted = static_cast<bUICheckbox&>(element);
                if (type == bUICallbackType::StateChange)
                {
                    if (casted.Checked)
                    {
                        m_Flags = ToggleBit(m_Flags, EditVertex);
                        int32_t id = casted.GetID();
                        bUI::ForEach<bUICheckbox>(GetName(), [id](bUICheckbox& checkbox) {
                            if (checkbox.GetID() != id)
                                checkbox.Checked = false;
                            });
                    }
                    else
                    {
                        m_Flags &= ~EditVertex;
                    }
                }
                });

            bUICheckbox* deleteVertex = allocator.GetElement<bUICheckbox>("Delete Vertex");
            deleteVertex->Callbacks.push_back([&](bUICallbackType type, bUIElement& element) {
                bUICheckbox& casted = static_cast<bUICheckbox&>(element);
                if (type == bUICallbackType::StateChange)
                {
                    if (casted.Checked)
                    {
                        m_Flags = ToggleBit(m_Flags, DeleteVertex);
                        int32_t id = casted.GetID();
                        bUI::ForEach<bUICheckbox>(GetName(), [id](bUICheckbox& checkbox) {
                            if (checkbox.GetID() != id)
                                checkbox.Checked = false;
                            });
                    }
                    else
                    {
                        m_Flags &= ~DeleteVertex;
                    }
                }
                });

            bUICheckbox* deleteTriangle = allocator.GetElement<bUICheckbox>("Delete Triangle");
            deleteTriangle->Callbacks.push_back([&](bUICallbackType type, bUIElement& element) {
                bUICheckbox& casted = static_cast<bUICheckbox&>(element);
                if (type == bUICallbackType::StateChange)
                {
                    if (casted.Checked)
                    {
                        m_Flags = ToggleBit(m_Flags, DeleteTriangle);
                        int32_t id = casted.GetID();
                        bUI::ForEach<bUICheckbox>(GetName(), [id](bUICheckbox& checkbox) {
                            if (checkbox.GetID() != id)
                                checkbox.Checked = false;
                            });
                    }
                    else
                    {
                        m_Flags &= ~DeleteTriangle;
                    }
                }
                });
            bUIButton* triangulate = allocator.GetElement<bUIButton>("Triangulate");
            triangulate->Callbacks.push_back([&](bUICallbackType type, bUIElement& element) {
                bUIButton& casted = static_cast<bUIButton&>(element);
                if (type == bUICallbackType::Active)
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
               });
        }
        void SkinningEditor::setupWeightsUI()
        {
            bUIAllocator& allocator = bUI::GetAllocator(GetName());
            
            bUICheckbox* weigthBrush = allocator.GetElement<bUICheckbox>("Weight Brush");
            weigthBrush->Callbacks.push_back([&](bUICallbackType type, bUIElement& element) {
                bUICheckbox& casted = static_cast<bUICheckbox&>(element);
                if (type == bUICallbackType::StateChange)
                {
                    if (casted.Checked)
                    {
                        m_Flags = ToggleBit(m_Flags, WeightBrush);
                        int32_t id = casted.GetID();
                        bUI::ForEach<bUICheckbox>(GetName(), [id](bUICheckbox& checkbox) {
                            if (checkbox.GetID() != id)
                                checkbox.Checked = false;
                            });
                    }
                    else
                    {
                        m_Flags &= ~WeightBrush;
                    }
                }
            });
        }
        void SkinningEditor::updateLayout(bUIAllocator& allocator)
        {
            bUIScrollbox* scrollbox = allocator.GetElement<bUIScrollbox>("Scrollbox");
            bUIButton* clearButton = allocator.GetElement<bUIButton>("Clear");
            bUIButton* saveButton = allocator.GetElement<bUIButton>("Save");
            bUIWindow* last = nullptr;
            bUI::ForEach<bUIWindow>(allocator, scrollbox, [&](bUIWindow& win) {
                win.Size.x = scrollbox->Size.x - 20.0f;
                win.Coords.y = 35.0f;
                if (last)
                    win.Coords.y = last->Coords.y + last->GetSize().y + 10.0f;
                if (win.Visible)
                    last = &win;
            });
            clearButton->Coords.y = last->Coords.y + last->GetSize().y;
            saveButton->Coords.y = clearButton->Coords.y + clearButton->Size.y + 10.0f;

            bUIWindow* boneWindow = allocator.GetElement<bUIWindow>("Bone Window");
            bUI::SetupLayout(allocator, *boneWindow, m_Layout);
            bUIWindow* vertexWindow = allocator.GetElement<bUIWindow>("Vertex Window");
            bUI::SetupLayout(allocator, *vertexWindow, m_Layout);
            bUIWindow* weightsWindow = allocator.GetElement<bUIWindow>("Weights Window");
            bUI::SetupLayout(allocator, *weightsWindow, m_Layout);
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
                    m_Flags |= EditBone;
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
                    m_FoundVertex = nullptr;
                }
                else if (IS_SET(m_Flags, DeleteVertex))
                {
                    // TODO: Implement
                    if (!m_Triangulated)
                        m_Mesh.EraseVertexAtPosition(m_MousePosition);
                    m_SelectedVertex = nullptr;
                    m_FoundVertex = nullptr;
                }
                else if (IS_SET(m_Flags, DeleteTriangle))
                {
                    if (m_Mesh.EraseTriangleAtPosition(m_MousePosition))
                    {
                        rebuildRenderBuffers();
                        m_SelectedTriangle = nullptr;
                        m_FoundTriangle = nullptr;

                        m_SelectedVertex = nullptr;
                        m_FoundVertex = nullptr;
                    }
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
                // Edit bone was set right after creation of bone
                if (IS_SET(m_Flags, (CreateBone & EditBone)))
                    m_Flags &= ~EditBone;
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
            if (event.IsKeyPressed(KeyCode::KEY_BACKSPACE))
            {
                if (m_SelectedSubmesh)
                {
                    bUIHierarchyItem& item = m_MeshTree->GetItem(m_SelectedSubmesh->ID);
                    if (item.Open && !item.Label.empty())
                    {
                        item.Label.pop_back();
                        return true;
                    }
                }
                else if (m_SelectedBone)
                {
                    bUIHierarchyItem& item = m_BoneTree->GetItem(m_SelectedBone->ID);
                    if (item.Open && !item.Label.empty())
                    {
                        item.Label.pop_back();
                        return true;
                    }
                }
            }
            return false;
        }
        bool SkinningEditor::onKeyType(KeyTypedEvent& event)
        {
            if (m_SelectedSubmesh)
            {
                bUIHierarchyItem& item = m_MeshTree->GetItem(m_SelectedSubmesh->ID);
                if (item.Open)
                {
                    item.Label.push_back(event.GetKey());
                    return true;
                }
            }
            else if (m_SelectedBone)
            {
                bUIHierarchyItem& item = m_BoneTree->GetItem(m_SelectedBone->ID);
                if (item.Open)
                {
                    item.Label.push_back(event.GetKey());
                    return true;
                }
            }
            return false;
        }
        void SkinningEditor::renderAll()
        {
            m_Framebuffer->Bind();
            Renderer::SetClearColor(m_Framebuffer->GetSpecification().ClearColor);
            Renderer::Clear();
            

            Renderer2D::BeginScene(m_Camera.ViewProjectionMatrix);
            
            if (s_SkeletalMesh.Raw())
            {
                s_SkeletalMesh->Render();
            }
            else
            {
                renderPreviewMesh();
            }
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
            auto [width, height] = Input::GetWindowSize();
            Renderer::SetViewPort(0, 0, (uint32_t)width, (uint32_t)height);
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
                        PreviewRenderer::RenderTriangle(*m_SelectedSubmesh, *m_SelectedTriangle, m_HighlightColor);
                    if (m_SelectedVertex)
                        Renderer2D::SubmitCircle(glm::vec3(m_SelectedVertex->Position, 0.0f), SkinnedMesh::PointRadius, 20, m_HighlightColor);
                }
            }
            if (m_FoundBone)
                PreviewRenderer::RenderBone(*m_FoundBone, m_HighlightColor, PreviewBone::PointRadius, IS_SET(m_Flags, PreviewPose));   
            if (m_SelectedBone)
                PreviewRenderer::RenderBone(*m_SelectedBone, m_HighlightColor, PreviewBone::PointRadius, IS_SET(m_Flags, PreviewPose));
        }
      
        void SkinningEditor::renderPreviewMesh()
        {
            m_Shader->Bind();
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
            m_BoneTree->Clear();
            m_MeshTree->Clear();
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
        void SkinningEditor::createSubmesh()
        {
            char buffer[20];
            sprintf(buffer, "submesh_%u", s_NextSubmesh);
            m_Mesh.Submeshes.emplace_back(glm::vec4(Helper::RandomColor(m_ColorIDs[s_NextSubmesh]), 1.0f), s_NextSubmesh);
            m_MeshTree->AddItem(s_NextSubmesh++, bUIHierarchyItem(buffer), true);
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
            m_BoneTree->RemoveItem(bone->ID);
            m_BonePool.Deallocate<PreviewBone>(bone);
        }
        void SkinningEditor::createVertex(const glm::vec2& pos)
        {
            if (!m_Triangulated)
            {
                if (m_Mesh.Submeshes.empty())
                    createSubmesh();            
                if (!m_SelectedSubmesh)
                    m_SelectedSubmesh = &m_Mesh.Submeshes.back();
                m_SelectedSubmesh->GeneratedVertices.push_back({ pos });
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
            updateBoneHierarchy();
            m_Mesh.InitializeVerticesLocalToBone(m_BoneHierarchy);
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
                offset += subMesh.GeneratedVertices.size();
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
                m_BoneTree->AddItem(newBone->ID, m_SelectedBone->ID, bUIHierarchyItem(newBone->Name));             
                newBone->LocalPosition = m_MousePosition - m_SelectedBone->WorldPosition;
            }
            else
            {
                newBone->ID = m_BoneHierarchy.Insert(newBone);
                m_BoneTree->AddItem(newBone->ID, bUIHierarchyItem(newBone->Name));
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
                for (auto& vertex : subMesh.GeneratedVertices)
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
            mx *= m_Image->Size.x / 2.0f;
            my *= m_Image->Size.y / 2.0f;
            mx += m_Camera.Position.x;
            my += m_Camera.Position.y;
            return { mx , my };
        }

        std::pair<float, float> SkinningEditor::getMouseViewportSpace() const
        {
            auto [mx, my] = Input::GetMousePosition();
            glm::vec2 position = m_Image->GetAbsolutePosition();
            mx -= position.x;
            my -= position.y;

            auto viewportWidth =  m_Image->Size.x;
            auto viewportHeight = m_Image->Size.y;

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


