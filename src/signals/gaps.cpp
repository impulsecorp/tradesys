#include "gaps.h"

ParameterConstraintMap GapSignal::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;

    SP_constraints t_time_st;
    t_time_st.usage_mode = CNT_MINMAX;
    t_time_st.min = Time(0,0);
    t_time_st.max = Time(23,59);
    t_time_st.mut_min = Time(0,0);
    t_time_st.mut_max = Time(23,59);
    t_time_st.mut_power = Time(1,0);
    t_time_st.is_mutable = false;
    t_time_st.default_value = Time(16,0);
    t_constraints["time_st"] = t_time_st;

    SP_constraints t_time_en;
    t_time_en.usage_mode = CNT_MINMAX;
    t_time_en.min = Time(0,0);
    t_time_en.max = Time(23,59);
    t_time_en.mut_min = Time(0,0);
    t_time_en.mut_max = Time(23,59);
    t_time_en.mut_power = Time(1,0);
    t_time_en.is_mutable = false;
    t_time_en.default_value = Time(9,30);
    t_constraints["time_en"] = t_time_en;

    SP_constraints t_min_size;
    t_min_size.usage_mode = CNT_MINMAX;
    t_min_size.min = 0.0;
    t_min_size.max = 100.0;
    t_min_size.mut_min = 0.0;
    t_min_size.mut_max = 1.0;
    t_min_size.mut_power = 0.1;
    t_min_size.is_mutable = true;
    t_min_size.default_value = 0.0;
    t_constraints["min_size"] = t_min_size;

    SP_constraints t_max_size;
    t_max_size.usage_mode = CNT_MINMAX;
    t_max_size.min = 0.0;
    t_max_size.max = 100.0;
    t_max_size.mut_min = 0.0;
    t_max_size.mut_max = 1.0;
    t_max_size.mut_power = 0.1;
    t_max_size.is_mutable = true;
    t_max_size.default_value = 1000000.0;
    t_constraints["max_size"] = t_max_size;

    SP_constraints t_in_percents;
    t_in_percents.usage_mode = CNT_POSSIBLE_VALUES;
    t_in_percents.possible_values.push_back( SP_type( true ) );
    t_in_percents.possible_values.push_back( SP_type( false ) );
    t_in_percents.is_mutable = false;
    t_in_percents.default_value = false;
    t_constraints["in_percents"] = t_in_percents;

    SP_constraints t_price_before;
    t_price_before.usage_mode = CNT_POSSIBLE_VALUES;
    t_price_before.possible_values.push_back( SP_type( STR("open") ) );
    t_price_before.possible_values.push_back( SP_type( STR("high") ) );
    t_price_before.possible_values.push_back( SP_type( STR("low") ) );
    t_price_before.possible_values.push_back( SP_type( STR("close") ) );
    t_price_before.is_mutable = false;
    t_price_before.default_value = SP_type( STR("close") );
    t_constraints["price_before"] = t_price_before;

    SP_constraints t_price_after;
    t_price_after.usage_mode = CNT_POSSIBLE_VALUES;
    t_price_after.possible_values.push_back( SP_type( STR("open") ) );
    t_price_after.possible_values.push_back( SP_type( STR("high") ) );
    t_price_after.possible_values.push_back( SP_type( STR("low") ) );
    t_price_after.possible_values.push_back( SP_type( STR("close") ) );
    t_price_after.is_mutable = false;
    t_price_after.default_value = SP_type( STR("close") );
    t_constraints["price_after"] = t_price_after;

    SP_constraints t_fade_pct;
    t_fade_pct.usage_mode = CNT_MINMAX;
    t_fade_pct.min = -200.0;
    t_fade_pct.max = 200.0;
    t_fade_pct.mut_min = 0.0;
    t_fade_pct.mut_max = 100.0;
    t_fade_pct.mut_power = 10.0;
    t_fade_pct.is_mutable = true;
    t_fade_pct.default_value = 0.0;
    t_constraints["fade_pct"] = t_fade_pct;

    return t_constraints;
}

void GapSignal::init_codegen_variables()
{
    // Define used variables for code generation
    m_variables["gap_dir"] = VAR_TYPE_INT;
    m_variables["gap_size"]  = VAR_TYPE_FLOAT;
    m_variables["pr_bef"]  = VAR_TYPE_FLOAT;
    m_variables["pr_aft"]  = VAR_TYPE_FLOAT;
    m_variables["fpct"]  = VAR_TYPE_FLOAT;
    m_variables["tbef"] = VAR_TYPE_INT;
    m_variables["tnow"] = VAR_TYPE_INT;
}

