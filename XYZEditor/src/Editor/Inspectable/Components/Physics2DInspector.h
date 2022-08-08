#pragma once
#include "Editor/Inspectable/Inspectable.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	namespace Editor {
		class RigidBody2DInspector : public Inspectable
		{
		public:
			RigidBody2DInspector();

			virtual bool OnEditorRender() override;
			virtual void SetSceneEntity(const SceneEntity& entity) override;

			virtual Type GetType() const override { return Inspectable::Type::Entity; }
		private:
			SceneEntity m_Context;
		};

		class BoxCollider2DInspector : public Inspectable
		{
		public:
			BoxCollider2DInspector();

			virtual bool OnEditorRender() override;
			virtual void SetSceneEntity(const SceneEntity& entity) override;

			virtual Type GetType() const override { return Inspectable::Type::Entity; }
		private:
			SceneEntity m_Context;
		};

		class CircleCollider2DInspector : public Inspectable
		{
		public:
			CircleCollider2DInspector();

			virtual bool OnEditorRender() override;
			virtual void SetSceneEntity(const SceneEntity& entity) override;

			virtual Type GetType() const override { return Inspectable::Type::Entity; }
		private:
			SceneEntity m_Context;
		};

		class ChainCollider2DInspector : public Inspectable
		{
		public:
			ChainCollider2DInspector();

			virtual bool OnEditorRender() override;
			virtual void SetSceneEntity(const SceneEntity& entity) override;


			virtual Type GetType() const override { return Inspectable::Type::Entity; }
		private:
			static constexpr size_t sc_InvalidIndex = SIZE_MAX;

			SceneEntity m_Context;

			size_t	    m_SelectedPointIndex = sc_InvalidIndex;

			static constexpr float sc_VSpeed = 0.01f;
		};
	}
}