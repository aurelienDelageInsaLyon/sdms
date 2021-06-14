
#include <sdm/algorithms/value_iteration.hpp>
namespace sdm
{
    ValueIteration::ValueIteration(std::shared_ptr<SolvableByHSVI> problem, double error, int horizon) : problem_(problem), error_(error), horizon_(horizon)   
    {
    }

    /**
     * @brief Initialize the algorithm
     */
    void ValueIteration::do_initialize()
    {
        auto under_pb = this->problem_->getUnderlyingProblem();

        auto tabular_backup = std::make_shared<TabularBackup>(this->problem_);

        policy_evaluation_1_ = std::make_shared<sdm::PointSetValueFunction>(tabular_backup, this->horizon_,under_pb->getMaxReward(0));
        policy_evaluation_2_ = std::make_shared<sdm::PointSetValueFunction>(tabular_backup, this->horizon_,under_pb->getMaxReward(0));

        policy_evaluation_1_->initialize();
        policy_evaluation_2_->initialize();
    }

    /**
     * @brief Solve a problem solvable by HSVI. 
     */
    void ValueIteration::do_solve()
    {
        auto under_pb = this->problem_->getUnderlyingProblem();

        do
        {
            this->policy_evaluation_1_ = this->policy_evaluation_2_;

            for(int t = this->horizon_ -1; t>=0;t--)
            {
                for(const auto &state : *under_pb->getStateSpace(t) )
                {
                    this->policy_evaluation_2_->updateValueAt(state,t,this->policy_evaluation_1_->getQValueAt(state, t)->max());
                }
            }
        } while (this->borne());
        std::cout<<"Final value : "<<this->policy_evaluation_2_->str();
    }

    /**
     * @brief Test the learnt value function on one episode
     */
    void ValueIteration::do_test()
    {
        
    }
    
    bool ValueIteration::borne()
    {
        double max_value = std::numeric_limits<double>::min();
        for(auto &state : this->policy_evaluation_1_->getSupport(0))
        {
            max_value = std::max(std::abs(policy_evaluation_1_->getValueAt(state,0) - policy_evaluation_2_->getValueAt(state,0)),max_value);
        }
        std::cout<<"\n Error : "<<max_value<<std::endl;

        if (max_value<= this->error_)
        {
            return false;
        }else
        {
            return true;
        }
    }

    std::shared_ptr<typename sdm::PointSetValueFunction> ValueIteration::getResult()
    {
        return this->policy_evaluation_2_;
    }

    double ValueIteration::getResultOpti() 
    {
        return this->policy_evaluation_2_->getValueAt(this->problem_->getInitialState());
    }
}
