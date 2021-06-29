#pragma once
#include "XYZ/Renderer/Framebuffer.h"
#include "XYZ/Renderer/RenderTexture.h"
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/SubTexture.h"

#include "XYZ/Editor/EditorOrthographicCamera.h"

#include "SkinnedMesh.h"
#include "PreviewBone.h"

#include <glm/glm.hpp>


namespace XYZ {
	namespace Editor {
		class SkinningEditor
		{
		public:		
			SkinningEditor(const std::string& filepath);
			~SkinningEditor();

			void OnImGuiRender();

			void SetContext(const Ref<SubTexture>& context);
		private:

		};
	}
}
