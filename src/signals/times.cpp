#include "times.h"


ParameterConstraintMap TimeIs::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;
    // Define the parameter constraints and the variables to be used
    SP_constraints t_time;
    t_time.usage_mode = CNT_MINMAX;
    t_time.min = Time(0,0);
    t_time.max = Time(23, 59);
    t_time.mut_min = Time(0,0);
    t_time.mut_max = Time(23, 59);
    t_time.mut_power = Time(1, 0);
    t_time.is_mutable = true;
    t_time.default_value = Time(12,0);
    t_constraints["time"] = t_time;
    return t_constraints;
}

void TimeIs::init_codegen_variables()
{
    // Define used variables for code generation
    m_variables["before"] = VAR_TYPE_INT;
    m_variables["now"] = VAR_TYPE_INT;
}

void TimeIs::update_data( const DataMatrix& data )
{
    dt = &(data.dt);
    time = (get<Time>(m_parameters["time"])).as_int();
}

void TimeIs::compute_init( const Env& env, int idx )
{
    before = -1;
    now = (*dt)[idx].time().as_int();
}

void TimeIs::compute_tick( const Env& env, int idx )
{
    before = now;
    now = (*dt)[idx].time().as_int();
    if (before > now)
        before = -1;

    if ((before < time) && (now >= time))
    {
    	m_outp = true;
    }
    else
    {
    	m_outp = false;
    }
}


std::string TimeIs::get_el_init()
{
    std::stringstream s;

    s << fmt("    {before} = -1;\n"
             "    {now} = {curtime};") % m_efd;

    return s.str();
}

std::string TimeIs::get_el_tick()
{
    std::stringstream s;
    s << fmt("    {before} = {now};\n"
             "    {now} = {curtime};\n"
             "    if ({before} > {now}) then \n"
             "        {before} = -1;\n"
             "    if (({before} < {time}) and ({now} >= {time})) then \n"
             "    begin \n"
             "    \t{outp} = true;\n"
             "    end \n"
             "    else \n"
             "    begin \n"
             "    \t{outp} = false;\n"
             "    end;") % m_efd;

    return s.str();
}

std::string TimeIs::get_mql_init()
{
    std::stringstream s;
    s << fmt("    {before} = -1;\n"
             "    {now} = {curtime};") % m_mfd;
    return s.str();
}

std::string TimeIs::get_mql_tick()
{
    std::stringstream s;
    s << fmt("    {before} = {now};\n"
             "    {now} = {curtime};\n"
             "    if ({before} > {now})\n"
             "        {before} = -1;\n"
             "\n"
             "    if (({before} < {time}) && ({now} >= {time}))\n"
             "    {be}\n"
             "    \t{outp} = true;\n"
             "    {en}\n"
             "    else\n"
             "    {be}\n"
             "    \t{outp} = false;\n"
             "    {en}") % m_mfd;
    return s.str();
}



ParameterConstraintMap TimeIsBefore::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;
    // Define the parameter constraints and the variables to be used
    SP_constraints t_time;
    t_time.usage_mode = CNT_MINMAX;
    t_time.min = Time(0,0);
    t_time.max = Time(23, 59);
    t_time.mut_min = Time(0,0);
    t_time.mut_max = Time(23, 59);
    t_time.mut_power = Time(1, 0);
    t_time.is_mutable = true;
    t_time.default_value = Time(12,0);
    t_constraints["time"] = t_time;
    return t_constraints;
}

void TimeIsBefore::init_codegen_variables()
{
    // Define used variables for code generation
    m_variables["before"] = VAR_TYPE_INT;
    m_variables["now"] = VAR_TYPE_INT;
}

void TimeIsBefore::update_data( const DataMatrix& data )
{
    dt = &(data.dt);
    time = (get<Time>(m_parameters["time"])).as_int();
}

void TimeIsBefore::compute_tick( const Env& env, int idx )
{
    if ((*dt)[idx].time().as_int() < time)
    	m_outp = true;
    else
    	m_outp = false;
}

