#include <sdm/core/state/belief_state.hpp>
#include <sdm/core/state/occupancy_state.hpp>
#include <sdm/world/base/pomdp_interface.hpp>
#include <sdm/world/belief_mdp.hpp>
#include <sdm/world/occupancy_mdp.hpp>
#include <sdm/utils/value_function/pwlc_value_function_interface.hpp>
#include <sdm/utils/value_function/update_operator/vupdate/approxW_update.hpp>
#include <sdm/utils/linear_algebra/hyperplane/balpha.hpp>
#include <sdm/utils/linear_algebra/hyperplane/oalpha.hpp>
#include <sdm/utils/value_function/vfunction/partial_Q_value_function.hpp>
#include <sdm/utils/value_function/action_selection/lp/action_maxplan_lp_mg.hpp>

namespace sdm
{

    namespace update
    {
        UpdateW::UpdateW(const std::shared_ptr<ValueFunctionInterface> &value_function)
            : UpdateOperator<partialQValueFunction>(value_function) {}


        void UpdateW::update(const std::shared_ptr<State> &state, number t,const std::shared_ptr<State> &lastState, const std::shared_ptr<StochasticDecisionRule> & dr)
        {
            std::shared_ptr<partialQValueFunction> vf =  std::dynamic_pointer_cast<partialQValueFunction>(this->getValueFunction());

            std::shared_ptr<ActionSelectionMaxplanLPMG> actionSelectionOperator = std::dynamic_pointer_cast<ActionSelectionMaxplanLPMG>(vf->getActionSelection());

            vf->addTuple(std::dynamic_pointer_cast<OccupancyStateMG>(state),std::dynamic_pointer_cast<OccupancyStateMG>(lastState),dr,actionSelectionOperator->getGreedyActionAndValues(vf,state,t).second,actionSelectionOperator->getTreeDeltaStretegy(),t);
        }


        void UpdateW::updateLastStep(const std::shared_ptr<State> &state, number t,const std::shared_ptr<State> &lastState, const std::shared_ptr<StochasticDecisionRule> & lastDr, std::map<std::shared_ptr<HistoryInterface>,double> valuesUpb,const std::shared_ptr<StochasticDecisionRule> & dr){

            std::shared_ptr<partialQValueFunction> vf =  std::dynamic_pointer_cast<partialQValueFunction>(this->getValueFunction());

            std::map<std::tuple<std::shared_ptr<OccupancyStateMG>,std::shared_ptr<StochasticDecisionRule>,std::shared_ptr<VectorMG>>, double> treeDeltaStrategy = {{std::tuple(nullptr,dr,nullptr),1.0}};

            vf->addTuple(std::dynamic_pointer_cast<OccupancyStateMG>(state),std::dynamic_pointer_cast<OccupancyStateMG>(lastState),lastDr,valuesUpb,treeDeltaStrategy, t);
        }

        std::shared_ptr<Hyperplane> UpdateW::computeNewHyperplane(const std::shared_ptr<BeliefInterface> &belief_state, number t)
        {
            std::shared_ptr<VectorMG> new_vector;
            return new_vector;
            /*
            auto pomdp = std::dynamic_pointer_cast<POMDPInterface>(this->getWorld()->getUnderlyingProblem());

            // Compute \alpha_ao (\beta_ao in the paper of Trey Smith)
            RecursiveMap<std::shared_ptr<Action>, std::shared_ptr<Observation>, std::shared_ptr<AlphaVector>> alpha_ao;
            for (const auto &action : *getWorld()->getActionSpaceAt(belief_state, t))
            {
                for (const auto &observation : *getWorld()->getObservationSpaceAt(belief_state, action->toAction(), t))
                {
                    auto next_belief_state = getWorld()->getNextStateAndProba(belief_state, action->toAction(), observation->toObservation(), t).first;
                    alpha_ao[action->toAction()][observation->toObservation()] = std::static_pointer_cast<AlphaVector>(this->getValueFunction()->getHyperplaneAt(next_belief_state, t + 1));
                }
            }

            // Creation of a new belief
            double best_value = std::numeric_limits<double>::lowest(), alpha_a_value;
            std::shared_ptr<AlphaVector> new_hyperplane = std::make_shared<bAlpha>(this->getValueFunction()->getDefaultValue(t));

            // TODO Ideallement tu n'as pas besoin de cette boucle car l'action gourmande est connue d'apres la ligne 21.
            // TODO Ideallement tu devrais ajouter en argument cette action et enlever cette boucle inutile.
            // TODO S'il y'a un probleme c'est qu'il vient d'ailleurs et pas de cette fonction.
            for (const auto &action : *getWorld()->getActionSpaceAt(belief_state, t))
            {
                // Creation of a new belief
                std::shared_ptr<AlphaVector> alpha_a = std::make_shared<bAlpha>(this->getValueFunction()->getDefaultValue(t));

                // Go over all state in the current belief
                for (const auto &state : belief_state->getStates())
                {
                    double next_expected_value = 0.0;

                    // Go over all hidden state reachable next state
                    for (const auto &next_state : pomdp->getReachableStates(state, action->toAction(), t))
                    {
                        // Go over all observation reachable observation
                        for (const auto &observation : pomdp->getReachableObservations(state, action->toAction(), next_state, t))
                        {

                            // Get the next value of an hyperplane
                            double next_alpha_value = alpha_ao[action->toAction()][observation]->getValueAt(next_state, nullptr);

                            // Determine the best next hyperplan for the next belief and compute the dynamics and probability of this best next hyperplan
                            next_expected_value += next_alpha_value * pomdp->getDynamics(state, action->toAction(), next_state, observation, t);
                        }
                    }
                    // For each hidden state with associate the value \beta^{new}(x) = r(x,u) + \gamma * \sum_{x_,z_} p(x,u,z_,x_) * best_next_hyperplan(x_);
                    alpha_a->setValueAt(state, nullptr, pomdp->getReward(state, action->toAction(), t) + this->getWorld()->getDiscount(t) * next_expected_value);
                }

                alpha_a_value = belief_state->product(alpha_a);
                if (alpha_a_value > best_value)
                {
                    new_hyperplane = alpha_a;
                    best_value = alpha_a_value;
                }
            }
            return new_hyperplane;*/
        }

