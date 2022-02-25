#include "stdafx.h"
#include "ScriptEngineRegistry.h"

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Utils/Delegate.h"

#include "ScriptWrappers.h"
#include "Wrappers/Components/TransformComponentWrappers.h"
#include "Wrappers/Components/SpriteRendererWrappers.h"
#include "Wrappers/Components/RigidBody2DWrapper.h"
#include "Wrappers/Components/MeshComponentWrapper.h"
#include "Wrappers/Components/AnimationComponentWrapper.h"

#include "Wrappers/Renderer/Texture2DWrapper.h"
#include "Wrappers/Renderer/SubTextureWrapper.h"
#include "Wrappers/Renderer/MaterialWrapper.h"
#include "Wrappers/Renderer/ShaderWrapper.h"
#include "Wrappers/Renderer/MeshWrapper.h"
#include "Wrappers/Animation/AnimationWrapper.h"

#include "Wrappers/SceneEntityNative.h"
#include "Wrappers/InputNative.h"
#include "Wrappers/AssetManagerNative.h"
#include "Wrappers/LoggerNative.h"

namespace XYZ {	

	void ScriptEngineRegistry::RegisterAll()
	{

		Script::InputNative::Register();
		Script::AssetManagerNative::Register();
		Script::LoggerNative::Register();

		// Components
		Script::SceneEntityNative::Register();
		Script::TransformComponentNative::Register();
		Script::SpriteRendererNative::Register();
		Script::RigidBody2DNative::Register();
		Script::MeshComponentNative::Register();
		Script::AnimatedMeshComponentNative::Register();
		Script::AnimationComponentNative::Register();
		////////////////////////
	
		// Renderer
		Script::Texture2DNative::Register();
		Script::SubTextureNative::Register();
		
		Script::ShaderNative::Register();
		Script::ShaderAssetNative::Register();
		
		Script::MaterialNative::Register();
		Script::MaterialInstanceNative::Register();
		Script::MaterialAssetNative::Register();
		
		Script::MeshSourceNative::Register();
		Script::MeshNative::Register();
		Script::AnimatedMeshNative::Register();


		Script::AnimationAssetNative::Register();
		Script::AnimationControllerNative::Register();
		Script::SkeletonAssetNative::Register();
		////////////////////////
	}
}