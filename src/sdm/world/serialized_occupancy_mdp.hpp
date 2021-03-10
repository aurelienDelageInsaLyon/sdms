#pragma once

#include <sdm/types.hpp>
#include <sdm/utils/linear_algebra/vector.hpp>
#include <sdm/core/space/discrete_space.hpp>
#include <sdm/world/solvable_by_hsvi.hpp>

namespace sdm
{

    class DiscreteDecPOMDP;

    /**
     * @brief 
     * 
     * @tparam oState 
     * @tparam oAction 
     */
    template <typename oState, typename oAction>
    class SerializedOccupancyMDP : public SolvableByHSVI<oState, oAction>
    {
    protected:
        std::shared_ptr<DiscreteDecPOMDP> dpomdp_;
        oState istate_;
        oState cstate_;

    public:
        using state_type = oState;
        using action_type = oAction;
        // using observation_type = oObservation;

        SerializedOccupancyMDP(std::shared_ptr<DiscreteDecPOMDP> underlying_dpomdp);
        SerializedOccupancyMDP(std::shared_ptr<DiscreteDecPOMDP> underlying_dpomdp, number hist_length);
        SerializedOccupancyMDP(std::string underlying_dpomdp);
        SerializedOccupancyMDP(std::string underlying_dpomdp, number hist_length);

        oState &getState();

        std::shared_ptr<Reward> getReward() const;
        double getDiscount() { return this->dpomdp_->getDiscount(); }
        void setDiscount(double discount) { return this->dpomdp_->setDiscount(discount); }

        std::shared_ptr<DiscreteSpace<oAction>> getActionSpaceAt(const oState &);
        double getReward(const oState &ostate, const oAction &oaction) const;
        oState getInitialState();
        double getExpectedNextValue(ValueFunction<oState, oAction> *value_function, const oState &ostate, const oAction &oaction, int t = 0) const;
        oState nextState(const oState &ostate, const oAction &oaction, int t = 0, HSVI<oState, oAction> *hsvi = nullptr) const;
    };
} // namespace sdm
#include <sdm/world/occupancy_mdp.tpp>