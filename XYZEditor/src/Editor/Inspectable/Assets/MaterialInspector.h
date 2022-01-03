#pragma once
#include "Editor/Inspectable/Inspectable.h"
#include "XYZ/Renderer/Material.h"

namespace XYZ {
	namespace Editor {
		class MaterialInspector : public Inspectable
		{
		public:
			MaterialInspector();

			virtual bool OnEditorRender() override;
			virtual void SetAsset(const Ref<Asset>& asset) override;

			virtual Type GetType() const { return Type::Asset; }
		private:
			Ref<Material> m_MaterialAsset;
		};
	}
}