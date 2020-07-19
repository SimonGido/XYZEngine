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
#include "XYZ/Editor/EditorCamera.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/RenderCommand.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SpriteAnimation.h"
#include "XYZ/Renderer/SpriteAnimationController.h"
#include "XYZ/Renderer/SpriteSystem.h"
#include "XYZ/Renderer/MaterialManager.h"
#include "XYZ/Renderer/Framebuffer.h"
#include "XYZ/Renderer/RenderSortSystem.h"
#include "XYZ/Renderer/MeshFactory.h"

//-----Scene-----//
#include "XYZ/Scene/SceneCamera.h"
#include "XYZ/Scene/SceneManager.h"


//------ECS-----//
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/ECS/Entity.h"

//-----Systems-----//
#include "XYZ/Physics/PhysicsSystem.h"
#include "XYZ/Physics/GridCollisionSystem.h"
#include "XYZ/Physics/RealGridCollisionSystem.h"
#include "XYZ/Physics/InterpolatedMovementSystem.h"
#include "XYZ/Particle/ParticleSystem2D.h"



//-----Audio-----//
#include "XYZ/Audio/Audio.h"



#include "XYZ/Utils/DataStructures/Tree.h"
#include "XYZ/Gui/Font.h"



//------UI-------//
#include "XYZ/Gui/Text.h"
#include "XYZ/Gui/Button.h"
#include "XYZ/Gui/Checkbox.h"
#include "XYZ/Gui/GuiSystem.h"


//------InUI-------//
#include "XYZ/InGui/InGui.h"