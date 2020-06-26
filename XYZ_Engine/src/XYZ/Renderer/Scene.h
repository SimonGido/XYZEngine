#pragma once

#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Renderer/OrthoCamera.h"
#include "XYZ/AssetManager/AssetManager.h"
#include "XYZ/Utils/DataStructures/HashGrid2D.h"
#include "XYZ/Utils/DataStructures/Tree.h"

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

        inline const std::string& GetName() const { return m_Name; }
        inline const OrthoCamera& GetCamera() const { return m_Camera; }

        /** Adds an entity to the scene */
        void AddEntity(Entity entity);

        /** Triggers when the scene is being rendered */
        void OnRender();

        /** Triggers when the scene is attached */
        void OnAttach();

        /** Triggers when the scene is detached */
        void OnDetach();


    private:
        std::string m_Name;
        OrthoCamera m_Camera;
    };
}