void GapSignal::update_data( const DataMatrix& data )
{
    dt = &(data.dt);
    if (get<std::string>(m_parameters["price_before"]) == STR("open"))
        data_before = &(data.open);
    else if (get<std::string>(m_parameters["price_before"]) == STR("high"))
        data_before = &(data.high);
    else if (get<std::string>(m_parameters["price_before"]) == STR("low"))
        data_before = &(data.low);
    else if (get<std::string>(m_parameters["price_before"]) == STR("close"))
        data_before = &(data.close);
    else 
        throw std::runtime_error("price_before is wrong, needs to be a string: open/high/low/close");

    if (get<std::string>(m_parameters["price_after"]) == STR("open"))
        data_after = &(data.open);
    else if (get<std::string>(m_parameters["price_after"]) == STR("high"))
        data_after = &(data.high);
    else if (get<std::string>(m_parameters["price_after"]) == STR("low"))
        data_after = &(data.low);
    else if (get<std::string>(m_parameters["price_after"]) == STR("close"))
        data_after = &(data.close);
    else 
        throw std::runtime_error("price_after is wrong, needs to be a string: open/high/low/close");

    time_st = (get<Time>(m_parameters["time_st"])).as_int();
    time_en = (get<Time>(m_parameters["time_en"])).as_int();
    in_percents = get<bool>(m_parameters["in_percents"]);
    min_size = convert_to_float(m_parameters["min_size"]);
    max_size = convert_to_float(m_parameters["max_size"]);
    fade_pct = convert_to_float(m_parameters["fade_pct"]);
}

void GapSignal::compute_init( const Env& env, int idx )
{
    gap_dir = 0;
    gap_size = std::numeric_limits<double>::min();
    pr_bef = std::numeric_limits<double>::min();
    pr_aft = std::numeric_limits<double>::min();
    fpct = std::numeric_limits<double>::min();
    tbef = -1;
    tnow = (*dt)[idx].time().as_int();
}

void GapSignal::compute_tick( const Env& env, int idx )
{
    tbef = tnow;
    tnow = (*dt)[idx].time().as_int(); 
    if (tbef > tnow)
    {
        tbef = -1;
    }

    if ((tbef < time_st) && (tnow >= time_st))
    {
        pr_bef = (*data_before)[idx];
    }

    if ((pr_bef != 0) && ((tbef < time_en) && (tnow >= time_en)))
    {
        pr_aft = (*data_after)[idx];

        if (in_percents)
        {
            gap_size = ABS((pr_aft - pr_bef) / pr_bef * 100.0);
        }
        else
        {
            gap_size = ABS(pr_aft - pr_bef);
        }

        if (pr_aft > pr_bef)
        {
            gap_dir = 1; // gap up
        }
        else
        {
            gap_dir = -1; // gap down
        }

        if (pr_bef == pr_aft)
        {
            gap_dir = 0;
        }
        if (gap_size < min_size)
        {
            gap_dir = 0;
        }
        if (gap_size > max_size)
        {
            gap_dir = 0;
        }

        if (gap_dir == 1)
        {
            fpct = pr_aft - (fade_pct/100.0) * ABS(pr_aft - pr_bef);
        }
        else if (gap_dir == -1)
        {
            fpct = pr_aft + (fade_pct/100.0) * ABS(pr_aft - pr_bef);
        }
        else
        {
            fpct = std::numeric_limits<double>::min();
        }
    }
}

std::string GapSignal::get_el_init()
{
    std::stringstream s;
    s << var("gap_dir") << " = 0;" 
      << var("gap_size") << " = " << limits["min"] << ";"
      << var("pr_bef") << " = " << limits["min"] << ";"
      << var("pr_aft") << " = " << limits["min"] << ";"
      << var("fpct") << " = " << limits["min"] << ";"
      << var("tbef") << " = -1;"
      << var("tnow") << " = " << el_cur_time() << ";";
    return s.str();
}

