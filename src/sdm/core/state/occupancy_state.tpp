#include <sdm/core/state/occupancy_state.hpp>
#include <sdm/core/state/private_occupancy_state.hpp>

namespace sdm
{

    template <typename TState, typename TJointHistory_p>
    OccupancyState<TState, TJointHistory_p>::OccupancyState() : OccupancyState<TState, TJointHistory_p>(0, 0)
    {
    }

    template <typename TState, typename TJointHistory_p>
    OccupancyState<TState, TJointHistory_p>::OccupancyState(double default_value) : OccupancyState<TState, TJointHistory_p>(0, default_value)
    {
    }

    template <typename TState, typename TJointHistory_p>
    OccupancyState<TState, TJointHistory_p>::OccupancyState(std::size_t size, double default_value)
        : BaseOccupancyState<TState, TJointHistory_p>(size, default_value)
    {
        // Build the private occupancy map
        for (number agent_id = 0; agent_id < this->num_agents; agent_id++)
        {
            this->tuple_of_maps_from_histories_to_private_occupancy_states_.push_back({});
            this->private_ihistory_map_.push_back({});
            this->map_label_to_pointer.push_back({});
        }
    }

    template <typename TState, typename TJointHistory_p>
    OccupancyState<TState, TJointHistory_p>::OccupancyState(const OccupancyState &occupancy_state)
        : BaseOccupancyState<TState, TJointHistory_p>(occupancy_state),
          tuple_of_maps_from_histories_to_private_occupancy_states_(occupancy_state.tuple_of_maps_from_histories_to_private_occupancy_states_),
          fully_uncompressed_occupancy_state(occupancy_state.fully_uncompressed_occupancy_state),
          one_step_left_compressed_occupancy_state(occupancy_state.one_step_left_compressed_occupancy_state),
          private_ihistory_map_(occupancy_state.private_ihistory_map_),
          map_label_to_pointer(occupancy_state.map_label_to_pointer),
          jhistory_map_(occupancy_state.jhistory_map_)
    {
    }

    template <typename TState, typename TJointHistory_p>
    const Joint<RecursiveMap<typename OccupancyState<TState, TJointHistory_p>::ihistory_type, std::shared_ptr<PrivateOccupancyState<TState, TJointHistory_p>>>> &OccupancyState<TState, TJointHistory_p>::getPrivateOccupancyStates() const
    {
        return this->tuple_of_maps_from_histories_to_private_occupancy_states_;
    }

    template <typename TState, typename TJointHistory_p>
    const std::shared_ptr<PrivateOccupancyState<TState, TJointHistory_p>> &OccupancyState<TState, TJointHistory_p>::getPrivateOccupancyState(const number &agent_id, const ihistory_type &ihistory) const
    {
        return this->tuple_of_maps_from_histories_to_private_occupancy_states_.at(agent_id).at(ihistory);
    }

    template <typename TState, typename TJointHistory_p>
    std::shared_ptr<OccupancyState<TState, TJointHistory_p>> OccupancyState<TState, TJointHistory_p>::getFullyUncompressedOccupancy() const
    {
        return this->fully_uncompressed_occupancy_state;
    }

    template <typename TState, typename TJointHistory_p>
    void OccupancyState<TState, TJointHistory_p>::setFullyUncompressedOccupancy(const std::shared_ptr<OccupancyState<TState, TJointHistory_p>> &fully_uncompressed_ostate)
    {
        this->fully_uncompressed_occupancy_state = fully_uncompressed_ostate;
    }

    template <typename TState, typename TJointHistory_p>
    typename OccupancyState<TState, TJointHistory_p>::ihistory_type OccupancyState<TState, TJointHistory_p>::getLabel(const typename OccupancyState<TState, TJointHistory_p>::ihistory_type &ihistory, number agent_id) const
    {
        if (this->private_ihistory_map_.at(agent_id).find(ihistory) == this->private_ihistory_map_.at(agent_id).end())
        {
            // if the ihistory was never compressed
            return ihistory;
        }
        else
        {
            // if the ihistory was compressed
            return *this->private_ihistory_map_.at(agent_id).at(ihistory);
        }
    }

