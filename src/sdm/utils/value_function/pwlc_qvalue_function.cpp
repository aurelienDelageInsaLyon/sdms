#include <sdm/utils/value_function/pwlc_qvalue_function.hpp>
#include <sdm/core/action/decision_rule.hpp>

namespace sdm
{
    double PieceWiseLinearConvexQValueFunction::GRANULARITY = 0.1;

    PieceWiseLinearConvexQValueFunction::PieceWiseLinearConvexQValueFunction(const std::shared_ptr<OccupancyMDP> &omdp, number horizon, double learning_rate, double default_value)
        : QValueFunction(horizon), omdp(omdp), default_value_(default_value), learning_rate_(learning_rate)
    {
        this->representation = std::vector<PSI>(this->isInfiniteHorizon() ? 1 : this->getHorizon() + 1, PSI());
    }

    void PieceWiseLinearConvexQValueFunction::initialize()
    {
    }

    void PieceWiseLinearConvexQValueFunction::initialize(double default_value, number t)
    {
    }

    TabularQValueFunction *PieceWiseLinearConvexQValueFunction::getQ(const std::shared_ptr<State> &state, number t)
    {
        auto casted_state = std::dynamic_pointer_cast<OccupancyState>(state);
        auto q_s = this->representation[t].find(*casted_state);
        return (q_s == this->representation[t].end()) ? nullptr : &q_s->second;
    }

    std::shared_ptr<VectorInterface<std::shared_ptr<Action>, double>> PieceWiseLinearConvexQValueFunction::getQValuesAt(const std::shared_ptr<State> &state, number t)
    {
        auto vector = std::make_shared<MappedVector<std::shared_ptr<Action>>>(this->default_value_);
        for (auto dr : *omdp->getActionSpaceAt(state, t))
        {
            vector->setValueAt(dr->toAction(), getQValueAt(state, dr->toAction(), t));
        }
        return vector;
    }

    double PieceWiseLinearConvexQValueFunction::getQValueAt(const std::shared_ptr<State> &state, const std::shared_ptr<Action> &action, number t)
    {
        double qvalue = 0;
        auto h = this->isInfiniteHorizon() ? 0 : t;
        auto q_s = this->getQ(state, h);
        if (q_s == nullptr)
        {
            qvalue = default_value_;
        }
        else
        {
            for (auto o : state->toOccupancyState()->getJointHistories())
            {
                auto u = omdp->applyDecisionRule(state->toOccupancyState(), o, action, h);
                qvalue += q_s->getQValueAt(o, u, h);
            }
        }
        return qvalue;
    }

    void PieceWiseLinearConvexQValueFunction::updateQValueAt(const std::shared_ptr<State> &state, const std::shared_ptr<Action> &action, double delta, number t)
    {
        auto h = this->isInfiniteHorizon() ? 0 : t;
        auto q_s = this->getQ(state, h);
        if (q_s == nullptr)
        {
            auto casted_state = std::dynamic_pointer_cast<OccupancyState>(state);
            representation[h][*casted_state] = TabularQValueFunction(0, this->learning_rate_);
            q_s = this->getQ(state, h);
        }
        for (auto o : state->toOccupancyState()->getJointHistories())
        {
            auto u = omdp->applyDecisionRule(state->toOccupancyState(), o, action, h);
            q_s->updateQValueAt(o, u, delta * state->toOccupancyState()->getProbability(o));
        }
    }

    void PieceWiseLinearConvexQValueFunction::updateQValueAt(const std::shared_ptr<State> &, const std::shared_ptr<Action> &, number)
    {
        throw sdm::exception::NotImplementedException();
    }

    int PieceWiseLinearConvexQValueFunction::getNumStates() const
    {
        return 0;
    }

    std::string PieceWiseLinearConvexQValueFunction::str() const
    {
        std::ostringstream res;
        // res << "<hierarchical_qvalue_function_v2 horizon=\"" << ((this->isInfiniteHorizon()) ? "inf" : std::to_string(this->getHorizon())) << "\">" << std::endl;
        // for (sdm::size_t i = 0; i < this->representation.size(); i++)
        // {
        //     res << "\t<timestep=\"" << ((this->isInfiniteHorizon()) ? "all" : std::to_string(i)) << "\" default=\"" << 0 << "\">" << std::endl;
        //     for (auto const& [s, q] : this->representation[i])
        //     {
        //         auto jh = *s->getJointHistories().begin();
        //         if (jh->getHorizon() < this->horizon_)
        //         {
        //             res << "\t\t<S-Q>" << std::endl;
        //             tools::indentedOutput(res, s->str().c_str(), 3);
        //             res << std::endl;
        //             tools::indentedOutput(res, q.str().c_str(), 3);
        //             res << "\t\t</S-Q>" << std::endl;
        //         }
        //     }
        //     res << "\t</timestep>" << std::endl;
        // }
        // res << "</hierarchical_qvalue_function_v2>" << std::endl;
        return res.str();
    }
} // namespace sdm