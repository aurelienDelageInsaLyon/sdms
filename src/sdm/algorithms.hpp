#pragma once

#include <sdm/algorithms/hsvi.hpp>
#include <sdm/algorithms/value_iteration.hpp>
#include <sdm/algorithms/q_learning.hpp>

namespace sdm
{
    namespace algo
    {
        /**
         * @brief Build the HSVI version that use TabularValueFunction Representation. 
         * 
         * @tparam TState Type of the state.
         * @tparam TAction Type of the action.
         * @param problem the problem to be solved
         * @param discount the discount factor
         * @param error the accuracy
         * @param horizon the planning horizon
         * @return pointer on HSVI instance
         */
        std::shared_ptr<sdm::HSVI> makeHSVI(std::shared_ptr<SolvableByHSVI> problem,
                                            std::string upper_bound_name,
                                            std::string lower_bound_name,
                                            std::string ub_init_name,
                                            std::string lb_init_name,
                                            double discount = 0.99,
                                            double error = 0.01,
                                            number horizon = 0,
                                            int trials = 1000,
                                            std::string name = "",
                                            double time_max = 5000,
                                            std::string current_type_of_resolution = "BigM",
                                            number BigM = 100,
                                            std::string type_sawtooth_linear_programming = "Full",
                                            TypeOfMaxPlanPrunning type_of_maxplan_prunning = TypeOfMaxPlanPrunning::PAIRWISE,
                                            int freq_prunning_lower_bound = -1,
                                            TypeOfSawtoothPrunning type_of_sawtooth_pruning = TypeOfSawtoothPrunning::NONE,
                                            int freq_prunning_upper_bound = -1);

        /**
         * @brief Build the ValueIteration version. 
         * 
         * @tparam TState Type of the state.
         * @tparam TAction Type of the action.
         * @param problem the problem to be solved
         * @param discount the discount factor
         * @param error the accuracy
         * @param horizon the planning horizon
         * @return pointer on HSVI instance
         */
        std::shared_ptr<sdm::ValueIteration> makeValueIteration(std::shared_ptr<SolvableByHSVI> problem,
                                                                double error,
                                                                number horizon);

        std::shared_ptr<sdm::QLearning<>> makeQLearning(std::shared_ptr<GymInterface> problem,
                                                        number horizon = 0,
                                                        double discount = 0.9,
                                                        double lr = 0.01,
                                                        double batch_size = 1,
                                                        unsigned long num_episodes = 10000,
                                                        std::string name = "qlearning");

        std::shared_ptr<SolvableByHSVI> makeFormalism(std::string problem_path,
                                                      std::string formalism,
                                                      double discount,
                                                      number horizon,
                                                      int truncation,
                                                      bool compression,
                                                      bool store_action,
                                                      bool store_state, 
                                                      number batch_size);

        std::shared_ptr<Algorithm> makeAlgorithm(std::string algo_name,
                                                 std::shared_ptr<SolvableByHSVI> formalism_problem,
                                                 std::string formalism_name,
                                                 std::string upper_bound,
                                                 std::string lower_bound,
                                                 std::string ub_init,
                                                 std::string lb_init,
                                                 double discount,
                                                 double error,
                                                 int trials,
                                                 std::string name,
                                                 double time_max,
                                                 std::string current_type_of_resolution,
                                                 number BigM,
                                                 std::string type_sawtooth_linear_programming,
                                                 TypeOfMaxPlanPrunning type_of_maxplan_prunning,
                                                 int freq_prunning_lower_bound,
                                                 TypeOfSawtoothPrunning type_of_sawtooth_pruning,
                                                 int freq_prunning_upper_bound);

        /**
         * @brief Build an algorithm given his name and the configurations required. 
         * 
         * @tparam TState Type of the state.
         * @tparam TAction Type of the action.
         * @param algo_name the name of the algorithm to be built* 
         * @param problem the problem to be solved
         * @param discount the discount factor
         * @param error the accuracy
         * @param horizon the planning horizon
         * @param trials the maximum number of trials 
         * @return auto pointer on algorithm instance
         */
        std::shared_ptr<Algorithm> make(std::string algo_name,
                                        std::string problem_path,
                                        std::string formalism,
                                        std::string upper_bound,
                                        std::string lower_bound,
                                        std::string ub_init,
                                        std::string lb_init,
                                        double discount = 0.99,
                                        double error = 0.001,
                                        number horizon = 0,
                                        int trials = 1000,
                                        int truncation = -1,
                                        std::string name = "",
                                        double time_max = 5000,
                                        std::string current_type_of_resolution = "BigM",
                                        number BigM = 100,
                                        std::string type_sawtooth_linear_programming = "Full",
                                        TypeOfMaxPlanPrunning type_of_maxplan_prunning = TypeOfMaxPlanPrunning::PAIRWISE,
                                        int freq_prunning_lower_bound = -1,
                                        TypeOfSawtoothPrunning type_of_sawtooth_pruning = TypeOfSawtoothPrunning::NONE,
                                        int freq_prunning_upper_bound = -1,
                                        bool compression = true,
                                        bool store_action = true,
                                        bool store_state = true,
                                        number batch_size = 0
                                        );

        /**
         * @brief Get all available algorithms.
         * 
         * @return the list of available algorithms.
         */
        std::vector<std::string> available()
        {
            return {"A*", "BackwardInduction", "HSVI", "QLearning", "ValueIteration"};
        }
    } // namespace algo
} // namespace sdm
