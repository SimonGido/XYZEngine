#pragma once

#include "XYZ/ECS/ECSManager.h"
#include "XYZ/AssetManager/AssetManager.h"
#include "XYZ/Event/Event.h"

#include "XYZ/Utils/DataStructures/HashGrid2D.h"
#include "XYZ/Utils/DataStructures/Tree.h"

#include "SceneCamera.h"
#include "SceneObject.h"
#include "Components.h"

namespace XYZ
{
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

        /** Triggers when the scene is being rendered */
        void OnRender();

        void SetParent(uint16_t parent, uint16_t child);

        /** Triggers when the scene is attached */
        void OnAttach();

        void OnEvent(Event& e);

        /** Triggers when the scene is detached */
        void OnDetach();

        SceneObject& GetObject(uint16_t index);

        inline const std::string& GetName() const { return m_Name; }
        inline const SceneCamera& GetMainCamera() const { return m_MainCamera->Camera; }

        inline const Tree<SceneObject>& GetSceneGraph() const { return m_SceneGraph; }
    private: 
        std::string m_Name;
        CameraComponent* m_MainCamera;

        uint16_t m_Root;
        Entity m_MainCameraEntity;
        SceneObject m_World;

        Tree<SceneObject> m_SceneGraph;
        std::unordered_map<uint32_t, uint16_t> m_GraphMap;
    };
}