#pragma once

#include "XYZ/ECS/ECSManager.h"
#include "XYZ/AssetManager/AssetManager.h"
#include "XYZ/Event/Event.h"

#include "XYZ/Utils/DataStructures/HashGrid2D.h"
#include "XYZ/Utils/DataStructures/Tree.h"
#include "XYZ/Renderer/RenderSortSystem.h"
#include "XYZ/Renderer/Camera.h"

#include "SceneCamera.h"
#include "SceneObject.h"
#include "Components.h"

namespace XYZ
{
    enum class SceneState
    {
        Play,
        Edit,
        Pause
    };
    /*! @class Scene
    *	@brief Holds all data relevant to a Scene
    */
    class Scene
    {
    public:
        Scene(const std::string& name);
        ~Scene();

      
        Entity CreateEntity(const std::string& name);

        void DestroyEntity(Entity entity);


        void SetParent(uint16_t parent, uint16_t child);

        /** Triggers when the scene is attached */
        void OnAttach();

        void OnEvent(Event& e);

        /** Triggers when the scene is detached */
        void OnDetach();

        void OnRender();

        void OnRenderEditor(float dt, const glm::mat4& viewProjectionMatrix);

        SceneObject& GetObject(uint16_t index);

        void SetState(SceneState state) { m_State = state; }
        SceneState GetState() const { return m_State; }

        inline const std::string& GetName() const { return m_Name; }
        inline const SceneCamera& GetMainCamera() const { return m_MainCamera->Camera; }

        inline const Tree<SceneObject>& GetSceneGraph() const { return m_SceneGraph; }
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
    };
}