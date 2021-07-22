#include <sdm/utils/value_function/action_vf/action_sawtooth_lp.hpp>
#include <sdm/utils/value_function/tabular_value_function.hpp>
#include <sdm/core/state/interface/occupancy_state_interface.hpp>
#include <sdm/core/state/interface/serial_interface.hpp>

#include <sdm/world/base/mpomdp_interface.hpp>
#include <sdm/world/occupancy_mdp.hpp>


namespace sdm
{
    ActionVFSawtoothLP::ActionVFSawtoothLP() {}
    ActionVFSawtoothLP::ActionVFSawtoothLP(const std::shared_ptr<SolvableByHSVI>& world,TypeOfResolution current_type_of_resolution, number bigM_value, TypeSawtoothLinearProgram type_sawtooth_resolution) : 
        ActionVFBase(world),DecentralizedLP(world) , current_type_of_resolution_(current_type_of_resolution), csawtooth_lp_(type_sawtooth_resolution)
    {
        // If the problem is relaxed, the Big have to be equal to 0
        switch (type_sawtooth_resolution)
        {
        case TypeSawtoothLinearProgram::RELAXED_SAWTOOTH_LINER_PROGRAMMING :
            this->bigM_value_ = 0;
            break;
        
        default:
            this->bigM_value_ = bigM_value;
            break;
        }
    }

    std::shared_ptr<Action> ActionVFSawtoothLP::selectBestAction(const std::shared_ptr<ValueFunction>& vf, const std::shared_ptr<State>& state, number t)
    {
        // Determine the type of Linear Program to use
        switch (this->csawtooth_lp_)
        {
        case TypeSawtoothLinearProgram::PLAIN_SAWTOOTH_LINER_PROGRAMMING:
            return this->createFullSawtooth(vf,state, t);
        case TypeSawtoothLinearProgram::RELAXED_SAWTOOTH_LINER_PROGRAMMING:
            return this->createRelaxedSawtooth(vf,state, t);
        default:
            return this->createFullSawtooth(vf,state, t);
        }
    }

    std::shared_ptr<Action> ActionVFSawtoothLP::createRelaxedSawtooth(const std::shared_ptr<ValueFunction>& vf,const std::shared_ptr<State> &state, number t)
    {
        std::shared_ptr<Action> best_action;
        double min_value = std::numeric_limits<double>::max(), value_tmp;
        
        if (vf->getSupport(t + 1).empty())
        {
            // Resolution of the problem when the support of Point Set is empty
            this->representation = {std::make_shared<MappedVector<std::shared_ptr<State>,double>>()};
            best_action = this->createLP(vf,state, t).first;
        }
        else
        {
            // For the Relaxation version of Sawtooth, we go over all element in the Point Set
            for (const auto &point : std::static_pointer_cast<TabularValueFunction>(vf)->getRepresentation(t + 1))
            {
                auto vector = std::make_shared<MappedVector<std::shared_ptr<State>,double>>();
                vector->setValueAt(point.first,point.second);

                this->representation = vector;

                auto [action,value] = this->createLP(vf,state, t);

                // We take the best action with the minimum value
                if (min_value > value)
                {
                    min_value = value;
                    best_action = action;
                }
            }
            // Verification of the Relaxation Contraint.
            if(min_value > (value_tmp = vf->getValueAt(state,t)))
            {
                // If the contraint is not verified , we used the decision rule previously stocked
                best_action = this->state_linked_to_decision_rule.at(state);
                min_value = value_tmp;
            }

        }
        // Save the best action associed to a state
        this->state_linked_to_decision_rule[state] = best_action;
        return best_action;
    }

    std::shared_ptr<Action> ActionVFSawtoothLP::createFullSawtooth(const std::shared_ptr<ValueFunction>&vf,const std::shared_ptr<State> &state, number t)
    {
        // For the Full version of Sawtooth, wo over all the Point Set
        this->representation = std::make_shared<MappedVector<std::shared_ptr<State>,double>>(std::static_pointer_cast<TabularValueFunction>(vf)->getRepresentation(t + 1));        
        return this->createLP(vf,state, t).first;
    }

