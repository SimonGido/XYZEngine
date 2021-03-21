#pragma once
#include "IGRenderer.h"


namespace XYZ {

	struct IGDockNode;
	class IGElement;
	struct IGMeshFactoryData
	{
		Ref<SubTexture>	  SubTexture;
		IGElement*		  Element;
		IGMesh*			  Mesh;
		IGRenderData*	  RenderData;
		uint32_t		  ScissorIndex = 0;
	};

	struct IGQuadData
	{
		glm::vec2		  Position;
		glm::vec2		  Size;
		glm::vec4		  Color;
		Ref<SubTexture>	  SubTexture;
		IGMesh*			  Mesh;
		uint32_t		  TextureID;
		uint32_t		  ScissorIndex;
	};

	class IGMeshFactory
	{
	public:
		template <typename T, typename ...Args>
		static glm::vec2 GenerateUI(const char* label, const glm::vec4& labelColor, const IGMeshFactoryData& data);
	
		static void GenerateDockNodeQuads(IGDockNode& node, const IGQuadData& data);
		static void GenerateQuad(const IGQuadData& data);
	};

}
