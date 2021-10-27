
// #pragma once

// #include <sdm/utils/value_function/update_operator/vupdate_operator.hpp>

// namespace sdm
// {
//     namespace update
//     {
//         class MaxPlanUpdateOperator : public PWLCUpdateOperator
//         {
//         public:
//             MaxPlanUpdateOperator(const std::shared_ptr<PWLCValueFunctionInterface> &value_function);

//             void update(const std::shared_ptr<State> &state, const std::shared_ptr<Action> &action, number t);

//             std::shared_ptr<PWLCValueFunctionInterface> getPWLCValueFunction() const;

//         protected:
//             std::shared_ptr<State> computeNewHyperplane(const std::shared_ptr<BeliefInterface> &belief_state, const std::shared_ptr<Action> &action, number t);
//             std::shared_ptr<State> computeNewHyperplane(const std::shared_ptr<OccupancyStateInterface> &occupancy_state, const std::shared_ptr<Action> &decision_rule, number t);
//         };
//     }
// }