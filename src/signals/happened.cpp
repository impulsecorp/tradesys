#include "happened.h"


void HappenedToday::init_codegen_variables()
{
    m_variables["ht"] = VAR_TYPE_BOOL;
}

void HappenedToday::update_data( const DataMatrix& data )
{
    dt = &(data.dt);
}

void HappenedToday::compute_init( const Env& env, int idx )
{
    ht = false;
}

void HappenedToday::compute_tick( const Env& env, int idx )
{
    if ((*dt)[idx].date() != (*dt)[prev_idx].date())
    {
        ht = false;
    }
    prev_idx = idx;

    if (get<bool>(m_children[0]->get_output()))
    {
        ht = true;
    }

	m_outp = ht;
}


std::string HappenedToday::get_el_init()
{
    std::stringstream s;
    s << var("ht") << " = false; ";
    return s.str();
}

std::string HappenedToday::get_el_tick()
{
    std::stringstream s;
    s << format(
        "if Date[0] <> Date[1] then "
        "begin "
        "%s = false; "
        "end; "
        "if %s then "
        "begin "
        "%s = true; "
        "end;")
        % var("ht") % m_children[0]->get_el_output() % var("ht");

   	s << format("%s = %s;") % var("outp") % var("ht");

    return s.str();
}

std::string HappenedToday::get_mql_init()
{
    std::stringstream s;
    s << var("ht") << " = false; ";
    return s.str();
}

std::string HappenedToday::get_mql_tick()
{
    std::stringstream s;
    s << format(
        "if (TimeDay(Time[0]) != TimeDay(Time[1])) "
        "{ "
        "%s = false; "
        "} "
        "if (%s) "
        "{ "
        "%s = true; "
        "} ")
        % var("ht") % m_children[0]->get_mql_output() % var("ht");

    s << format("%s = %s;") % var("outp") % var("ht");

    return s.str();
}


ParameterConstraintMap HappenedAtTimeSignal::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;

	// Define the parameter constraints and the variables to be used
    SP_constraints t_time;
    t_time.usage_mode = CNT_MINMAX;
    t_time.min = Time(0,0);
    t_time.max = Time(23, 59);
    t_time.mut_min = Time(0,0);
    t_time.mut_max = Time(23,59);
    t_time.mut_power = Time(1,0);
    t_time.is_mutable = true;
    t_time.default_value = Time(12,0);
    t_constraints["time"] = t_time;

    return t_constraints;
}

void HappenedAtTimeSignal::init_codegen_variables()
{
    m_variables["ht"] = VAR_TYPE_BOOL;
    m_variables["tbef"] = VAR_TYPE_INT;
    m_variables["tnow"] = VAR_TYPE_INT;
}

void HappenedAtTimeSignal::update_data( const DataMatrix& data )
{
    dt = &(data.dt);
    time = (get<Time>(m_parameters["time"])).as_int();
}

void HappenedAtTimeSignal::compute_init( const Env& env, int idx )
{
    tbef = -1;
    tnow = (*dt)[idx].time().as_int();
    ht = false;
}

std::string HappenedAtTimeSignal::get_el_init()
{
    std::stringstream s;
    s << var("ht")   << " = false; " <<
         var("tbef") << " = -1; " <<
         var("tnow") << " = " << el_cur_time() << "; ";
    return s.str();
}

std::string HappenedAtTimeSignal::get_mql_init()
{
    std::stringstream s;
    s << var("ht")   << " = false; " <<
    	 var("tbef") << " = -1; " <<
    	 var("tnow") << " = " << mql_cur_time() << "; ";
    return s.str();
}

void HappenedAtTime::compute_tick( const Env& env, int idx )
{
    tbef = tnow;
    tnow = (*dt)[idx].time().as_int();
    if (tbef > tnow)
        tbef = -1;
    
    // reset it on new days
    if ((*dt)[idx].date() != (*dt)[prev_idx].date())
    {
        ht = false;
    }
    prev_idx = idx;

    if ((get<bool>(m_children[0]->get_output())) &&
        (tbef < time) && (tnow >= time))
        ht = true;

	m_outp = ht;
}

