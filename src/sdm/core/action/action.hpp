/**
 * @file action.hpp
 * @author David Albert (david.albert@insa-lyon.fr)
 * @brief The file for interface action class
 * @version 0.1
 * @date 11/12/2020
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#pragma once

#include <sdm/types.hpp>
#include <sdm/core/item.hpp>

namespace sdm
{
    class DecisionRule;

    // Action from the P.O.V. of the central agent.
    class Action : public Item
    {
    public:
        virtual ~Action() {}
        virtual std::string str() const = 0;

        virtual std::shared_ptr<DecisionRule> toDecisionRule();
        virtual TypeAction getTypeAction() const;
    };
} // namespace sdm