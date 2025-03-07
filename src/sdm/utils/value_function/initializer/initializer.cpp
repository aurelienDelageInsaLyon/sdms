#include <sdm/exception.hpp>
#include <sdm/utils/value_function/initializer/initializer.hpp>
#include <sdm/world/solvable_by_dp.hpp>
#include <sdm/utils/value_function/value_function_interface.hpp>
#include <sdm/utils/value_function/value_function.hpp>
#include <sdm/utils/value_function/qvalue_function.hpp>

namespace sdm
{

    // ******************
    // Value Initializer
    // ******************
    ValueInitializer::ValueInitializer(double v) : value(v) {}
    ValueInitializer::ValueInitializer(const std::shared_ptr<SolvableByDP> &, Config config)
        : ValueInitializer(config.get("value", 0)) {}

    void ValueInitializer::init(std::shared_ptr<ValueFunctionInterface> value_function)
    {
        if (value_function->isInfiniteHorizon())
        {
            value_function->initialize(this->value);
        }
        else
        {
            for (number t = 0; t < value_function->getHorizon(); t++)
            {
                value_function->initialize(this->value, t);
            }
            value_function->initialize(0, value_function->getHorizon());
        }
    }

    // ******************
    // Bound Initializer
    // ******************
    BoundInitializer::BoundInitializer() {}

    BoundInitializer::BoundInitializer(std::shared_ptr<SolvableByDP> world, double value) : value_(value), world_(world) {}

    void BoundInitializer::init(std::shared_ptr<ValueFunctionInterface> value_function)
    {
        if (value_function->isInfiniteHorizon())
        {
            value_function->initialize(this->computeValueInfiniteHorizon(value_function));
        }
        else
        {
            double tot = 0.0;
            value_function->initialize(tot, value_function->getHorizon());
            for (number t = value_function->getHorizon(); t > 0; t--)
            {
                tot = this->getValue(value_function, t - 1) + this->world_->getUnderlyingProblem()->getDiscount(t) * tot;
                value_function->initialize(tot, t - 1);
            }
        }
    }

    double BoundInitializer::getValue(std::shared_ptr<ValueFunctionInterface>, number t)
    {
        //std::cout << "\n callback_value : " << this->callback_value;
        return (this->callback_value == nullptr) ? this->value_ : ((*world_->getUnderlyingProblem()).*callback_value)(t);
    }

    double BoundInitializer::computeValueInfiniteHorizon(std::shared_ptr<ValueFunctionInterface> value_function)
    {
        auto underlying_problem = world_->getUnderlyingProblem();

        // long l = log(1 - world_->getDiscount()) * this->error_ / this->reward_->getMaxReward();
        number t = 0;
        double value = this->getValue(value_function, t), factor = 1.;
        do
        {
            factor *= underlying_problem->getDiscount(t);
            value += factor * this->getValue(value_function, t + 1);
            t++;
        } while (factor > 1.e-10);
        value = floor(value) + (value > 0); // value = -2.99 --> floor(-2.99) + 0 = -3.0 and 2.99 --> floor(2.99) + 1 = 2 + 1 = 3.0
        return value;
    }

    // ******************
    // Min Initializer
    // ******************
    MinInitializer::MinInitializer(std::shared_ptr<SolvableByDP> world, Config)
    {
        this->world_ = world;
    }

    void MinInitializer::init(std::shared_ptr<ValueFunctionInterface> value_function)
    {
        // this->world_->getUnderlyingProblem()->getMinReward();
        this->callback_value = &MDPInterface::getMinReward;
        BoundInitializer::init(value_function);
    }

    // ******************
    // Max Initializer
    // ******************
    MaxInitializer::MaxInitializer(std::shared_ptr<SolvableByDP> world, Config)
    {
        this->world_ = world;
    }

    void MaxInitializer::init(std::shared_ptr<ValueFunctionInterface> value_function)
    {
        // this->value_ = this->world_->getUnderlyingProblem()->getMaxReward(t);
        this->callback_value = &MDPInterface::getMaxReward;
        BoundInitializer::init(value_function);
    }

} // namespace sdm