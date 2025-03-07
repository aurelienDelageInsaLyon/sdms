#include <sdm/core/state/belief_state.hpp>
#include <sdm/utils/struct/graph.hpp>
#include <sdm/world/registry.hpp>
#include <sdm/core/state/private_br_occupancy_state.hpp>
namespace sdm
{

    template <class TBelief>
    BaseBeliefMDP<TBelief>::BaseBeliefMDP()
    {
    }

    template <class TBelief>
    BaseBeliefMDP<TBelief>::BaseBeliefMDP(const std::shared_ptr<POMDPInterface> &pomdp, int batch_size, bool store_states, bool store_actions)
        : SolvableByMDP(pomdp), pomdp(pomdp), batch_size_(batch_size), store_states_(store_states), store_actions_(store_actions)
    {
        auto initial_state = std::make_shared<TBelief>();

        // For each state at t=0:
        for (const auto &state : *pomdp->getStateSpace(0))
        {
            // Get the state's probability
            double probability = pomdp->getStartDistribution()->getProbability(state->toState(), nullptr);
            // If the state is possible:
            if (probability > 0)
            {
                // Set the probability
                initial_state->setProbability(state->toState(), probability);
            }
        }

        initial_state->finalize();
        this->initial_state_ = initial_state;

        this->mdp_graph_ = std::make_shared<Graph<std::shared_ptr<State>, Pair<std::shared_ptr<Action>, std::shared_ptr<Observation>>>>();
        this->getMDPGraph()->addNode(this->initial_state_);

        this->state_space_[*initial_state] = this->initial_state_;

        this->reward_graph_ = std::make_shared<Graph<double, Pair<std::shared_ptr<State>, std::shared_ptr<Action>>>>();
        this->reward_graph_->addNode(0.0);
    }

    template <class TBelief>
    BaseBeliefMDP<TBelief>::BaseBeliefMDP(const std::shared_ptr<POMDPInterface> &pomdp, Config config)
        : BaseBeliefMDP<TBelief>(pomdp, config.get("batch_size", 0))
    {
    }

    template <class TBelief>
    BaseBeliefMDP<TBelief>::BaseBeliefMDP(Config config)
        : BaseBeliefMDP<TBelief>(std::dynamic_pointer_cast<POMDPInterface>(sdm::world::createFromConfig(config)), config.get("batch_size", 0))
    {
    }

    template <class TBelief>
    BaseBeliefMDP<TBelief>::~BaseBeliefMDP()
    {
    }

    template <class TBelief>
    std::shared_ptr<Space> BaseBeliefMDP<TBelief>::getObservationSpaceAt(const std::shared_ptr<State> &, const std::shared_ptr<Action> &, number t)
    {
        return this->pomdp->getObservationSpace(t);
    }

    template <class TBelief>
    std::shared_ptr<Space> BaseBeliefMDP<TBelief>::getActionSpaceAt(const std::shared_ptr<State> &, number t)
    {
        return this->mdp->getActionSpace(t);
    }

    template <class TBelief>
    std::shared_ptr<POMDPInterface> BaseBeliefMDP<TBelief>::getUnderlyingPOMDP() const
    {
        return pomdp;
    }

    template <class TBelief>
    std::shared_ptr<BeliefMDPInterface> BaseBeliefMDP<TBelief>::getUnderlyingBeliefMDP()
    {
        return std::dynamic_pointer_cast<BeliefMDPInterface>(this->shared_from_this());
    }

    template <class TBelief>
    Pair<std::shared_ptr<State>, double> BaseBeliefMDP<TBelief>::computeNextStateAndProbability(const std::shared_ptr<State> &belief, const std::shared_ptr<Action> &action, const std::shared_ptr<Observation> &observation, number t)
    {
        //std::cout << "\n blabla i'm here\n" << std::flush;
        //std::cout << "\n belief : " << belief;
        //std::exit(1);

        // Compute next state
        if (this->batch_size_ == 0)
        {
            //std::cout << " normally i should call next "<< std::flush;
            //std::cout << belief->str() << std::flush << std::endl;

            return belief->next(this->mdp, action, observation, t);
            //std::cout << " end calling next " << std::flush << std::endl;

        }
        else
        {
            //std::cout << " weird, normally i should call next " << std::flush << std::endl;
            return this->computeSampledNextState(belief, action, observation, t);
        }
    }

