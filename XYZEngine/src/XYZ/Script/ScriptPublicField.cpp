#include "stdafx.h"
#include "ScriptPublicField.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>


#include "ScriptEngine.h"

namespace XYZ {
	static MonoObject* GetInstance(uint32_t handle)
	{
		XYZ_ASSERT(handle, "Entity has not been instantiated!");
		return mono_gchandle_get_target(handle);
	}

	static uint32_t GetFieldSize(PublicFieldType type)
	{
		switch (type)
		{
		case PublicFieldType::Float:       return 4;
		case PublicFieldType::Int:         return 4;
		case PublicFieldType::UnsignedInt: return 4;
		case PublicFieldType::String:	   return 8;
		case PublicFieldType::Vec2:        return 4 * 2;
		case PublicFieldType::Vec3:        return 4 * 3;
		case PublicFieldType::Vec4:        return 4 * 4;
		}
		XYZ_ASSERT(false, "Unknown field type!");
		return 0;
	}


	PublicField::PublicField(const std::string& name, PublicFieldType type)
		:
		m_Name(name),
		m_Type(type)
	{
		m_StoredValueBuffer = allocateBuffer(GetFieldSize(type));
	}

	PublicField::PublicField(const PublicField& other)
		:
		m_Name(other.m_Name),
		m_Type(other.m_Type),
		m_Size(other.m_Size)
	{
		m_Handle = other.m_Handle;
		m_MonoClassField = other.m_MonoClassField;

		if (m_StoredValueBuffer && m_Type != PublicFieldType::String)
			delete[]m_StoredValueBuffer;

		m_StoredValueBuffer = allocateBuffer(other.m_Size);
		memcpy(m_StoredValueBuffer, other.m_StoredValueBuffer, other.m_Size);
	}

	PublicField::PublicField(PublicField&& other) noexcept
		:
		m_Name(std::move(other.m_Name)),
		m_Type(other.m_Type),
		m_Size(other.m_Size)
	{
		m_Handle = other.m_Handle;
		m_MonoClassField = other.m_MonoClassField;
		m_StoredValueBuffer = other.m_StoredValueBuffer;

		other.m_MonoClassField = nullptr;
		other.m_StoredValueBuffer = nullptr;
	}
	PublicField::~PublicField()
	{
		if (m_StoredValueBuffer && m_Type != PublicFieldType::String)
			delete[]m_StoredValueBuffer;
	}

	PublicField& PublicField::operator=(const PublicField& other)
	{
		if (m_StoredValueBuffer && m_Type != PublicFieldType::String)
			delete[]m_StoredValueBuffer;
		
		m_Name = other.m_Name;
		m_Type = other.m_Type;
		m_Handle = other.m_Handle;
		m_MonoClassField = other.m_MonoClassField;

		m_StoredValueBuffer = allocateBuffer(GetFieldSize(m_Type));
		memcpy(m_StoredValueBuffer, other.m_StoredValueBuffer, GetFieldSize(other.m_Type));
		return *this;
	}

	void PublicField::CopyStoredValueToRuntime() const
	{
		mono_field_set_value(GetInstance(m_Handle), m_MonoClassField, m_StoredValueBuffer);
	}

	void PublicField::StoreRuntimeValue()
	{
		if (m_Type != PublicFieldType::String)
			getRuntimeValue_Internal(m_StoredValueBuffer);
		else
		{
			delete[]m_StoredValueBuffer;
			getRuntimeString_Internal((char**)&m_StoredValueBuffer);
		}
	}

	void PublicField::SetStoredValueRaw(void* src)
	{
		uint32_t size = GetFieldSize(m_Type);
		memcpy(m_StoredValueBuffer, src, size);

	}
	uint8_t* PublicField::allocateBuffer(uint32_t size) const
	{
		m_Size = size;
		uint8_t* buffer = new uint8_t[m_Size];
		memset(buffer, 0, size);
		return buffer;
	}
	void PublicField::setStoredValue_Internal(void* value) const
	{
		uint32_t size = GetFieldSize(m_Type);
		memcpy(m_StoredValueBuffer, value, size);
	}
	void PublicField::getStoredValue_Internal(void* outValue) const
	{
		uint32_t size = GetFieldSize(m_Type);
		memcpy(outValue, m_StoredValueBuffer, size);
	}
	void PublicField::setStoredString_Internal(const char* value) const
	{
		size_t size = strlen(value) + 1;
		if (m_Size < size)
		{
			delete[]m_StoredValueBuffer;
			m_StoredValueBuffer = allocateBuffer(size);
		}
		memcpy(m_StoredValueBuffer, value, size);
	}
	void PublicField::getStoredString_Internal(char** outValue) const
	{
		size_t size = strlen((char*)m_StoredValueBuffer) + 1;
		*outValue = new char[size];
		memcpy(*outValue, m_StoredValueBuffer, size);
	}

	void PublicField::setRuntimeValue_Internal(void* value) const
	{
		auto instance = GetInstance(m_Handle);
		XYZ_ASSERT(instance, "");
		mono_field_set_value(instance, m_MonoClassField, value);

	}
	void PublicField::getRuntimeValue_Internal(void* outValue) const
	{
		auto instance = GetInstance(m_Handle);
		XYZ_ASSERT(instance, "");
		mono_field_get_value(instance, m_MonoClassField, outValue);
	}
	void PublicField::setRuntimeString_Internal(const char* value) const
	{
		MonoString* string = mono_string_new(ScriptEngine::GetMonoDomain(), value);
		auto instance = GetInstance(m_Handle);
		XYZ_ASSERT(instance, "");
		mono_field_set_value(instance, m_MonoClassField, string);
	}
	void PublicField::getRuntimeString_Internal(char** outValue) const
	{
		auto instance = GetInstance(m_Handle);
		XYZ_ASSERT(instance, "");
		MonoString* string = nullptr;
		mono_field_get_value(instance, m_MonoClassField, &string);
		*outValue = mono_string_to_utf8(string);
	}

}