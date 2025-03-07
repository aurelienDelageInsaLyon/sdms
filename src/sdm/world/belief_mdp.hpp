/**
 * @file belief_mdp.hpp
 * @author David Albert (david.albert@insa-lyon.fr)
 * @brief File that contains the implementation of the belief mdp process class.
 * @version 1.0
 * @date 03/02/2021
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once

#include <sdm/types.hpp>
#include <sdm/utils/config.hpp>
#include <sdm/core/state/state.hpp>
#include <sdm/core/state/belief_state.hpp>
#include <sdm/core/state/interface/belief_interface.hpp>
#include <sdm/core/action/action.hpp>
#include <sdm/utils/struct/recursive_map.hpp>
#include <sdm/utils/struct/graph.hpp>
#include <sdm/world/base/belief_mdp_interface.hpp>
#include <sdm/world/solvable_by_mdp.hpp>
#include <sdm/world/base/pomdp_interface.hpp>
#include <sdm/world/gym_interface.hpp>
#include <sdm/world/mdp.hpp>

namespace sdm
{
    /**
     * @brief This class provides a way to transform a POMDP into belief MDP formalism.
     *
     * This problem reformulation can be used to solve the underlying POMDP thanks to 
     * standard dynamic programming algorithms. 
     *  
     */
    template <class TBelief>
    class BaseBeliefMDP : virtual public SolvableByMDP,
                           virtual public BeliefMDPInterface,
                           public GymInterface
    {
    public:
        BaseBeliefMDP();
        BaseBeliefMDP(Config config);
        BaseBeliefMDP(const std::shared_ptr<POMDPInterface> &pomdp, Config config);
        BaseBeliefMDP(const std::shared_ptr<POMDPInterface> &pomdp, int batch_size = 0, bool store_states = true, bool store_actions = true);
        ~BaseBeliefMDP();

        /** @brief Get the address of the underlying POMDP */
        std::shared_ptr<POMDPInterface> getUnderlyingPOMDP() const;

        /** @brief Get the address of the underlying BeliefMDP */
        std::shared_ptr<BeliefMDPInterface> getUnderlyingBeliefMDP();

        /**
         * @brief Get the action space at a specific belief and time step.
         * 
         * The time dependency is required in extensive-form games in which some agents 
         * have a different action space. 
         * 
         * @param belief the belief
         * @param t the time step
         * @return the action space 
         * 
         */
        virtual std::shared_ptr<Space> getActionSpaceAt(const std::shared_ptr<State> &belief, number t = 0);

        /**
         * @brief Get the observation space at a specific state and given an action. 
         * 
         * @param belief the belief
         * @param action the action
         * @param t the time step
         * @return the observation space 
         */
        virtual std::shared_ptr<Space> getObservationSpaceAt(const std::shared_ptr<State> &, const std::shared_ptr<Action> &, number t);

        /**
         * @brief Get the expected reward of executing a specific action in a specific belief at timestep t. 
         * 
         * The time dependency can be required in non-stationnary problems.   
         * 
         * @param belief the belief
         * @param action the action
         * @param t the time step
         * @return the reward
         * 
         */
        virtual double getReward(const std::shared_ptr<State> &belief, const std::shared_ptr<Action> &action, number t = 0);


        /**
         * @brief Get the expected next value
         * 
         * @param value_function a pointer on the value function to use to perform the calculus.
         * @param belief the state on which to evaluate the next expected value
         * @param action the action executed 
         * @param t the time step
         * @return double 
         */
        virtual double getExpectedNextValue(const std::shared_ptr<ValueFunction> &value_function, const std::shared_ptr<State> &belief, const std::shared_ptr<Action> &action, number t = 0);

        Pair<std::shared_ptr<State>, double> getNextState(const std::shared_ptr<State> &belief, const std::shared_ptr<Action> &action, const std::shared_ptr<Observation> &observation, number t);

        /**
         * @brief Get the next generic state (i.e. belief, occupancy state, etc).
         * 
         * This function returns the next occupancy state. To do so, we check in the MDP graph the existance 
         * of an edge (action / observation) starting from the current occupancy state. If exists, we return 
         * the associated next belief. Otherwise, we compute the next belief using  "computeNextStateAndProba" 
         * function and add the edge from the current belief to the next belief in the graph.
         *
         * @param state the current state
         * @param action the action
         * @param observation the observation
         * @param t the time step
         * @return the next belief
         *
         */
        Pair<std::shared_ptr<State>, double> getNextStateAndProba(const std::shared_ptr<State> &state, const std::shared_ptr<Action> &action, const std::shared_ptr<Observation> &observation, number t);

        /** @brief Get the Observation Probability p(o | b', a) */
        virtual double getObservationProbability(const std::shared_ptr<State> &belief, const std::shared_ptr<Action> &action, const std::shared_ptr<State> &next_belief, const std::shared_ptr<Observation> &obs, number t = 0) const;

        // *****************
        //    RL methods
        // *****************

        virtual std::shared_ptr<State> reset();
        virtual std::tuple<std::shared_ptr<State>, std::vector<double>, bool> step(std::shared_ptr<Action> action);
        virtual std::shared_ptr<Action> getRandomAction(const std::shared_ptr<State> &state, number t);

        /**
         * @brief Get the MDP graph. 
         * 
         * In the case where the variables `store_states` and `store_actions` are set to true, 
         * we iteratively construct and save the graph of state transitions. 
         * 
         * @return the graph representing a markov decision process 
         */
        std::shared_ptr<Graph<std::shared_ptr<State>, Pair<std::shared_ptr<Action>, std::shared_ptr<Observation>>>> getMDPGraph();
        std::vector<std::shared_ptr<State>> getStoredStates() const;

        /** @brief A pointer on the bag containing all states. */
        RecursiveMap<TBelief, std::shared_ptr<State>> state_space_;

        std::shared_ptr<Graph<double, Pair<std::shared_ptr<State>, std::shared_ptr<Action>>>> reward_graph_;

    public:
        /** @brief The underlying well defined POMDP */
        std::shared_ptr<POMDPInterface> pomdp;

        // If 0, it means the exact transitions will be used and not sampled ones.
        int batch_size_;

        /** @brief The current state (used in RL). */
        std::shared_ptr<State> current_state_;

        /** @brief The current timestep (used in RL). */
        int step_;

        /** @brief Hyperparameters. */
        bool store_states_ = true, store_actions_ = true;

        /** @brief The probability transition. (i.e. p(o | b, a) */
        RecursiveMap<std::shared_ptr<State>, std::shared_ptr<Action>, std::shared_ptr<Observation>, double> transition_probability;

        /** @brief the MDP Graph (graph of state transition) */
        std::shared_ptr<Graph<std::shared_ptr<State>, Pair<std::shared_ptr<Action>, std::shared_ptr<Observation>>>> mdp_graph_;

        /**
         * @brief Compute the state transition in order to return next state and associated probability.
         * 
         * This function can be modify in an inherited class to define a belief MDP with a different representation of the belief state. 
         * (i.e. OccupancyMDP inherit from BaseBeliefMDP with TBelief = OccupancyState)
         * 
         * @param belief the belief
         * @param action the action
         * @param observation the observation
         * @param t the timestep
         * @return the couple (next state, transition probability in the next state)
         * 
         * 
         */
        virtual Pair<std::shared_ptr<State>, double> computeNextStateAndProbability(const std::shared_ptr<State> &belief, const std::shared_ptr<Action> &action, const std::shared_ptr<Observation> &observation, number t = 0);
        virtual Pair<std::shared_ptr<State>, double> computeSampledNextState(const std::shared_ptr<State> &belief, const std::shared_ptr<Action> &action, const std::shared_ptr<Observation> &observation, number t = 0);
    };

    using BeliefMDP = BaseBeliefMDP<Belief>;

}
#include <sdm/world/belief_mdp.tpp>
