#pragma once
#include <XYZ.h>

#include "../Panels/InspectorPanel.h"

namespace XYZ {
	class EntityInspectorLayout : public InspectorLayout
	{
	public:
		EntityInspectorLayout() = default;
		EntityInspectorLayout(Entity context);

		void SetContext(Entity context);
		virtual void OnInGuiRender() override;
	private:
		enum
		{
			FOLDER = InGuiRenderConfiguration::DOCKSPACE + 1,
			SPRITE,
			TEXTURE,
			MATERIAL,
			SHADER,
			LOGO
		};

		enum Axis
		{
			X,
			Y,
			Z,
			W,
			NUM_AXIS
		};

		const uint32_t m_InspectorID = 2;

		Entity m_Context;
		bool m_ScriptsOpen = false;
		bool m_AddComponentOpen = false;
		bool m_SceneTagModified = false;


		bool m_SceneTagOpen = false;
		bool m_TransformOpen = false;
		bool m_SpriteRendererOpen = false;
		bool m_NativeScriptOpen = false;



		glm::vec4 m_ColorPallete = { 0,1,1,1 };
		bool m_PickColor = false;
		bool m_SpriteModified = false;
		bool m_MaterialModified = false;

		float m_Color[NUM_AXIS];
		int m_ColorLengths[NUM_AXIS] = { 4,4,4,4 };
		int m_ColorSelected = -1;

		float m_Position[W];
		int m_PositionLengths[W] = { 4,4,4 };
		int m_PositionSelected = -1;

		float m_Rotation[W];
		int m_RotationLengths[W] = { 4,4,4 };
		int m_RotationSelected = -1;
		
		float m_Scale[W];
		int m_ScaleLengths[W] = { 4,4,4 };
		int m_ScaleSelected = -1;

		std::string m_NativeScriptObject;
		std::string m_Sprite;
		std::string m_Material;
	private:
		Ref<Material> m_DefaultMaterial;
		Ref<SubTexture2D> m_DefaultSubTexture;

	};

}