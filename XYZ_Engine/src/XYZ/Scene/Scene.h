#pragma once
#include "XYZ/Core/Ref.h"
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Event/Event.h"

#include "XYZ/Utils/DataStructures/HashGrid2D.h"
#include "XYZ/Utils/DataStructures/Tree.h"
#include "XYZ/Renderer/RenderSortSystem.h"


#include "XYZ/Renderer/Camera.h"

#include "XYZ/ECS/Entity.h"
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
    class Scene : public RefCount
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

        void OnRenderEditor(float dt, const glm::mat4& viewProjectionMatrix);

        SceneState GetState() const { return m_State; }

        inline const std::string& GetName() const { return m_Name; }
        inline const SceneCamera& GetMainCamera() const { return m_MainCamera->Camera; }
 
    private:        
        template <typename T>
        void onEntityModified(T* component, Entity entity)
        {
            static_assert(std::is_base_of<Type<T>, T>::value, "Trying to add component that has no type");
            uint16_t id = IComponent::GetID<T>();
            if (id == IComponent::GetID<RenderComponent2D>())
            {
                uint16_t index = m_SceneGraphMap[entity];
                m_SceneGraph[index].Renderable = (RenderComponent2D*)component;
            }
        }

    private:
        struct SceneObject
        {
            RenderComponent2D* Renderable = nullptr;
            Transform2D* Transform = nullptr;
            Entity Entity;
        };

        struct SceneSetup
        {
            void operator()(SceneObject& parent, SceneObject& child)
            {
                child.Transform->SetParent(parent.Transform);
            }
        };
    private:
        std::string m_Name;
        SceneState m_State = SceneState::Edit;

        Entity m_MainCameraEntity;
        CameraComponent* m_MainCamera;
        Transform2D* m_MainCameraTransform;

        uint16_t m_Root;
        SceneObject m_SceneWorld;
        RenderSortSystem m_SortSystem;

        Tree<SceneObject> m_SceneGraph;
        std::unordered_map<uint32_t, uint16_t> m_SceneGraphMap;


        friend class Entity;
    };
}