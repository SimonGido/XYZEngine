#pragma once
#include <XYZ.h>

#include "../Panels/InspectorPanel.h"

namespace XYZ {
	class InspectableEntity : public Inspectable
	{
	public:
		InspectableEntity();
		virtual void OnInGuiRender();
		virtual void OnUpdate(Timestep ts);
		virtual void OnEvent(Event& event);
		
		void SetContext(Entity context);
		void AttemptSetAsset(const std::string& filepath, AssetManager& assetManager);
		bool ValidExtension(const std::string& filepath);
	private:
		enum Axis
		{
			X,
			Y,
			Z,
			W,
			NUM_AXIS
		};
		enum Groups
		{
			SCENE_TAG_COMPONENT,
			TRANSFORM_COMPONENT,
			SPRITE_RENDERER_COMPONENT,
			NATIVE_SCRIPT_COMPONENT,
			CAMERA_COMPONENT,
			PARTICLE_COMPONENT,
			POINT_LIGHT,
			NUM_GROUPS
		};
		bool m_GroupOpen[NUM_GROUPS];

		Entity m_Context;
		bool m_ScriptsOpen = false;
		bool m_AddComponentOpen = false;
		bool m_SceneTagModified = false;
		bool m_TextureIDOpen = false;
		bool m_CameraTypeOpen = false;

		glm::vec4 m_ColorPallete = { 0,1,1,1 };
		bool m_PickColor = false;
		bool m_SpriteModified = false;
		bool m_MaterialModified = false;

		int32_t m_TextureIDLength = 3;
		int32_t m_TextureIDSelected = -1;

		int32_t m_SortLayerLength = 3;
		int32_t m_SortLayerSelected = -1;

		int32_t m_ColorLengths[NUM_AXIS] = { 7,7,7,7 };
		int32_t m_ColorSelected = -1;

		int32_t m_LightColorLengths[NUM_AXIS] = { 7,7,7,7 };
		int32_t m_LightColorSelected = -1;

		int32_t m_PositionLengths[W] = { 7,7,7 };
		int32_t m_PositionSelected = -1;

		int32_t m_RotationLengths[W] = { 7,7,7 };
		int32_t m_RotationSelected = -1;

		int32_t m_ScaleLengths[W] = { 7,7,7 };
		int32_t m_ScaleSelected = -1;

		int32_t m_SizeSelected = -1;
		int32_t m_FarPlaneSelected = -1;
		int32_t m_NearPlaneSelected = -1;

		int32_t m_CameraSizeLength = 7;
		int32_t m_CameraFarLength = 7;
		int32_t m_CameraNearLength = 7;

		uint8_t m_MaterialTextFlags = 0;
		uint8_t m_SpriteTextFlags = 0;

		
		std::vector<int32_t> m_MaterialUniformLengths;
		std::vector<int32_t> m_MaterialUniformSelected;

		std::string m_NativeScriptObject;
		std::string m_Sprite;
		std::string m_Material;

	private:
		Ref<Material> m_DefaultMaterial;
		Ref<SubTexture2D> m_DefaultSubTexture;

	};
}