    void ActionVFSawtoothLP::createVariables(const std::shared_ptr<ValueFunction>&vf,const std::shared_ptr<State> &state, IloEnv &env, IloNumVarArray &var,number &index, number t)
    {
        try
        {
            //<! tracking variables
            std::string VarName;

            this->variables.clear();

            //<! 0.b Build variables v_0 = objective variable!
            VarName = this->getVarNameWeight(0);
            var.add(IloNumVar(env, -IloInfinity, 0.0, VarName.c_str()));
            this->setNumber(VarName, index++);

            //<! Define variables \omega_k(x',o')

            // Go over all Point Set in t+1
            for (const auto &element_state_AND_upper_bound : *this->representation)
            {
                const auto &next_one_step_uncompressed_occupancy_state = element_state_AND_upper_bound.first->toOccupancyState()->getOneStepUncompressedOccupancy();

                // Go over all Joint History Next
                for (const auto &next_joint_history : next_one_step_uncompressed_occupancy_state->getJointHistories())
                {
                    // Go over all Hidden State in the next one step uncomppresed occupancy state
                    for(const auto next_hidden_state : next_one_step_uncompressed_occupancy_state->getBeliefAt(next_joint_history)->getStates())
                    {
                        // <! \omega_k(x',o')
                        VarName = this->getVarNameWeightedStateJointHistory(next_one_step_uncompressed_occupancy_state, next_hidden_state, next_joint_history);
                        var.add(IloBoolVar(env, 0, 1, VarName.c_str()));
                        this->setNumber(VarName, index++);
                    }
                }
            }

            // Create Decentralized Variables
            this->createDecentralizedVariables(vf,state, env, var, index, t);
        }
        catch (const std::exception &exc)
        {
            std::cerr << "SawtoothLPBackup::createVariables(..) exception caught: " << exc.what() << std::endl;
            exit(-1);
        }
    }

    double ActionVFSawtoothLP::getQValueRelaxation(const std::shared_ptr<ValueFunction>&vf,const std::shared_ptr<State> &state, const std::shared_ptr<JointHistoryInterface>& joint_history, const std::shared_ptr<Action>& action, number t)
    {
        // \sum_{o} a(u|o) \sum_{x} s(x,o) * Q_MDP(x,u)
        double weight = 0.0;
        try
        {
            auto compressed_occupancy_state = state->toOccupancyState();

            // Relaxation of the problem
            auto relaxation = std::static_pointer_cast<RelaxedValueFunction>(vf->getInitFunction());

            auto belief = compressed_occupancy_state->getBeliefAt(joint_history);
            weight = compressed_occupancy_state->getProbability(joint_history) * relaxation->operator()(std::make_pair(belief, action), t);
        }
        catch(const std::exception &exc)
        {
            std::cerr << "SawtoothLPBackup::getQValueRelaxation(..) exception caught: " << exc.what() << std::endl;
            exit(-1);
        }
        return weight;
    }
    