    template <class TBelief>
    Pair<std::shared_ptr<State>, double> BaseBeliefMDP<TBelief>::computeSampledNextState(const std::shared_ptr<State> &belief, const std::shared_ptr<Action> &action, const std::shared_ptr<Observation> &observation, number)
    {
        // Create next belief.
        auto next_belief = std::make_shared<TBelief>();
        //
        std::shared_ptr<State> true_state = this->mdp->getInternalState();
        //
        int k = 0;
        // while
        while (k < this->batch_size_)
        {
            std::shared_ptr<State> state = belief->toBelief()->sampleState();
            this->mdp->setInternalState(state);
            auto [possible_observation, rewards, is_done] = this->mdp->step(action, false);
            if (observation == possible_observation)
            {
                std::shared_ptr<State> next_state = this->mdp->getInternalState();
                next_belief->addProbability(next_state, 1.0 / double(this->batch_size_));
                k++;
            }
        }
        //
        this->mdp->setInternalState(true_state);

        // Finalize belief
        next_belief->finalize();

        // Compute the coefficient of normalization (eta)
        double eta = next_belief->norm_1();

        // Normalize to belief
        next_belief->normalizeBelief(eta);

        // Return next belief.
        return std::make_pair(next_belief, eta);
    }

    // ------------------------------------------------------
    // FONCTIONS COMMON TO ALL BELIEFMDP / OCCUPANCYMDP
    // ------------------------------------------------------

    template <class TBelief>
    Pair<std::shared_ptr<State>, double> BaseBeliefMDP<TBelief>::getNextStateAndProba(const std::shared_ptr<State> &belief, const std::shared_ptr<Action> &action, const std::shared_ptr<Observation> &observation, number t)
    {
        //std::cout << "\n baseBeliefMdp : action : " << action->str() << " and observation : " << observation->str()<< std::flush;
        auto action_observation = std::make_pair(action, observation);

        // If we store data in the graph
        if (this->store_states_ && this->store_actions_)
        {
            // Get the successor
            auto successor = this->getMDPGraph()->getSuccessor(belief, action_observation);
            // auto successor = this->mdp_graph_->getSuccessor(belief, action_observation);

            // If already in the successor list
            if (successor != nullptr)
            {
                // Return the successor node
                return {successor->getData(), this->transition_probability.at(belief).at(action).at(observation)};
            }
            else
            {
                //std::cout << "trying to compute next state and proba" << std::flush;

                // Build next belief and proba
                auto [computed_next_belief, next_belief_probability] = this->computeNextStateAndProbability(belief, action, observation, t);

                // Store the probability of next belief
                this->transition_probability[belief][action][observation] = next_belief_probability;

                // Check if the next belief is already in the graph
                //std::cout << "\n computed next belief : " << typeid(computed_next_belief).name();
                auto bla = std::dynamic_pointer_cast<PrivateBrOccupancyState>(computed_next_belief);
                //std::cout << "\n is bla null?"<< bla << std::flush;
                //std::exit(1);
                TBelief b = *std::dynamic_pointer_cast<TBelief>(computed_next_belief);
                //std::cout << "\n is b null?" << b<<std::flush;
                //std::exit(1);
                if (this->state_space_.find(b) == this->state_space_.end())
                {
                    // Add the belief in the space of beliefs
                    this->state_space_.emplace(b, computed_next_belief);
                }

                // Get the next belief
                auto next_belief = this->state_space_.at(b);

                // Add the sucessor in the list of successors
                this->getMDPGraph()->addSuccessor(belief, action_observation, next_belief);

                return {next_belief, next_belief_probability};
            }
        }
        else if (this->store_states_)
        {
            // Return next belief without storing its value in the graph
            auto [computed_next_belief, proba_belief] = this->computeNextStateAndProbability(belief, action, observation, t);
            TBelief b = *std::dynamic_pointer_cast<TBelief>(computed_next_belief);
            if (this->state_space_.find(b) == this->state_space_.end())
            {
                // Add the belief in the space of beliefs
                this->state_space_.emplace(b, computed_next_belief);
            }
            return {this->state_space_.at(b), proba_belief};
        }
        else
        {
            // Return next belief without storing its value in the graph
            return this->computeNextStateAndProbability(belief, action, observation, t);
        }
    }

    template <class TBelief>
    Pair<std::shared_ptr<State>, double> BaseBeliefMDP<TBelief>::getNextState(const std::shared_ptr<State> &belief, const std::shared_ptr<Action> &action, const std::shared_ptr<Observation> &observation, number t)
    {
        bool skip_compute_next_state = (this->isFiniteHorizon() && ((t + 1) > this->getHorizon()));
        // Compute next state (if required)
        return (skip_compute_next_state) ? Pair<std::shared_ptr<State>, double>({nullptr, 1.}) : this->getNextStateAndProba(belief, action, observation, t);
    }

