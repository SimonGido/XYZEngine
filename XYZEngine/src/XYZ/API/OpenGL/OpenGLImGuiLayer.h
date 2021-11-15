#pragma once
#include "XYZ/ImGui/ImGuiLayer.h"

namespace  XYZ
{
    class OpenGLImGuiLayer : public ImGuiLayer
    {
    public:
        virtual void Begin() override;
        virtual void End() override;

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        virtual void OnEvent(Event& event) override;
    };
}

