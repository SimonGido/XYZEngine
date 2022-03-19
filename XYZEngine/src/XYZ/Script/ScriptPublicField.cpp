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



	PublicField::PublicField(const std::string& name, PublicFieldType type, 
		uint32_t offset, ByteBuffer* data, uint32_t instanceHandle, MonoClassField* monoClassField)
		:
		m_MonoClassField(monoClassField),
		m_Size(GetFieldSize(type)),
		m_Offset(offset),
		m_Data(data),
		m_InstanceHandle(instanceHandle),
		m_Name(name),
		m_Type(type)
	{
	}

	void PublicField::CopyStoredValueToRuntime() const
	{
		if (m_Type == PublicFieldType::String)
		{
			setRuntimeString_Internal(getStrData());
		}
		else
		{
			setRuntimeValue_Internal(getData());
		}
	}

	void PublicField::StoreRuntimeValue()
	{
		if (m_Type != PublicFieldType::String)
		{
			getRuntimeValue_Internal(getData());
		}
		else
		{
			getRuntimeString_Internal(getStrData());
		}
	}

	void PublicField::SetStoredValueRaw(void* src)
	{
		memcpy(getData(), src, m_Size);
	}
	
	void PublicField::setStoredValue_Internal(void* value) const
	{
		memcpy(getData(), value, m_Size);
	}
	void PublicField::getStoredValue_Internal(void* outValue) const
	{
		memcpy(outValue, getData(), m_Size);
	}

	void PublicField::setRuntimeValue_Internal(void* value) const
	{
		const auto instance = GetInstance(m_InstanceHandle);
		XYZ_ASSERT(instance, "");
		mono_field_set_value(instance, m_MonoClassField, value);

	}
	void PublicField::getRuntimeValue_Internal(void* outValue) const
	{
		const auto instance = GetInstance(m_InstanceHandle);
		XYZ_ASSERT(instance, "");
		mono_field_get_value(instance, m_MonoClassField, outValue);
	}
	void PublicField::setRuntimeString_Internal(const std::string& value) const
	{
		MonoString* string = mono_string_new(ScriptEngine::GetMonoDomain(), value.c_str());
		const auto instance = GetInstance(m_InstanceHandle);
		XYZ_ASSERT(instance, "");
		mono_field_set_value(instance, m_MonoClassField, string);
	}
	void PublicField::getRuntimeString_Internal(std::string& outValue) const
	{
		const auto instance = GetInstance(m_InstanceHandle);
		XYZ_ASSERT(instance, "");
		MonoString* string = nullptr;
	
		mono_field_get_value(instance, m_MonoClassField, &string);
		outValue = mono_string_to_utf8(string);
	}

	void* PublicField::getData() const
	{
		return &m_Data->Data[m_Offset];
	}

	std::string& PublicField::getStrData() const
	{
		return *(std::string*)getData();
	}


	void PublicFieldData::CreateBuffer()
	{
		const uint32_t size = m_Fields.back().m_Size + m_Fields.back().m_Offset;
		m_Data.Allocate(size);
		m_Data.ZeroInitialize();

		// We must construct strings before using them
		for (auto& field : m_Fields)
		{
			if (field.m_Type == PublicFieldType::String)
			{
				new (field.getData())std::string();
			}
		}
		for (auto& field : m_Fields)
			field.StoreRuntimeValue();
	}


	void PublicFieldData::AddField(const std::string& name, PublicFieldType type, uint32_t instanceHandle, MonoClassField* monoClassField)
	{
		uint32_t offset = 0;
		if (!m_Fields.empty())
		{
			auto& lastField = m_Fields.back();
			offset = lastField.m_Offset + lastField.m_Size;
		}
		m_Fields.emplace_back(name, type, offset, &m_Data, instanceHandle, monoClassField);
	}

	void PublicFieldData::Clear()
	{
		m_Fields.clear();
	}

}