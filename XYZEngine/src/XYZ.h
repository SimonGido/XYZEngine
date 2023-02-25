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
#include "XYZ/Core/Platform.h"

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


//-----Scene-----//
#include "XYZ/Scene/SceneCamera.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/SceneSerializer.h"


#include "XYZ/Project/Project.h"


//-----Audio-----//
#include "XYZ/Audio/Audio.h"



//------Scripting-------//
#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Plugin/PluginManager.h"
#include "XYZ/Plugin/PluginInterface.h"

//------Utils-------//
#include "XYZ/Utils/DataStructures/ByteBuffer.h"
#include "XYZ/Utils/DataStructures/MemoryPool.h"
#include "XYZ/Utils/Math/Ray.h"
#include "XYZ/Utils/Math/Math.h"



//------Net---------//
#include "XYZ/Net/NetServer.h"
#include "XYZ/Net/NetClient.h"
#include "XYZ/Net/NetConnection.h"
#include "XYZ/Net/UDPServer.h"


//------Asset---------//
#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Asset/AssetLifeManager.h"
#include "XYZ/Asset/Animation/SkeletonAsset.h"
#include "XYZ/Asset/Animation/AnimationAsset.h"
#include "XYZ/Scene/Prefab.h"

//------UI-----------//
#include "XYZ/ImGui/ImGui.h"
#include "XYZ/ImGui/Node/ImGuiNodeContext.h"