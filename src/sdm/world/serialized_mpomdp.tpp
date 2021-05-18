#include <sdm/world/serialized_mpomdp.hpp>
#include <sdm/parser/parser.hpp>

namespace sdm
{

    SerializedMPOMDP::SerializedMPOMDP()
    {
    }

    SerializedMPOMDP::SerializedMPOMDP(std::shared_ptr<DiscreteDecPOMDP> underlying_decpomdp) : decpomdp_(underlying_decpomdp)
    {
        this->mmdp_ = this->decpomdp_->toMMDP();

        this->setPlanningHorizon(this->mmdp_->getPlanningHorizon());
        this->setDiscount(this->mmdp_->getDiscount());
        this->setActionSpace(this->mmdp_->getActionSpace());

        this->setStartDistrib(decpomdp_->getStartDistrib());

        this->createInitSerializedStateSpace();
        this->createInitReachableStateSpace();

        this->setObsSpace();
        this->setReachableObsSpace();
    }

    SerializedMPOMDP::SerializedMPOMDP(std::string filename) : SerializedMPOMDP(std::make_shared<DiscreteDecPOMDP>(filename))
    {
    }

    void SerializedMPOMDP::setObsSpace()
    {
        std::vector<number> v;
        std::vector<std::shared_ptr<DiscreteSpace<number>>> all_observation_space;

        //Creation of Individual Serial Observation Space
        for (number ag_id = 0; ag_id < this->getNumAgents(); ag_id++)
        {
            auto obs = this->decpomdp_->getObsSpace()->getSpace(ag_id)->getAll();
            std::vector<number> observation = obs;
            observation.push_back(sdm::DEFAULT_OBSERVATION);
            v.push_back(sdm::DEFAULT_OBSERVATION);

            auto serial_ind_space = std::make_shared<DiscreteSpace<number>>(observation);
            all_observation_space.push_back(serial_ind_space);
        }
        // Creation of all serial state
        this->serialized_observation_space_ = std::make_shared<MultiDiscreteSpace<number>>(all_observation_space);

        // Creation of the empty observation
        this->empty_serial_observation = Joint<number>(v);

        // Creation of dynamics and observation_probability
        // Go over serial states 
        for (const auto serialized_state : this->serialized_state_space_->getAll())
        { 
            this->dynamics.emplace(serialized_state,std::unordered_map<number, std::unordered_map<Joint<number>, std::unordered_map<SerializedState,double>>>());
            this->observation_probability.emplace(serialized_state,std::unordered_map<number, std::unordered_map<Joint<number>, std::unordered_map<SerializedState,double>>>());

            // Go over serial action 
            for (const auto serial_action : this->getActionSpace(serialized_state.getCurrentAgentId())->getAll())
            {
                this->dynamics[serialized_state].emplace(serial_action,std::unordered_map<Joint<number>, std::unordered_map<SerializedState,double>>());
                this->observation_probability[serialized_state].emplace(serial_action,std::unordered_map<Joint<number>, std::unordered_map<SerializedState,double>>());

                // Go over joint_obs 
                for(const auto joint_obs: this->serialized_observation_space_->getAll())
                {                    
                    this->dynamics[serialized_state][serial_action].emplace(joint_obs,std::unordered_map<SerializedState,double>());
                    this->observation_probability[serialized_state][serial_action].emplace(joint_obs,std::unordered_map<SerializedState,double>());

                    // Go over serial states  
                    for (const auto next_serialized_state : this->serialized_state_space_->getAll())
                    {
                        //Update action
                        std::vector<number> all_action(serialized_state.getAction());
                        all_action.push_back(serial_action);

                        double proba_dynamics =0, proba_observation_probability = 0;

                        // Determine dynamics and observation_probability 

                        // If not the new agent
                        if (next_serialized_state.getCurrentAgentId() != 0)
                        {
                            // If it is a intermediate agent, the probability is worth 1 only if : the joint_obs is the empty observation, 
                            // the currentand next hidden state are equal, the action of the next serial state is equal to the action of current serial state + serial action
                            proba_dynamics = (this->empty_serial_observation == joint_obs && serialized_state.getState() == next_serialized_state.getState() && next_serialized_state.getAction() == all_action) ? 1 : 0;
                            proba_observation_probability = proba_dynamics;
                        }
                        // If it is new agent, the obs is possible (not empty serial observation) and action corresponding to the number of agent 
                        else if( this->decpomdp_->getObsSpace()->contains(joint_obs) && all_action.size() == this->getNumAgents())
                        {
                            // The probability is the same of the decpomdp, if the condition are verified
                            proba_dynamics = this->decpomdp_->getObsDynamics()->getDynamics(serialized_state.getState(), this->decpomdp_->getActionSpace()->joint2single(Joint<number>(all_action)), this->decpomdp_->getObsSpace()->joint2single(joint_obs), next_serialized_state.getState());
                            proba_observation_probability = this->decpomdp_->getObsDynamics()->getObservationProbability(serialized_state.getState(), this->decpomdp_->getActionSpace()->joint2single(Joint<number>(all_action)), this->decpomdp_->getObsSpace()->joint2single(joint_obs), next_serialized_state.getState());
                        }
                        this->dynamics[serialized_state][serial_action][joint_obs].emplace(next_serialized_state,proba_dynamics);
                        this->observation_probability[serialized_state][serial_action][joint_obs].emplace(next_serialized_state,proba_observation_probability);
                    }
                }
            }
        }

    }