std::string GapSignal::get_el_tick()
{
    std::stringstream s;
    s << format(
    "%s = %s; "
    "%s = %s; "
    "if %s > %s then %s = -1;"

    "if (%s < %s) and (%s >= %s) then "
    "    %s = %s[0]; "

    "if (%s < %s) and (%s >= %s) and (%s <> 0) then "
    "    begin "
    "    %s = %s[0]; "

    "if %s then "
    "    %s = Absvalue((%s - %s)/%s*100.0)"
    "else "
    "    %s = Absvalue(%s - %s);"

    "if %s > %s then "
    "    %s = 1 "
    "else "
    "    %s = -1; "

    "if Absvalue(%s - %s) = 0 then "
    "    %s = 0; "

    "if %s < %s then "
    "    %s = 0; "

    "if %s > %s then "
    "    %s = 0; "

    "if %s = 1 then "
    "    %s = %s - %s/100.0 * Absvalue(%s - %s); "
    "if %s = -1 then "
    "    %s = %s + %s/100.0 * Absvalue(%s - %s); "
    "if %s = 0 then "
    "    %s = %s; "
    "end;"
    ) 
    % var("tbef") % var("tnow") % 
    var("tnow") % el_cur_time() % 
    var("tbef") % var("tnow") % var("tbef") % 

    var("tbef") % var("time_st") % 
    var("tnow") % var("time_st") % 
    var("pr_bef") % get<std::string>(m_parameters["price_before"]) % 
    var("tbef") % var("time_en") % 
    var("tnow") % var("time_en") % 
    var("pr_bef") % 
    var("pr_aft") % get<std::string>(m_parameters["price_after"]) % 

    var("in_percents") % 
    var("gap_size") % 
    var("pr_aft") % 
    var("pr_bef") % 
    var("pr_bef") % 
    var("gap_size") % 
    var("pr_aft") % 
    var("pr_bef") % 

    var("pr_aft") % var("pr_bef") % 
    var("gap_dir") % var("gap_dir") % 
    var("pr_aft") % var("pr_bef") % 
    var("gap_dir") % 
    var("gap_size") % var("min_size") % 
    var("gap_dir") % 
    var("gap_size") % var("max_size") % 
    var("gap_dir") % 

    // the fade gap calc
    var("gap_dir") % var("fpct") % var("pr_aft") % var("fade_pct") % 
    var("pr_aft") % var("pr_bef") % 
    var("gap_dir") % var("fpct") % var("pr_aft") % var("fade_pct") % 
    var("pr_aft") % var("pr_bef") % 
    var("gap_dir") % var("fpct") % limits["min"];

    return s.str();
}

std::string GapSignal::get_mql_init()
{
    std::stringstream s;
    s << var("gap_dir") << " = 0;" 
        << var("gap_size") << " = " << limits["min"] << ";"
        << var("pr_bef") << " = " << limits["min"] << ";"
        << var("pr_aft") << " = " << limits["min"] << ";"
        << var("fpct") << " = " << limits["min"] << ";"
        << var("tbef") << " = -1;"
        << var("tnow") << " = " << mql_cur_time() << ";";
    return s.str();
}

std::string GapSignal::get_mql_tick()
{
    std::stringstream s;
    s << format(
        "%s = %s; "
        "%s = %s; "
        "if (%s > %s) { %s = -1; }"

        "if ((%s < %s) && (%s >= %s)) "
        "{  %s = %s[0]; }"

        "if ((%s < %s) && (%s >= %s) && (%s != 0)) "
        "{ "
        "   %s = %s[0]; "

        "   if (%s) "
        "   {  %s = MathAbs((%s - %s)/%s*100.0); } "
        "   else "
        "   {  %s = MathAbs(%s - %s); }"

        "   if (%s > %s) "
        "   {   %s = 1;  } "
        "   else "
        "   {   %s = -1; } "

        "   if (MathAbs(%s - %s) == 0) "
        "   {   %s = 0; } "

        "   if (%s < %s) "
        "   {   %s = 0; } "

        "   if (%s > %s) "
        "   {   %s = 0; } "

        "   if (%s == 1) "
        "   {    %s = %s - %s/100.0 * MathAbs(%s - %s); } "
        "   if (%s == -1) "
        "   {    %s = %s + %s/100.0 * MathAbs(%s - %s); } "
        "   if (%s == 0) "
        "   {    %s = %s; } "
        "} "
        ) 
        % var("tbef") % var("tnow") % 
        var("tnow") % el_cur_time() % 
        var("tbef") % var("tnow") % var("tbef") % 

        var("tbef") % var("time_st") % 
        var("tnow") % var("time_st") % 
        var("pr_bef") % capitalize(get<std::string>(m_parameters["price_before"])) % 
        var("tbef") % var("time_en") % 
        var("tnow") % var("time_en") % 
        var("pr_bef") % 
        var("pr_aft") % capitalize(get<std::string>(m_parameters["price_after"])) % 

        var("in_percents") % 
        var("gap_size") % 
        var("pr_aft") % 
        var("pr_bef") % 
        var("pr_bef") % 
        var("gap_size") % 
        var("pr_aft") % 
        var("pr_bef") % 

        var("pr_aft") % var("pr_bef") % 
        var("gap_dir") % var("gap_dir") % 
        var("pr_aft") % var("pr_bef") % 
        var("gap_dir") % 
        var("gap_size") % var("min_size") % 
        var("gap_dir") % 
        var("gap_size") % var("max_size") % 
        var("gap_dir") % 

        // the fade gap calc
        var("gap_dir") % var("fpct") % var("pr_aft") % var("fade_pct") % 
        var("pr_aft") % var("pr_bef") % 
        var("gap_dir") % var("fpct") % var("pr_aft") % var("fade_pct") % 
        var("pr_aft") % var("pr_bef") % 
        var("gap_dir") % var("fpct") % limits["min"];

    return s.str();
}


