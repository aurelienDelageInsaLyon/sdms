#include <sdm/utils/value_function/value_function.hpp>
#include <sdm/utils/value_function/qvalue_function.hpp>
#include <sdm/utils/linear_algebra/mapped_vector.hpp>
#include <sdm/utils/linear_algebra/vector_interface.hpp>

namespace sdm
{
    ValueFunctionInterface::ValueFunctionInterface()
    {
    }

    ValueFunctionInterface::ValueFunctionInterface(const std::shared_ptr<SolvableByDP> &world,
                                                   const std::shared_ptr<Initializer> &initializer,
                                                   const std::shared_ptr<ActionSelectionInterface> &action_selection)
        : horizon_(world->getHorizon()), world_(world), initializer_(initializer), action_selection_(action_selection)
    {
    }

    // ValueFunctionInterface::ValueFunctionInterface(const std::shared_ptr<SolvableByDP> &world, Config configs) : horizon_(world->getHorizon()), world_(world)
    // {
    //     this->initializer_ = sdm::initializer::make(configs.get("initializer", "Zero"), world);
    //     this->action_selection_ = sdm::action_selection::make(configs.get("action_selection", "Exhaustive"), world);
    // }

    ValueFunctionInterface::~ValueFunctionInterface() {}

    void ValueFunctionInterface::initialize()
    {
        std::cout << "trying to initialize value function in value_function_interface, i'm exiting";
        std::exit(1);
        this->getInitializer()->init(this->getptr());
    }

    std::shared_ptr<Action> ValueFunctionInterface::getGreedyAction(const std::shared_ptr<State> &state, number t)
    {
        return this->getGreedyActionAndValue(state, t).first;
    }

    Pair<std::shared_ptr<Action>, double> ValueFunctionInterface::getGreedyActionAndValue(const std::shared_ptr<State> &state, number t)
    {
        auto action_selector = this->getActionSelection();
        if (!action_selector)
        {
            throw sdm::exception::Exception("Action selector not set. Please use the method <ValueFunction>::setActionSelector.");
        }
        return this->getActionSelection()->getGreedyActionAndValue(this->getptr(), state, t);
    }

    std::shared_ptr<SolvableByDP> ValueFunctionInterface::getWorld() const
    {
        return this->world_;
    }

    std::shared_ptr<ValueFunctionInterface> ValueFunctionInterface::getptr()
    {
        return shared_from_this();
    }

    void ValueFunctionInterface::save(std::string)
    {
        throw exception::Exception("This class cannot be saved.");
    }

    void ValueFunctionInterface::load(std::string)
    {
        throw exception::Exception("This class cannot be load.");
    }

    number ValueFunctionInterface::getHorizon() const
    {
        return this->horizon_;
    }

    bool ValueFunctionInterface::isFiniteHorizon() const
    {
        return (this->getHorizon() > 0);
    }

    bool ValueFunctionInterface::isInfiniteHorizon() const
    {
        return (!isFiniteHorizon());
    }

    std::shared_ptr<Initializer> ValueFunctionInterface::getInitializer() const
    {
        return this->initializer_;
    }

    std::shared_ptr<ActionSelectionInterface> ValueFunctionInterface::getActionSelection() const
    {
        return this->action_selection_;
    }

    std::shared_ptr<ValueFunction> ValueFunctionInterface::toValueFunction()
    {
        return std::dynamic_pointer_cast<ValueFunction>(this->getptr());
    }

    std::shared_ptr<QValueFunction> ValueFunctionInterface::toQValueFunction()
    {
        return std::dynamic_pointer_cast<QValueFunction>(this->getptr());
    }

} // namespace sdm