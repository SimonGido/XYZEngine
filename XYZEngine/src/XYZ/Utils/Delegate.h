#pragma once
#include <stdint.h>
#include <functional>
// This class is inspired by entt delegate class

namespace XYZ {

	template<typename>
	class Delegate;

	template<typename Ret, typename... Args>
	class Delegate<Ret(Args...)>
	{
		using FuncType = Ret(const void*, Args...);
	public:
		Delegate()
			: m_Func(nullptr), m_Data(nullptr)
		{}

		template <auto Callable>
		void Connect()
		{
			m_Func = [](const void*, Args... args) -> Ret {
				return Ret(std::invoke(Callable, std::forward<Args>(args)...));
			};
		}

		template <auto Callable, typename Type>
		void Connect(Type* instance)
		{
			m_Data = instance;
			m_Func = [](const void* payload, Args... args) -> Ret {
				Type* curr = static_cast<Type*>(const_cast<std::conditional_t<std::is_const_v<Type>, const void*, void*>>(payload));
				return Ret(std::invoke(Callable, *curr, std::forward<Args>(args)...));
			};
		}

		Ret operator()(Args... args) const 
		{
			return m_Func(m_Data, std::forward<Args>(args)...);
		}

		bool operator==(const Delegate<Ret(Args...)>& other) const
		{
			return m_Func == other.m_Func && m_Data == other.m_Data;
		}
		const void* GetData() const { return m_Data; }
	private:
		FuncType* m_Func;
		const void* m_Data;
	};

}