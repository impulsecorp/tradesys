#ifndef CONSTRAINTS_H
#define CONSTRAINTS_H

/* This module contains classes regarding the signal parameter description/constraints
 * and declares the global constraint bank
 */

#include "base_signal.h"
#include "common_types.h"

// This class describes the min/max range and optimization behavior of a parameter
// it also contains functions for setting the default value and mutation (TODO).
enum ConstraintUsageMode { CNT_MINMAX = 0, CNT_POSSIBLE_VALUES = 1};

class SP_constraints
{
public:
    ConstraintUsageMode usage_mode; // MinMax / PossibleValues

    // can be mutated/randomized or not
    bool is_mutable;

    // absolute range
    SP_type min, max; // for numeric types

    // range for replacement mutation or randomization
    SP_type mut_min, mut_max; // for numeric types
    // max power for +/- mutation
    SP_type mut_power;

    std::vector<SP_type> possible_values; // for all types
    SP_type default_value;

    SP_constraints();
};

extern ParameterConstraintDataBase GlobalConstraints;
void init_system_constraints();


#endif // CONSTRAINTS_H
