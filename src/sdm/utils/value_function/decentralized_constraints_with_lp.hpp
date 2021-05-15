

#pragma once
#include <ilcplex/ilocplex.h>
#include <sdm/utils/linear_algebra/vector.hpp>
#include <sdm/utils/value_function/variable_naming.hpp>
#include <sdm/core/states.hpp>
#include <sdm/exception.hpp>

/**
 * @brief Namespace grouping all tools required for sequential decision making.
 * @namespace  sdm
 */
namespace sdm
{
    /**
     * @brief 
     * @tparam TVector 
     * @tparam TAction 
     * @tparam TValue 
     */
    template <typename TVector, typename TAction, typename TValue = double>
    class DecentralizedConstraintsLP  : public VarNaming<TVector, TAction, TValue>
    {
    public:
        DecentralizedConstraintsLP(std::shared_ptr<SolvableByHSVI<TVector, TAction>>);

        /**
         * @brief Get the decentralized (joint) decision rule from the result
         * @param const IloCplex&
         * @param const IloNumVarArray&
         * @param const TVector&
         * @param number : time step
         * @return TAction 
         */
        template <typename T, std::enable_if_t<std::is_any<T, OccupancyState<>, OccupancyState<BeliefStateGraph_p<number, number>, JointHistoryTree_p<number>>>::value, int> = 0>
        TAction getDecentralizedVariables(const IloCplex&, const IloNumVarArray&, const TVector&, number );

        /**
         * @brief Set decentralized variables 
         * @param const TVector&
         * @param const IloEnv&
         * @param const IloNumVarArray&
         * @param const number&
         * @param number : time step
         */
        template <typename T, std::enable_if_t<std::is_any<T, OccupancyState<>, OccupancyState<BeliefStateGraph_p<number, number>, JointHistoryTree_p<number>>>::value, int> = 0>
        void setDecentralizedVariables(const TVector&, IloEnv&, IloNumVarArray&, number&, number );

        /**
         * @brief Set decentralized constraints 
         * @param const TVector& 
         * @param IloEnv& 
         * @param IloRangeArray&
         * @param IloNumVarArray&
         * @param number&
         * @param number : time step
         */
        template <typename T, std::enable_if_t<std::is_any<T, OccupancyState<>, OccupancyState<BeliefStateGraph_p<number, number>, JointHistoryTree_p<number>>>::value, int> = 0>
        void setDecentralizedConstraints(const TVector&, IloEnv&, IloRangeArray&, IloNumVarArray&, number&, number);

        /**
         * @brief Get the decentralized (joint) decision rule from the result
         * @param const IloCplex&
         * @param const IloNumVarArray&
         * @param const TVector&
         * @param number : time step
         * @return TAction 
         */
        template <typename T, std::enable_if_t<std::is_same_v<SerializedOccupancyState<>, T>, int> = 0>
        TAction getDecentralizedVariables(const IloCplex&, const IloNumVarArray&, const TVector&, number);

        /**
         * @brief Set decentralized variables 
         * @param const TVector&
         * @param const IloEnv&
         * @param const IloNumVarArray&
         * @param const number&
         * @param number : time step
         */
        template <typename T, std::enable_if_t<std::is_same_v<SerializedOccupancyState<>, T>, int> = 0>
        void setDecentralizedVariables(const TVector&, IloEnv&, IloNumVarArray&, number&, number);

        /**
         * @brief Set decentralized constraints 
         * @param const TVector& 
         * @param IloEnv& 
         * @param IloRangeArray&
         * @param IloNumVarArray&
         * @param number&
         * @param number : time step
         */
        template <typename T, std::enable_if_t<std::is_same_v<SerializedOccupancyState<>, T>, int> = 0>
        void setDecentralizedConstraints(const TVector&, IloEnv&, IloRangeArray&, IloNumVarArray&, number&, number);

    protected : 
        std::shared_ptr<SolvableByHSVI<TVector, TAction>> world_;

    };
}
#include <sdm/utils/value_function/decentralized_constraints_with_lp.tpp>
