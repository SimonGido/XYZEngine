#pragma once
#include "XYZ/Event/Event.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {

	enum class EditorEventType
	{
		None = 0,
		EntitySelected,
		AssetSelected,
		SceneLoaded
	};

	class EditorEvent : public Event
	{
	public:
		virtual EventType GetEventType() const override { return EventType::Editor; }
		virtual EditorEventType GetEditorEventType() const = 0;

		static EventType GetStaticType() { return EventType::Editor; }

		bool Handled = false;
	};

	class EntitySelectedEvent : public EditorEvent
	{
	public:
		EntitySelectedEvent(const SceneEntity& entity)
			: m_Entity(entity)
		{}
		virtual EditorEventType GetEditorEventType() const override { return GetStaticType(); }

		const SceneEntity& GetEntity() const { return m_Entity; }

		static EditorEventType GetStaticType() { return EditorEventType::EntitySelected; }
	private:
		SceneEntity m_Entity;
	};

	class AssetSelectedEvent : public EditorEvent
	{
	public:
		AssetSelectedEvent(Ref<Asset> asset)
			: m_Asset(asset)
		{}

		virtual EditorEventType GetEditorEventType() const override { return EditorEventType::AssetSelected; }
		
		const Ref<Asset>& GetAsset() const { return m_Asset; }

		static EditorEventType GetStaticType() { return EditorEventType::AssetSelected; }

	private:
		Ref<Asset> m_Asset;
	};

	class SceneLoadedEvent : public EditorEvent
	{
	public:
		SceneLoadedEvent(Ref<Scene> scene)
			: m_Scene(scene)
		{}

		virtual EditorEventType GetEditorEventType() const override { return EditorEventType::SceneLoaded; }

		const Ref<Scene>& GetScene() const { return m_Scene; }

		static EditorEventType GetStaticType() { return EditorEventType::SceneLoaded; }

	private:
		Ref<Scene> m_Scene;
	};
}