    void ActionVFSawtoothLP::createConstraints(const std::shared_ptr<ValueFunction>&vf,const std::shared_ptr<State>&state, IloEnv &env, IloModel &model, IloRangeArray &con, IloNumVarArray &var, number &index, number t)
    {
        assert(vf->getInitFunction() != nullptr);

        //<!  Build sawtooth constraints v - \sum_{u} a(u|o) * Q(k,s,o,u,y,z, diff, t  ) + \omega_k(y,<o,z>)*M <= M,  \forall k, y,<o,z>
        //<!  Build sawtooth constraints  Q(k,s,o,u,y,z, diff, t ) = (v_k - V_k) \frac{\sum_{x} s(x,o) * p(x,u,z,y)}}{s_k(y,<o,z>)},  \forall a(u|o)

       try{
            number recover = 0;

            auto compressed_occupancy_state = state->toOccupancyState();

            // Go over all points in the point set at t+1
            for (const auto &element_state_AND_upper_bound : *this->representation)
            {
                const auto &next_one_step_uncompressed_occupancy_state = element_state_AND_upper_bound.first->toOccupancyState()->getOneStepUncompressedOccupancy();
                
                // Compute the difference i.e. (v_k - V_k)
                double current_upper_bound = element_state_AND_upper_bound.second;
                double initial_upper_bound = vf->getInitFunction()->operator()(next_one_step_uncompressed_occupancy_state, t + 1);
                double difference = current_upper_bound - initial_upper_bound;

                // Go over all joint histories in over the support of next_one_step_uncompressed_occupancy_state
                for (const auto &next_joint_history : next_one_step_uncompressed_occupancy_state->getJointHistories())
                {
                    // Go over all Hidden State in the Belief for a precise Joint History
                    for(const auto &next_hidden_state : next_one_step_uncompressed_occupancy_state->getBeliefAt(next_joint_history)->getStates())
                    {
                        double probability = next_one_step_uncompressed_occupancy_state->getProbability(next_joint_history,next_hidden_state);

                        auto next_joint_observation = this->determineNextJointObservation(compressed_occupancy_state,next_joint_history);

                        // We search for the joint_history which allow us to obtain the current next_joint_history conditionning to the next joint observation
                        for(const auto &joint_history : this->determineJointHistory(compressed_occupancy_state,next_joint_history,next_joint_observation))
                        {
                            switch (this->current_type_of_resolution_)
                            {
                            case TypeOfResolution::BigM:
                                // this->createSawtoothBigM(compressed_occupancy_state,joint_history,next_hidden_state,next_joint_observation,next_joint_history,next_one_step_uncompressed_occupancy_state,probability,difference,env,con,var,index, t);
                                break;
                            case TypeOfResolution::IloIfThenResolution:
                                this->createSawtoothIloIfThen(vf,compressed_occupancy_state,joint_history,next_hidden_state,next_joint_observation,next_joint_history,next_one_step_uncompressed_occupancy_state,probability,difference,env,model,var, t);
                                break;
                            }
                        }
                    }
                }

                // Build constraint \sum{x',o'} \omega_k(x',o') = 1
                con.add(IloRange(env, 1.0, 1.0));

                // Go over all joint histories in over the support of next_one_step_uncompressed_occupancy_state
                for (const auto &next_joint_history : next_one_step_uncompressed_occupancy_state->getJointHistories())
                {
                    for(const auto &next_hidden_state : next_one_step_uncompressed_occupancy_state->getBeliefAt(next_joint_history)->getStates())
                    {
                        // <! \omega_k(x',o')
                        auto VarName = this->getVarNameWeightedStateJointHistory(next_one_step_uncompressed_occupancy_state, next_hidden_state, next_joint_history);
                        recover = this->getNumber(VarName);
                        con[index].setLinearCoef(var[recover], +1.0);
                    }
                }
                index++;
            }
            this->createDecentralizedConstraints(vf,state, env, con, var, index, t);
        }
        catch (const std::exception &exc)
        {
            std::cerr << "ActionVFSawtoothLP::createConstraints(..) exception caught: " << exc.what() << std::endl;
            exit(-1);
        }
    }

    double ActionVFSawtoothLP::getQValueRealistic(const std::shared_ptr<ValueFunction>&vf,const std::shared_ptr<State> &state, const std::shared_ptr<JointHistoryInterface>& joint_history, const std::shared_ptr<Action>& action, std::shared_ptr<State> next_hidden_state, const std::shared_ptr<Observation> next_observation, double denominator, double difference, number t)
    {
        return difference * this->getSawtoothMinimumRatio(vf,state, joint_history, action, next_hidden_state, next_observation, denominator,t);
    }
    
