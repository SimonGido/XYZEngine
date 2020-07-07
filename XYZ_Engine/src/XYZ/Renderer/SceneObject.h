#pragma once
#include "XYZ/Physics/Transform.h"
#include "XYZ/Renderer/RenderComponent.h"
#include "XYZ/ECS/ECSManager.h"


namespace XYZ {
    struct SceneObject
    {
        RenderComponent* Renderable;
        Transform2D* Transform;
        Entity Ent;
    };

    struct SceneSetup
    {
        void operator()(SceneObject& parent, SceneObject& child)
        {
            child.Transform->SetParent(parent.Transform);
        }
    };
}