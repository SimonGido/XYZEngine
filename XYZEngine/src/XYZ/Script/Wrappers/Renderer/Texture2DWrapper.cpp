#include "stdafx.h"
#include "Texture2DWrapper.h"
#include "XYZ/Scene/Prefab.h"


#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace XYZ {
	namespace Script {


		void Texture2DNative::Register()
		{
			mono_add_internal_call("XYZ.Texture2D::Constructor_Native", Constructor);
			mono_add_internal_call("XYZ.Texture2D::ConstructorPath_Native", Constructor_Path);
			mono_add_internal_call("XYZ.Texture2D::Destructor_Native", Destructor);
			mono_add_internal_call("XYZ.Texture2D::SetData_Native", SetData);
		}
		Ref<Texture2D>* Texture2DNative::Constructor(uint32_t width, uint32_t height)
		{
			Ref<Texture2D> texture = Texture2D::Create(ImageFormat::RGBA, width, height);
			return new Ref<Texture2D>(texture);
		}
		Ref<Texture2D>* Texture2DNative::Constructor_Path(MonoString* path)
		{
			Ref<Texture2D> texture = Texture2D::Create(mono_string_to_utf8(path));
			return new Ref<Texture2D>(texture);
		}
		void Texture2DNative::Destructor(Ref<Texture2D>* instance)
		{
			delete instance;
		}
		void Texture2DNative::SetData(Ref<Texture2D>* instance, MonoArray* inData, int32_t count)
		{
			Ref<Texture2D>& texture = *instance;

			const uint32_t dataSize = count * sizeof(glm::vec4) / 4;

			texture->Lock();
			ByteBuffer buffer = texture->GetWriteableBuffer();
			XYZ_ASSERT(dataSize <= buffer.Size, "");
			// Convert RGBA32F color to RGBA8
			uint8_t* pixels = (uint8_t*)buffer.Data;
			uint32_t index = 0;
			for (uint32_t i = 0; i < texture->GetWidth() * texture->GetHeight(); i++)
			{
				glm::vec4& value = mono_array_get(inData, glm::vec4, i);
				*pixels++ = (uint32_t)(value.x * 255.0f);
				*pixels++ = (uint32_t)(value.y * 255.0f);
				*pixels++ = (uint32_t)(value.z * 255.0f);
				*pixels++ = (uint32_t)(value.w * 255.0f);
			}
			// TODO: invalidate or something
			texture->Unlock();
		}
	}
}