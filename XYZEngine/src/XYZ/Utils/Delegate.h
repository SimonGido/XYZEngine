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


	template <typename Ret, typename... Args>
	class DelegateManager
	{
	public:
		template <auto Callable>
		void Add();

		template <auto Callable>
		void Remove();

		template <auto Callable, typename Type>
		void Add(Type* instance);

		template <auto Callable, typename Type>
		void Remove(Type* instance);

		void Execute(Args&& ...args);

	private:
		template <typename T>
		static void eraseFromVector(std::vector<T>& vec, const T& value);
		
	private:
		std::vector<Delegate<Ret(Args...)>> m_Delegates;
	};

	template<typename Ret, typename ...Args>
	template<auto Callable>
	inline void DelegateManager<Ret, Args...>::Add()
	{
		Delegate<Ref(Args...)> deleg;
		deleg.Connect<Callable>();
		m_Delegates.push_back(deleg);
	}

	template<typename Ret, typename ...Args>
	template<auto Callable>
	inline void DelegateManager<Ret, Args...>::Remove()
	{
		Delegate<Ref(Args...)> deleg;
		deleg.Connect<Callable>();
		eraseFromVector(m_Delegates, deleg);
	}

	template<typename Ret, typename ...Args>
	template<auto Callable, typename Type>
	inline void DelegateManager<Ret, Args...>::Add(Type* instance)
	{
		Delegate<Ref(Args...)> deleg;
		deleg.Connect<Callable>(instance);
		m_Delegates.push_back(deleg);
	}

	template<typename Ret, typename ...Args>
	template<auto Callable, typename Type>
	inline void DelegateManager<Ret, Args...>::Remove(Type* instance)
	{
		Delegate<Ref(Args...)> deleg;
		deleg.Connect<Callable>(instance);
		eraseFromVector(m_Delegates, deleg);
	}

	template<typename Ret, typename ...Args>
	inline void DelegateManager<Ret, Args...>::Execute(Args && ...args)
	{
		for (auto& deleg : m_Delegates)
			deleg(std::forward<Args>(args)...);
	}

	template<typename Ret, typename ...Args>
	template <typename T>
	inline void DelegateManager<Ret, Args...>::eraseFromVector(std::vector<T>& vec, const T& value)
	{
		for (auto it = vec.begin(); it != vec.end();)
		{
			if ((*it) == value)
			{
				it = vec.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

}