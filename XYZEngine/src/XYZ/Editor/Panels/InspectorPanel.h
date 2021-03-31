#pragma once

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/IG/IG.h"

namespace XYZ {

	class InspectorPanel
	{
	public:
		InspectorPanel();
		void SetContext(SceneEntity context);

		void OnUpdate();

	private:
		void updateTransformComponentUI();
		void updateSpriteRendererUI();

		size_t transformComponentUI(IGHierarchyElement& parent);
		size_t spriteRendererUI(IGHierarchyElement& parent);
		size_t pointLight2DUI(IGHierarchyElement& parent);
		size_t rigidBody2DComponentUI(IGHierarchyElement& parent);
		size_t boxCollider2DComponentUI(IGHierarchyElement& parent);
		size_t scriptComponentUI(IGHierarchyElement& parent);

		void prepareTransformComponentUI();
		void prepareSpriteRendererUI();

		void setupTransformComponentUI();
		void setupSpriteRendererUI();
		void setupPointLight2DUI();
		void setupRigidBody2DComponentUI();
		void setupBoxCollider2DComponentUI();
		void setupScriptComponentUI();
	private:
		SceneEntity m_Context;

		enum ComponentType
		{
			TransformComponent,
			SpriteRenderer,
			PointLight2D,
			RigidBody2D,
			BoxCollider2D,
			ScriptComponent,
			NumComponents
		};

		size_t* m_Handles;
		size_t m_HandleCount;
		size_t m_PoolHandle;
		size_t m_HandleStart[NumComponents];
	};
}