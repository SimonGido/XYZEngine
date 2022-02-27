#pragma once
#include "Editor/Inspectable/Inspectable.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Asset/Renderer/MaterialAsset.h"

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
			void handleShader();
			void handleTextures();
			void handleTextureArrays();

		private:
			Ref<MaterialAsset> m_MaterialAsset;
		};
	}
}