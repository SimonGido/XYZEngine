#pragma once
//-----Core-----//
#include "XYZ/Core/Application.h"
#include "XYZ/Core/Input.h"
#include "XYZ/Core/KeyCodes.h"
#include "XYZ/Core/MouseCodes.h" 
#include "XYZ/Core/WindowCodes.h"
#include "XYZ/Core/Ref.h"
#include "XYZ/Timer.h"


//-----Events-----//
#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Event/InputEvent.h"
#include "XYZ/Event/GuiEvent.h"


//-----Renderer-----//
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/SceneRenderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/RenderPass.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/RenderTexture.h"


#include "XYZ/Renderer/Framebuffer.h"
#include "XYZ/Renderer/SubTexture.h"

//-----Scene-----//
#include "XYZ/Scene/SceneCamera.h"
#include "XYZ/Scene/AssetManager.h"
#include "XYZ/Scene/Animation.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Scene.h"



//---------Editor-----------//
#include "XYZ/Editor/EditorCamera.h"

//------ECS-----//
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/ECS/Entity.h"


//-----Audio-----//
#include "XYZ/Audio/Audio.h"

#include "XYZ/Utils/DataStructures/Graph.h"

#include "XYZ/Renderer/Font.h"



//------UI-------//
#include "XYZ/Gui/Text.h"
#include "XYZ/Gui/Button.h"
#include "XYZ/Gui/Checkbox.h"
#include "XYZ/Gui/Dockspace.h"


#include "XYZ/FSM/StateMachine.h"

//------Scripting-------//
#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Script/LuaModule.h"
#include "XYZ/Script/LuaEntity.h"

//------Utils-------//
#include "XYZ/Utils/DataStructures/HashGrid2D.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"


//------Net---------//
#include "XYZ/Net/NetServer.h"
#include "XYZ/Net/NetClient.h"
#include "XYZ/Net/NetConnection.h"



#include "XYZ/InGui/InGui.h"
#include "XYZ/InGui/InGuiDockspace.h"