    template <typename TState, typename TJointHistory_p>
    std::vector<typename OccupancyState<TState, TJointHistory_p>::ihistory_type> OccupancyState<TState, TJointHistory_p>::getJointLabels(const std::vector<typename OccupancyState<TState, TJointHistory_p>::ihistory_type> &list_ihistories) const
    {
        std::vector<ihistory_type> new_list_ihistories;
        for (int agent_id = 0; agent_id < this->num_agents; ++agent_id)
        {
            // if the ihistory was never compressed
            new_list_ihistories.push_back(this->getLabel(list_ihistories.at(agent_id), agent_id));
        }
        return new_list_ihistories;
    }

    template <typename TState, typename TJointHistory_p>
    void OccupancyState<TState, TJointHistory_p>::updateLabel(number agent_id, const typename OccupancyState<TState, TJointHistory_p>::ihistory_type &ihistory, const typename OccupancyState<TState, TJointHistory_p>::ihistory_type &label)
    {
        using ihistory_type = typename OccupancyState<TState, TJointHistory_p>::ihistory_type;
        // if there is a label for ihistory
        if (this->private_ihistory_map_[agent_id].find(ihistory) != this->private_ihistory_map_[agent_id].end())
        {
            // Get the old label
            auto old_label = this->private_ihistory_map_[agent_id].at(ihistory);
            // Change every labels of ihistories that have old_label as label
            *old_label = label;
        }
        else
        {
            // Check if the label is already used for another indiv history
            if (this->map_label_to_pointer[agent_id].find(label) != this->map_label_to_pointer[agent_id].end())
            {
                this->private_ihistory_map_[agent_id][ihistory] = this->map_label_to_pointer[agent_id].at(label);
            }
            else
            {
                // If no such label is already used, create a pointer on it and store it
                auto new_ptr_on_label = std::make_shared<ihistory_type>(label);
                this->map_label_to_pointer[agent_id][label] = new_ptr_on_label;
                this->private_ihistory_map_[agent_id][ihistory] = new_ptr_on_label;
            }
        }
    }

    template <typename TState, typename TJointHistory_p>
    void OccupancyState<TState, TJointHistory_p>::updateJointLabels(const std::vector<typename OccupancyState<TState, TJointHistory_p>::ihistory_type> &list_ihistories, const std::vector<typename OccupancyState<TState, TJointHistory_p>::ihistory_type> &list_labels)
    {
        for (number agent_id = 0; agent_id < this->num_agents; ++agent_id)
        {
            this->updateLabel(agent_id, list_ihistories.at(agent_id), list_labels.at(agent_id));
        }
    }

    template <typename TState, typename TJointHistory_p>
    std::shared_ptr<OccupancyState<TState, TJointHistory_p>> OccupancyState<TState, TJointHistory_p>::getOneStepUncompressedOccupancy() const
    {
        return this->one_step_left_compressed_occupancy_state;
    }

    template <typename TState, typename TJointHistory_p>
    void OccupancyState<TState, TJointHistory_p>::setOneStepUncompressedOccupancy(const std::shared_ptr<OccupancyState<TState, TJointHistory_p>> &one_step_uncompress_ostate)
    {
        this->one_step_left_compressed_occupancy_state = one_step_uncompress_ostate;
    }

    template <typename TState, typename TJointHistory_p>
    TJointHistory_p OccupancyState<TState, TJointHistory_p>::getCompressedJointHistory(const TJointHistory_p &joint_history) const
    {
        auto labels = this->getJointLabels(joint_history->getIndividualHistories());
        return this->jhistory_map_.at(labels);
    }

    template <typename TState, typename TJointHistory_p>
    bool OccupancyState<TState, TJointHistory_p>::areIndividualHistoryLPE(const typename TJointHistory_p::element_type::ihistory_type &ihistory_1, const typename TJointHistory_p::element_type::ihistory_type &ihistory_2, number agent_identifier)
    {
        return *this->getPrivateOccupancyState(agent_identifier, ihistory_1) == *this->getPrivateOccupancyState(agent_identifier, ihistory_2);
    }

