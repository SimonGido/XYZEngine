#pragma once

#include "Camera.h"
#include "Material.h"
#include "Mesh.h"
#include "SkeletalMesh.h"
#include "Buffer.h"

namespace XYZ {
	
	class CustomRenderer2DLayout
	{
	public:
		CustomRenderer2DLayout();
		CustomRenderer2DLayout(
			const BufferLayout& quadLayout,
			const BufferLayout& lineLayout
		);
		CustomRenderer2DLayout(const CustomRenderer2DLayout& other);
		CustomRenderer2DLayout& operator =(const CustomRenderer2DLayout& other);

		void SetQuadBufferLayout(const BufferLayout& layout);
		void SetLineBufferLayout(const BufferLayout& layout);
	
	private:
		Ref<VertexArray>  m_QuadVertexArray;
		Ref<VertexBuffer> m_QuadVertexBuffer;
		uint32_t		  m_QuadLayoutSize;

		Ref<VertexArray>  m_LineVertexArray;
		Ref<VertexBuffer> m_LineVertexBuffer;
		uint32_t		  m_LineLayoutSize;

		friend class CustomRenderer2D;
	};

	struct CustomRenderer2DData
	{
		static constexpr uint32_t MaxTextures = 32;
		static constexpr uint32_t QuadBufferSize = 1024 * 1024;
		static constexpr uint32_t LineBufferSize = 1024 * 1024;
		struct Writer
		{
			template <typename T>
			Writer& operator << (const T& val)
			{
				size_t size = sizeof(T);
				memcpy((void*)Ptr, (void*)&val, size);
				Ptr += size;
				return *this;
			}
			uint8_t* Ptr = nullptr;
		};

		uint32_t MaxQuadIndices;
		uint32_t MaxLineIndices;

		void ResetQuads();
		void ResetLines();

		Ref<Material> QuadMaterial;
		Ref<Shader>   LineShader;
		Ref<Texture>  TextureSlots[MaxTextures];
		uint32_t	  TextureSlotIndex = 0;

		
		uint32_t QuadIndexCount = 0;
		uint8_t* QuadBufferBase = nullptr;
		Writer	 QuadBufferWriter;


		uint32_t LineIndexCount = 0;
		uint8_t* LineBufferBase = nullptr;
		Writer	 LineBufferWriter;

		glm::vec4 QuadVertexPositions[4] = {
			{ -0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f,  0.5f, 0.0f, 1.0f },
			{ -0.5f,  0.5f, 0.0f, 1.0f }
		};
	
		CustomRenderer2DLayout Layout;
	};

	class CustomRenderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void SetLayout(const CustomRenderer2DLayout& layout);
		
		static void SetMaterial(const Ref<Material>& material);
		static void SetLineShader(const Ref<Shader>& lineShader);
		static uint32_t SetTexture(const Ref<Texture>& texture);

		template <typename ...Args>
		static void SubmitQuad(const glm::mat4& transform, const glm::vec4& color, Args&& ...args);
		
		template <typename ...Args>
		static void SubmitQuad(const glm::mat4& transform, const glm::vec4& texCoord, const glm::vec4& color, Args&& ...args);

