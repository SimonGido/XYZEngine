#include "stdafx.h"
#include "ScriptPublicField.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>

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
			// case FieldType::String:   return 8; // TODO
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
		m_StoredValueBuffer = allocateBuffer(type);
	}

	PublicField::PublicField(const PublicField& other)
		:
		m_Name(other.m_Name),
		m_Type(other.m_Type)
	{
		m_Handle = other.m_Handle;
		m_MonoClassField = other.m_MonoClassField;

		if (m_StoredValueBuffer)
			delete m_StoredValueBuffer;

		m_StoredValueBuffer = allocateBuffer(m_Type);
		memcpy(m_StoredValueBuffer, other.m_StoredValueBuffer, GetFieldSize(other.m_Type));
	}

	PublicField::PublicField(PublicField&& other) noexcept
		:
		m_Name(std::move(other.m_Name)),
		m_Type(other.m_Type)
	{
		m_Handle = other.m_Handle;
		m_MonoClassField = other.m_MonoClassField;
		m_StoredValueBuffer = other.m_StoredValueBuffer;

		other.m_MonoClassField = nullptr;
		other.m_StoredValueBuffer = nullptr;
	}
	PublicField::~PublicField()
	{
		delete[] m_StoredValueBuffer;
	}

	PublicField& PublicField::operator=(const PublicField& other)
	{
		m_Name = other.m_Name;
		m_Type = other.m_Type;
		m_Handle = other.m_Handle;
		m_MonoClassField = other.m_MonoClassField;

		if (m_StoredValueBuffer)
			delete m_StoredValueBuffer;

		m_StoredValueBuffer = allocateBuffer(m_Type);
		memcpy(m_StoredValueBuffer, other.m_StoredValueBuffer, GetFieldSize(other.m_Type));
		return *this;
	}

	void PublicField::CopyStoredValueToRuntime()
	{
		mono_field_set_value(GetInstance(m_Handle), m_MonoClassField, m_StoredValueBuffer);
	}

	void PublicField::SetStoredValueRaw(void* src)
	{
		uint32_t size = GetFieldSize(m_Type);
		memcpy(m_StoredValueBuffer, src, size);

	}
	uint8_t* PublicField::allocateBuffer(PublicFieldType type)
	{
		uint32_t size = GetFieldSize(type);
		uint8_t* buffer = new uint8_t[size];
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
}