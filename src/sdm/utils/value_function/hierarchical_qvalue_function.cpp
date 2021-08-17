// #include <sdm/utils/value_function/hierarchical_qvalue_function.hpp>

// // #include <sdm/core/state/occupancy_state.hpp>

// namespace sdm
// {
//     HierarchicalQValueFunction::HierarchicalQValueFunction(number horizon, double learning_rate, std::shared_ptr<QInitializer> initializer, double ball_r, bool keep_map)
//         : QValueFunction(horizon), horizon_(horizon), learning_rate_(learning_rate), initializer_(initializer), ball_r_(ball_r), keep_map_(keep_map)
//     {
//         this->Psi = std::vector<psi>(this->isInfiniteHorizon() ? 1 : this->horizon_ + 1, psi());
//         this->num_states_ = 0;
//         this->num_key_states_ = 0;
//         // this->num_states_vector_ = std::vector<int>(this->isInfiniteHorizon() ? 1 : this->horizon_ + 1, 0);
//         this->closest_s_map_ = std::vector<std::unordered_map<std::shared_ptr<OccupancyStateInterface>, std::shared_ptr<OccupancyStateInterface>>>(this->isInfiniteHorizon() ? 1 : this->horizon_ + 1, std::unordered_map<std::shared_ptr<OccupancyStateInterface>, std::shared_ptr<OccupancyStateInterface>>());
//     }

//     HierarchicalQValueFunction::HierarchicalQValueFunction(number horizon, double learning_rate, double default_value, double ball_r, bool keep_map) : HierarchicalQValueFunction(horizon, learning_rate, std::make_shared<ValueInitializer>(default_value), ball_r, keep_map)
//     {
        
//     }

//     void HierarchicalQValueFunction::initialize()
//     {

//     }

//     void HierarchicalQValueFunction::initialize(double default_value, number t)
//     {

//     }

//     std::shared_ptr<VectorInterface<std::shared_ptr<Action>, double>> HierarchicalQValueFunction::getQValuesAt(const std::shared_ptr<State> &state, number t)
//     {
//         throw sdm::exception::NotImplementedException();
//     }

//     double HierarchicalQValueFunction::getQValueAt(const std::shared_ptr<State> &state, const std::shared_ptr<Action> &action, number t)
//     {
//         throw sdm::exception::NotImplementedException();
//     }

//     double HierarchicalQValueFunction::getQValueAt(const std::shared_ptr<OccupancyStateInterface> &s, const std::shared_ptr<JointHistoryInterface> &o, const std::shared_ptr<Action> &u, number t)
//     {
//         auto h = this->isInfiniteHorizon() ? 0 : t;
//         auto s_ = this->getHyperPlaneIndex(s, t);
//         return this->Psi[h].at(s_).getQValueAt(o, u, t);

//     }

//     void HierarchicalQValueFunction::updateQValueAt(const std::shared_ptr<State> &state, const std::shared_ptr<Action> &action, number t, double delta)
//     {
//         throw sdm::exception::NotImplementedException();
//     }

//     void HierarchicalQValueFunction::updateQValueAt(const std::shared_ptr<OccupancyStateInterface> &s, const std::shared_ptr<JointHistoryInterface> &o, const std::shared_ptr<Action> &u, number t, double delta)
//     {
//         // std::cout << "HierarchicalQValueFunction::updateQValueAt()" << std::endl;
//         auto h = this->isInfiniteHorizon() ? 0 : t;
//         auto s_ = this->getHyperPlaneIndex(s, t);
//         this->Psi[h].at(s_).updateQValueAt(o, u, t, delta);
//     }

//     void HierarchicalQValueFunction::updateQValueAt(const std::shared_ptr<State> &, const std::shared_ptr<Action> &, number)
//     {
//         throw sdm::exception::NotImplementedException();
//     }

//     bool HierarchicalQValueFunction::isNotSeen(const std::shared_ptr<State> &state, number t)
//     {
//         return false;
//     }

//     int HierarchicalQValueFunction::getNumStates() const
//     {
//         // return this->num_states_;
//         return this->num_key_states_;
//     }

//     void HierarchicalQValueFunction::initializeQValueFunctionAtWith(const std::shared_ptr<OccupancyStateInterface> &s, TabularQValueFunction &q_, number t)
//     {   
//         // std::cout << "HierarchicalQValueFunction::initializeQValueFunctionAtWith()" << std::endl;
//         auto h = this->isInfiniteHorizon() ? 0 : t;
  
