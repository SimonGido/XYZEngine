#pragma once
#include "XYZ/IG/IG.h"
#include "XYZ/Renderer/Framebuffer.h"
#include "XYZ/Renderer/RenderTexture.h"
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Core/Timestep.h"

#include "XYZ/Editor/EditorOrthographicCamera.h"
#include "SkinnedMesh.h"
#include "PreviewBone.h"

#include <glm/glm.hpp>

namespace XYZ {
	namespace Editor {
		class SkinningEditor
		{
		public:		
			SkinningEditor();
			void SetContext(Ref<SubTexture> context);
			void OnUpdate(Timestep ts);
			void OnEvent(Event& event);


			void SetupUI(std::vector<IGHierarchyElement>& elements);

			static constexpr size_t MaxBones = 60;
		private:
			// Events
			bool onMouseButtonPress(MouseButtonPressEvent& event);
			bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
			bool onMouseScroll(MouseScrollEvent& event);
			bool onKeyPress(KeyPressedEvent& event);

			// Rendering
			void renderAll();
			void renderPreviewMesh(const glm::mat4& viewProjection);

			// Buffers and mesh data
			void clear();
			void eraseBone(PreviewBone* bone);
			void createVertex(const glm::vec2& pos);
			void initializePose();
			void updateBoneHierarchy();
			void updateRenderBuffers();
			void rebuildRenderBuffers();

			void handleBoneCreate();
			void handleBoneEdit();
			void handleVertexEdit();
			void handleWeightsBrush();

			// Helper functions
			glm::vec2 getMouseWindowSpace() const;
			std::pair<float, float> getMouseViewportSpace() const;

			std::pair<Submesh*, BoneVertex*> findVertex(const glm::vec2& pos);
			std::pair<Submesh*, Triangle*> findTriangle(const glm::vec2& pos);
			PreviewBone* findBone(const glm::vec2& pos) const;
		private:
			enum Categories
			{
				Bones,
				Geometry,
				Weights,
				NumCategories
			};

			enum Flags
			{
				PreviewPose = BIT(0),
				CreateBone = BIT(1),
				EditBone = BIT(2),
				DeleteBone = BIT(3),

				CreateSubMesh = BIT(4),
				CreateVertex = BIT(5),
				EditVertex = BIT(6),
				DeleteVertex = BIT(7),
				DeleteTriangle = BIT(8),

				WeightBrush = BIT(9)
			};
		private:
			size_t m_PoolHandle;
			IGImageWindow* m_Window;
			IGTree* m_Tree;

			Tree m_BoneHierarchy;
			MemoryPool m_BonePool;
			EditorOrthographicCamera m_Camera;

			Submesh* m_SelectedSubmesh;
			PreviewBone* m_SelectedBone;
			BoneVertex* m_SelectedVertex;
			Triangle* m_SelectedTriangle;

			Submesh* m_FoundSubmesh;
			PreviewBone* m_FoundBone;
			BoneVertex* m_FoundVertex;
			Triangle* m_FoundTriangle;

			Ref<SubTexture> m_Context;
			glm::vec2 m_ContextSize;

			glm::vec2 m_ViewportSize;
			glm::vec2 m_MousePosition;
			Ref<Framebuffer> m_Framebuffer;
			Ref<RenderTexture> m_RenderTexture;
			Ref<SubTexture> m_RenderSubTexture;

			SkinnedMesh m_Mesh;
			std::vector<PreviewBone*> m_Bones;

			Ref<VertexArray> m_VertexArray;
			Ref<VertexBuffer> m_VertexBuffer;
			Ref<Shader> m_Shader;

			float m_WeightBrushRadius;
			float m_WeightBrushStrength;

			uint32_t m_ColorIDs[MaxBones];
			uint16_t m_Flags;

			bool m_CategoriesOpen[Categories::NumCategories];
			bool m_Triangulated = false;
		};
	}
}