std::string TimeIsBefore::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + el_cur_time() + " < " + var("time") + ")");
	return s.str();
}

std::string TimeIsBefore::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + mql_cur_time() + " < " + var("time") + ")");
	return s.str();
}


ParameterConstraintMap TimeIsAfter::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;
    // Define the parameter constraints and the variables to be used
    SP_constraints t_time;
    t_time.usage_mode = CNT_MINMAX;
    t_time.min = Time(0,0);
    t_time.max = Time(23, 59);
    t_time.mut_min = Time(0,0);
    t_time.mut_max = Time(23, 59);
    t_time.mut_power = Time(1, 0);
    t_time.is_mutable = true;
    t_time.default_value = Time(12,0);
    t_constraints["time"] = t_time;
    return t_constraints;
}

void TimeIsAfter::init_codegen_variables()
{
    // Define used variables for code generation
    m_variables["before"] = VAR_TYPE_INT;
    m_variables["now"] = VAR_TYPE_INT;
}

void TimeIsAfter::update_data( const DataMatrix& data )
{
    dt = &(data.dt);
    time = (get<Time>(m_parameters["time"])).as_int();
}

void TimeIsAfter::compute_tick( const Env& env, int idx )
{
    if ((*dt)[idx].time().as_int() > time)
    	m_outp = true;
    else
    	m_outp = false;
}

std::string TimeIsAfter::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + el_cur_time() + " > " + var("time") + ")");
	return s.str();
}

std::string TimeIsAfter::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + mql_cur_time() + " > " + var("time") + ")");
	return s.str();
}


ParameterConstraintMap TimeIsBetween::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;

    // Define the parameter constraints and the variables to be used
    SP_constraints t_start_time;
    t_start_time.usage_mode = CNT_MINMAX;
    t_start_time.min = Time(0,0);
    t_start_time.max = Time(23, 59);
    t_start_time.mut_min = Time(0,0);
    t_start_time.mut_max = Time(23, 59);
    t_start_time.mut_power = Time(1, 0);
    t_start_time.is_mutable = true;
    t_start_time.default_value = Time(12,0);
    t_constraints["start_time"] = t_start_time;

    SP_constraints t_end_time;
    t_end_time.usage_mode = CNT_MINMAX;
    t_end_time.min = Time(0,0);
    t_end_time.max = Time(23, 59);
    t_end_time.mut_min = Time(0,0);
    t_end_time.mut_max = Time(23, 59);
    t_end_time.mut_power = Time(1, 0);
    t_end_time.is_mutable = true;
    t_end_time.default_value = Time(12,0);
    t_constraints["end_time"] = t_end_time;

    return t_constraints;
}

void TimeIsBetween::update_data( const DataMatrix& data )
{
    dt = &(data.dt);
    st = (get<Time>(m_parameters["start_time"])).as_int();
    et = (get<Time>(m_parameters["end_time"])).as_int();
}

void TimeIsBetween::compute_tick( const Env& env, int idx )
{
    m_outp = ((*dt)[idx].time().as_int() >= st) && ((*dt)[idx].time().as_int() <= et);
}

std::string TimeIsBetween::get_el_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("((%s > %s) and (%s < %s))")
        % el_cur_time() % var("start_time") % el_cur_time() % var("end_time"));
    return s.str();
}

std::string TimeIsBetween::get_mql_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("((%s > %s) && (%s < %s))")
        % mql_cur_time() % var("start_time") % mql_cur_time() % var("end_time"));
    return s.str();
}


void NewDay::update_data(const DataMatrix& data)
{
	dt = &(data.dt);
}

void NewDay::compute_tick(const Env& env, int idx)
{
	m_outp = (*dt)[idx].date() != (*dt)[prev_idx].date();
	prev_idx = idx;
}

std::string NewDay::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % "(Date[0] <> Date[1])";
	return s.str();
}

std::string NewDay::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % "(TimeDay(Time[0]) != TimeDay(Time[1]))";
	return s.str();
}
