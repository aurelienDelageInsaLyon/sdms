/**
 * @file discrete_pomdp.hpp
 * @author David Albert (david.albert@insa-lyon.fr)
 * @brief 
 * @version 1.0
 * @date 02/02/2021
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once

#include <sdm/types.hpp>
#include <sdm/world/po_decision_process.hpp>
#include <sdm/world/discrete_mdp.hpp>
// #include <sdm/world/belief_mdp.hpp>

#include <sdm/core/space/discrete_space.hpp>
#include <sdm/core/state_dynamics.hpp>
#include <sdm/core/reward.hpp>

namespace sdm
{

    class DiscretePOMDP : public PartiallyObservableDecisionProcess<DiscreteSpace<number>,
                                                                    DiscreteSpace<number>,
                                                                    DiscreteSpace<number>,
                                                                    StateDynamics,
                                                                    ObservationDynamics,
                                                                    Reward,
                                                                    std::discrete_distribution<number>>
    {
    public:
        DiscretePOMDP();
        DiscretePOMDP(std::shared_ptr<DiscreteSpace<number>> state_sp, std::shared_ptr<DiscreteSpace<number>> action_sp, std::shared_ptr<DiscreteSpace<number>> obs_sp);
        DiscretePOMDP(std::shared_ptr<DiscreteSpace<number>> state_sp, std::shared_ptr<DiscreteSpace<number>> action_sp, std::shared_ptr<DiscreteSpace<number>> obs_sp, std::discrete_distribution<number> start_distrib);
        DiscretePOMDP(std::shared_ptr<DiscreteSpace<number>> state_sp, std::shared_ptr<DiscreteSpace<number>> action_sp, std::shared_ptr<DiscreteSpace<number>> obs_sp, std::shared_ptr<StateDynamics> state_dyn, std::shared_ptr<ObservationDynamics> obs_dyn, std::shared_ptr<Reward>, std::discrete_distribution<number> start_distrib, number planning_horizon = 0, double discount = 0.9, Criterion criterion = Criterion::REW_MAX);

        // // Other methods
        std::shared_ptr<DiscreteMDP> toMDP();
        // std::shared_ptr<BeliefMDP> toBeliefMDP();
    };
} // namespace sdm
