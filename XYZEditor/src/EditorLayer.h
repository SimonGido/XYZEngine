#pragma once

#include <XYZ.h>


namespace XYZ {

	class EditorLayer : public Layer
	{
	public:
		virtual ~EditorLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(float ts) override;
		virtual void OnEvent(Event& event) override;
		virtual void OnImGuiRender() override;


	private:
		Ref<OrthoCameraController> m_CameraController;
		Ref<RenderSortSystem> m_SortSystem;

		Entity m_TestObject;
		Entity m_TestChild;
		Entity m_TestChild2;

		Ref<Material> m_Material;
		Transform2D* m_WorldTransform;
		Transform2D* m_Transform;
		RenderComponent* m_Renderable;

		Transform2D* m_ChildTransform;
		RenderComponent* m_ChildRenderable;

		Transform2D* m_ChildTransform2;
		RenderComponent* m_ChildRenderable2;

		struct SceneObject
		{
			RenderComponent* Renderable;
			Transform2D* Transform;
		};
		struct SceneSetup
		{
			void operator()(SceneObject& parent, SceneObject& child)
			{
				child.Transform->SetParent(parent.Transform);
			}
		};

		Tree<SceneObject> m_SceneGraph;

		Ref<Font> m_Font;

		Entity m_Text;
		RenderComponent* m_TextRenderable;
		Transform2D* m_TextTransform;
		Ref<Material> m_TextMaterial;


		Button m_ButtonTest;


		Entity m_Button;
		RenderComponent* m_ButtonRenderable;
		Button* m_ButtonComponent;
		Transform2D* m_ButtonTransform;

		Ref<GuiSystem> m_GuiSystem;
	};
}