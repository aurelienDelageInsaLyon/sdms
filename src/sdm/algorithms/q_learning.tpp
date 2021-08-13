#include <sdm/algorithms/q_learning.hpp>
#include <sdm/world/occupancy_mdp.hpp>

namespace sdm
{

    template <class TInput>
    QLearning<TInput>::QLearning(std::shared_ptr<GymInterface> &env,
                         std::shared_ptr<ExperienceMemoryInterface> experience_memory,
                         std::shared_ptr<QValueFunction<TInput>> q_value_table,
                         std::shared_ptr<QValueFunction<TInput>> q_value_table_target,
                         std::shared_ptr<QValueBackupInterface> backup,
                         std::shared_ptr<EpsGreedy> exploration,
                         number horizon,
                         double discount,
                         double lr,
                         double smooth,
                         unsigned long num_episodes,
                         std::string name
                         ) : env_(env),
                             experience_memory_(experience_memory),
                             q_value_table_(q_value_table),
                             q_value_table_target_(q_value_table_target),
                             backup_(backup),
                             exploration_process(exploration),
                             horizon_(horizon),
                             discount_(discount),
                             lr_(lr),
                             smooth_(smooth),
                             num_episodes_(num_episodes),
                             target_update_(1),
                             name_(name)
    {
    }

    template <class TInput>
    void QLearning<TInput>::initLogger()
    {
        std::string format = "#> Episode : {}\tStep : {}/?\tEpsilon : {}\tValue : {}\tValue_p : {}\tT(s) : {}\tN(S) : {}\n";

        auto std_logger = std::make_shared<sdm::StdLogger>(format);
        auto file_logger = std::make_shared<sdm::FileLogger>(this->name_ + ".txt", format);
        auto csv_logger = std::make_shared<sdm::CSVLogger>(this->name_, std::vector<std::string>{"Episode", "Step", "Epsilon", "Value", "Value_p", "Time", "N(S)"});

        this->logger_ = std::make_shared<sdm::MultiLogger>(std::vector<std::shared_ptr<Logger>>{std_logger, file_logger, csv_logger});
    }

    template <class TInput>
    void QLearning<TInput>::do_initialize()
    {
        this->initLogger();

        this->q_value_table_->initialize();
        this->q_value_table_target_->initialize();
    }

    template <class TInput>
    // std::shared_ptr<GymInterface> env, long nb_timesteps, number horizon, number test_freq, number save_freq, std::string save_folder, number verbose, long timestep_init, std::string log_file
    void QLearning<TInput>::do_solve()
    {
        this->global_step = 0;
        this->episode = 0;
        this->E_R = 0.0;
        this->t_begin = clock();

        this->exploration_process->reset(this->num_episodes_);

        while (this->episode < this->num_episodes_)
        {
            // Update exploration process
            this->exploration_process->update(this->episode);

            // Do one episode
            this->do_episode();

            // Test current policy and write logs
            if (this->do_log_)
            {
                this->logger_->log(
                    this->episode, 
                    this->global_step, 
                    this->exploration_process->getEpsilon(),
                    this->backup_->getValueAt(this->env_->reset()->toState(), 0), 
                    this->E_R,
                    (float)(clock() - this->t_begin) / CLOCKS_PER_SEC,
                    this->q_value_table_->getNumStates()
                );
                this->do_log_ = false;
            }
            if (this->do_test_)
            {
                this->do_test();
                this->do_test_ = false;
            }
        }

        // std::cout << *this->q_value_table_ << std::endl;


        // std::ofstream QValueStream(this->name_ + ".qvalue");
        // QValueStream << *this->q_value_table_ << std::endl;
        // QValueStream.close();

    }

    template <class TInput>
    void QLearning<TInput>::do_save()
    {
        this->q_value_table_->save(this->name_ + "_qvalue.bin");
    }

    template <class TInput>
    void QLearning<TInput>::do_test()
    {
    }

    template <class TInput>
    void QLearning<TInput>::do_episode()
    {
        this->step = 0;
        this->R = 0.0;
        this->episode++;
        this->observation = this->env_->reset();
        this->action = this->select_action(this->observation, this->step);

        unsigned long stop_cond = this->global_step + this->horizon_;
        while (this->global_step < stop_cond)
        {
            this->do_step();

            //Save the model
            this->do_save_ = (this->global_step % this->save_freq == 0);
            this->do_log_ = (this->global_step % this->log_freq == 0);
            this->do_test_ = (this->global_step % this->test_freq == 0);
            if (this->is_done)
            {
                break;
            }
        }
        this->E_R = this->smooth_ * this->E_R + (1 - this->smooth_) * this->R;
    }

