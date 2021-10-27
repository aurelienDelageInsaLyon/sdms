#pragma once

#include <sdm/types.hpp>
#include <sdm/utils/value_function/pwlc_value_function_interface.hpp>

namespace sdm
{

    class TabularQValueFunctionInterface;
    // class PWLCValueFunctionInterface;

    namespace update
    {
        /**
         * @brief This interface is the interface that is common to all update operators.
         *
         * Any class inheriting from this interface can be used to update a value function.
         *
         */
        class QUpdateOperatorInterface
        {
        public:
            /**
             * @brief Update the value function.
             *
             * @param t the time step
             */
            virtual void update(number t) = 0;
        };

        /**
         * @brief Update operator for reinforcement learning.
         *
         * @tparam TQValueFunction the type of Q-value function
         */
        template <class TQValueFunction>
        class QUpdateOperator : public QUpdateOperatorInterface
        {
        public:
            /**
             * @brief Construct an update operator for RL.
             *
             * @param qvalue_function the qvalue function
             */
            QUpdateOperator(const std::shared_ptr<ValueFunctionInterface> &qvalue_function)
            {
                if (auto derived = std::dynamic_pointer_cast<TQValueFunction>(qvalue_function))
                {
                    this->qvalue_function = derived;
                }
                else
                {
                    throw sdm::exception::TypeError("Cannot instanciate QUpdateOperator<T> with q-value function that does not derive from T.");
                }
            }

            /**
             * @brief Update the value function.
             *
             * @param t the time step
             */
            virtual void update(number t) = 0;

        protected:
            /**
             * @brief the Q-value function
             */
            std::shared_ptr<TQValueFunction> qvalue_function;
        };

        using TabularQUpdateOperator = QUpdateOperator<TabularQValueFunctionInterface>;
        using PWLCQUpdateOperator = QUpdateOperator<PWLCValueFunctionInterface>;

    } // namespace update
} // namespace sdm
