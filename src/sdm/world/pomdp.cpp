#include <sdm/world/pomdp.hpp>

namespace sdm
{
    POMDP::POMDP() {}

    POMDP::POMDP(const std::shared_ptr<Space> &state_space,
                 const std::shared_ptr<Space> &action_space,
                 const std::shared_ptr<Space> &observation_space,
                 const std::shared_ptr<RewardModel> &reward,
                 const std::shared_ptr<StateDynamicsInterface> &state_dynamics,
                 const std::shared_ptr<ObservationDynamicsInterface> &observation_dynamics,
                 const std::shared_ptr<Distribution<std::shared_ptr<State>>> &start_distrib,
                 number horizon,
                 double discount,
                 Criterion criterion) : MDP(state_space, action_space, reward, state_dynamics, start_distrib, horizon, discount, criterion),
                                        observation_space_(observation_space),
                                        observation_dynamics_(observation_dynamics)
    {
    }

    std::shared_ptr<Space> POMDP::getObservationSpace(number) const
    {
        return this->observation_space_;
    }

    std::set<std::shared_ptr<Observation>> POMDP::getReachableObservations(const std::shared_ptr<State> &state, const std::shared_ptr<Action> &action, const std::shared_ptr<State> &next_state, number t) const
    {
        return this->observation_dynamics_->getReachableObservations(state, action, next_state, t);
    }

    double POMDP::getObservationProbability(const std::shared_ptr<State> &state, const std::shared_ptr<Action> &action, const std::shared_ptr<State> &next_state, const std::shared_ptr<Observation> &observation, number t) const
    {   
        //std::cout << "\n obs probabilities : " << this->observation_dynamics_->str();
        return this->observation_dynamics_->getObservationProbability(state, action, next_state, observation, t);
    }

    double POMDP::getDynamics(const std::shared_ptr<State> &state, const std::shared_ptr<Action> &action, const std::shared_ptr<State> &next_state, const std::shared_ptr<Observation> &observation, number t) const
    {   
        double a  = this->getTransitionProbability(state, action, next_state, t) * this->getObservationProbability(state, action, next_state, observation, t);
        //std::cout << "old_state : " << state->str() << " new state : " << next_state->str();
        //std::cout << "value : " << a;
        //std::cout << "\n action : " << action->str();
        //std::cout << "\n observation : " << observation->str();
        //std::exit(1);
        return this->getTransitionProbability(state, action, next_state, t) * this->getObservationProbability(state, action, next_state, observation, t);
    }

    std::shared_ptr<ObservationDynamicsInterface> POMDP::getObservationDynamics() const
    {
        return this->observation_dynamics_;
    }

    std::shared_ptr<Observation> POMDP::sampleNextObservation(const std::shared_ptr<State> &state, const std::shared_ptr<Action> &action, number t)
    {
        MDP::sampleNextObservation(state, action, t);
        return this->observation_dynamics_->getNextObservationDistribution(state, action, this->getInternalState(), t)->sample();
    }
}