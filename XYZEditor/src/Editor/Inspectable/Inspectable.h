#pragma once
#include "XYZ/Core/Ref/Ref.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	namespace Editor {
		class Inspectable : public RefCount
		{
		public:
			enum class Type { None, Entity, Asset };

			Inspectable(std::string name)
				: m_Name(name)
			{}
			virtual ~Inspectable() = default;

			virtual bool OnEditorRender() = 0;
			virtual void SetSceneEntity(const SceneEntity& entity) {};
			virtual void SetAsset(const Ref<Asset>& asset) {};

			virtual Type GetType() const { return Type::None; }

			const std::string& GetName() const { return m_Name; }
		private:
			std::string m_Name;
		};
	}
}