    template <class TBelief>
    double BaseBeliefMDP<TBelief>::getReward(const std::shared_ptr<State> &belief, const std::shared_ptr<Action> &action, number t)
    {
        //std::cout<< "\n i'm in get reward from belief_mdp !" << std::flush << std::endl;
        //std::exit(1);
        double reward = 0.;

        if (this->store_states_ && this->store_actions_)
        {
            auto belief_action = std::make_pair(belief, action);
            auto successor = this->reward_graph_->getSuccessor(0.0, belief_action);
            if (successor != nullptr)
            {
                // Return the successor node
                reward = successor->getData();
            }
            else
            {
                // Return the reward
                reward = belief->getReward(this->mdp, action, t);
                if (this->store_states_ && this->store_actions_)
                    this->reward_graph_->addSuccessor(0.0, belief_action, reward);
            }
        }
        else
        {
            reward = belief->getReward(this->mdp, action, t);
        }
        //std::cout<< "\n i'm leaving get reward from belief_mdp !" << std::flush << std::endl;
        return reward;
    }

    template <class TBelief>
    double BaseBeliefMDP<TBelief>::getObservationProbability(const std::shared_ptr<State> &belief, const std::shared_ptr<Action> &action, const std::shared_ptr<State> &, const std::shared_ptr<Observation> &observation, number) const
    {
        return this->transition_probability.at(belief).at(action).at(observation);
    }

    template <class TBelief>
    double BaseBeliefMDP<TBelief>::getExpectedNextValue(const std::shared_ptr<ValueFunction> &value_function, const std::shared_ptr<State> &belief, const std::shared_ptr<Action> &action, number t)
    {
        double exp_next_v = 0.0;
        // For all observations from the controller point of view
        auto accessible_observation_space = this->getObservationSpaceAt(belief, action, t);
        for (const auto &observation : *accessible_observation_space)
        {
            // Compute next state
            auto [next_state, state_transition_proba] = this->getNextState(belief, action, observation->toObservation(), t);

            // Update the next expected value at the next state
            exp_next_v += state_transition_proba * value_function->getValueAt(next_state, t + 1);
        }
        return exp_next_v;
    }

    // ------------------------------------------------------
    // FONCTIONS REQUIRED IN LEARNING ALGORITHMS
    // ------------------------------------------------------

    template <class TBelief>
    std::shared_ptr<State> BaseBeliefMDP<TBelief>::reset()
    {
        this->step_ = 0;
        this->current_state_ = this->initial_state_;
        return this->current_state_;
    }

    template <class TBelief>
    std::tuple<std::shared_ptr<State>, std::vector<double>, bool> BaseBeliefMDP<TBelief>::step(std::shared_ptr<Action> action)
    {
        // Compute reward
        double belief_reward = this->getReward(this->current_state_, action, this->step_);

        double cumul = 0.0, prob = 0.0;
        std::shared_ptr<State> candidate_state = nullptr;

        // Get a random number between 0 and 1
        double epsilon = std::rand() / (double(RAND_MAX));


        // Go over all observations of the lower-level agent
        auto accessible_observation_space = this->getObservationSpaceAt(this->current_state_, action, this->step_);
        for (auto observation : *accessible_observation_space)
        {
            std::tie(candidate_state, prob) = this->getNextStateAndProba(this->current_state_, action, observation->toObservation(), this->step_);

            cumul += prob;
            if (epsilon < cumul)
            {
                this->step_++;
                this->current_state_ = candidate_state;
                break;
            }
        }
        bool is_done = (this->getHorizon() > 0) ? (this->step_ >= this->getHorizon()) : false;
        return std::make_tuple(this->current_state_, std::vector<double>(this->mdp->getNumAgents(), belief_reward), is_done);
    }

    template <class TBelief>
    std::shared_ptr<Action> BaseBeliefMDP<TBelief>::getRandomAction(const std::shared_ptr<State> &, number t)
    {
        return this->mdp->getActionSpace(t)->sample()->toAction();
    }

    // ------------------------------------------------------
    // ACCESSORS OF SOME SPECIAL DATA COMMON TO ALL BELIEF MDP
    // ------------------------------------------------------

    template <class TBelief>
    std::shared_ptr<Graph<std::shared_ptr<State>, Pair<std::shared_ptr<Action>, std::shared_ptr<Observation>>>> BaseBeliefMDP<TBelief>::getMDPGraph()
    {
        return this->mdp_graph_;
    }

    template <class TBelief>
    std::vector<std::shared_ptr<State>> BaseBeliefMDP<TBelief>::getStoredStates() const
    {
        std::vector<std::shared_ptr<State>> list_states;
        for (const auto &state : this->state_space_)
        {
            list_states.push_back(state.second);
        }
        return list_states;
    }

} // namespace sdm