    void SerializedMPOMDP::setReachableObsSpace()
    {
        // Creation of all reachable Observation Space 

        // Go over serial states 
        for (const auto serialized_state : this->serialized_state_space_->getAll())
        {
            this->reachable_obs_state_space.emplace(serialized_state, std::unordered_map<number, std::unordered_map<SerializedState, std::set<Joint<number>>>>());
            
            // Go over serial action             
            for (const auto serial_action : this->getActionSpace(serialized_state.getCurrentAgentId())->getAll())
            {
                this->reachable_obs_state_space[serialized_state].emplace(serial_action, std::unordered_map<SerializedState, std::set<Joint<number>>>());

                // Go over joint_obs 
               for (const auto next_serialized_state : this->serialized_state_space_->getAll())
                {
                    //Update action
                    std::vector<number> all_action(serialized_state.getAction());
                    all_action.push_back(serial_action);
                    Joint<number> joint_action(all_action);

                    std::set<Joint<number>> all_obs;

                    // Insert the next reachable Observation State
                    if (next_serialized_state.getCurrentAgentId() == 0 && serialized_state.getCurrentAgentId() == this->getNumAgents() - 1)
                    {
                        try
                        {
                            for (const auto obs : this->decpomdp_->getReachableObservations(serialized_state.getState(), joint_action, next_serialized_state.getState()))
                            {
                                all_obs.insert(obs);
                            }
                        }
                        catch (const std::exception &e)
                        {
                            all_obs.insert(this->empty_serial_observation);
                        }
                    }
                    else
                    {
                        all_obs.insert(this->empty_serial_observation);
                    }
                    this->reachable_obs_state_space[serialized_state][serial_action].emplace(next_serialized_state, all_obs);
                }
            }
        }
    }

    const std::set<Joint<number>> &SerializedMPOMDP::getReachableObservations(const SerializedState serial_state, const number serial_action, const SerializedState next_serial_state) const
    {
        return this->reachable_obs_state_space.at(serial_state).at(serial_action).at(next_serial_state);
    }

    std::shared_ptr<SerializedMMDP> SerializedMPOMDP::toMDP()
    {
        return std::make_shared<SerializedMMDP>(this->mmdp_);
    }

    std::shared_ptr<SerializedBeliefMDP<SerializedBeliefState,number,Joint<number>>> SerializedMPOMDP::toBeliefMDP()
    {
        return std::make_shared<SerializedBeliefMDP<SerializedBeliefState,number,Joint<number>>>(this->getptr());
    }

    std::shared_ptr<MultiDiscreteSpace<number>> SerializedMPOMDP::getObsSpace() const
    {
        return this->serialized_observation_space_;
    }

    std::shared_ptr<DiscreteSpace<number>> SerializedMPOMDP::getObsSpaceAt(number ag_id) const
    {
        return this->decpomdp_->getObsSpace()->getSpace(ag_id);
    }

    double SerializedMPOMDP::getObservationProbability(const SerializedState serialized_state, const number action, const Joint<number> joint_obs, const SerializedState serialized_state_next) const
    {
       return this->observation_probability.at(serialized_state).at(action).at(joint_obs).at(serialized_state_next);
    }

    double SerializedMPOMDP::getDynamics(const SerializedState serialized_state, const number action, const Joint<number> joint_obs, const SerializedState serialized_state_next) const
    {
        return this->dynamics.at(serialized_state).at(action).at(joint_obs).at(serialized_state_next);
    }

    std::shared_ptr<SerializedMPOMDP>  SerializedMPOMDP::getptr()
    {
        return  std::static_pointer_cast<SerializedMPOMDP>(SerializedMMDPStructure::getptr());
    }


}