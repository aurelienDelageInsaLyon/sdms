/*=============================================================================
  Copyright (c) 2016 Jilles Steeve Dibangoye
==============================================================================*/
#pragma once

#include <cstddef>
#include <iostream>
#include <boost/bimap.hpp>

//!
//! \file     types.hpp
//! \author   Jilles S. Dibangoye
//! \brief    defining several types
//! \version  1.0
//! \date     12 Avril 2016
//!
//! This class provides basic type alias for dpomdp.
//!

//! \namespace  sdm
//!
//! Namespace grouping all tools required for sequential decision making.
namespace sdm
{

  template <class I, class T>
  std::shared_ptr<T> cast(const std::shared_ptr<I> &item)
  {
    return std::static_pointer_cast<T>(item);
  }

  namespace ptr
  {
    template <typename T>
    using shared = std::shared_ptr<T>;

    template <typename T>
    using unique = std::unique_ptr<T>;

    template <typename T>
    using weak = std::weak_ptr<T>;
  } // namespace ptr

  const size_t SUCCESS = 0;
  const size_t ERROR_IN_COMMAND_LINE = 1;
  const size_t ERROR_UNHANDLED_EXCEPTION = 2;

  typedef std::size_t size_t; // = uint32_t

  typedef unsigned short number; // = uint16_t

  typedef boost::bimaps::bimap<std::string, sdm::size_t> bimap;

  template <typename TItem>
  using bimap_item_index = boost::bimaps::bimap<TItem, sdm::size_t>;

  typedef typename bimap::value_type name2index;

  const number DEFAULT_OBSERVATION = std::numeric_limits<number>::max();

  enum TState_t
  {
    COMPRESSED,
    ONE_STEP_UNCOMPRESSED,
    FULLY_UNCOMPRESSED
  };

  /**
  * Enumerator for the types of statistics that can be plotted.
  */
  enum Metric
  {
    CUMULATIVE_REWARD_PER_STEP,
    CUMULATIVE_REWARD_PER_EPISODE,
    AVERAGE_EPISODE_REWARD,
    AVERAGE_EPISODE_VALUE,
    MEDIAN_EPISODE_REWARD,
    CUMULATIVE_STEPS_PER_EPISODE,
    STEPS_PER_EPISODE
  };

  /**
  * Enumerator for the types of statistics that can be recorded.
  */
  enum Statistic
  {
    MIN,
    MAX,
    MEAN,
    RANGE,
    VARIANCE,
    STANDARD_DEVIATION
  };

  enum TypeSoftmax
  {
    BELIEF,
    BEHAVIOR,
    OCCUPANCY
  };

  enum Criterion
  {
    COST_MIN,
    REW_MAX
  };

  enum TypeOfResolution
  {
    BigM,
    IloIfThenResolution
  };

  enum TypeSawtoothLinearProgram
  {
    PLAIN_SAWTOOTH_LINER_PROGRAMMING,
    RELAXED_SAWTOOTH_LINER_PROGRAMMING
  };

  enum
  {
    STANDARD,
    XML,
    JSON
  };

  //using boost::hash_combine
  template <class T>
  inline void hash_combine(std::size_t &seed, T const &v)
  {
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }

  template <>
  inline void hash_combine(std::size_t &seed, double const &v)
  {
    float tr_double = (float)v;
    sdm::hash_combine(seed, tr_double);
  }
} // namespace sdm

namespace std
{

  template <class T, class... Ts>
  struct is_any : std::disjunction<std::is_same<T, Ts>...>
  {
  };

} // namespace std