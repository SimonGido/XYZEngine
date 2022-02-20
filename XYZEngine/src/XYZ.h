#pragma once
//-----Core-----//
#include "XYZ/Core/Application.h"
#include "XYZ/Core/Input.h"
#include "XYZ/Core/KeyCodes.h"
#include "XYZ/Core/MouseCodes.h" 
#include "XYZ/Core/WindowCodes.h"
#include "XYZ/Core/Ref/Ref.h"
#include "XYZ/Core/Ref/WeakRef.h"
#include "XYZ/Debug/Timer.h"


//-----Events-----//
#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Event/InputEvent.h"


//-----Renderer-----//
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/SceneRenderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/RenderPass.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Renderer/Framebuffer.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/MeshFactory.h"

//-----Animation-----//
#include "XYZ/Asset/Animation/SkeletonAsset.h"
#include "XYZ/Asset/Animation/AnimationAsset.h"

//-----Scene-----//
#include "XYZ/Scene/SceneCamera.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/SceneSerializer.h"


//------ECS-----//
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/ECS/Entity.h"

//-----Audio-----//
#include "XYZ/Audio/Audio.h"



//------Scripting-------//
#include "XYZ/Script/ScriptEngine.h"


//------Utils-------//
#include "XYZ/Utils/DataStructures/ByteBuffer.h"
#include "XYZ/Utils/DataStructures/MemoryPool.h"
#include "XYZ/Utils/Math/Ray.h"



//------Net---------//
#include "XYZ/Net/NetServer.h"
#include "XYZ/Net/NetClient.h"
#include "XYZ/Net/NetConnection.h"


//------Asset---------//
#include "XYZ/Asset/AssetManager.h"


//------UI-----------//
#include "XYZ/ImGui/ImGui.h"