/**
 * @file occupancy_state.hpp
 * @author David Albert (david.albert@insa-lyon.fr)
 * @brief 
 * @version 1.0
 * @date 29/03/2021
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once

#include <sdm/types.hpp>
#include <sdm/utils/struct/pair.hpp>
#include <sdm/core/joint.hpp>
#include <sdm/core/state/history.hpp>
#include <sdm/core/state/base/base_occupancy_state.hpp>
#include <sdm/core/state/private_occupancy_state.hpp>

namespace sdm
{

  /**
   * @brief An occupancy state refers to the whole knowledge that a central planner can have access to take decisions.
   * 
   * @tparam TState refers to a number
   * @tparam TJointHistory_p refers to a joint histories
   */
  template <typename TState = number, typename TJointHistory_p = JointHistoryTree_p<number>>
  class OccupancyState
      : public BaseOccupancyState<TState, TJointHistory_p>
  {
  public:
    using jhistory_type = typename BaseOccupancyState<TState, TJointHistory_p>::jhistory_type;
    using state_type = typename BaseOccupancyState<TState, TJointHistory_p>::state_type;

    OccupancyState();
    OccupancyState(double default_value);
    OccupancyState(std::size_t size, double default_value);
    OccupancyState(const OccupancyState &v);

    auto compress();
    bool areIndividualHistoryLPE(const typename TJointHistory_p::element_type::ihistory_type &hist1, const typename TJointHistory_p::element_type::ihistory_type &hist2, number ag_id);
    bool areStateJointHistoryPairsLPE(const Pair<TState, TJointHistory_p> &p1, const Pair<TState, TJointHistory_p> &p2);
  };
} // namespace sdm
#include <sdm/core/state/occupancy_state.tpp>