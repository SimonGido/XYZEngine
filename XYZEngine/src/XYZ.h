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


//-----Renderer-----//
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/SceneRenderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/RenderPass.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/RenderTexture.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Renderer/Framebuffer.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Animation.h"

#include "XYZ/Particle/GPU/ParticleMaterial.h"
#include "XYZ/Particle/GPU/ParticleSystem.h"

//-----Scene-----//
#include "XYZ/Scene/SceneCamera.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/EditorComponents.h"

//---------Editor-----------//
#include "XYZ/Editor/EditorCamera.h"
#include "XYZ/Editor/Panels/ScenePanel.h"
#include "XYZ/Editor/Panels/SceneHierarchyPanel.h"
#include "XYZ/Editor/Inspector/InspectorPanel.h"
#include "XYZ/Editor/Inspector/SceneEntityInspectorContext.h"
#include "XYZ/Editor/Inspector/AssetInspectorContext.h"

#include "XYZ/Editor/Panels/AssetBrowser.h"
#include "XYZ/Editor/SkinningEditor/SkinningEditor.h"
#include "XYZ/Editor/AnimationEditor/AnimationEditor.h"

//------ECS-----//
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/ECS/Entity.h"
#include "XYZ/ECS/Serialization/ECSSerializer.h"


//-----Audio-----//
#include "XYZ/Audio/Audio.h"



//------Scripting-------//
#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Script/LuaModule.h"
#include "XYZ/Script/LuaEntity.h"

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