    template <class TInput>
    void QLearning<TInput>::do_step()
    {   
        // One step in env and get next observation and rewards
        std::tie(this->next_observation, this->rewards_, this->is_done) = this->env_->step(this->action);

        // Action selection following policy and exploration process
        this->next_action = this->select_action(this->next_observation, this->step + 1);

        auto next_greedy_action = this->select_greedy_action(this->next_observation, this->step + 1);

        // Push experience to memory
        // this->experience_memory_->push(this->observation, this->action, this->rewards_[0], this->next_observation, this->next_action, this->step);
        this->experience_memory_->push(this->observation, this->action, this->rewards_[0], this->next_observation, next_greedy_action, this->step);

        // Backup and get Q Value Error
        double delta = this->backup_->update(this->step);

        this->observation = this->next_observation;
        this->action = this->next_action;
        this->R += this->rewards_[1];
        this->step++;
        this->global_step++;
    }

    template <class TInput>
    void QLearning<TInput>::update_model()
    {
        // auto batch = this->experience_->sample(this->batch_size_);

        // for (const auto &transition : batch)
        // {
        //     auto [s, a, r, s_] = transition;
        //     target = r + this->discount_ * this->getQValueAt(s_, h + 1)->max() - this->getQValueAt(s, a, h);
        //     this->q_value_table_->updateQValueAt(s, a, h, target);
        // }

        // if (this->off_policy && t % this->target_update_freq == 0)
        // {
        //     this->update_target_model()
        // }
    }

    template <class TInput>
    void QLearning<TInput>::update_target()
    {
        *this->q_value_table_target_ = *this->q_value_table_;
    }

    template <class TInput>
    std::shared_ptr<Action> QLearning<TInput>::select_action(const std::shared_ptr<Observation> &observation, number t)
    {
        // std::cout << "-------- QLearning<TInput>::select_action ---------" << std::endl;
        
        // if (((rand() / double(RAND_MAX)) < this->exploration_process->getEpsilon()) || this->q_value_table_->isNotSeen(observation->toState(), t))
        if ((rand() / double(RAND_MAX)) < this->exploration_process->getEpsilon())
        {
            // std::cout << "-------- RANDOM ---------" << std::endl;
            return this->env_->getRandomAction(observation, t);
        }
        else
        {
            // std::cout << "-------- GREEDY ---------" << std::endl;
            auto a = this->backup_->getGreedyAction(observation->toState(), t);
            if (a == nullptr)
            {
                return this->env_->getRandomAction(observation, t);
            }
            return a;
            // return this->env_->getRandomAction(observation, t);
        }

    }

    template <class TInput>
    std::shared_ptr<Action> QLearning<TInput>::select_greedy_action(const std::shared_ptr<Observation> &observation, number t)
    {
        return this->backup_->getGreedyAction(observation->toState(), t);
    }

    // template <class TInput>
    // void QLearning<TInput>::saveResults(std::string filename, double other)
    // {
    //     std::ofstream ofs;
    //     ofs.open(filename, std::ios::out | std::ios::app);
    //     ofs << other << ",";
    //     ofs << this->backup_->getValueAt(this->env_->reset()->toState(), 0) << ",";
    //     ofs << std::static_pointer_cast<OccupancyMDP>(this->env_)->getMDPGraph()->getNumNodes() << ",";
    //     // number num_max_jhist = 0, tmp;
    //     // for (const auto &state : std::static_pointer_cast<OccupancyMDP>(this->env_)->getStoredStates())
    //     // {
    //     //     if (num_max_jhist < (tmp = state->toOccupancyState()->getJointHistories().size()))
    //     //     {
    //     //         num_max_jhist = tmp;
    //     //     }
    //     // }
    //     // ofs << num_max_jhist << ",";
    //     ofs << ((float)(clock() - this->t_begin) / CLOCKS_PER_SEC);

    //     ofs << "\n";
    //     ofs.close();
    // }

} // namespace sdm