//         this->Psi[h].emplace(s, TabularQValueFunction(0, learning_rate_, initializer_));
//         this->Psi[h].at(s) = q_;
//         this->num_key_states_++;
        
        
//     }

//     void HierarchicalQValueFunction::initializeToZeroQValueFunctionAt(const std::shared_ptr<OccupancyStateInterface> &s, number t)
//     {   
//         // std::cout << "HierarchicalQValueFunction::initializeToZeroQValueFunctionAt()" << std::endl;

//         auto h = this->isInfiniteHorizon() ? 0 : t;
  
//         this->Psi[h].emplace(s, TabularQValueFunction(0, learning_rate_, initializer_));
//         this->Psi[h].at(s).initialize(0);
//         this->num_key_states_++;
//         this->closest_s_map_[h].emplace(s, s);
        
//     }

//     std::shared_ptr<OccupancyStateInterface> HierarchicalQValueFunction::getHyperPlaneIndex(const std::shared_ptr<OccupancyStateInterface> &s, number t)
//     {
//         // std::cout << "HierarchicalQValueFunction::getClosestS()" << std::endl;

//         auto h = this->isInfiniteHorizon() ? 0 : t;

//         // If s already has a label
//         if (this->closest_s_map_[h].find(s) != this->closest_s_map_[h].end())
//         {
//             return this->closest_s_map_[h].find(s)->second;
//         }
        
//         // If t=h is empty, s is the very first one to arrive.
//         else if (this->Psi[h].size() == 0)
//         {
//             this->num_states_++;
//             // std::cout << "A" << std::endl;
//             this->initializeToZeroQValueFunctionAt(s, t);
//             return s;
//         }

//         else
//         {
//             this->num_states_++;
//             // std::cout << "C" << std::endl;
//             double smallest_distance = 10000.0;
//             std::shared_ptr<OccupancyStateInterface> closest_s;
            
//             for (auto const& [s_, q_] : this->Psi[h])
//             {
//                 // std::cout << *s_ << std::endl;
//                 double distance = s_->minus(s);
//                 // std::cout << "distance " << distance << std::endl;
//                 if (distance < smallest_distance)
//                 {
//                     smallest_distance = distance;
//                     closest_s = s_;
//                 }
//             }
//             if (this->areInTheSameBall(s, closest_s, t))
//             {
//                 return closest_s;
//             }
//             else
//             {
//                 this->initializeQValueFunctionAtWith(s, this->Psi[h][closest_s], t);
//                 return s;
//             }
//         }
//     }

//     bool HierarchicalQValueFunction::areInTheSameBall(const std::shared_ptr<OccupancyStateInterface> &s, const std::shared_ptr<OccupancyStateInterface> &s_, number t)
//     {
//         // std::cout << "HierarchicalQValueFunction::areInTheSameBall()" << std::endl;
//         return (s_->minus(s) < this->ball_r_);
//     }

//     std::string HierarchicalQValueFunction::str() const
//     {
//         std::ostringstream res;
//         res << "<hierarchical_qvalue_function_v2 horizon=\"" << ((this->isInfiniteHorizon()) ? "inf" : std::to_string(this->getHorizon())) << "\">" << std::endl;
//         for (sdm::size_t i = 0; i < this->Psi.size(); i++)
//         {
//             res << "\t<timestep=\"" << ((this->isInfiniteHorizon()) ? "all" : std::to_string(i)) << "\" default=\"" << 0 << "\">" << std::endl;
//             for (auto const& [s, q] : this->Psi[i])
//             {
//                 auto jh = *s->getJointHistories().begin();
//                 if (jh->getHorizon() < this->horizon_)
//                 {
//                     res << "\t\t<S-Q>" << std::endl;
//                     tools::indentedOutput(res, s->str().c_str(), 3);
//                     res << std::endl;
//                     tools::indentedOutput(res, q.str().c_str(), 3);
//                     res << "\t\t</S-Q>" << std::endl;
//                 }
//             }
//             res << "\t</timestep>" << std::endl;
//         }
//         res << "</hierarchical_qvalue_function_v2>" << std::endl;
//         return res.str();
//     }
// } // namespace sdm