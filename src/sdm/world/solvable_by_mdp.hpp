#pragma once

#include <sdm/types.hpp>
#include <sdm/core/action/action.hpp>
#include <sdm/core/state/state.hpp>
#include <sdm/utils/value_function/value_function.hpp>
#include <sdm/world/base/mdp_interface.hpp>
#include <sdm/world/solvable_by_hsvi.hpp>
#include <sdm/algorithms/hsvi.hpp>

namespace sdm
{
    /**
     * @brief The class for Markov Decision Processes.
     *
     */
    class SolvableByMDP : public SolvableByHSVI
    {
    public:
        /** @brief Default constructor */
        SolvableByMDP();

        /**
         * @brief Construct a problem solvable by HSVI.
         * @param mdp the underlying MDP
         */
        SolvableByMDP(const std::shared_ptr<MDPInterface> &mdp);

        /** @brief Destructor */
        ~SolvableByMDP();

        /** @brief Get the initial state */
        std::shared_ptr<State> getInitialState();

        void setInitialState(const std::shared_ptr<State> &state);

        /**
         * @brief Select the next state.
         * 
         * @param state the state
         * @param action the action
         * @param t the timestep
         * @param hsvi a pointer on the algorithm that makes the call
         * @return the next state
         */
        virtual std::shared_ptr<State> nextState(const std::shared_ptr<State> &state, const std::shared_ptr<Action> &action, number t = 0, const std::shared_ptr<HSVI> &hsvi = nullptr);

        /**
         * @brief Get the action space at a specific state and timestep.
         * The state dependency is required when the game forbid the usage of a number of actions in this state. It is also used in some reformulated problems where actions are decision rules.
         * The time dependency is required in extensive-form games in which some agents have a different action space.   
         * 
         * @param state the state
         * @param t the timestep
         * @return the action space 
         */
        std::shared_ptr<Space> getActionSpaceAt(const std::shared_ptr<State> &state, number t = 0);

        /**
         * @brief Get the reward of executing a specific action in an specific state at timestep t. 
         * The time dependency can be required in non-stationnary problems.   
         * 
         * @param state the state
         * @param action the action
         * @param t the timestep
         * @return the reward
         */
        double getReward(const std::shared_ptr<State> &state, const std::shared_ptr<Action> &action, number t = 0);

        /**
         * @brief Get the expected next value
         * 
         * @param value_function a pointer on the value function to use to perform the calculus.
         * @param state the state on which to evaluate the next expected value *
         * @param action 
         * @param t 
         * @return double 
         */
        double getExpectedNextValue(const std::shared_ptr<ValueFunction> &value_function, const std::shared_ptr<State> &state, const std::shared_ptr<Action> &action, number t = 0);

        /**
         * @brief Get the well defined underlying problem. 
         * Some problems are solvable by DP algorithms even if they are not well defined. Usually, they simply are reformulation of an underlying well defined problem. 
         * For instance, the underlying DecPOMDP of the OccupancyMDP or the underlying POMDP of the current BeliefMDP.  
         * 
         * @return the underlying problem 
         */
        const std::shared_ptr<MDPInterface> &getUnderlyingProblem() const;

        /**
         * @brief Check if the problem is serialized.
         * 
         * @return true if the problem is serialized.
         * @return false if the problem is not serialized.
         */
        bool isSerialized() const;

        /**
         * @brief Get the specific discount factor for the problem at hand
         * 
         * @param number decision epoch or any other parameter 
         * @return double discount factor
         */
        double getDiscount(number t = 0);

        /**
         * @brief Get the specific weighted discount factor for the problem at hand
         * 
         * @param number decision epoch or any other parameter 
         * @return double discount factor
         */
        double getWeightedDiscount(number t);

        /**
         * @brief Compute the excess of the HSVI paper. It refers to the termination condition.
         * 
         * @param double : incumbent 
         * @param double : lb value
         * @param double : ub value
         * @param double : cost_so_far 
         * @param double : error 
         * @param number : horizon 
         * @return double 
         */
        double do_excess(double incumbent, double lb_value, double ub_value, double cost_so_far, double error, number horizon);

        /**
         * @brief Select the next action
         * 
         * @param const std::shared_ptr<ValueFunction<TState, TAction>>& : the lower bound
         * @param const std::shared_ptr<ValueFunction<TState, TAction>>& : the upper bound
         * @param const TState & s : current state
         * @param number h : horizon
         * @return TAction 
         */
        std::shared_ptr<Action> selectNextAction(const std::shared_ptr<ValueFunction> &lb, const std::shared_ptr<ValueFunction> &ub, const std::shared_ptr<State> &s, number h);

    protected:
        /** @brief Get the underlying mdp */
        const std::shared_ptr<MDPInterface> &getUnderlyingMDP() const;

        /** @brief The underlying well defined problem */
        std::shared_ptr<MDPInterface> underlying_problem_;

        /** @brief The initial state */
        std::shared_ptr<State> initial_state_;
    };
} // namespace sdm