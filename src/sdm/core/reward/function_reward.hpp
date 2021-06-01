#include <sdm/core/state/state.hpp>
#include <sdm/core/action/action.hpp>
#include <sdm/core/reward.hpp>

namespace sdm
{
    /**
     * @brief This class provides getter and setter methods for the reward model.
     */
    class FunctionReward : public BaseReward
    {
    public:
        FunctionReward();
        FunctionReward(const std::string &expression);
        FunctionReward(const FunctionReward &copy);

        double getReward(const std::shared_ptr<State> &state, const std::shared_ptr<Action> &action, number t) const;

        double getMaxReward(number t) const;
        double getMinReward(number t) const;

        void setMaxReward(number t, double max_value);
        void setMinReward(number t, double min_value);

    protected:
        std::string expression;
        double max = -999999, min = +999999;
    };
}