#include <sdm/core/state/interface/belief_interface.hpp>
#include <sdm/core/state/interface/occupancy_state_interface.hpp>

#include <sdm/utils/value_function/action_selection/lp/action_sawtooth_lp.hpp>
#include <sdm/utils/value_function/action_selection/lp/action_sawtooth_lp_serial.hpp>

namespace sdm
{
    template <class Hash, class KeyEqual>
    BasePointSetValueFunction<Hash, KeyEqual>::BasePointSetValueFunction(const std::shared_ptr<SolvableByDP> &world,
                                                                         const std::shared_ptr<Initializer> &initializer,
                                                                         const std::shared_ptr<ActionSelectionInterface> &action_selection,
                                                                         const std::shared_ptr<TabularUpdateOperator> &update_operator,
                                                                         int freq_pruning,
                                                                         SawtoothPruning::Type type_of_sawtooth_prunning)
        : ValueFunctionInterface(world, initializer, action_selection),
          BaseTabularValueFunction<Hash, KeyEqual>(world, initializer, action_selection, update_operator),
          PrunableStructure(world->getHorizon(), freq_pruning),
          type_of_sawtooth_prunning_(type_of_sawtooth_prunning)
    {
        this->relaxation = std::vector<Container>(this->isInfiniteHorizon() ? 1 : this->horizon_ + 1, Container());

#ifdef WITH_CPLEX
        if (isInstanceOf<ActionSelectionSawtoothLP>(action_selection))
        {
            this->is_sawtooth_lp = true;
        }
#endif
    }

    template <class Hash, class KeyEqual>
    BasePointSetValueFunction<Hash, KeyEqual>::BasePointSetValueFunction(const BasePointSetValueFunction &copy)
        : ValueFunctionInterface(copy.world_, copy.initializer_, copy.action_selection_),
          BaseTabularValueFunction<Hash, KeyEqual>(copy),
          PrunableStructure(copy.world_->getHorizon(), copy.freq_pruning),
          type_of_sawtooth_prunning_(copy.type_of_sawtooth_prunning_),
          is_sawtooth_lp(copy.is_sawtooth_lp)
    {
    }

    template <class Hash, class KeyEqual>
    double BasePointSetValueFunction<Hash, KeyEqual>::computeSawtooth(const std::shared_ptr<State> &state, const std::shared_ptr<Action> &action, const std::shared_ptr<JointHistoryInterface> &joint_history, const std::shared_ptr<State> &next_hidden_state, const std::shared_ptr<Observation> &next_observation, const std::shared_ptr<JointHistoryInterface> &next_joint_history, double denominator, double difference, number t)
    {
        double Qrelaxation = this->getQValueRelaxation(state, joint_history, action, t);
        double SawtoothRatio = 0.0;
        if (joint_history->expand(next_observation) == next_joint_history)
        {
            SawtoothRatio = this->getSawtoothMinimumRatio(state, joint_history, action, next_hidden_state, next_observation, denominator, t);
        }

        return Qrelaxation + SawtoothRatio * difference;
    }


    template <class Hash, class KeyEqual>
    double BasePointSetValueFunction<Hash, KeyEqual>::getQValueRelaxation(const std::shared_ptr<State> &state, const std::shared_ptr<JointHistoryInterface> &joint_history, const std::shared_ptr<Action> &action, number t)
    {
        // \sum_{o} a(u|o) \sum_{x} s(x,o) * Q_MDP(x,u)
        double weight = 0.0;
        auto compressed_occupancy_state = state->toOccupancyState();

        // Relaxation of the problem
        auto relaxation = std::static_pointer_cast<RelaxedValueFunction>(this->getInitFunction());

        auto belief = compressed_occupancy_state->getBeliefAt(joint_history);
        weight = compressed_occupancy_state->getProbability(joint_history) * relaxation->operator()(std::make_pair(belief, action), t);

        return weight;
    }