std::string HappenedAtTime::get_el_tick()
{
    std::stringstream s;
    s << format(
        "%s = %s;"
        "%s = %s;"
        "if %s > %s then %s = -1;"
        "if Date[0] <> Date[1] then "
        "begin "
        "%s = false; "
        "end; "
        "if (%s) and (((%s < %s) and (%s >= %s))) then "
        "begin "
        "%s = true; "
        "end;")
        % var("tbef") % var("tnow") 
        % var("tnow") % el_cur_time() 
        % var("tbef") % var("tnow") % var("tbef") 
        % var("ht") 
        % m_children[0]->get_el_output()
        % var("tbef") % var("time") 
        % var("tnow") % var("time") 
        % var("ht");

    s << format("%s = %s;") % var("outp") % var("ht");

    return s.str();
}

std::string HappenedAtTime::get_mql_tick()
{
    std::stringstream s;
    s << format(
        "%s = %s;"
        "%s = %s;"
        "if (%s > %s) { %s = -1; }"
        "if (TimeDay(Time[0]) != TimeDay(Time[1])) "
        "{ "
        "%s = false; "
        "} "
        "if ((%s) && (((%s < %s) && (%s >= %s)))) "
        "{ "
        "%s = true; "
        "} ")
        % var("tbef") % var("tnow") 
        % var("tnow") % mql_cur_time() 
        % var("tbef") % var("tnow") % var("tbef") 
        % var("ht") 
        % m_children[0]->get_mql_output()
        % var("tbef") % var("time") 
        % var("tnow") % var("time") 
        % var("ht");

    s << format("%s = %s;") % var("outp") % var("ht");

    return s.str();
}


void HappenedBeforeTime::compute_tick( const Env& env, int idx )
{
    tbef = tnow;
    tnow = (*dt)[idx].time().as_int();
    if (tbef > tnow)
        tbef = -1;

    // reset it on new days
    if ((*dt)[idx].date() != (*dt)[prev_idx].date())
    {
        ht = false;
    }
    prev_idx = idx;

    if ((get<bool>(m_children[0]->get_output())) && (tnow < time))
    {
        ht = true;
    }

	m_outp = ht;
}

std::string HappenedBeforeTime::get_el_tick()
{
    std::stringstream s;
    s << format(
        "%s = %s;"
        "%s = %s;"
        "if %s > %s then %s = -1;"
        "if Date[0] <> Date[1] then "
        "begin "
        "%s = false; "
        "end; "
        "if (%s) and (%s < %s) then "
        "begin "
        "%s = true; "
        "end;")
        % var("tbef") % var("tnow") 
        % var("tnow") % el_cur_time() 
        % var("tbef") % var("tnow") % var("tbef") 
        % var("ht") 
        % m_children[0]->get_el_output()
        % var("tnow") % var("time") 
        % var("ht");

    s << format("%s = %s;") % var("outp") % var("ht");

    return s.str();
}

std::string HappenedBeforeTime::get_mql_tick()
{
    std::stringstream s;
    s << format(
        "%s = %s;"
        "%s = %s;"
        "if (%s > %s) { %s = -1; }"
        "if (TimeDay(Time[0]) != TimeDay(Time[1])) "
        "{ "
        "   %s = false; "
        "} "
        "if ((%s) && (%s < %s)) "
        "{ "
        "   %s = true; "
        "} ")
        % var("tbef") % var("tnow") 
        % var("tnow") % mql_cur_time() 
        % var("tbef") % var("tnow") % var("tbef") 
        % var("ht") 
        % m_children[0]->get_mql_output()
        % var("tnow") % var("time") 
        % var("ht");

    s << format("%s = %s;") % var("outp") % var("ht");

    return s.str();
}

void HappenedAfterTime::compute_tick( const Env& env, int idx )
{
    tbef = tnow;
    tnow = (*dt)[idx].time().as_int();
    if (tbef > tnow)
        tbef = -1;

    // reset it on new days
    if ((*dt)[idx].date() != (*dt)[prev_idx].date())
    {
        ht = false;
    }
    prev_idx = idx;

    if ((get<bool>(m_children[0]->get_output())) && (tnow > time))
    {
        ht = true;
    }

	m_outp = ht;
}

