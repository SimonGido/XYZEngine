#pragma once
#include <tuple>

namespace XYZ {

	/**
	* @interface CommandI
	* pure virtual (interface) class.
	*/
	class CommandI
	{
	public:
		virtual void Execute() = 0;

	};

	/**
	* @class Command 
	* @brief represents command, stores void function and it's arguments.
	* @tparam[in] ...Args	Pack of the arguments of the function
	*/
	template <typename ...Args>
	class Command : public CommandI
	{
	public:
		/**
		* Construct a command which stores function and it's arguments,
		* Function arguments must be same as ...Args
		* @param[in] func	 Pointer to the void function
		* @arg[in]	 args	 Variadic parameter pack
		*/
		Command(void(*func)(Args...), Args ... args)
		{
			m_Func = func;
			m_Args = std::forward_as_tuple(args...);
		}

		/**
		* Call stored function with stored arguments
		*/
		virtual void Execute() override
		{
			std::apply(m_Func, m_Args);
		}

	private:
		void(*m_Func)(Args...);
		std::tuple<Args...> m_Args;
	};

	/**
	* class represents queue of the CommandI commands,
	* stores them in continuous block of memory.
	* Can loop through them and execute them
	*/
	class RenderCommandQueue
	{
	public:
		/**
		* Construct a command queue, with memory buffer of size 10mb
		*/
		RenderCommandQueue();

		/**
		* Destruct memory buffer
		*/
		~RenderCommandQueue();

		/**
		* Store command and it's size in the buffer
		* @param[in] cmd	Pointer to the CommandI
		* @param[in] size	Size of the command
		*/
		void Allocate(CommandI* cmd, unsigned int size);

		/**
		* Execute all the commands in the buffer
		*/
		void Execute();

		/**
		* Clear all data in the buffer
		*/
		void Clear();

	private:
		unsigned char* m_CommandBuffer;
		unsigned char* m_CommandBufferPtr;
		unsigned int m_CommandCount;
		unsigned int m_CommandSize;

		static constexpr unsigned int sc_MaxBufferSize = 10 * 1024 * 1024;
	};
}