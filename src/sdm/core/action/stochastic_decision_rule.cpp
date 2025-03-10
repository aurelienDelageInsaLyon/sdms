#include <sdm/core/action/stochastic_decision_rule.hpp>
#include <sdm/exception.hpp>
#include <sdm/tools.hpp>


namespace sdm
{

    StochasticDecisionRule::StochasticDecisionRule(){
    
    }

    StochasticDecisionRule::StochasticDecisionRule(int nb){
        this->numActions=nb;
    }

    StochasticDecisionRule::StochasticDecisionRule(DiscreteSpace action_space) : action_space(action_space){
        
        this->numActions = action_space.getNumItems();

    }

    StochasticDecisionRule::StochasticDecisionRule(std::vector<std::shared_ptr<Item>> listHistories, std::vector<std::pair<std::shared_ptr<Action>,double>> probaActionsForHistories)
    {
        throw sdm::exception::NotImplementedException("NotImplementedException raised in StochasticDecisionRule::constructor(listHistories,probaActionsForHistories)");
    }
    std::shared_ptr<Action> StochasticDecisionRule::act(const std::shared_ptr<State> &) const
    {
        // return this->at(s);
        throw sdm::exception::NotImplementedException("NotImplementedException raised in StochasticDecisionRule::act");
    }

    std::shared_ptr<Action> StochasticDecisionRule::act(const std::shared_ptr<HistoryInterface> &state) const
    {
        // return this->at(s);
        throw sdm::exception::NotImplementedException("NotImplementedException raised in StochasticDecisionRule::act");
    }

    std::shared_ptr<std::vector<std::pair<Action,double>>> StochasticDecisionRule::actStochastically(const std::shared_ptr<HistoryInterface> &state) const
    {
        // return this->at(s);
        throw sdm::exception::NotImplementedException("NotImplementedException raised in StochasticDecisionRule::act");
    }


    std::shared_ptr<Action> act(const std::shared_ptr<HistoryInterface> &state)
    {
        // return this->at(s);
        throw sdm::exception::NotImplementedException("NotImplementedException raised in StochasticDecisionRule::act");
    }

    // template <typename std::shared_ptr<State>, typename std::shared_ptr<Action>>
    // std::shared_ptr<Action> StochasticDecisionRule::operator()(const std::shared_ptr<State> &s)
    // {
    //     // return this->at(s);
    //     throw sdm::exception::NotImplementedException();
    // }

    std::map<std::shared_ptr<Action>,double> StochasticDecisionRule::getProbabilities(const std::shared_ptr<HistoryInterface> &state) const
    {
        try
        {
            return this->stratMap.at(state);
        }
        catch(const std::exception& e)
        {
            std::cerr <<"State not found in the StochasticDecisionRule::getProbabilities"<< e.what() << '\n';
            exit(-1) ;
        }
    }

    double StochasticDecisionRule::getProbability(const std::shared_ptr<HistoryInterface> &state, const std::shared_ptr<Action> &action) const
    {
        //std::cout << "numActions : " << this->numActions;
        
        //std::cout << "\n asking for o : " << state->short_str();
        
        
      //std::cout << "\n state : " << state->str();
        if (this->stratMap.count(state)){
            //std::cout << "\n returning proba : "  << this->getProbabilities(state).at(action);
            return this->getProbabilities(state).at(action);
        }
        for(const auto & it :stratMap)
        {
            //std::cout << "\n hist in map : " << it.first->str()<<std::flush;// << " " << it.second.str();// << " " << it->second.second << "\n";
            
            if(state->short_str()==it.first->short_str()){
                //std::cout << "\n WARNING : this test of str() of histories is weird unless you have initialized two different mdps" << std::flush;
                for (auto & tmp : it.second){
                    //std::cout << "\n tmp : " << tmp.first->str()<<std::flush;
                    //std::cout << "\n asking for action : " << action->str() << " and tmp : " << tmp.first->str();
                    if (tmp.first->str()==action->str()){
                        return tmp.second;
                    }
                }
                return 0.0;
                return it.second.at(action);
            }
        }
        //std::cout << "\n stochastic DR returning random uniform probability";
        return 1/(double)this->numActions;
    }
    
    void StochasticDecisionRule::setProbability(const std::shared_ptr<HistoryInterface> &state, const std::shared_ptr<Action> &action, double proba)
    {
        this->stratMap[state][action] = proba;
    }

void StochasticDecisionRule::setProbability(const std::shared_ptr<HistoryTree> &state, const std::shared_ptr<Action> &action, double proba)
    {
        this->stratMap[state][action] = proba;
    }


    std::vector<std::shared_ptr<Action>> StochasticDecisionRule::getActions(const std::shared_ptr<HistoryInterface> &state) const
    {
        std::vector<std::shared_ptr<Action>> res;

        if (this->stratMap.count(state)){

            for (auto &act : this->stratMap.at(state)){
                if (act.second>0.0){
                res.push_back(act.first);
                }
            }
            return res;
        }
        return res;
    }

    void StochasticDecisionRule::testValidity(){
        for (auto & hist : this->stratMap){
            double val = 0.0;
            for (auto & key : hist.second){
                val+=key.second;
            }
            if (val >1.001 || val < 0.999){
                std::cout << "\n bug validity strat" << " val : " << val;
                std::cout << this->str();
                std::exit(1);
            }
        }
    }

    std::string StochasticDecisionRule::str() const
    {
        std::ostringstream res;
        res << "<decision-rule type=\"stochastic\">" << std::endl;
        for (const auto &pair_state__pair_action__proba : this->stratMap)
        {
            res << "\t<decision state=\"" << pair_state__pair_action__proba.first->str() << "\">" << std::endl;

            for(const auto &pair_action_proba : pair_state__pair_action__proba.second)
            {
                res << "\t < action=\"" << pair_action_proba.first->str()<<"\" probability=\"" <<this->getProbability(pair_state__pair_action__proba.first,pair_action_proba.first)<<"/>"  << std::endl;
            }

            res << "\t<decision/>" << std::endl;
        }
        res << "<decision-rule/>" << std::endl;
        return res.str();
    }

} // namespace sdm

namespace std
{
    // template <>
    // struct hash<sdm::StochasticDecisionRule>
    // {
    //     typedef sdm::StochasticDecisionRule argument_type;
    //     typedef std::size_t result_type;
    //     inline result_type operator()(const argument_type &in) const
    //     {
    //         size_t seed = 0;
    //         for (auto &input : in)
    //         {
    //             sdm::hash_combine(seed, input.first);
    //             sdm::hash_combine(seed, input.second);
    //         }
    //         return seed;
    //     }
    // };
}