std::string HappenedAfterTime::get_el_tick()
{
    std::stringstream s;
    s << format(
        "%s = %s;"
        "%s = %s;"
        "if %s > %s then %s = -1;"
        "if Date[0] <> Date[1] then "
        "begin "
        "%s = false; "
        "end; "
        "if (%s) and (%s > %s) then "
        "begin "
        "%s = true; "
        "end;")
        % var("tbef") % var("tnow") 
        % var("tnow") % el_cur_time() 
        % var("tbef") % var("tnow") % var("tbef") 
        % var("ht") 
        % m_children[0]->get_el_output()
        % var("tnow") % var("time") 
        % var("ht");

    s << format("%s = %s;") % var("outp") % var("ht");

    return s.str();
}

std::string HappenedAfterTime::get_mql_tick()
{
    std::stringstream s;
    s << format(
        "%s = %s;"
        "%s = %s;"
        "if (%s > %s) { %s = -1; }"
        "if (TimeDay(Time[0]) != TimeDay(Time[1])) "
        "{ "
        "   %s = false; "
        "} "
        "if ((%s) && (%s > %s)) "
        "{ "
        "   %s = true; "
        "} ")
        % var("tbef") % var("tnow") 
        % var("tnow") % el_cur_time() 
        % var("tbef") % var("tnow") % var("tbef") 
        % var("ht") 
        % m_children[0]->get_mql_output()
        % var("tnow") % var("time") 
        % var("ht");

    s << format("%s = %s;") % var("outp") % var("ht");

    return s.str();
}


ParameterConstraintMap HappenedInTimePeriod::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;

    // Define the parameter constraints and the variables to be used
    SP_constraints t_start_time;
    t_start_time.usage_mode = CNT_MINMAX;
    t_start_time.min = Time(0,0);
    t_start_time.max = Time(23, 59);
    t_start_time.mut_min = Time(0,0);
    t_start_time.mut_max = Time(23,59);
    t_start_time.mut_power = Time(1,0);
    t_start_time.is_mutable = false;
    t_start_time.default_value = Time(9,30);
    t_constraints["start_time"] = t_start_time;

    // Define the parameter constraints and the variables to be used
    SP_constraints t_end_time;
    t_end_time.usage_mode = CNT_MINMAX;
    t_end_time.min = Time(0,0);
    t_end_time.max = Time(23, 59);
    t_end_time.mut_min = Time(0,0);
    t_end_time.mut_max = Time(23,59);
    t_end_time.mut_power = Time(1,0);
    t_end_time.is_mutable = false;
    t_end_time.default_value = Time(16,0);
    t_constraints["end_time"] = t_end_time;

    return t_constraints;
}

void HappenedInTimePeriod::init_codegen_variables()
{
    m_variables["ht"] = VAR_TYPE_BOOL;
}

void HappenedInTimePeriod::update_data( const DataMatrix& data )
{
    dt = &(data.dt);
    start_time = get<Time>(m_parameters["start_time"]).as_int();
    end_time = get<Time>(m_parameters["end_time"]).as_int();
}

void HappenedInTimePeriod::compute_init( const Env& env, int idx )
{
    ht = false;
}

void HappenedInTimePeriod::compute_tick( const Env& env, int idx )
{
    // reset it on new days
    if ((*dt)[idx].date() != (*dt)[prev_idx].date())
    {
        ht = false;
    }
    prev_idx = idx;

    if ((get<bool>(m_children[0]->get_output())) &&
        ((*dt)[idx].time().as_int() > start_time) && 
        ((*dt)[idx].time().as_int() < end_time))
        ht = true;

	m_outp = ht;
}

std::string HappenedInTimePeriod::get_el_init()
{
    std::stringstream s;
    s << var("ht")   << " = false; " <<
        var("tbef") << " = -1; " <<
        var("tnow") << " = " << el_cur_time() << "; ";
    return s.str();
}

