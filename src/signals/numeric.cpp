#include "numeric.h"


ParameterConstraintMap IntConst::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;

    // Define the parameter constraints and the variables to be used
    SP_constraints t_val;
    t_val.usage_mode = CNT_MINMAX;
    t_val.min = -100000;
    t_val.max = 100000;
    t_val.mut_min = 0;
    t_val.mut_max = 100;
    t_val.mut_power = 10;
    t_val.is_mutable = true;
    t_val.default_value = 0;
    t_constraints["val"] = t_val;

    return t_constraints;
}

void IntConst::compute_tick( const Env& env, int idx )
{
	m_outp = val;
}

void IntConst::update_data( const DataMatrix& data )
{
    val = convert_to_int(m_parameters["val"]);
}

std::string IntConst::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % var("val");
	return s.str();
}

std::string IntConst::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % var("val");
	return s.str();
}

ParameterConstraintMap FloatConst::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;

    // Define the parameter constraints and the variables to be used
    SP_constraints t_val;
    t_val.usage_mode = CNT_MINMAX;
    t_val.min = -100000.0;
    t_val.max = 100000.0;
    t_val.mut_min = 0.0;
    t_val.mut_max = 100.0;
    t_val.mut_power = 10.0;
    t_val.is_mutable = true;
    t_val.default_value = 0.0;
    t_constraints["val"] = t_val;

    return t_constraints;
}

void FloatConst::compute_tick( const Env& env, int idx )
{
	m_outp = val;
}

void FloatConst::update_data( const DataMatrix& data )
{
    val = convert_to_float(m_parameters["val"]);
}

std::string FloatConst::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % var("val");
	return s.str();
}

std::string FloatConst::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % var("val");
	return s.str();
}


ParameterConstraintMap BoolConst::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;

    // Define the parameter constraints and the variables to be used
    SP_constraints t_val;
    t_val.usage_mode = CNT_POSSIBLE_VALUES;
    t_val.possible_values.push_back( SP_type( false ) );
    t_val.possible_values.push_back( SP_type( true ) );
    t_val.is_mutable = true;
    t_val.default_value = false;
    t_constraints["val"] = t_val;

    return t_constraints;
}

void BoolConst::update_data( const DataMatrix& data )
{
    val = get<bool>(m_parameters["val"]);
}

void BoolConst::compute_tick( const Env& env, int idx )
{
	m_outp = val;
}

std::string BoolConst::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % var("val");
	return s.str();
}

std::string BoolConst::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % var("val");
	return s.str();
}