    template <class Hash, class KeyEqual>
    double BasePointSetValueFunction<Hash, KeyEqual>::getSawtoothMinimumRatio(const std::shared_ptr<State> &state, const std::shared_ptr<JointHistoryInterface> &joint_history, const std::shared_ptr<Action> &action, const std::shared_ptr<State> &next_hidden_state, const std::shared_ptr<Observation> &next_observation, double denominator, number t)
    {
        double numerator = 0.0;

            // Compute the numerator for the Sawtooth Ratio, i.e. we compute the \sum_{x} s(x,o) * T(x,u,x_,z_)
            // This formulation allow us to tranform the problem at t+1, like the next_one_step_uncompressed_occupancy_state
            // therefore, we can have the sawtooth ration equivalent to 1.

            auto compressed_occupancy_state = state->toOccupancyState();
            auto underlying_problem = std::dynamic_pointer_cast<MPOMDPInterface>(ActionSelectionBase::world_->getUnderlyingProblem());

            // Go over all hidden state  in a belief conditionning to a joint history
            for (const auto &hidden_state : compressed_occupancy_state->getBeliefAt(joint_history)->getStates())
            {
                numerator += compressed_occupancy_state->getProbability(joint_history, hidden_state) * underlying_problem->getDynamics(hidden_state, action, next_hidden_state, next_observation, t);
            }

        return numerator / denominator;
    }


    template <class Hash, class KeyEqual>
    double BasePointSetValueFunction<Hash, KeyEqual>::computeDifference(const Pair<std::shared_ptr<State>, double> &state_AND_value, number t)
    {

        double current_upper_bound = state_AND_value.second;                                                                                                 //Get Value for the state at t+1
        double initial_upper_bound = this->getInitFunction()->operator()(state_AND_value.first->toOccupancyState()->getOneStepUncompressedOccupancy(), t + 1); // Get relaxation value of the state at t+1

        return current_upper_bound - initial_upper_bound;
    }


    template <class Hash, class KeyEqual>
    double BasePointSetValueFunction<Hash, KeyEqual>::getValueAt(const std::shared_ptr<State> &state, number t)
    {
        return this->evaluate(state, t).second;
    }

    template <class Hash, class KeyEqual>
    void BasePointSetValueFunction<Hash, KeyEqual>::setValueAt(const std::shared_ptr<State> &state, double new_value, number t)
    {
        assert((getValueAt(state, t) > new_value) && "New value is higher than the old");
        BaseTabularValueFunction<Hash, KeyEqual>::setValueAt(state, new_value, t);
    }

    template <class Hash, class KeyEqual>
    double BasePointSetValueFunction<Hash, KeyEqual>::getRelaxedValueAt(const std::shared_ptr<State> &state, number t)
    {
        double relaxed_value = 0;
        auto iter_relax = this->relaxation[t].find(state);
        if (iter_relax == this->relaxation[t].end())
        {
            if (this->getInitFunction())
                relaxed_value = this->getInitFunction()->operator()(state, t);
            else
                relaxed_value = this->representation[t].getDefault();
            this->relaxation[t].emplace(state, relaxed_value);
        }
        else
        {
            relaxed_value = iter_relax->second;
        }
        return relaxed_value;
    }

    template <class Hash, class KeyEqual>
    Pair<std::shared_ptr<State>, double> BasePointSetValueFunction<Hash, KeyEqual>::evaluate(const std::shared_ptr<State> &state, number t)
    {

        auto iterator_on_point = this->representation[t].find(state);
        if (iterator_on_point != this->representation[t].end())
        {
            return *iterator_on_point;
        }
        else
        {
            // Compute v(s) = v^{relax}(s) + min_k min_{x\in Supp(s^k)} \frac{s(x)}{s^k(x)} \left( v^{relax}(s^k) - v^k\right)
            std::shared_ptr<State> argmin_k = state;
            double min_k = (this->getSupport(t).size() == 0) ? 0.0 : std::numeric_limits<double>::max();
            double v_relax = this->getRelaxedValueAt(state, t);

            // Go over all points in the representation
            for (const auto &point_k : this->representation[t])
            {
                // Dissociate element of the k-th point (state / value)
                auto [s_k, v_k] = point_k;

                // Estimate the value of the relaxation at s_k
                double v_relax_k = this->getRelaxedValueAt(s_k, t);

                // Get minimum ratio s(x)/s^k(x)
                double ratio = this->computeRatio(state, s_k);

                // Determine the "value" for k-th point
                double min_int = ratio * (v_k - v_relax_k);

                // If the "value" of k-th point is minimal, keep it
                if (min_int < min_k)
                {
                    min_k = min_int;
                    argmin_k = s_k;
                }
            }
            return std::make_pair(argmin_k, v_relax + min_k);
        }
    }

