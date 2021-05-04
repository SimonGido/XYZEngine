#pragma once
#include "Material.h"
#include "VertexArray.h"
#include "Buffer.h"
#include "Shader.h"

namespace XYZ {

	class Pipeline : public RefCount
	{
	public:
		Ref<VertexArray>  QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Material>	  QuadMaterial;

		Ref<VertexArray>  LineVertexArray;
		Ref<VertexBuffer> LineVertexBuffer;
		Ref<Shader>		  LineShader;
	};

	class GenericRenderer
	{
	public:
		static void Init();
		static void Shutdown();

		template <typename ...Args>
		static void SubmitQuad(const glm::mat4& transform, Args&& ...args)
		{

		}

		template <typename ...Args>
		static void SubmitLine(const glm::vec3& p0, const glm::vec3& p1, Args&& ...args)
		{

		}

	private:
		
	};

}