		template <typename ...Args>
		static void SubmitQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, const glm::vec4& color, Args&& ...args);

		template <typename ...Args>
		static void SubmitQuadNotCentered(const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, const glm::vec4& color, Args&& ... args);


		template <typename ...Args>
		static void SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, Args&& ...args);


		static void Flush();
		static void FlushLines();
		static void FlushLast();
		static void EndScene();

	private:
		static CustomRenderer2DData s_Data;
	};


	template<typename ...Args>
	inline void CustomRenderer2D::SubmitQuad(const glm::mat4& transform, const glm::vec4& color, Args && ...args)
	{
		constexpr size_t quadVertexCount = 4;
		if (s_Data.QuadIndexCount >= s_Data.MaxQuadIndices)
			Flush();

		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			s_Data.QuadBufferWriter << color;
			s_Data.QuadBufferWriter << transform * s_Data.QuadVertexPositions[i];
			(s_Data.QuadBufferWriter << ... << std::forward<Args>(args));
		}
		s_Data.QuadIndexCount += 6;
	}

	template<typename ...Args>
	inline void CustomRenderer2D::SubmitQuad(const glm::mat4& transform, const glm::vec4& texCoord, const glm::vec4& color, Args && ...args)
	{
		constexpr size_t quadVertexCount = 4;
		if (s_Data.QuadIndexCount >= s_Data.MaxQuadIndices)
			Flush();

		glm::vec2 texCoords[quadVertexCount] = {
			{texCoord.x,texCoord.y},
			{texCoord.z,texCoord.y},
			{texCoord.z,texCoord.w},
			{texCoord.x,texCoord.w}
		};
		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			s_Data.QuadBufferWriter << color;
			s_Data.QuadBufferWriter << transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadBufferWriter << texCoords[i];
			(s_Data.QuadBufferWriter << ... << std::forward<Args>(args));
		}
		s_Data.QuadIndexCount += 6;
	}

	template<typename ...Args>
	inline void CustomRenderer2D::SubmitQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, const glm::vec4& color, Args && ...args)
	{
		constexpr size_t quadVertexCount = 4;
		if (s_Data.QuadIndexCount >= s_Data.MaxQuadIndices)
			Flush();

		glm::vec2 texCoords[quadVertexCount] = {
			{texCoord.x,texCoord.y},
			{texCoord.z,texCoord.y},
			{texCoord.z,texCoord.w},
			{texCoord.x,texCoord.w}
		};
		glm::vec3 vertices[quadVertexCount] = {
			{  position.x - size.x / 2.0f,  position.y - size.y / 2.0f, 0.0f},
			{  position.x + size.x / 2.0f,  position.y - size.y / 2.0f, 0.0f},
			{  position.x + size.x / 2.0f,  position.y + size.y / 2.0f, 0.0f},
			{  position.x - size.x / 2.0f,  position.y + size.y / 2.0f, 0.0f}
		};
		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			s_Data.QuadBufferWriter << color;
			s_Data.QuadBufferWriter << vertices[i];
			s_Data.QuadBufferWriter << texCoords[i];
			(s_Data.QuadBufferWriter << ... << std::forward<Args>(args));
		}
		s_Data.QuadIndexCount += 6;
	}

	template<typename ...Args>
	inline void CustomRenderer2D::SubmitQuadNotCentered(const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, const glm::vec4& color, Args&& ... args)
	{
		constexpr size_t quadVertexCount = 4;
		if (s_Data.QuadIndexCount >= s_Data.MaxQuadIndices)
			Flush();

		glm::vec2 texCoords[quadVertexCount] = {
			{texCoord.x,texCoord.y},
			{texCoord.z,texCoord.y},
			{texCoord.z,texCoord.w},
			{texCoord.x,texCoord.w}
		};
		glm::vec3 vertices[quadVertexCount] = {
			{  position.x ,  position.y, 0.0f},
			{  position.x + size.x ,  position.y , 0.0f},
			{  position.x + size.x ,  position.y + size.y, 0.0f},
			{  position.x ,			  position.y + size.y , 0.0f}
		};

		uint32_t dataOffset = s_Data.QuadBufferWriter.Ptr - s_Data.QuadBufferBase;
		for (size_t i = 0; i < quadVertexCount; ++i)
		{			
			s_Data.QuadBufferWriter << color;
			s_Data.QuadBufferWriter << vertices[i];
			s_Data.QuadBufferWriter << texCoords[i];
			(s_Data.QuadBufferWriter << ... << std::forward<Args>(args));	
		}
		
		s_Data.QuadIndexCount += 6;
	}

	template <typename ...Args>
	inline void CustomRenderer2D::SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, Args&& ...args)
	{
		if (s_Data.LineIndexCount >= s_Data.MaxLineIndices)
			FlushLines();

		s_Data.LineBufferWriter << p0;
		s_Data.LineBufferWriter << color;
		(s_Data.LineBufferWriter << ... << args);

		s_Data.LineBufferWriter << p1;
		s_Data.LineBufferWriter << color;
		(s_Data.LineBufferWriter << ... << std::forward<Args>(args));

		s_Data.LineIndexCount += 2;
	}

}