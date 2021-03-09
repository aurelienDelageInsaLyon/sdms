/**
 * @file discrete_decpomdp.hpp
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
#include <sdm/world/gym_interface.hpp>
#include <sdm/world/po_decision_process.hpp>
#include <sdm/world/discrete_pomdp.hpp>
#include <sdm/world/discrete_mmdp.hpp>
// #include <sdm/world/occupancy_mdp.hpp>

#include <sdm/core/space/discrete_space.hpp>
#include <sdm/core/state_dynamics.hpp>
#include <sdm/core/reward.hpp>

namespace sdm
{
    /**
     * @brief The class for Discrete Decentralized Partially Observable Markov Decision Processes. T 
     * 
     */
    class DiscreteDecPOMDP : public PartiallyObservableDecisionProcess<DiscreteSpace<number>, MultiDiscreteSpace<number>, MultiDiscreteSpace<number>, StateDynamics, ObservationDynamics, Reward, std::discrete_distribution<number>>
    {
    public:
        DiscreteDecPOMDP();
        DiscreteDecPOMDP(std::string &filename);
        DiscreteDecPOMDP(DiscreteDecPOMDP &copy);
        DiscreteDecPOMDP(std::shared_ptr<DiscreteSpace<number>> state_sp, std::shared_ptr<MultiDiscreteSpace<number>> action_sp, std::shared_ptr<MultiDiscreteSpace<number>> obs_sp, std::shared_ptr<StateDynamics> state_dyn, std::shared_ptr<ObservationDynamics> obs_dyn, std::shared_ptr<Reward>, std::discrete_distribution<number> start_distrib, number planning_horizon = 0, double discount = 0.9, Criterion criterion = Criterion::REW_MAX);

        /**
         * @brief Get the corresponding Partially Observable Markov Decision Process. The induced POMDP is the DecPOMP problem transformed as it was a single agent problem. The joint actions and observations are represented as single actions.
         * 
         * @return The corresponding POMDP.
         */
        std::shared_ptr<DiscretePOMDP> toPOMDP();

        /**
         * @brief Get the corresponding Multi-agent Markov Decision Process. It corresponds to the relaxation of the original DecPOMP assuming that all agents can observation the entire state of the environment. 
         * 
         * @return The corresponding MMDP.
         */
        std::shared_ptr<DiscreteMMDP> toMMDP();

        // std::shared_ptr<OccupancyMDP> toOccupancyMDP();
        // std::shared_ptr<SerializedOccupancyMDP> toSerializedOccupancyMDP();

        /**
         * @brief Encodes DiscreteDecPOMDP class into a string (standard .posg or .dpomdp or .zsposg format).
         * 
         * @return the process as XML
         */
        std::string toStdFormat();

        /**
         * @brief Encodes DiscreteDecPOMDP class into a string (XML format).
         * 
         * @return the process as XML
         */
        std::string toXML();

        /**
         * @brief Encodes DiscreteDecPOMDP class into a string (JSON format).
         * 
         */
        std::string toJSON();

        /**
         * @brief Save problem in file with given format (.xml, .json or .{dpomdp, posg, zsposg}).
         * 
         * @param filename the file name
         */
        void generateFile(std::string);

        friend std::ostream &operator<<(std::ostream &os, DiscreteDecPOMDP &model)
        {
            os << model.toXML();
            return os;
        }
    };
} // namespace sdm
