#pragma once
#include <XYZ.h>

namespace XYZ {
	class EntityComponentPanel
	{
	public:
		EntityComponentPanel() = default;
		EntityComponentPanel(Entity context);

		void SetContext(Entity context);
		void OnInGuiRender();
	private:
		Entity m_Context;
		bool m_ChooseScript = false;
		bool m_ScriptsOpen = false;
		bool m_AddComponentOpen = false;

		bool m_SceneTagModified = false;
		bool m_NativeScriptModified = false;

		bool m_XPosModified = false;
		bool m_YPosModified = false;
		bool m_ZPosModified = false;

		bool m_XRotModified = false;
		bool m_YRotModified = false;
		bool m_ZRotModified = false;

		bool m_XScaleModified = false;
		bool m_YScaleModified = false;
		bool m_ZScaleModified = false;

		std::string m_XPos;
		std::string m_YPos;
		std::string m_ZPos;

		std::string m_XRot;
		std::string m_YRot;
		std::string m_ZRot;

		std::string m_XScale;
		std::string m_YScale;
		std::string m_ZScale;

	private:
		Ref<Material> m_DefaultMaterial;
		Ref<SubTexture2D> m_DefaultSubTexture;

	};

}