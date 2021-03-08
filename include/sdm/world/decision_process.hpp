/**
 * @file decision_process.hpp
 * @author David Albert (david.albert@insa-lyon.fr)
 * @brief 
 * @version 1.0
 * @date 02/02/2021
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once

#include <vector>

#include <sdm/types.hpp>
#include <sdm/exception.hpp>
#include <sdm/world/gym_interface.hpp>
#include <sdm/world/base/decision_process_base.hpp>

#include <sdm/core/space/discrete_space.hpp>
#include <sdm/core/space/multi_space.hpp>
#include <sdm/core/space/multi_discrete_space.hpp>
#include <sdm/core/state_dynamics.hpp>
#include <sdm/core/reward.hpp>

namespace sdm
{

    template <typename TStateSpace, typename TActionSpace, typename TObsSpace, typename TStateDynamics, typename TReward, typename TDistrib>
    class DecisionProcess : public DecisionProcessBase<TStateSpace, TActionSpace, TDistrib>,
                            public GymInterface<TObsSpace, TActionSpace, std::is_same<typename TReward::value_type, std::vector<double>>::value>
    {
    public:
        using state_type = typename DecisionProcessBase<TStateSpace, TActionSpace, TDistrib>::state_type;
        using observation_type = typename GymInterface<TObsSpace, TActionSpace, std::is_same<typename TReward::value_type, std::vector<double>>::value>::observation_type;
        using action_type = typename DecisionProcessBase<TStateSpace, TActionSpace, TDistrib>::action_type;

        DecisionProcess();
        DecisionProcess(std::shared_ptr<TStateSpace> state_sp, std::shared_ptr<TActionSpace> action_sp);
        DecisionProcess(std::shared_ptr<TStateSpace> state_sp, std::shared_ptr<TActionSpace> action_sp, TDistrib);
        DecisionProcess(std::shared_ptr<TStateSpace> state_sp, std::shared_ptr<TActionSpace> action_sp, std::shared_ptr<TStateDynamics>, std::shared_ptr<TReward>, TDistrib start_distrib, number planning_horizon = 0, double discount = 0.9, Criterion criterion = Criterion::REW_MAX);
        DecisionProcess(std::shared_ptr<TStateSpace> state_sp, std::shared_ptr<TActionSpace> action_sp, std::shared_ptr<TObsSpace> obs_sp, std::shared_ptr<TStateDynamics>, std::shared_ptr<TReward>, TDistrib start_distrib, number planning_horizon = 0, double discount = 0.9, Criterion criterion = Criterion::REW_MAX);

        /**
         * \brief Get the state dynamics
         */
        std::shared_ptr<TStateDynamics> getStateDynamics() const;

        /**
         * \brief Set the state dynamics
         */
        void setStateDynamics(std::shared_ptr<TStateDynamics> state_dyn);

        /**
         * \brief Get the reward function
         */
        std::shared_ptr<TReward> getReward() const;

        /**
         * \brief Set the reward function
         */
        void setReward(std::shared_ptr<TReward> reward_function);

        /**
         * @brief Get the distribution over next states
         * 
         * @param cstate the current state
         * @param caction the current action
         * @return the distribution over next states
         */
        TDistrib getNextStateDistrib(state_type cstate, action_type caction);

        /**
         * @brief Get the distribution over next states
         * 
         * @param cstate the current state
         * @param caction the current action
         * @return the distribution over next states
         */
        TDistrib getNextStateDistrib(action_type caction);

        /**
         * @brief Reset the process to initial settings.
         * 
         * @return the initial state (which is the internal state)
         */
        observation_type reset();

        template <bool TBool = std::is_same<typename TReward::value_type, std::vector<double>>::value>
        std::enable_if_t<TBool, std::tuple<observation_type, std::vector<double>, bool>> step(action_type a);

        template <bool TBool = std::is_same<typename TReward::value_type, std::vector<double>>::value>
        std::enable_if_t<!TBool, std::tuple<observation_type, double, bool>> step(action_type a);

        template <bool TBool = std::is_same<TDistrib, std::discrete_distribution<number>>::value>
        std::enable_if_t<TBool> setupDynamicsGenerator();

        template <bool TBool = std::is_same<TDistrib, std::discrete_distribution<number>>::value>
        std::enable_if_t<!TBool> setupDynamicsGenerator();

        std::shared_ptr<TActionSpace> getActionSpace() const;

    protected:
        /**
         *  @brief Space of agents (contain number of agents and their names).
         */
        number num_agents_;

        long ctimestep_ = 0;

        /**
         * @brief State dynamics.
         */
        std::shared_ptr<TStateDynamics> state_dynamics_;

        /**
         * @brief Reward functions.
         */
        std::shared_ptr<TReward> reward_function_;

        /**
         * @brief Map (state, jaction) to probability of (next_state, next_observation) --> i.e. s_{t+1}, o_{t+1} ~ P(S_{t+1}, O_{t+1}  | S_t = s, A_t = a )
         */
        std::unordered_map<state_type, std::unordered_map<action_type, TDistrib>> dynamics_generator;

        template <bool TBool = std::is_same<TActionSpace, MultiDiscreteSpace<number>>::value>
        std::enable_if_t<TBool, number>
        getAction(action_type a);

        template <bool TBool = std::is_same<TActionSpace, MultiDiscreteSpace<number>>::value>
        std::enable_if_t<!TBool, action_type>
        getAction(action_type a);

        template <bool TBool = std::is_same<TStateSpace, TObsSpace>::value>
        std::enable_if_t<TBool, observation_type>
        resetProcess();

        template <bool TBool = std::is_same<TStateSpace, TObsSpace>::value>
        std::enable_if_t<!TBool, observation_type>
        resetProcess();
    };

    template <typename TStateSpace, typename TActionSpace, typename TStateDynamics, typename TReward, typename TDistrib>
    using FullyObservableDecisionProcess = DecisionProcess<TStateSpace, TActionSpace, TStateSpace, TStateDynamics, TReward, TDistrib>;

   using DiscreteSG = FullyObservableDecisionProcess<DiscreteSpace<number>, MultiDiscreteSpace<number>, StateDynamics, std::vector<Reward>, std::discrete_distribution<number>>;
} // namespace sdm
#include <sdm/world/decision_process.tpp>
