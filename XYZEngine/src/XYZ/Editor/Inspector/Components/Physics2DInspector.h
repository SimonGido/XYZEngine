#pragma once
#include "XYZ/Editor/Inspector/InspectorEditable.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {

	class RigidBody2DInspector : public InspectorEditable
	{
	public:
		RigidBody2DInspector();

		virtual bool OnEditorRender() override;
		virtual void SetSceneEntity(const SceneEntity& entity) override;

		virtual Type GetType() const override { return InspectorEditable::Type::Entity; }
	private:
		SceneEntity m_Context;
	};

	class BoxCollider2DInspector : public InspectorEditable
	{
	public:
		BoxCollider2DInspector();

		virtual bool OnEditorRender() override;
		virtual void SetSceneEntity(const SceneEntity& entity) override;

		virtual Type GetType() const override { return InspectorEditable::Type::Entity; }
	private:
		SceneEntity m_Context;
	};
	
	class CircleCollider2DInspector : public InspectorEditable
	{
	public:
		CircleCollider2DInspector();

		virtual bool OnEditorRender() override;
		virtual void SetSceneEntity(const SceneEntity& entity) override;

		virtual Type GetType() const override { return InspectorEditable::Type::Entity; }
	private:
		SceneEntity m_Context;
	};

	class ChainCollider2DInspector : public InspectorEditable
	{
	public:
		ChainCollider2DInspector();

		virtual bool OnEditorRender() override;
		virtual void SetSceneEntity(const SceneEntity& entity) override;


		virtual Type GetType() const override { return InspectorEditable::Type::Entity; }
	private:
		SceneEntity m_Context;
	};
}