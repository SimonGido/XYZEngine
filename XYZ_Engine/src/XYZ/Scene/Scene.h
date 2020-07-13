#pragma once
#include "XYZ/Core/Ref.h"
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Event/Event.h"

#include "XYZ/Utils/DataStructures/HashGrid2D.h"
#include "XYZ/Utils/DataStructures/Tree.h"
#include "XYZ/Renderer/RenderSortSystem.h"
#include "XYZ/Renderer/SpriteRenderComponent.h"
#include "XYZ/Gui/Text.h"
#include "XYZ/Gui/Image.h"

#include "XYZ/Renderer/Camera.h"

#include "XYZ/ECS/Entity.h"
#include "SceneCamera.h"
#include "Components.h"

namespace XYZ {

    struct SceneObject
    {
        RenderComponent* Renderable;
        Transform2D* Transform;
        Entity Entity;
    };
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

        void SetParent(uint16_t parent, uint16_t child);

        void SetState(SceneState state) { m_State = state; }

        /** Triggers when the scene is attached */
        void OnAttach();

        /** Triggers when the scene is detached */
        void OnDetach();

        void OnEvent(Event& e);

        void OnRender();

        void OnRenderEditor(float dt, const glm::mat4& viewProjectionMatrix);

        SceneState GetState() const { return m_State; }
        SceneObject& GetObject(uint16_t index);
        inline const std::string& GetName() const { return m_Name; }
        inline const SceneCamera& GetMainCamera() const { return m_MainCamera->Camera; }

        inline const Tree<SceneObject>& GetSceneGraph() const { return m_SceneGraph; }
   
    private:        
        template <typename T>
        void onEntityModified(T* component, Entity entity)
        {
            static_assert(std::is_base_of<Type<T>, T>::value, "Trying to add component that has no type");
            uint16_t id = IComponent::GetID<T>();
            if (id == IComponent::GetID<SpriteRenderComponent>() ||
                id == IComponent::GetID<Text>()                  ||
                id == IComponent::GetID<Image>())
            {
                uint16_t index = m_GraphMap[entity];
                m_SceneGraph[index].Renderable = (RenderComponent*)component;
            }
        }

    private:
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
        SceneObject m_World;

        Tree<SceneObject> m_SceneGraph;
        RenderSortSystem m_SortSystem;

        std::unordered_map<uint32_t, uint16_t> m_GraphMap;


        friend class Entity;
    };
}