    double ActionVFSawtoothLP::getSawtoothMinimumRatio(const std::shared_ptr<ValueFunction>&,const std::shared_ptr<State> &state, const std::shared_ptr<JointHistoryInterface>& joint_history, const std::shared_ptr<Action>& action, const std::shared_ptr<State>& next_hidden_state, const std::shared_ptr<Observation>& next_observation, double denominator, number t)
    {
        double numerator = 0.0;

        try
        {
            // Compute the numerator for the Sawtooth Ratio, i.e. we compute the \sum_{x} s(x,o) * T(x,u,x_,z_)
            // This formulation allow us to tranform the problem at t+1, like the next_one_step_uncompressed_occupancy_state
            // therefore, we can have the sawtooth ration equivalent to 1.

            auto compressed_occupancy_state = state->toOccupancyState();
            auto under_pb = std::dynamic_pointer_cast<MPOMDPInterface>(ActionVFBase::world_->getUnderlyingProblem());

            // Go over all hidden state  in a belief conditionning to a joint history
            for(const auto &hidden_state : compressed_occupancy_state->getBeliefAt(joint_history)->getStates())
            {
                numerator += compressed_occupancy_state->getProbability(joint_history,hidden_state)*under_pb->getDynamics(hidden_state,action,next_hidden_state,next_observation,t);
            }
        }
        catch (const std::exception &exc)
        {
            std::cerr << "ActionVFSawtoothLP::getSawtoothMinimumRatio(..) exception caught: " << exc.what() << std::endl;
            exit(-1);
        }
        return numerator / denominator;
    }

    void ActionVFSawtoothLP::createSawtoothBigM(const std::shared_ptr<ValueFunction>&vf,const std::shared_ptr<State> &state, const std::shared_ptr<JointHistoryInterface>& joint_history, const std::shared_ptr<State> &next_hidden_state, const std::shared_ptr<Observation> &next_observation, const std::shared_ptr<JointHistoryInterface> &next_joint_history, const std::shared_ptr<State> &next_one_step_uncompressed_occupancy_state, double probability, double difference, IloEnv &env, IloRangeArray &con, IloNumVarArray &var, number &index, number t)
    {
        //Specialisation for type of state
        switch (state->getTypeState())
        {
        case TypeState::OCCUPANCY_STATE :
            this->createSawtoothBigMOccupancy(vf,state, joint_history, next_hidden_state, next_observation, next_joint_history, next_one_step_uncompressed_occupancy_state, probability, difference, env, con, var, index, t);
            break;
        case TypeState::SERIAL_OCCUPANCY_STATE :
            this->createSawtoothBigMSerial(vf,state, joint_history, next_hidden_state, next_observation, next_joint_history, next_one_step_uncompressed_occupancy_state, probability, difference, env, con, var, index, t);
            break;
        default:
            break;
        }
    }

    void ActionVFSawtoothLP::createSawtoothIloIfThen(const std::shared_ptr<ValueFunction>&vf,const std::shared_ptr<State> &state, const std::shared_ptr<JointHistoryInterface>& joint_history, const std::shared_ptr<State> &next_hidden_state, const std::shared_ptr<Observation> &next_observation, const std::shared_ptr<JointHistoryInterface> &next_joint_history, const std::shared_ptr<State> &next_state, double probability, double difference, IloEnv &env, IloModel &model, IloNumVarArray &var,number t)
    {
        //Specialisation for type of state
        switch (state->getTypeState())
        {
        case TypeState::OCCUPANCY_STATE :
            this->createSawtoothIloIfThenOccupancy(vf,state, joint_history, next_hidden_state, next_observation, next_joint_history, next_state, probability, difference, env, model, var,t);
            break;
        case TypeState::SERIAL_OCCUPANCY_STATE :
            this->createSawtoothIloIfThenSerial(vf,state, joint_history, next_hidden_state, next_observation, next_joint_history, next_state, probability, difference, env, model, var,t);
            break;
        default:
            break;
        }
    }

    void ActionVFSawtoothLP::createObjectiveFunction(const std::shared_ptr<ValueFunction>&vf, const std::shared_ptr<State> &state, IloNumVarArray &var, IloObjective &obj, number t)
    {
        //Specialisation for type of state
        switch (state->getTypeState())
        {
        case TypeState::OCCUPANCY_STATE :
            this->createObjectiveFunctionOccupancy(vf, state, var, obj, t);
            break;
        case TypeState::SERIAL_OCCUPANCY_STATE :
            this->createObjectiveFunctionSerial(vf, state, var, obj, t);
            break;
        default:
            break;
        }
    }

