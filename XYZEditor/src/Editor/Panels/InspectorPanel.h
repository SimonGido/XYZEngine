#pragma once
#include "Editor/EditorPanel.h"
#include "Editor/Inspectable/Components/ComponentInspectors.h"

#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Asset/Renderer/MaterialAsset.h"

namespace XYZ {
	namespace Editor {
		class InspectorPanel : public EditorPanel
		{
		public:
			InspectorPanel(std::string name);
			
			virtual void OnImGuiRender(bool &open) override;
			virtual void OnUpdate(Timestep ts) override;
			virtual bool OnEvent(Event& e) override;
			
			virtual void SetSceneContext(const Ref<Scene>& scene) override;

			template <typename T, typename ...Args>
			void RegisterEditable(const std::string& name, Args&& ...args);

			template <typename T>
			void RemoveEditable(const std::string& name);

			template <typename T>
			Ref<T> GetEditable(const std::string& name) const;
		private:
			void drawSelectedEntity();
			void drawSelectedAsset();

			void drawAddComponent();
			void selectEditablesInUse();
	
		private:
			enum class State { None, Entity, Asset };

			State m_State = State::None;

			SceneEntity m_SelectedEntity;
			Ref<Asset>  m_SelectedAsset;

			Ref<MaterialAsset>	m_DefaultMaterial;
			Ref<SubTexture>		m_DefaultSubTexture;
		private:
			std::unordered_map<std::string, Ref<Inspectable>> m_Editables;

			std::vector<WeakRef<Inspectable>> m_EditablesInUse;

			// Default editables for components
			CameraInspector		          m_CameraInspector;
			PointLight2DInspector         m_PointLight2DInspector;
			SpotLight2DInspector          m_SpotLight2DInspector;
			ParticleComponentInspector	  m_ParticleInspector;
			SceneTagInspector			  m_SceneTagInspector;
			ScriptComponentInspector	  m_ScriptComponentInspector;
			TransformInspector			  m_TransformInspector;
			SpriteRendererInspector		  m_SpriteRendererInspector;
			RigidBody2DInspector		  m_RigidBodyInspector;
			BoxCollider2DInspector		  m_BoxCollider2DInspector;
			CircleCollider2DInspector	  m_CircleCollider2DInspector;
			ChainCollider2DInspector	  m_ChainCollider2DInspector;
		};

		template<typename T, typename ...Args>
		inline void InspectorPanel::RegisterEditable(const std::string& name, Args && ...args)
		{
			XYZ_ASSERT(m_Editables.find(name) == m_Editables.end(), "Inspector Editable already exists");
			static_assert(std::is_base_of_v<Inspectable, T>, "Editable must inherit from Inspectable");
			m_Editables[name] = Ref<T>::Create(name, std::forward<Args>(args)...);
			selectEntityEditablesInUse();
		}
		template<typename T>
		inline void InspectorPanel::RemoveEditable(const std::string& name)
		{
			auto it = m_Editables.find(name);
			XYZ_ASSERT(it != m_Editables.end(), "Inspector Editable does not exist");
			m_Editables.erase(it);
			selectEntityEditablesInUse();
		}
		template<typename T>
		inline Ref<T> InspectorPanel::GetEditable(const std::string& name) const
		{
			auto it = m_Editables.find(name);
			XYZ_ASSERT(it != m_Editables.end(), "Inspector Editable does not exist");
			return it->second;
		}
	}
}