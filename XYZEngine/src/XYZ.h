#pragma once
//-----Core-----//
#include "XYZ/Core/Application.h"
#include "XYZ/Core/Input.h"
#include "XYZ/Core/KeyCodes.h"
#include "XYZ/Core/MouseCodes.h" 
#include "XYZ/Core/Ref.h"
#include "XYZ/Timer.h"


//-----Events-----//
#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Event/InputEvent.h"
#include "XYZ/Event/GuiEvent.h"


//-----Renderer-----//
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/RenderPass.h"
#include "XYZ/Renderer/Material.h"

#include "XYZ/Renderer/MaterialManager.h"
#include "XYZ/Renderer/Framebuffer.h"
#include "XYZ/Renderer/MeshFactory.h"
#include "XYZ/Renderer/SpriteRenderer.h"
#include "XYZ/Renderer/SubTexture2D.h"

//-----Scene-----//
#include "XYZ/Scene/SceneCamera.h"
#include "XYZ/Scene/SceneManager.h"
#include "XYZ/Scene/AssetManager.h"
#include "XYZ/Scene/Animation.h"

//------ECS-----//
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/ECS/Entity.h"

//-----Systems-----//
#include "XYZ/Physics/PhysicsSystem.h"
#include "XYZ/Physics/GridCollisionSystem.h"
#include "XYZ/Physics/RealGridCollisionSystem.h"
#include "XYZ/Particle/ParticleSystem2D.h"



//-----Audio-----//
#include "XYZ/Audio/Audio.h"



#include "XYZ/Utils/DataStructures/Tree.h"
#include "XYZ/Utils/DataStructures/Graph.h"

#include "XYZ/Renderer/Font.h"



//------UI-------//
#include "XYZ/Gui/Text.h"
#include "XYZ/Gui/Button.h"
#include "XYZ/Gui/Checkbox.h"

//------InGui-------//
#include "XYZ/InGui/InGui.h"


#include "XYZ/FSM/StateMachine.h"


//------NativeScript-------//
#include "XYZ/NativeScript/ScriptableEntity.h"

