#include "stdafx.h"
#include "ScriptEngineRegistry.h"

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Animation/Animation.h"
#include "XYZ/Animation/Animator.h"
#include "XYZ/Utils/Delegate.h"

#include "ScriptWrappers.h"
#include "Wrappers/Components/TransformComponentWrappers.h"
#include "Wrappers/Components/SpriteRendererWrappers.h"
#include "Wrappers/Components/RigidBody2DWrapper.h"
#include "Wrappers/Components/SpriteRendererWrappers.h"

#include "Wrappers/Renderer/Texture2DWrapper.h"
#include "Wrappers/Renderer/SubTextureWrapper.h"
#include "Wrappers/Renderer/MaterialWrapper.h"
#include "Wrappers/Renderer/ShaderWrapper.h"


#include "Wrappers/SceneEntityNative.h"
#include "Wrappers/InputNative.h"



namespace XYZ {	

	void ScriptEngineRegistry::RegisterAll()
	{

		Script::InputNative::Register();

		// Components
		Script::SceneEntityNative::Register();
		Script::TransformComponentNative::Register();
		Script::SpriteRendererNative::Register();
		Script::RigidBody2DNative::Register();
		////////////////////////
	
		// Renderer
		Script::Texture2DNative::Register();
		Script::SubTextureNative::Register();
		Script::ShaderNative::Register();
		Script::MaterialNative::Register();
		////////////////////////
	}
}