void GapUp::compute_tick( const Env& env, int idx )
{
	GapSignal::compute_tick(env, idx);

    if (gap_dir == 1)
    	m_outp = true;
    else
    	m_outp = false;
}

std::string GapUp::get_el_tick()
{
	std::stringstream s;
	s << GapSignal::get_el_tick();
	s << format("%s = %s;") % var("outp") % ("(" + var("gap_dir") + " > 0)");
	return s.str();
}

std::string GapUp::get_mql_tick()
{
	std::stringstream s;
	s << GapSignal::get_mql_tick();
	s << format("%s = %s;") % var("outp") % ("(" + var("gap_dir") + " > 0)");
	return s.str();
}

void GapDown::compute_tick( const Env& env, int idx )
{
	GapSignal::compute_tick(env, idx);

    if (gap_dir == -1)
    	m_outp = true;
    else
    	m_outp = false;
}

std::string GapDown::get_el_tick()
{
	std::stringstream s;
	s << GapSignal::get_el_tick();
	s << format("%s = %s;") % var("outp") % ("(" + var("gap_dir") + " < 0)");
	return s.str();
}

std::string GapDown::get_mql_tick()
{
	std::stringstream s;
	s << GapSignal::get_mql_tick();
	s << format("%s = %s;") % var("outp") % ("(" + var("gap_dir") + " < 0)");
	return s.str();
}

void FadeGapPercentPrice::compute_tick( const Env& env, int idx )
{
	GapSignal::compute_tick(env, idx);

	m_outp = fpct;
}

std::string FadeGapPercentPrice::get_el_tick()
{
	std::stringstream s;
	s << GapSignal::get_el_tick();
	s << format("%s = %s;") % var("outp") % var("fpct");
	return s.str();
}

std::string FadeGapPercentPrice::get_mql_tick()
{
	std::stringstream s;
	s << GapSignal::get_mql_tick();
	s << format("%s = %s;") % var("outp") % var("fpct");
	return s.str();
}

void GapPriceBefore::compute_tick( const Env& env, int idx )
{
	GapSignal::compute_tick(env, idx);

	m_outp = price_before;
}

std::string GapPriceBefore::get_el_tick()
{
	std::stringstream s;
	s << GapSignal::get_el_tick();
	s << format("%s = %s;") % var("outp") % var("pr_bef");
	return s.str();
}

std::string GapPriceBefore::get_mql_tick()
{
	std::stringstream s;
	s << GapSignal::get_mql_tick();
	s << format("%s = %s;") % var("outp") % var("pr_bef");
	return s.str();
}

void GapPriceAfter::compute_tick( const Env& env, int idx )
{
	GapSignal::compute_tick(env, idx);

	m_outp = price_after;
}

std::string GapPriceAfter::get_el_tick()
{
	std::stringstream s;
	s << GapSignal::get_el_tick();
	s << format("%s = %s;") % var("outp") % var("pr_aft");
	return s.str();
}

std::string GapPriceAfter::get_mql_tick()
{
	std::stringstream s;
	s << GapSignal::get_mql_tick();
	s << format("%s = %s;") % var("outp") % var("pr_aft");
	return s.str();
}

void GapSize::compute_tick( const Env& env, int idx )
{
	GapSignal::compute_tick(env, idx);

	m_outp = gap_size;
}

std::string GapSize::get_el_tick()
{
	std::stringstream s;
	s << GapSignal::get_el_tick();
	s << format("%s = %s;") % var("outp") % var("gap_size");
	return s.str();
}

std::string GapSize::get_mql_tick()
{
	std::stringstream s;
	s << GapSignal::get_mql_tick();
	s << format("%s = %s;") % var("outp") % var("gap_size");
	return s.str();
}
