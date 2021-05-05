#include "stdafx.h"
#include "Renderer2DTest.h"


namespace XYZ {

	struct Renderer2DData
	{
		static const uint32_t MaxTextures = 32;
		static const uint32_t QuadBufferSize = 1024 * 10 * 10;
		static const uint32_t LineBufferSize = 1024 * 10 * 10;

		void ResetQuads();
		void ResetLines();

		Ref<Texture> TextureSlots[MaxTextures];
		uint32_t	 TextureSlotIndex = 0;

		uint32_t QuadIndexCount = 0;
		uint8_t* QuadBufferBase = nullptr;
		uint8_t* QuadBufferPtr = nullptr;


		uint32_t LineIndexCount = 0;
		uint8_t* LineBufferBase = nullptr;
		uint8_t* LineBufferPtr = nullptr;


		struct CameraData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;
		Ref<Renderer2DLayout> Layout;
	};

	void Renderer2DData::ResetQuads()
	{
		if (!QuadBufferBase)
		{
			QuadBufferBase = new uint8_t[QuadBufferSize];
		}
		QuadIndexCount = 0;
		QuadBufferPtr = QuadBufferBase;
	}

	void Renderer2DData::ResetLines()
	{
		if (!LineBufferBase)
		{
			LineBufferBase = new uint8_t[LineBufferSize];
		}
		LineIndexCount = 0;
		LineBufferPtr = LineBufferBase;
	}

	static Renderer2DData s_Data;

	void Renderer2DTest::Init()
	{
		s_Data.ResetQuads();
		s_Data.ResetLines();
		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);
	}
	void Renderer2DTest::Shutdown()
	{
		delete[]s_Data.QuadBufferBase;
		delete[]s_Data.LineBufferBase;
	}
	void Renderer2DTest::BeginScene(const glm::mat4& viewProjectionMatrix, const Ref<Renderer2DLayout>& layout)
	{
		XYZ_ASSERT(s_Data.Layout.Raw(), "Layout is still active, forgot to call EndScene?");
		s_Data.CameraBuffer.ViewProjectionMatrix = viewProjectionMatrix;
		s_Data.CameraUniformBuffer->Update(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData), 0);
	}
	void Renderer2DTest::Flush()
	{
	}
}