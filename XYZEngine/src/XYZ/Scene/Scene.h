#pragma once
#include "XYZ/Core/Ref.h"
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Event/Event.h"

#include "XYZ/Utils/DataStructures/HashGrid2D.h"
#include "XYZ/Utils/DataStructures/Tree.h"

#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/SpriteRenderer.h"
#include "XYZ/Renderer/Camera.h"
#include "Serializable.h"


#include "SceneCamera.h"
#include "Components.h"

namespace XYZ {

   
    enum class SceneState
    {
        Play,
        Edit,
        Pause
    };
    /*! @class Scene
    *	@brief Holds all data relevant to a Scene
    */
    template <typename T>
    class Asset;
    class Entity;
    class Scene : public RefCount,
                  public Serializable
    {
    public:
        Scene(const std::string& name);
        ~Scene();
    
        Entity CreateEntity(const std::string& name);

        void DestroyEntity(Entity entity);

        void SetParent(Entity parent, Entity child);

        void SetState(SceneState state) { m_State = state; }

        /** Triggers when the scene is attached */
        void OnAttach();

        /** Triggers when the scene is detached */
        void OnDetach();

        void OnEvent(Event& e);

        void OnRender();

        void OnRenderEditor(float dt, const SceneRenderData& renderData);

        SceneState GetState() const { return m_State; }

        Entity GetEntity(uint16_t index);

        inline const std::string& GetName() const { return m_Name; }

        inline const SceneCamera& GetMainCamera() const { return m_MainCamera->Camera; }
 
   
    private:
        struct SceneObject
        {
            TransformComponent* Transform = nullptr;
            uint32_t Entity;
            // Temporary
            int32_t Parent = -1;
        };

       
        struct RenderComparator
        {
            bool operator()(const SpriteRenderer& a, const SpriteRenderer& b) const
            {
                bool aTransparent = a.Material->IsSet(RenderFlags::TransparentFlag);
                bool bTransparent = b.Material->IsSet(RenderFlags::TransparentFlag);

                if (aTransparent && !bTransparent)
                    return false;

                if (!aTransparent && bTransparent)
                    return true;

                if (aTransparent && bTransparent)
                {
                    if (a.SortLayer == b.SortLayer)
                        return a.Material->GetSortKey() < b.Material->GetSortKey();
                    return a.SortLayer > b.SortLayer;
                }
                else
                {
                    if (a.Material->GetSortKey() == b.Material->GetSortKey())
                        return a.SortLayer < b.SortLayer;
                    return a.Material->GetSortKey() < b.Material->GetSortKey();
                }
            }
        };


    private:
        ECSManager* m_ECS = nullptr;
        ComponentGroup<TransformComponent, SpriteRenderer>* m_RenderGroup = nullptr;

        std::string m_Name;
        SceneState m_State = SceneState::Edit;

        uint32_t m_MainCameraEntity;
        CameraComponent* m_MainCamera;
        TransformComponent* m_MainCameraTransform;

        uint16_t m_Root;
        SceneObject m_SceneWorld;

        Tree<SceneObject> m_SceneGraph;
        std::unordered_map<uint32_t, uint16_t> m_SceneGraphMap;

     

        friend class Entity;
        friend class Asset<Scene>;
    };
}