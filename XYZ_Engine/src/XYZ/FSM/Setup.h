#pragma once


namespace XYZ {
	template <typename... Handlers>
	struct Setup : Handlers...
	{
		using Handlers::Handle...;
	};


	template <typename Event, typename Action>
	struct On
	{
		Action Handle(const Event&) const
		{
			return {};
		}
	};

	template <typename Action>
	struct Default
	{
		template <typename Event>
		Action Handle(const Event&) const
		{
			return Action{};
		}
	};

	struct Nothing
	{
		template <typename Machine, typename State, typename Event>
		void Execute(Machine&, State&, const Event&)
		{
		}
	};
}