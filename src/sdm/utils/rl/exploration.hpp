#pragma once

#include <sdm/utils/value_function/qvalue_function.hpp>

namespace sdm
{

    class Exploration
    {
    public:
        virtual ~Exploration() {}
        virtual void reset(unsigned long) = 0;
        virtual void update(unsigned long) = 0;
        // virtual Space getAction(std::shared_ptr<QValueFunction>, std::shared_ptr<Space>, number) = 0;
    };

    class EpsGreedy : public Exploration
    {
    protected:
        double epsilon, eps_deb_, eps_fin_, deb_expl_, fin_expl_;

        unsigned long init_expl_step, final_expl_step;

    public:
        EpsGreedy(double eps_deb = 1.0, double eps_fin = 0.1, double deb_expl = 0.1, double fin_expl = 0.9) : epsilon(eps_deb), eps_deb_(eps_deb), eps_fin_(eps_fin), deb_expl_(deb_expl), fin_expl_(fin_expl)
        {
            assert(fin_expl >= deb_expl);
        }

        EpsGreedy(Config config) : EpsGreedy(config.get("eps_deb", 1.0),
                                             config.get("eps_fin", 0.1),
                                             config.get("deb_expl", 0.1),
                                             config.get("fin_expl", 0.9))
        {
            assert(this->fin_expl_ >= this->deb_expl_);
        }

        void reset(unsigned long nb_timesteps)
        {
            this->epsilon = this->eps_deb_;
            this->init_expl_step = (long)(this->deb_expl_ * nb_timesteps);
            this->final_expl_step = (long)(this->fin_expl_ * nb_timesteps);
        }

        void update(unsigned long t)
        {
            if (t > this->init_expl_step && t < this->final_expl_step)
            {
                if (this->eps_deb_ >= this->eps_fin_)
                {
                    this->epsilon = std::max(this->eps_fin_, this->eps_deb_ - (t - this->init_expl_step) * (this->eps_deb_ - this->eps_fin_) / (this->final_expl_step - this->init_expl_step));
                }
                else
                {
                    this->epsilon = std::min(this->eps_fin_, this->eps_deb_ - (t - this->init_expl_step) * (this->eps_deb_ - this->eps_fin_) / (this->final_expl_step - this->init_expl_step));
                }
            }
        }

        double getEpsilon()
        {
            return this->epsilon;
        }

        // Space getAction(std::shared_ptr<QValueFunction>, std::shared_ptr<Space>, number)
        // {
        //     throw sdm::exception::NotImplementedException();
        // }
    };

    class EpsGreedyDecay : public EpsGreedy
    {
    protected:
        double eps_start, eps_end, eps_decay;

    public:
        EpsGreedyDecay(double eps_start = 1.0, double eps_end = 0.001, double eps_decay = 10000) : eps_start(eps_start), eps_end(eps_end), eps_decay(eps_decay)
        {
            this->epsilon = eps_start;
        }

        EpsGreedyDecay(Config config) : EpsGreedyDecay(config.get("eps_start", 1.0),
                                                       config.get("eps_end", 0.001),
                                                       config.get("eps_decay", 10000))
        {
        }

        void reset(unsigned long nb_epsisode)
        {
            this->epsilon = this->eps_start;
        }

        void update(unsigned long episode)
        {
            this->epsilon = this->eps_end + (this->eps_start - this->eps_end) * exp(-1.0 * episode / this->eps_decay);
        }

        // Space getAction(std::shared_ptr<QValueFunction>, std::shared_ptr<Space>, number)
        // {
        //     throw sdm::exception::NotImplementedException();
        // }
    };
} // namespace sdm