std::string HappenedInTimePeriod::get_el_tick()
{
    std::stringstream s;
    s << format(
        "%s = %s;"
        "%s = %s;"
        "if %s > %s then %s = -1;"
        "if Date[0] <> Date[1] then "
        "begin "
        "%s = false; "
        "end; "
        "if (%s) and (((%s > %s) and (%s < %s))) then "
        "begin "
        "%s = true; "
        "end;")
        % var("tbef") % var("tnow") 
        % var("tnow") % el_cur_time() 
        % var("tbef") % var("tnow") % var("tbef") 
        % var("ht") 
        % m_children[0]->get_el_output()
        % var("tbef") % var("start_time") 
        % var("tnow") % var("end_time") 
        % var("ht");

    s << format("%s = %s;") % var("outp") % var("ht");

    return s.str();
}

std::string HappenedInTimePeriod::get_mql_init()
{
    std::stringstream s;
    s << var("ht")   << " = false; " <<
        var("tbef") << " = -1; " <<
        var("tnow") << " = " << mql_cur_time() << "; ";
    return s.str();
}

std::string HappenedInTimePeriod::get_mql_tick()
{
    std::stringstream s;
    s << format(
        "%s = %s;"
        "%s = %s;"
        "if (%s > %s) { %s = -1; }"
        "if (TimeDay(Time[0]) != TimeDay(Time[1])) "
        "{ "
        "   %s = false; "
        "} "
        "if ((%s) && (((%s > %s) && (%s < %s)))) "
        "{ "
        "   %s = true; "
        "} ")
        % var("tbef") % var("tnow") 
        % var("tnow") % mql_cur_time() 
        % var("tbef") % var("tnow") % var("tbef") 
        % var("ht") 
        % m_children[0]->get_mql_output()
        % var("tbef") % var("start_time") 
        % var("tnow") % var("end_time") 
        % var("ht");

    s << format("%s = %s;") % var("outp") % var("ht");

    return s.str();
}


ParameterConstraintMap HappenedBarsBack::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;

    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 16384;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 20;
    t_bars_back.is_mutable = true;
    t_bars_back.default_value = 1;
    t_constraints["bars_back"] = t_bars_back;

    return t_constraints;
}

void HappenedBarsBack::init_codegen_variables()
{
    m_variables["val_array"] = VAR_TYPE_ARRAY_BOOL;
}

void HappenedBarsBack::update_data( const DataMatrix& data )
{
    bars_back = convert_to_int(m_parameters["bars_back"]);
}

void HappenedBarsBack::compute_init(const Env& env, int idx)
{
    val_array.clear();
    val_array.resize(bars_back+1);
}

void HappenedBarsBack::compute_tick( const Env& env, int idx )
{
    // shift all values with 1 to the right
    for(unsigned int i=val_array.size()-1; i>0; i--)
    {
        val_array[i] = val_array[i-1];
    }
    val_array[0] = get<bool>(m_children[0]->get_output());
	m_outp = val_array[bars_back];
}

std::string HappenedBarsBack::get_el_init()
{
	std::stringstream s;
	s << format(
			"Array_SetMaxIndex(%s, %s+1); "
	     ) % var("val_array") % var("bars_back");
	return s.str();
}

std::string HappenedBarsBack::get_el_tick()
{
    std::stringstream s;

    s << format(
        "for idx = %s downto 1 "
        "begin "
        "   %s[idx] = %s[idx-1]; "
        "end;"
        "%s[0] = %s;")
        % var("bars_back") % var("val_array") % var("val_array") 
        % var("val_array") % m_children[0]->get_el_output();

    s << format("%s = %s;") % var("outp") % ("("+var("val_array")+"["+var("bars_back")+"])");
    return s.str();
}


std::string HappenedBarsBack::get_mql_init()
{
	return ""; // TODO MQL4
}
std::string HappenedBarsBack::get_mql_tick()
{
    std::stringstream s;
    
    s << format(
        "for(idx = %s; idx > 0; idx--) "
        "{ "
        "   %s[idx] = %s[idx-1]; "
        "} "
        "%s[0] = %s;")
        % var("bars_back") % var("val_array") % var("val_array") 
        % var("val_array") % m_children[0]->get_mql_output();

    s << format("%s = %s;") % var("outp") % ("("+var("val_array")+"["+var("bars_back")+"])");

    return s.str();
}


