#pragma once

#include "XYZ/Utils/DataStructures/ByteBuffer.h"

extern "C" {

	typedef struct _MonoObject MonoObject;
	typedef struct _MonoClassField MonoClassField;
}

namespace XYZ {
	
	enum class PublicFieldType
	{
		None = 0,
		Float, 
		Int, 
		UnsignedInt, 
		String, 
		Vec2, 
		Vec3, 
		Vec4
	};

	struct PublicField
	{
		PublicField(const std::string& name, PublicFieldType type, 
			uint32_t offset, ByteBuffer* data, uint32_t instanceHandle, MonoClassField* monoClassField);


		void CopyStoredValueToRuntime() const;
		void StoreRuntimeValue();

		template<typename T>
		T GetStoredValue() const
		{
			T value;
			getStoredValue_Internal(&value);
			return value;
		}

		template <>
		const std::string& GetStoredValue() const
		{
			return *(std::string*)&m_Data->Data[m_Offset];
		}

		template<typename T>
		void SetStoredValue(T value) const
		{
			setStoredValue_Internal(&value);
		}

		template<>
		void SetStoredValue(const std::string& value) const
		{
			(*(std::string*)&m_Data->Data[m_Offset]).assign(value);
		}

		template<typename T>
		T GetRuntimeValue() const
		{
			T value;
			getRuntimeValue_Internal(&value);
			return value;
		}

		template <>
		std::string GetRuntimeValue() const
		{
			std::string value;
			getRuntimeValue_Internal(&value);
			return value;
		}

		template<typename T>
		void SetRuntimeValue(T value) const
		{
			setRuntimeValue_Internal(&value);
		}

		template <>
		void SetRuntimeValue(std::string val) const
		{
			setRuntimeString_Internal(val.c_str());
		}

		void SetStoredValueRaw(void* src);

		const std::string& GetName() const { return m_Name; }
		PublicFieldType GetType() const { return m_Type; }
	private:
		void setStoredValue_Internal(void* value) const;
		void getStoredValue_Internal(void* outValue) const;
		
		void setRuntimeValue_Internal(void* value) const;
		void getRuntimeValue_Internal(void* outValue) const;
		
		void setRuntimeString_Internal(const std::string& value) const;
		void getRuntimeString_Internal(std::string& outValue) const;

		void* getData() const;
		std::string& getStrData() const;
	private:
		MonoClassField*  m_MonoClassField;

		uint32_t		 m_Size;
		uint32_t		 m_Offset;
		ByteBuffer*		 m_Data;


		uint32_t	     m_InstanceHandle;
		std::string		 m_Name;
		PublicFieldType  m_Type;

		friend class ScriptEngine;
		friend class PublicFieldData;
	};


	class PublicFieldData
	{
	public:
		void CreateBuffer();

		void AddField(const std::string& name, PublicFieldType type, uint32_t instanceHandle, MonoClassField* monoClassField);
		void Clear();

		const std::vector<PublicField>& GetFields() const { return m_Fields; }

		typename std::vector<PublicField>::const_iterator begin() const { return m_Fields.begin(); }
		typename std::vector<PublicField>::const_iterator end()   const { return m_Fields.end(); }
	private:
		std::vector<PublicField> m_Fields;
		ByteBuffer				 m_Data;
	};
}