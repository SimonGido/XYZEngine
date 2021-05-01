#pragma once
#include "XYZ/BasicUI/BasicUI.h"
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
			~SkinningEditor();

			void SetContext(Ref<SubTexture> context);
			void OnUpdate(Timestep ts);
			void OnEvent(Event& event);

			static constexpr size_t sc_MaxBones = 60;
		private:
			void save();

			void setupUI();
			void setupBoneUI();
			void setupVertexUI();
			void setupWeightsUI();
			void updateLayout(bUIAllocator& allocator);
			

			// Events
			bool onMouseButtonPress(MouseButtonPressEvent& event);
			bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
			bool onMouseScroll(MouseScrollEvent& event);
			bool onKeyPress(KeyPressedEvent& event);

			// Rendering
			void renderAll();
			void renderSelection();
			void renderPreviewMesh();
	
			// Buffers and mesh data
			void clear();
			void eraseBone(PreviewBone* bone);
			void createBone();
			void createVertex(const glm::vec2& pos);
			void initializePose();
			void updateBoneHierarchy();
			void updateRenderBuffers();
			void rebuildRenderBuffers();

			void handleSelection();
			void handleBoneEdit();
			void handleVertexEdit();
			void handleWeightsBrush();

			
			// Helper functions
			glm::vec2 getMouseWindowSpace() const;
			std::pair<float, float> getMouseViewportSpace() const;
			uint8_t findEditBoneFlag(const PreviewBone& bone);
			std::vector<BoneVertex*> findVerticesInRadius(const glm::vec2& pos, float radius);

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
				// Bones
				PreviewPose    = BIT(0),
				CreateBone     = BIT(1),
				EditBone       = BIT(2),
				DeleteBone     = BIT(3),

				// Vertices / Triangles
				CreateVertex   = BIT(5),
				EditVertex     = BIT(6),
				DeleteVertex   = BIT(7),
				DeleteTriangle = BIT(8),

				// Weights
				WeightBrush    = BIT(9)
			};

		private:
			bUIWindow* m_Window;
			bUIWindow* m_PreviewWindow;
			bUIImage* m_Image;
			bUITree* m_Tree;
			bUILayout m_Layout;

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
			static constexpr float sc_WeightBrushDivisor = 100.0f;

			glm::vec4 m_HighlightColor;
			glm::vec4 m_UIHighlightColor;
			uint32_t m_ColorIDs[sc_MaxBones];
			uint16_t m_Flags;
			uint8_t m_BoneEditFlags;

			bool m_CategoriesOpen[Categories::NumCategories];
			bool m_Triangulated = false;			
		};
	}
}