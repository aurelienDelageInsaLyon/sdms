
// #include <sdm/utils/global_Test.hpp>
#include <sdm/utils/value_function/initializer/initializers.hpp>

#include <sdm/utils/value_function/point_set_value_function.hpp>
#include <sdm/utils/value_function/hyperplan_value_function.hpp>

#include <sdm/utils/value_function/backup/maxplan_backup.hpp>
#include <sdm/utils/value_function/backup/tabular_backup.hpp>

#include <sdm/utils/value_function/action_vf/action_maxplan.hpp>
#include <sdm/utils/value_function/action_vf/action_tabulaire.hpp>

#include <sdm/parser/parser.hpp>
#include <sdm/exception.hpp>
#include <sdm/world/occupancy_mdp.hpp>
#include <sdm/algorithms.hpp>
#include <sdm/types.hpp>

int main(int argc, char **argv)
{

    std::string filename = "../data/world/dpomdp/mabc.dpomdp";
    int horizon = 50;
    int discount = 1;
    double error = 0.001;
    int trials = 100;
	int truncation = 1 ;

    int freq_prunning = 1;    

    auto problem = sdm::parser::parse_file(filename);
    problem->setHorizon(horizon);
    problem->setDiscount(discount);

    std::shared_ptr<sdm::SolvableByHSVI> oMDP = std::make_shared<sdm::OccupancyMDP>(problem, (truncation > 0) ? truncation : horizon);

    auto tabular_backup = std::make_shared<sdm::TabularBackup>(oMDP);
    auto maxplan_backup = std::make_shared<sdm::MaxPlanBackup>(oMDP);

    auto action_maxplan = std::make_shared<sdm::ActionVFMaxplan>(oMDP);
    auto action_tabular = std::make_shared<sdm::ActionVFTabulaire>(oMDP);

    auto init_lb = std::make_shared<sdm::MinInitializer>(oMDP);
    auto init_ub = std::make_shared<sdm::MDPInitializer>(oMDP,"");

    // Instanciate bounds
    std::shared_ptr<sdm::ValueFunction> lower_bound = std::make_shared<sdm::TabularValueFunction>(horizon,init_lb,tabular_backup,action_tabular);
    std::shared_ptr<sdm::ValueFunction> upper_bound = std::make_shared<sdm::PointSetValueFunction>(horizon,init_ub,tabular_backup, action_tabular,freq_prunning);

    auto algo = std::make_shared<sdm::HSVI>(oMDP, lower_bound, upper_bound, problem->getHorizon(), error, trials);

    algo->initialize();
    algo->solve();

    return 0;
} // END main
