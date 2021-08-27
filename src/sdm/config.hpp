#pragma once

namespace sdm
{
    namespace config
    {
        /** Precision used to compare elements */
        const double PRECISION_SDMS_VECTOR = 0.0001;
        const double PRECISION_MAPPED_VECTOR = 0.0001;
        const double PRECISION_VECTO_INTERFACE = PRECISION_MAPPED_VECTOR;

        const double PRECISION_BELIEF = 0.001;
        const double PRECISION_OCCUPANCY_STATE = 0.001;
        const double PRECISION_COMPRESSION = 0.1;

        /** Number of decimal to display */
        const unsigned short BELIEF_DECIMAL_PRINT = 5;
        const unsigned short OCCUPANCY_DECIMAL_PRINT = 5;
        const unsigned short VALUE_DECIMAL_PRINT = 4;

        /** Problem path */
        const std::string PROBLEM_PATH = "/usr/local/share/sdms/world/";

    } // namespace config

}