    std::shared_ptr<Joint<std::shared_ptr<Observation>>> ActionVFSawtoothLP::determineNextJointObservation(const std::shared_ptr<State> &compressed_occupancy_state, const std::shared_ptr<JointHistoryInterface>& next_joint_history)
    {
        std::shared_ptr<Joint<std::shared_ptr<Observation>>> next_joint_observation;
        // Get next observation
        if (compressed_occupancy_state->getTypeState() == TypeState::SERIAL_OCCUPANCY_STATE)
        {
            // int next_agent_id = compressed_occupancy_state->toSerialOccupancyState()->getCurrentAgentId();
            // next_joint_observation = next_agent_id == 0 ? next_joint_history->getData() : next_joint_history->getDefaultObs();
        }
        else
        {
            next_joint_observation = std::static_pointer_cast<Joint<std::shared_ptr<Observation>>>(next_joint_history->getObservation());
        }
        return next_joint_observation;
    }

    std::set<std::shared_ptr<JointHistoryInterface>> ActionVFSawtoothLP::determineJointHistory(const std::shared_ptr<State> &state, const std::shared_ptr<JointHistoryInterface>& next_joint_history, const std::shared_ptr<Observation>&next_joint_observation)
    {
        std::set<std::shared_ptr<JointHistoryInterface>> joint_histories;

        auto compressed_occupancy_state = state->toOccupancyState();

        for(const auto &joint_history : compressed_occupancy_state->getJointHistories())
        {
            auto verification = joint_history->expand(next_joint_observation);
            if(verification == next_joint_history)
            {
                joint_histories.insert(joint_history);
            }
        }
        return joint_histories;
    }


    // *********************************************
    // Specialisation for the Occupancy State 
    // *********************************************

    void ActionVFSawtoothLP::createSawtoothBigMOccupancy(const std::shared_ptr<ValueFunction>&vf,const std::shared_ptr<State> &state, const std::shared_ptr<JointHistoryInterface>& joint_history, const std::shared_ptr<State> &next_hidden_state, const std::shared_ptr<Observation> &next_observation, const std::shared_ptr<JointHistoryInterface> &next_joint_history, const std::shared_ptr<State> &next_state, double probability, double difference, IloEnv &env, IloRangeArray &con, IloNumVarArray &var, number &index, number t)
    {
        try
        {
            auto under_pb = ActionVFBase::world_->getUnderlyingProblem();

            number recover = 0;

            con.add(IloRange(env, -IloInfinity, this->bigM_value_));
            con[index].setLinearCoef(var[this->getNumber(this->getVarNameWeight(0))], +1.0);

            // Go over all actions
            for (const auto &action : *under_pb->getActionSpace(t))
            {
                //<! 1.c.4 get variable a(u|o) and set constant
                recover = this->getNumber(this->getVarNameJointHistoryDecisionRule(action->toAction(), joint_history));
                con[index].setLinearCoef(var[recover], - this->getQValueRealistic(vf,state, joint_history, action->toAction(), next_hidden_state, next_observation, probability, difference,t));
            }

            // <! \omega_k(x',o') * BigM
            recover = this->getNumber(this->getVarNameWeightedStateJointHistory(next_state, next_hidden_state, next_joint_history));
            con[index].setLinearCoef(var[recover], this->bigM_value_);

            index++;
        }
        catch (const std::exception &exc)
        {
            // catch anything thrown within try block that derives from std::exception
            std::cerr << "SawtoothValueFunctionLP<TState, TAction, TValue>::setGreedySawtoothBigM(..) exception caught: " << exc.what() << std::endl;
            exit(-1);
        }
    }