    template <class Hash, class KeyEqual>
    double BasePointSetValueFunction<Hash, KeyEqual>::computeRatio(const std::shared_ptr<State> &s, const std::shared_ptr<State> &s_k)
    {
        // Check available ratio in the map and return it
        auto iter_s = ratios.find(s);
        if (iter_s != ratios.end())
        {
            auto iter_s_k = iter_s->second.find(s_k);
            if (iter_s_k != iter_s->second.end())
            {
                return iter_s_k->second;
            }
        }
        double ratio = 0.;
        // If no such ratio is already stored, compute it and store it 
        if (sdm::isInstanceOf<OccupancyStateInterface>(s))
        {
            ratio = this->ratioOccupancy(sdm::to<OccupancyStateInterface>(s), sdm::to<OccupancyStateInterface>(s_k));
        }
        else if (sdm::isInstanceOf<BeliefInterface>(s))
        {
            ratio = this->ratioBelief(sdm::to<BeliefInterface>(s), sdm::to<BeliefInterface>(s_k));
        }
        else
        {
            throw sdm::exception::Exception("(PointSet::computeRatio) States must inherit from 'BeliefInterface' or 'OccupancyStateInterface'");
        }
        ratios[s][s_k] = ratio;
        return ratio;
    }

    template <class Hash, class KeyEqual>
    double BasePointSetValueFunction<Hash, KeyEqual>::ratioBelief(const std::shared_ptr<BeliefInterface> &b, const std::shared_ptr<BeliefInterface> &b_k)
    {
        // Determine the ratio for the specific case when the state is a belief
        double min_ratio = 1.0;

        for (auto &x : b_k->getStates())
        {
            double ratio_k_x = (b->getProbability(x) / b_k->getProbability(x));
            // determine the min ratio
            if (ratio_k_x < min_ratio)
            {
                min_ratio = ratio_k_x;
            }
        }
        return min_ratio;
    }

    template <class Hash, class KeyEqual>
    double BasePointSetValueFunction<Hash, KeyEqual>::ratioOccupancy(const std::shared_ptr<OccupancyStateInterface> &s, const std::shared_ptr<OccupancyStateInterface> &s_k)
    {
        // Determine the ratio for the specific case when the state is a Occupancy State
        double min_ratio;
        std::shared_ptr<OccupancyStateInterface> point, occupancy_state;
        if (!this->is_sawtooth_lp)
        {
            min_ratio = 1.0;
            point = s_k;
            occupancy_state = s;
        }
        else
        {
            min_ratio = std::numeric_limits<double>::max();
            point = s_k->getOneStepUncompressedOccupancy();
            occupancy_state = s->getOneStepUncompressedOccupancy();
        }

        // Go over all joint history
        for (auto &o : point->getJointHistories())
        {
            // Go over all hidden state in the belief conditionning to the joitn history
            for (const auto &x : point->getBeliefAt(o)->getStates())
            {
                double ratio_k_o_x = (occupancy_state->getProbability(o, x) / point->getProbability(o, x));
                // determine the min int
                if (ratio_k_o_x < min_ratio)
                {
                    min_ratio = ratio_k_o_x;
                }
            }
        }
        return min_ratio;
    }

