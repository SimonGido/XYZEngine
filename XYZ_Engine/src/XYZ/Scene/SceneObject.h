#pragma once
#include "XYZ/Physics/Transform.h"
#include "XYZ/Renderer/RenderComponent.h"
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/ECS/Entity.h"

namespace XYZ {
    struct SceneObject
    {
        RenderComponent* Renderable;
        Transform2D* Transform;
        Entity Entity;
    };

    struct SceneSetup
    {
        void operator()(SceneObject& parent, SceneObject& child)
        {
            child.Transform->SetParent(parent.Transform);
        }
    };
}