    void ActionVFSawtoothLP::createSawtoothIloIfThenOccupancy(const std::shared_ptr<ValueFunction>&vf,const std::shared_ptr<State> &state, const std::shared_ptr<JointHistoryInterface>& joint_history, const std::shared_ptr<State> &next_hidden_state, const std::shared_ptr<Observation> &next_observation, const std::shared_ptr<JointHistoryInterface> &next_joint_history, const std::shared_ptr<State> &next_state, double probability, double difference, IloEnv &env, IloModel &model, IloNumVarArray &var,number t)
    {
        try
        {
            number recover = 0;
            auto under_pb = ActionVFBase::world_->getUnderlyingProblem();

            IloExpr expr(env);
            //<! 1.c.1 get variable v and set coefficient of variable v
            expr = var[this->getNumber(this->getVarNameWeight(0))];

            // Go over all actions
            for(const auto & action : *under_pb->getActionSpace(t))
            { 
                recover = this->getNumber(this->getVarNameJointHistoryDecisionRule(action->toAction(), joint_history));
                //<! 1.c.4 get variable a(u|o) and set constant 
                expr -= this->getQValueRealistic(vf,state, joint_history, action->toAction(), next_hidden_state, next_observation, probability, difference,t) * var[recover];
            }               

            // <! get variable \omega_k(x',o')
            recover = this->getNumber(this->getVarNameWeightedStateJointHistory(next_state, next_hidden_state, next_joint_history));
            model.add(IloIfThen(env, var[recover] > 0, expr <= 0));
        }
        catch (const std::exception &exc)
        {
            // catch anything thrown within try block that derives from std::exception
            std::cerr << "SawtoothValueFunctionLP<TState, TAction, TValue>::setGreedySawtoothIloIfThen(..) exception caught: " << exc.what() << std::endl;
            exit(-1);
        }
    }

    void ActionVFSawtoothLP::createObjectiveFunctionOccupancy(const std::shared_ptr<ValueFunction>&vf, const std::shared_ptr<State> &state, IloNumVarArray &var, IloObjective &obj, number t)
    {
        try
        {
            auto under_pb = ActionVFBase::world_->getUnderlyingProblem();
            auto compressed_occupancy_state = state->toOccupancyState();

            // <! 1.a get variable v
            auto recover = this->getNumber(this->getVarNameWeight(0));

            //<! 1.b set coefficient of objective function "\sum_{o,u} a(u|o) \sum_x s(x,o) Q_MDP(x,u) + discount * v0"
            obj.setLinearCoef(var[recover], under_pb->getDiscount(t));

            // Go over all action
            for (const auto &action : *under_pb->getActionSpace(t))
            {
                // Go over all joint history
                for (const auto &joint_history : compressed_occupancy_state->getJointHistories())
                {
                    //<! 1.c.4 get variable a(u|o)
                    recover = this->getNumber(this->getVarNameJointHistoryDecisionRule(action->toAction(), joint_history));

                    //<! 1.c.5 set coefficient of variable a(u|o) i.e., \sum_x s(x,o) Q_MDP(x,u)
                    obj.setLinearCoef(var[recover], this->getQValueRelaxation(vf,compressed_occupancy_state, joint_history, action->toAction(), t));
                }
            }
        }
        catch (const std::exception &exc)
        {
            std::cerr << "ActionVFSawtoothLP::createObjectiveFunctionOccupancy(..) exception caught: " << exc.what() << std::endl;
            exit(-1);
        }
    }


    // *********************************************
    // Specialisation for the Serial Occupancy State 
    // *********************************************

