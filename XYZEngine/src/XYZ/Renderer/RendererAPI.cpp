#include "stdafx.h"
#include "RendererAPI.h"


namespace XYZ {

    static RenderAPICapabilities s_Capabilities;

    const RenderAPICapabilities& RendererAPI::GetCapabilities()
    {
        return s_Capabilities;
    }
    RenderAPICapabilities& RendererAPI::getCapabilities()
    {
        return s_Capabilities;
    }
}