    template <class Hash, class KeyEqual>
    void BasePointSetValueFunction<Hash, KeyEqual>::pairwise_prune(number t, double epsilon)
    {
        // List of points that are \eps-dominated
        std::vector<std::shared_ptr<State>> point_to_delete;
        // List of points that are not dominated
        std::vector<std::shared_ptr<State>> point_to_keep;

        // Go over all current points
        for (const auto &state_i : this->getSupport(t))
        {
            bool point_dominated = false;

            // Go over all points in point_to_keep
            for (auto state_j = point_to_keep.begin(); state_j != point_to_keep.end(); state_j++)
            {
                double estimation_i = this->getRelaxedValueAt(state_i, t) + this->computeRatio(state_i, *state_j) * (this->getValueAt(*state_j, t) - this->getRelaxedValueAt(*state_j, t));
                // If state_j pairwise epsilon-dominates state_i, we add state_i to the set of points to be deleted.
                if (estimation_i < this->getValueAt(state_i, t) + epsilon)
                {
                    point_to_delete.push_back(state_i);
                    point_dominated = true;
                    break;
                }
            }
            // If point is dominated, we reject alpha and go to the next iteration of outer loop
            if (point_dominated)
                continue;

            // Go over all points in point_to_keep
            for (auto state_j = point_to_keep.begin(); state_j != point_to_keep.end();)
            {
                // Compute estimation of state_i based upon that of state_j
                double estimation_j = this->getRelaxedValueAt(*state_j, t) + this->computeRatio(*state_j, state_i) * (this->getValueAt(state_i, t) - this->getRelaxedValueAt(state_i, t));

                // If state_i pairwise epsilon-dominates state_j in point_to_keep, we deleted this state
                if (estimation_j < this->getValueAt(*state_j, t) + epsilon)
                {
                    point_to_delete.push_back(*state_j);
                    state_j = point_to_keep.erase(state_j);
                }
                else
                {
                    state_j++;
                }
            }

            point_to_keep.push_back(state_i);
        }

        // Erase pairwise epsilon-dominated points
        for (const auto &to_delete : point_to_delete)
        {
            this->representation[t].erase(to_delete);
        }
    }

    template <class Hash, class KeyEqual>
    void BasePointSetValueFunction<Hash, KeyEqual>::prune(number t)
    {
        if (this->type_of_sawtooth_prunning_ == SawtoothPruning::PAIRWISE)
            this->pairwise_prune(t);
    }

    template <class Hash, class KeyEqual>
    std::shared_ptr<ValueFunctionInterface> BasePointSetValueFunction<Hash, KeyEqual>::copy()
    {
        auto casted_value = std::dynamic_pointer_cast<BasePointSetValueFunction<Hash, KeyEqual>>(this->getptr());
        return std::make_shared<BasePointSetValueFunction<Hash, KeyEqual>>(*casted_value);
    }

    template <class Hash, class KeyEqual>
    std::string BasePointSetValueFunction<Hash, KeyEqual>::str() const
    {
        std::ostringstream res;
        res << "<point_set_representation horizon=\"" << ((this->isInfiniteHorizon()) ? "inf" : std::to_string(this->getHorizon())) << "\">" << std::endl;
        for (std::size_t i = 0; i < this->representation.size(); i++)
        {
            res << "\t<value timestep=\"" << ((this->isInfiniteHorizon()) ? "all" : std::to_string(i)) << "\" default=\"" << this->representation.at(i).getDefault() << "\">" << std::endl;
            for (const auto &pair_st_val : this->representation.at(i))
            {
                // res << "\t\t<state id=\"" << pair_st_val.first << "\">" << std::endl;
                // res << "\t\t</state>" << std::endl;
                std::ostringstream state_str;
                state_str << pair_st_val.first->str();
                res << "\t\t<state>" << std::endl;
                res << tools::addIndent(state_str.str(), 3) << std::endl;
                res << "\t\t</state>" << std::endl;
                res << "\t\t<value>" << std::endl;
                res << "\t\t\t" << pair_st_val.second << std::endl;
                res << "\t\t</value>" << std::endl;
            }
            res << "\t</value>" << std::endl;
        }

        res << "</point_set_representation>" << std::endl;
        return res.str();
    }

} // namespace sdm