    template <typename TState, typename TJointHistory_p>
    auto OccupancyState<TState, TJointHistory_p>::compress()
    {
        OccupancyState<TState, TJointHistory_p> current_compact_ostate;
        OccupancyState<TState, TJointHistory_p> previous_compact_ostate = *this;
        for (int agent_id = 0; agent_id < this->num_agents; ++agent_id)
        {
            // Get support (a set of individual histories for agent i)
            auto support_set = this->getIndividualHistories(agent_id);
            auto support = tools::set2vector(support_set);

            // Sort support
            std::sort(support.begin(), support.end());

            for (auto iter_first = support.begin(); iter_first != support.end();)
            {
                auto ihistory_label = *iter_first;      // Get the ihistory "label"
                iter_first = support.erase(iter_first); // Erase the ihistory "label" from the support

                // Set probability of labels
                for (const auto &pair_s_o_prob : *previous_compact_ostate.getPrivateOccupancyState(agent_id, ihistory_label))
                {
                    current_compact_ostate.setProbabilityAt(pair_s_o_prob.first, pair_s_o_prob.second);
                }
                for (auto iter_second = iter_first; iter_second != support.end();)
                {
                    auto ihistory_one_step_left = *iter_second; // Get the ihistory we want check the equivalence
                    if (this->areIndividualHistoryLPE(ihistory_label, ihistory_one_step_left, agent_id))
                    {
                        this->updateLabel(agent_id, ihistory_one_step_left, ihistory_label); // Store new label
                        iter_second = support.erase(iter_second);                            // Erase unecessary equivalent individual history

                        for (const auto &pair_s_o_prob : *previous_compact_ostate.getPrivateOccupancyState(agent_id, ihistory_one_step_left))
                        {
                            auto partial_jhist = previous_compact_ostate.getPrivateOccupancyState(agent_id, ihistory_one_step_left)->getPartialJointHistory(pair_s_o_prob.first.second);
                            auto joint_history = previous_compact_ostate.getPrivateOccupancyState(agent_id, ihistory_label)->getJointHistory(partial_jhist);
                            current_compact_ostate.addProbabilityAt(std::make_pair(pair_s_o_prob.first.first, joint_history), pair_s_o_prob.second);
                        }
                    }
                    else
                    {
                        iter_second++;
                    }
                }
            }

            previous_compact_ostate = current_compact_ostate;
            previous_compact_ostate.private_ihistory_map_ = this->private_ihistory_map_;
            previous_compact_ostate.finalize();
            current_compact_ostate.clear();
        }
        return previous_compact_ostate;
    }

    template <typename TState, typename TJointHistory_p>
    void OccupancyState<TState, TJointHistory_p>::finalize()
    {
        BaseOccupancyState<TState, TJointHistory_p>::finalize();

        for (const auto &pair_state_jhist : *this)
        {
            auto jhist = this->getHistory(pair_state_jhist.first);
            auto proba = this->getProbability(pair_state_jhist.first);

            // Store relation between joint history and list of individual histories
            this->jhistory_map_.emplace(jhist->getIndividualHistories(), jhist);

            for (number agent_id = 0; agent_id < this->num_agents; agent_id++)
            {
                // Instanciation empty private occupancy state associated to ihitory and agent i if not exists
                if (this->tuple_of_maps_from_histories_to_private_occupancy_states_[agent_id].find(jhist->getIndividualHistory(agent_id)) == this->tuple_of_maps_from_histories_to_private_occupancy_states_[agent_id].end())
                {
                    this->tuple_of_maps_from_histories_to_private_occupancy_states_[agent_id].emplace(jhist->getIndividualHistory(agent_id), std::make_shared<PrivateOccupancyState<TState, TJointHistory_p>>(agent_id, this->default_value_));
                }
                // Set private occupancy measure
                this->tuple_of_maps_from_histories_to_private_occupancy_states_[agent_id][jhist->getIndividualHistory(agent_id)]->addProbabilityAt(pair_state_jhist.first, proba);
            }
        }
        for (number agent_id = 0; agent_id < this->num_agents; agent_id++)
        {
            for (const auto &pair_ihist_private_occupancy_state : this->tuple_of_maps_from_histories_to_private_occupancy_states_[agent_id])
            {
                pair_ihist_private_occupancy_state.second->finalize();
            }
        }
    }

    template <typename TState, typename TJointHistory_p>
    std::shared_ptr<OccupancyState<TState, TJointHistory_p>> OccupancyState<TState, TJointHistory_p>::getptr()
    {
        return std::static_pointer_cast<OccupancyState<TState, TJointHistory_p>>(this->shared_from_this());
    }

} // namespace sdm

namespace std
{
    template <typename S, typename V>
    struct hash<sdm::OccupancyState<S, V>>
    {
        typedef sdm::OccupancyState<S, V> argument_type;
        typedef std::size_t result_type;
        inline result_type operator()(const argument_type &in) const
        {
            return std::hash<sdm::BaseOccupancyState<S, V>>()(in);
        }
    };
}