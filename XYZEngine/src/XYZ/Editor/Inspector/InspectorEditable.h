#pragma once
#include "XYZ/Renderer/Renderer2D.h"

namespace XYZ {

	class InspectorEditable : public RefCount
	{
	public:
		enum class Type { None, Entity, Asset };

		InspectorEditable(std::string name)
			: m_Name(name)
		{}
		virtual ~InspectorEditable() = default;

		virtual bool OnEditorRender() = 0;
		virtual void SetSceneEntity(const SceneEntity& entity) {};
		virtual void SetAsset(const Ref<Asset>& asset) {};

		virtual Type GetType() const { return Type::None; }

		const std::string& GetName() const { return m_Name; }
	private:
		std::string m_Name;
	};

}