ParameterConstraintMap HappenedLastBars::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;

    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 16384;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 20;
    t_bars_back.is_mutable = true;
    t_bars_back.default_value = 1;
    t_constraints["bars_back"] = t_bars_back;

    return t_constraints;
}

void HappenedLastBars::init_codegen_variables()
{
    m_variables["val_array"] = VAR_TYPE_ARRAY_BOOL;
    m_variables["hp"] = VAR_TYPE_BOOL;
}

void HappenedLastBars::update_data( const DataMatrix& data )
{
    bars_back = convert_to_int(m_parameters["bars_back"]);
}

void HappenedLastBars::compute_init(const Env& env, int idx)
{
    val_array.clear();
    val_array.resize(bars_back+1);
    hp = false;
}

void HappenedLastBars::compute_tick( const Env& env, int idx )
{
    // shift all values with 1 to the right
    for(unsigned int i=val_array.size()-1; i>0; i--)
    {
        val_array[i] = val_array[i-1];
    }
    val_array[0] = get<bool>(m_children[0]->get_output());

    hp = false;
    for(unsigned int i=0; i<val_array.size(); i++)
    {
        if (val_array[i] == true)
        {
            hp = true;
            break;
        }
    }

	m_outp = hp;
}

std::string HappenedLastBars::get_el_init()
{
	std::stringstream s;
	s << format(
			"Array_SetMaxIndex(%s, %s+1); "
			"%s = false; "
	     ) % var("val_array") % var("bars_back") % var("hp");
	return s.str();
}

std::string HappenedLastBars::get_el_tick()
{
    std::stringstream s;
    
    s << format(
        "for idx = %s downto 1 "
        "begin "
        "   %s[idx] = %s[idx-1]; "
        "end;"
        "%s[0] = %s;")
        % var("bars_back") % var("val_array") % var("val_array") 
        % var("val_array") % m_children[0]->get_el_output();

    s << format(
        "%s = false;"
        "for idx = 0 to %s "
        "begin "
        "  if (%s[idx] = true) then %s = true; "
        "end; "
        ) 
        % var("hp") % var("bars_back")
        % var("val_array") % var("hp");

    s << format("%s = %s;") % var("outp") % var("hp");

    return s.str();
}

std::string HappenedLastBars::get_mql_init()
{
	return ""; // TODO MQL4
}

std::string HappenedLastBars::get_mql_tick()
{
    std::stringstream s;
    s << Signal::get_mql_tick();
    
    s << format(
        "for(idx = %s; idx > 0; idx--) "
        "{ "
        "   %s[idx] = %s[idx-1]; "
        "} "
        "%s[0] = %s;")
        % var("bars_back") % var("val_array") % var("val_array") 
        % var("val_array") % m_children[0]->get_mql_output();

    s << format(
        "%s = false;"
        "for(idx = 0; idx < %s; idx++) "
        "{ "
        "  if (%s[idx] == true) "
        "  { "
        "       %s = true; break; "
        "  } "
        "} "
        ) 
        % var("hp") % var("bars_back")
        % var("val_array") % var("hp");

    s << format("%s = %s;") % var("outp") % var("hp");

    return s.str();
}


ParameterConstraintMap Seqn::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;

    SP_constraints t_timeout_bars;
    t_timeout_bars.usage_mode = CNT_MINMAX;
    t_timeout_bars.min = -1;
    t_timeout_bars.max = 1000000;
    t_timeout_bars.mut_min = 0;
    t_timeout_bars.mut_max = 1200;
    t_timeout_bars.mut_power = 100;
    t_timeout_bars.is_mutable = false;
    t_timeout_bars.default_value = 1;
    t_constraints["timeout_bars"] = t_timeout_bars;

    return t_constraints;
}

void Seqn::init_codegen_variables()
{
    m_variables["ht1"] = VAR_TYPE_BOOL;
    m_variables["ht2"] = VAR_TYPE_BOOL;
    m_variables["happened"] = VAR_TYPE_BOOL;
    m_variables["timeout_counter"] = VAR_TYPE_INT;
}

