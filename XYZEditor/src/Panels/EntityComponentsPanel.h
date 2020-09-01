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
		bool m_ScriptsOpen = false;
		bool m_AddComponentOpen = false;

		bool m_SceneTagModified = false;

		bool m_XPosModified = false;
		bool m_YPosModified = false;
		bool m_ZPosModified = false;

		bool m_XRotModified = false;
		bool m_YRotModified = false;
		bool m_ZRotModified = false;

		bool m_XScaleModified = false;
		bool m_YScaleModified = false;
		bool m_ZScaleModified = false;

		bool m_SceneTagOpen = false;
		bool m_TransformOpen = false;
		bool m_SpriteRendererOpen = false;
		bool m_NativeScriptOpen = false;
		

		glm::vec4 m_ColorPallete = { 0,1,1,1 };
		bool m_PickColor = false;
		bool m_RColorModified = false;
		bool m_GColorModified = false;
		bool m_BColorModified = false;
		bool m_AColorModified = false;

		std::string m_XPos;
		std::string m_YPos;
		std::string m_ZPos;

		std::string m_XRot;
		std::string m_YRot;
		std::string m_ZRot;

		std::string m_XScale;
		std::string m_YScale;
		std::string m_ZScale;

		std::string m_RColor;
		std::string m_GColor;
		std::string m_BColor;
		std::string m_AColor;


		std::string m_NativeScriptObject;
	private:
		Ref<Material> m_DefaultMaterial;
		Ref<SubTexture2D> m_DefaultSubTexture;

	};

}