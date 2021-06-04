#pragma once

extern "C" {

	typedef struct _MonoObject MonoObject;
	typedef struct _MonoClassField MonoClassField;
}

namespace XYZ {
	
	enum class PublicFieldType
	{
		None = 0, Float, Int, UnsignedInt, String, Vec2, Vec3, Vec4
	};

	struct PublicField
	{
		PublicField(const std::string& name, PublicFieldType type);
		PublicField(const PublicField& other);
		PublicField(PublicField&& other) noexcept;
		~PublicField();

		PublicField& operator = (const PublicField& other);
		
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
		char* GetStoredValue() const
		{
			char* value;
			getStoredString_Internal(&value);
			return value;
		}

		template<typename T>
		void SetStoredValue(T value) const
		{
			setStoredValue_Internal(&value);
		}

		template<>
		void SetStoredValue(const char* value) const
		{
			setStoredString_Internal(value);
		}

		template<typename T>
		T GetRuntimeValue() const
		{
			T value;
			getRuntimeValue_Internal(&value);
			return value;
		}

		template <>
		char* GetRuntimeValue() const
		{
			char* value;
			getRuntimeString_Internal(&value);
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
		uint8_t* allocateBuffer(uint32_t size) const;
		void setStoredValue_Internal(void* value) const;
		void getStoredValue_Internal(void* outValue) const;

		void setStoredString_Internal(const char* value) const;
		void getStoredString_Internal(char** outValue) const;
		
		void setRuntimeValue_Internal(void* value) const;
		void getRuntimeValue_Internal(void* outValue) const;
		
		void setRuntimeString_Internal(const char* value) const;
		void getRuntimeString_Internal(char** outValue) const;

	private:
		MonoClassField* m_MonoClassField;
		mutable uint8_t* m_StoredValueBuffer = nullptr;
		mutable uint32_t m_Size;
		uint32_t m_Handle;
		
		std::string m_Name;
		PublicFieldType m_Type;

		friend class ScriptEngine;
	};

	using ScriptModuleFieldMap = std::unordered_map<std::string, std::vector<PublicField>>;
}