    void ActionVFSawtoothLP::createSawtoothBigMSerial(const std::shared_ptr<ValueFunction>&,const std::shared_ptr<State> &, const std::shared_ptr<JointHistoryInterface>& , const std::shared_ptr<State> &, const std::shared_ptr<Observation> &, const std::shared_ptr<JointHistoryInterface> &, const std::shared_ptr<State> &, double , double , IloEnv &, IloRangeArray &, IloNumVarArray &, number &, number )
    {
        throw sdm::exception::NotImplementedException();
        // try
        // {
        //     auto under_pb = std::dynamic_pointer_cast<SerialMMDP>(this->world_->getUnderlyingProblem());

        //     auto agent_id = under_pb->getAgentId(t);
        //     // Gets the current individual history conditional on the current joint history
        //     auto indiv_history = joint_history->getIndividualHistory(agent_id);

        //     number recover = 0;

        //     con.add(IloRange(env, -IloInfinity, this->bigM_value_));
        //     con[index].setLinearCoef(var[this->getNumber(this->getVarNameWeight(0))], +1.0);

        //     // Go over all actions
        //     for (const auto &action : *under_pb->getActionSpace(t))
        //     {
        //         //<! 1.c.4 get variable a(u|o) and set constant
        //         recover = this->getNumber(this->getVarNameIndividualHistoryDecisionRule(action->toAction(), indiv_history, agent_id))
        //         con[index].setLinearCoef(var[recover], - this->getQValueRealistic(state, joint_history, action->toAction(), next_hidden_state, next_observation, probability, difference));
        //     }

        //     // <! \omega_k(x',o') * BigM
        //     recover = this->getNumber(this->getVarNameWeightedStateJointHistory(next_state, next_hidden_state, next_joint_history));
        //     con[index].setLinearCoef(var[recover], this->bigM_value_);

        //     index++;
        // }
        // catch (const std::exception &exc)
        // {
        //     // catch anything thrown within try block that derives from std::exception
        //     std::cerr << "SawtoothValueFunctionLP<TState, TAction, TValue>::setGreedySawtoothBigM(..) exception caught: " << exc.what() << std::endl;
        //     exit(-1);
        // }
    }

    void ActionVFSawtoothLP::createSawtoothIloIfThenSerial(const std::shared_ptr<ValueFunction>&,const std::shared_ptr<State> &, const std::shared_ptr<JointHistoryInterface>& , const std::shared_ptr<State> &, const std::shared_ptr<Observation> &, const std::shared_ptr<JointHistoryInterface> &, const std::shared_ptr<State> &, double , double , IloEnv &, IloModel &, IloNumVarArray &,number )
    {
        throw sdm::exception::NotImplementedException();
        // try
        // {
        //     auto under_pb = std::dynamic_pointer_cast<SerialMMDP>(this->world_->getUnderlyingProblem());

        //     auto agent_id = under_pb->getAgentId(t);
        //     // Gets the current individual history conditional on the current joint history
        //     auto indiv_history = joint_history->getIndividualHistory(agent_id);

        //     number recover = 0;

        //     IloExpr expr(env);
        //     //<! 1.c.1 get variable v and set coefficient of variable v
        //     expr = var[this->getNumber(this->getVarNameWeight(0))];

        //     // Go over all actions
        //     for(const auto & action : *under_pb->getActionSpace(agent,t))
        //     { 
        //         recover = this->getNumber(this->getVarNameIndividualHistoryDecisionRule(action->toAction(), indiv_history, agent_id))
        //         //<! 1.c.4 get variable a(u|o) and set constant 
        //         expr -= this->getQValueRealistic(compressed_occupancy_state, joint_history, action, next_hidden_state, next_observation, probability, difference) * var[recover];
        //     }               

        //     // <! get variable \omega_k(x',o')
        //     recover = this->getNumber(this->getVarNameWeightedStateJointHistory(next_one_step_uncompressed_occupancy_state, next_hidden_state, next_joint_history));
        //     model.add(IloIfThen(env, var[recover] > 0, expr <= 0));
        // }
        // catch (const std::exception &exc)
        // {
        //     // catch anything thrown within try block that derives from std::exception
        //     std::cerr << "SawtoothValueFunctionLP<TState, TAction, TValue>::setGreedySawtoothIloIfThen(..) exception caught: " << exc.what() << std::endl;
        //     exit(-1);
        // }
    }
    void ActionVFSawtoothLP::createObjectiveFunctionSerial(const std::shared_ptr<ValueFunction>&, const std::shared_ptr<State> &, IloNumVarArray &, IloObjective &, number ){throw sdm::exception::NotImplementedException();}
}
