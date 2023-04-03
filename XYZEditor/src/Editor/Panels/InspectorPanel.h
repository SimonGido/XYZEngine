#pragma once
#include "Editor/EditorPanel.h"
#include "Editor/Inspectable/Components/ComponentInspectors.h"
#include "Editor/Inspectable/Assets/MaterialInspector.h"
#include "Editor/Inspectable/Assets/AnimationControllerInspector.h"
#include "Editor/Inspectable/Components/ComponentInspector.h"

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
	
			template <typename T, typename R = Inspectable>
			R* findComponentInspector();
			
			template <typename T>
			void activateInspector();

			template <typename ...Args>
			void activateInspectors();

		private:
			enum class State { None, Entity, Asset };

			State m_State = State::None;

			SceneEntity m_SelectedEntity;
			Ref<Asset>  m_SelectedAsset;

			Ref<MaterialAsset>	m_DefaultMaterial;
			Ref<SubTexture>		m_DefaultSubTexture;
		private:
			std::unordered_map<std::string, Ref<Inspectable>> m_Inspectables;

			std::vector<WeakRef<Inspectable>> m_InspectablesInUse;

			
			std::tuple<
				ComponentInspector<CameraComponent,				CameraInspector>,
				ComponentInspector<PointLightComponent2D,		PointLight2DInspector>,
				ComponentInspector<SpotLightComponent2D,		SpotLight2DInspector>,
				ComponentInspector<PointLightComponent3D,		PointLightComponent3DInspector>,
				ComponentInspector<DirectionalLightComponent,   DirectionalLightComponentInspector>,
				ComponentInspector<ParticleComponent,			ParticleComponentInspector>,
				ComponentInspector<ParticleRenderer,			ParticleRendererInspector>,
				ComponentInspector<SceneTagComponent,			SceneTagInspector>,
				ComponentInspector<ScriptComponent,				ScriptComponentInspector>,
				ComponentInspector<TransformComponent,			TransformInspector>,
				ComponentInspector<SpriteRenderer,				SpriteRendererInspector>,
				ComponentInspector<RigidBody2DComponent,		RigidBody2DInspector>,
				ComponentInspector<BoxCollider2DComponent,		BoxCollider2DInspector>,
				ComponentInspector<CircleCollider2DComponent,	CircleCollider2DInspector>,
				ComponentInspector<ChainCollider2DComponent,	ChainCollider2DInspector>,
				ComponentInspector<MeshComponent,				MeshComponentInspector>,
				ComponentInspector<AnimatedMeshComponent,		AnimatedMeshComponentInspector>,
				ComponentInspector<AnimationComponent,			AnimationComponentInspector>,
				ComponentInspector<ParticleComponentGPU,		ParticleComponentGPUInspector>
			> m_ComponentInspectors;

			MaterialInspector			  m_MaterialInspector;
			AnimationControllerInspector  m_AnimationControllerInspector;
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

		template<typename T, typename R>
		inline R* InspectorPanel::findComponentInspector()
		{
			R* result = nullptr;
			Utils::ForEach(m_ComponentInspectors, [&](auto& inspector) {
				if constexpr (inspector.IsComponentType<T>())
				{
					result = &inspector.Inspector;
				}
				});
			return result;
		}
		template<typename T>
		inline void InspectorPanel::activateInspector()
		{
			if (m_SelectedEntity.HasComponent<T>())
			{
				Inspectable* inspector = findComponentInspector<T, Inspectable>();
				if (inspector != nullptr)
				{
					inspector->SetSceneEntity(m_SelectedEntity);
					m_InspectablesInUse.push_back(inspector);
				}
			}
		}
		template<typename ...Args>
		inline void InspectorPanel::activateInspectors()
		{
			(activateInspector<Args>(), ...);
		}
	}
}