void Seqn::update_data( const DataMatrix& data )
{
    timeout = convert_to_int(m_parameters["timeout_bars"]);
}

void Seqn::compute_init( const Env& env, int idx )
{
    ht1 = false;
    ht2 = false;
    happened = false;
    timeout_counter = 0;
}

void Seqn::compute_tick( const Env& env, int idx )
{
    if (ht1 && (get<bool>(m_children[1]->get_output())))
    {
        ht2 = true;
    }
    else 
    {
        timeout_counter++;
        // check for timeout here
        if ((timeout != -1) && (timeout_counter >= timeout))
        {
            ht1 = false; 
            ht2 = false;
        }
    }

    if (get<bool>(m_children[0]->get_output()))
    {
        ht1 = true;
    }

    // if all become true, make them false so we can start over
    if (ht1 && ht2)
    {
        happened = true;
        ht1 = false;
        ht2 = false;
        timeout_counter = 0;
    }
    else
    {
        happened = false;
    }

	m_outp = happened;
}


std::string Seqn::get_el_init()
{
    std::stringstream s;
    s << format(
        "%s = false;"
        "%s = false;"
        "%s = false;"
        "%s = 0;"
        )
        % var("ht1") 
        % var("ht2") 
        % var("happened") 
        % var("timeout_counter");

    return s.str();
}

std::string Seqn::get_el_tick()
{
    std::stringstream s;
    s << format(
        "if (%s) and (%s) then "
        "begin "
        "    %s = true; "
        "end "
        "else " 
        "begin " 
        "   %s = %s + 1;" 
        "   if (%s <> -1) and (%s >= %s) then "
        "   begin "
        "      %s = false; %s = false; " 
        "   end; "
        "end; "
        "if (%s) then %s = true; "
        "if ((%s) and (%s)) then "
        "begin "
        "    %s = true; "
        "    %s = false; "
        "    %s = false; "
        "    %s = 0;"
        "end "
        "else "
        "begin "
        "    %s = false; "
        "end;"
        )
        % var("ht1") % m_children[1]->get_el_output()
        % var("ht2") 
        % var("timeout_counter") % var("timeout_counter")
        % var("timeout_counter") % var("timeout_counter") % var("timeout_bars")
        % var("ht1") % var("ht2")
        % m_children[0]->get_el_output() % var("ht1")
        % var("ht1") % var("ht2")
        % var("happened")
        % var("ht1") 
        % var("ht2")
        % var("timeout_counter")
        % var("happened");

    s << format("%s = %s;") % var("outp") % var("happened");

    return s.str();
}

std::string Seqn::get_mql_init()
{
    std::stringstream s;
    s << format(
        "%s = false;"
        "%s = false;"
        "%s = false;"
        "%s = 0;"
        )
        % var("ht1") 
        % var("ht2") 
        % var("happened") 
        % var("timeout_counter");

    return s.str();
}

std::string Seqn::get_mql_tick()
{
    std::stringstream s;
    s << format(
        "if ((%s) && (%s)) "
        "{ "
        "    %s = true; "
        "} "
        "else " 
        "{ " 
        "   %s = %s + 1;" 
        "   if ((%s != -1) && (%s >= %s)) "
        "   { "
        "      %s = false; %s = false; " 
        "   } "
        "} "
        "if (%s) { %s = true; } "
        "if ((%s) && (%s)) "
        "{ "
        "    %s = true; "
        "    %s = false; "
        "    %s = false; "
        "    %s = 0;"
        "} "
        "else "
        "{ "
        "    %s = false; "
        "} "
        )
        % var("ht1") % m_children[1]->get_mql_output()
        % var("ht2") 
        % var("timeout_counter") % var("timeout_counter")
        % var("timeout_counter") % var("timeout_counter") % var("timeout_bars")
        % var("ht1") % var("ht2")
        % m_children[0]->get_mql_output() % var("ht1")
        % var("ht1") % var("ht2")
        % var("happened")
        % var("ht1") 
        % var("ht2")
        % var("timeout_counter")
        % var("happened");

    s << format("%s = %s;") % var("outp") % var("happened");

    return s.str();
}

