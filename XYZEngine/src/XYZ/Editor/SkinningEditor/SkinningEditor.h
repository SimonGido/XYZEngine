#pragma once
#include "XYZ/Editor/EditorOrthographicCamera.h"

#include "XYZ/Renderer/RenderPass.h"
#include "XYZ/Renderer/RenderTexture.h"
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Shader.h"


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

			void OnImGuiRender();

			void SetContext(const Ref<SubTexture>& context);
		private:
			Ref<Shader> m_Shader;
			Ref<RenderPass> m_Pass;
			Ref<VertexArray> m_VertexArray;
			Ref<VertexBuffer> m_VertexBuffer;

			Ref<SubTexture> m_Context;
		};
	}
}
