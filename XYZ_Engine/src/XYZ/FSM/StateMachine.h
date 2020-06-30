#pragma once
// Following tutorial https://sii.pl/blog/implementing-a-state-machine-in-c17-part-2/


#include <tuple>
#include <variant>


namespace XYZ {
    template <typename... States>
    class StateMachine
    {
    public:
        StateMachine() = default;

        StateMachine(States... states)
            : m_States(std::move(states)...)
        {
        }

        template <typename State>
        State& TransitionTo()
        {
            State& state = std::get<State>(m_States);
            m_CurrentState = &state;
            return state;
        }

        template <typename Event>
        void Handle(const Event& event)
        {
            handleBy(event, *this);
        }


    private:
        template <typename Event, typename Machine>
        void handleBy(const Event& event, Machine& machine)
        {
            auto passEventToState = [&machine, &event](auto statePtr) {
                auto action = statePtr->Handle(event);
                // Transition to another state handled by action
                action.Execute(machine, *statePtr, event);
            };
            std::visit(passEventToState, m_CurrentState);
        }

    private:
        std::tuple<States...> m_States;
        std::variant<States*...> m_CurrentState{ &std::get<0>(m_States) };
    };

}