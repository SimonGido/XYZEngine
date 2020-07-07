#pragma once


namespace XYZ {

    template <typename TargetState>
    class TransitionTo
    {
    public:
        template <typename Machine, typename State, typename Event>
        bool Execute(Machine& machine, State& prevState, const Event& event)
        {
            leave(prevState, event);
            TargetState& newState = machine.template TransitionTo<TargetState>();
            enter(newState, event);
            return true;
        }

    private:
        void leave(...)
        {
        }

        template <typename State, typename Event>
        auto leave(State& state, const Event& event) -> decltype(state.OnLeave(event))
        {
            return state.OnLeave(event);
        }

        void enter(...)
        {
        }

        template <typename State, typename Event>
        auto enter(State& state, const Event& event) -> decltype(state.OnEnter(event))
        {
            return state.OnEnter(event);
        }
    };



    struct Nothing
    {
        template <typename Machine, typename State, typename Event>
        bool Execute(Machine&, State&, const Event&)
        {
            return false;
        }
    };
}