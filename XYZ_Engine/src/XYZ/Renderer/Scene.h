#pragma once

#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Renderer/OrthoCamera.h"
#include "XYZ/AssetManager/AssetManager.h"

#include "XYZ/Utils/DataStructures/HashGrid2D.h"
#include "XYZ/Utils/DataStructures/Tree.h"

#include "SceneObject.h"


namespace XYZ
{
    /*! @class Scene
    *	@brief Holds all data relevant to a Scene
    */
    class Scene
    {
    public:
        Scene(std::string& name);
        ~Scene();

      
        /** Adds an scene object to the scene */
        uint16_t AddObject(const SceneObject& object);

        /** Triggers when the scene is being rendered */
        void OnRender();

        void SetParent(uint16_t parent, uint16_t child);

        /** Triggers when the scene is attached */
        void OnAttach();

        /** Triggers when the scene is detached */
        void OnDetach();

        SceneObject& GetObject(uint16_t index);

        inline const std::string& GetName() const { return m_Name; }
        inline const OrthoCamera& GetCamera() const { return m_Camera; }

        inline const Tree<SceneObject>& GetSceneGraph() const { return m_SceneGraph; }
    private:
  
        std::string m_Name;
        OrthoCamera m_Camera;

        uint16_t m_Root;
        SceneObject m_World;
        Tree<SceneObject> m_SceneGraph;
    };
}