#pragma once
#include "XYZ/Editor/Inspector/InspectorEditable.h"

namespace XYZ {
	namespace Editor {
		class MaterialInspector : public InspectorEditable
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