        std::shared_ptr<Hyperplane> UpdateW::computeNewHyperplane(const std::shared_ptr<OccupancyStateInterface> &occupancy_state, const std::shared_ptr<Action> &decision_rule, number t)
        {
            std::shared_ptr<VectorMG> new_vector;
            return new_vector;
            /*
            auto pomdp = std::dynamic_pointer_cast<POMDPInterface>(this->getWorld()->getUnderlyingProblem());
            auto occupancy_mdp = std::dynamic_pointer_cast<BeliefMDPInterface>(this->getWorld());

            // Create the new hyperplan
            std::shared_ptr<AlphaVector> new_hyperplane = std::make_shared<oAlpha>(this->getValueFunction()->getDefaultValue(t));

            // Get the next occupancy state associated to the decision rule
            auto next_occupancy_state = occupancy_mdp->getNextStateAndProba(occupancy_state, decision_rule, sdm::NO_OBSERVATION, t).first;

            // Determine the best next hyperplan associated to the next occupancy state
            auto best_next_hyperplane = this->getValueFunction()->getHyperplaneAt(next_occupancy_state, t + 1);

            // Go over all joint history for the occupancy state
            for (const auto &jhistory : occupancy_state->getFullyUncompressedOccupancy()->getJointHistories())
            {
                // Select the joint action
                auto action = occupancy_state->applyDR(decision_rule->toDecisionRule(), occupancy_state->getCompressedJointHistory(jhistory));

                // Create new belief
                for (const auto &state : occupancy_state->getFullyUncompressedOccupancy()->getBeliefAt(jhistory)->getStates())
                {
                    // For each hidden state with associate the value r(x,u) + discount* \sum_{x_,z_} p(x,u,z_,x_)* best_next_hyperplan(x_);
                    new_hyperplane->setValueAt(state, jhistory, this->getValueFunction()->getBeta(best_next_hyperplane, state, jhistory, action, t));
                }
            }
            return new_hyperplane;*/
        }


        void UpdateW::update(const std::shared_ptr<State> &state, number t)
        {
            std::cout << "\n should never be called, i'm exiting";
            std::exit(1);
            std::shared_ptr<partialQValueFunction> vf =  std::dynamic_pointer_cast<partialQValueFunction>(this->getValueFunction());
            std::shared_ptr<ActionSelectionMaxplanLPMG> actionSelectionOperator = std::dynamic_pointer_cast<ActionSelectionMaxplanLPMG>(vf->getActionSelection());
            //this->update(state, this->getValueFunction()->getGreedyAction(state, t), t);
            this->update(state, actionSelectionOperator->getGreedyActionAndValues(vf,state,t).first, t);

        }

        void UpdateW::update(const std::shared_ptr<State> &state, const std::shared_ptr<Action> &action, number t)
        {   
            std::cout << "\n should never be called, exiting";
            std::exit(1);
            /*
            if (auto ostate = sdm::isInstanceOf<OccupancyStateInterface>(state))
            {
                this->getValueFunction()->addHyperplaneAt(state, this->computeNewHyperplane(ostate, action, t), t);
            }
            else if (auto bstate = sdm::isInstanceOf<BeliefInterface>(state))
            {
                this->getValueFunction()->addHyperplaneAt(state, this->computeNewHyperplane(bstate, t), t);
            }*/
        }

    }
}