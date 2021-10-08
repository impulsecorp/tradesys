#include "raw.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

ParameterConstraintMap MovementSignal::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    // Define the parameter constraints and the variables to be used
    SP_constraints t_min_movement;
    t_min_movement.usage_mode = CNT_MINMAX;
    t_min_movement.min = 0.0;
    t_min_movement.max = 100.0;
    t_min_movement.mut_min = 0.0;
    t_min_movement.mut_max = 0.001;
    t_min_movement.mut_power = 0.001;
    t_min_movement.is_mutable = true;
    t_min_movement.default_value = 0.0;
    t_constraints["min"] = t_min_movement;
    
    SP_constraints t_max_movement;
    t_max_movement.usage_mode = CNT_MINMAX;
    t_max_movement.min = 0.0;
    t_max_movement.max = 100.0;
    t_max_movement.mut_min = 0.0;
    t_max_movement.mut_max = 0.01;
    t_max_movement.mut_power = 0.001;
    t_max_movement.is_mutable = true;
    t_max_movement.default_value = 100.0;
    t_constraints["max"] = t_max_movement;
    
    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 1000000;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 20;
    t_bars_back.is_mutable = true;
    t_bars_back.default_value = 1;
    t_constraints["bars_back"] = t_bars_back;
    
    SP_constraints t_since;
    t_since.usage_mode = CNT_MINMAX;
    t_since.min = Time(0, 0);
    t_since.max = Time(23, 59);
    t_since.mut_min = Time(0, 0);
    t_since.mut_max = Time(23, 59);
    t_since.mut_power = Time(1, 0);
    t_since.is_mutable = true;
    t_since.default_value = Time(0, 0);
    t_constraints["since"] = t_since;
    
    SP_constraints t_to;
    t_to.usage_mode = CNT_MINMAX;
    t_to.min = Time(0, 0);
    t_to.max = Time(23, 59);
    t_to.mut_min = Time(0, 0);
    t_to.mut_max = Time(23, 59);
    t_to.mut_power = Time(1, 0);
    t_to.is_mutable = true;
    t_to.default_value = Time(0, 0);
    t_constraints["to"] = t_to;
    
    SP_constraints t_use_to;
    t_use_to.usage_mode = CNT_POSSIBLE_VALUES;
    t_use_to.possible_values.push_back(SP_type(true));
    t_use_to.possible_values.push_back(SP_type(false));
    t_use_to.is_mutable = false;
    t_use_to.default_value = false;
    t_constraints["use_to"] = t_use_to;
    
    SP_constraints t_mode;
    t_mode.usage_mode = CNT_POSSIBLE_VALUES;
    t_mode.possible_values.push_back(SP_type(STR("bars_back")));
    t_mode.possible_values.push_back(SP_type(STR("since")));
    t_mode.is_mutable = false;
    t_mode.default_value = STR("bars_back");
    t_constraints["mode"] = t_mode;
    
    return t_constraints;
}

void MovementSignal::init_codegen_variables()
{
    // Define used variables for code generation
    m_variables["before"] = VAR_TYPE_FLOAT;
    m_variables["now"] = VAR_TYPE_FLOAT;
    m_variables["tbef"] = VAR_TYPE_INT;
    m_variables["tnow"] = VAR_TYPE_INT;
    m_variables["val_array"] = VAR_TYPE_ARRAY_FLOAT;
}

void MovementSignal::update_data(const DataMatrix &data)
{
    dt = &(data.dt);
    bars_back = convert_to_int(m_parameters["bars_back"]);
    min_movement = convert_to_float(m_parameters["min"]);
    max_movement = convert_to_float(m_parameters["max"]);
    if (get<std::string>(m_parameters["mode"]) == "since")
    {
        mode = SINCE;
    }
    else if (get<std::string>(m_parameters["mode"]) == "bars_back")
    {
        mode = BARS_BACK;
    }
    else
    {
        throw std::runtime_error("Unknown mode supplied to Movement signal");
    }
    time_since = (get<Time>(m_parameters["since"])).as_int();
    time_to = (get<Time>(m_parameters["to"])).as_int();
    use_to = get<bool>(m_parameters["use_to"]);
    val_array.clear();
    val_array.resize((unsigned long) (bars_back + 1));
}

void MovementSignal::compute_init(const Env &env, int idx)
{
    value_before = std::numeric_limits<double>::min();
    value_now = std::numeric_limits<double>::min();
    tbef = -1;
    tnow = (*dt)[idx].time().as_int();
    
    // init the array
    for (unsigned int i = 0; i < val_array.size(); i++)
    {
        val_array[i] = std::numeric_limits<double>::min();
    }
}

void MovementSignal::compute_tick(const Env &env, int idx)
{
    // update current time
    tbef = tnow;
    tnow = (*dt)[idx].time().as_int();
    if (tbef > tnow)
    {
        tbef = -1;
    }
    
    // shift all values with 1 to the right
    for (int i = val_array.size() - 1; i > 0; i--)
    {
        val_array[i] = val_array[i - 1];
    }
    val_array[0] = convert_to_float(m_children[0]->get_output());
    
    if (mode == SINCE)
    {
        if (tnow < time_since)
        {
            value_before = std::numeric_limits<double>::min();
        }
        
        if ((tbef < time_since) && (tnow >= time_since))
        {
            value_before = val_array[0];
        }
        
        if (use_to)
        {
            if (tnow < time_to)
            {
                value_now = std::numeric_limits<double>::min();
            }
            
            if ((tbef < time_to) && (tnow >= time_to))
            {
                value_now = val_array[0];
            }
        }
        else
        {
            value_now = val_array[0];
        }
    }
    else
    {
        value_now = val_array[0];
        value_before = std::numeric_limits<double>::min();
        double tmp = -1;
        for (int i = 0; i < val_array.size(); i++)
        {
            if (val_array[i] != std::numeric_limits<double>::min())
            {
                if (ABS(value_now - val_array[i]) > tmp)
                {
                    tmp = ABS(value_now - val_array[i]);
                    value_before = val_array[i];
                }
            }
        }
    }
}

std::string MovementSignal::get_el_init()
{
    std::stringstream s;
    
    s << fmt(
            "    {before} = {lmin}; "
                    "    {now} = {lmin}; "
                    "    {tbef} = -1; "
                    "    {tnow} = {curtime}; "
                    "    Array_SetMaxIndex({val_array}, {bars_back}+1); "
                    " "
                    " "
                    "    for idx = 0 to ({bars_back}+1)  "
                    "    begin "
                    "        {val_array}[idx] = {lmin}; "
                    "    end; "
    ) % m_efd;
    
    return s.str();
}

std::string MovementSignal::get_el_tick()
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
            "%s = %s;"
                    "%s = %s;"
                    "if %s > %s then %s = -1;"
    ) % var("tbef") % var("tnow") %
         var("tnow") % el_cur_time() %
         var("tbef") % var("tnow") % var("tbef");
    
    s << "if (" << var("mode") << " = \"since\") then begin "; // if since
    
    s << format("if %s < %s then %s = %s;")
         % var("tnow") % var("since") % var("before") % limits["min"];
    
    s << format("if (%s < %s) and (%s >= %s) then %s = %s[0];")
         % var("tbef") % var("since")
         % var("tnow") % var("since")
         % var("before") % var("val_array");
    
    s << format(
            "if (%s) then "
                    "begin "
                    "   if (%s < %s) then "
                    "       %s = %s; "
                    "   if (%s < %s) and (%s >= %s) then "
                    "       %s = %s[0]; "
                    "end "
                    "else "
                    "begin "
                    "   %s = %s[0];"
                    "end; "
    )
         % var("use_to")
         % var("tnow") % var("to")
         % var("now") % limits["min"]
         % var("tbef") % var("to")
         % var("tnow") % var("to")
         % var("now") % var("val_array")
         % var("now") % var("val_array");
    
    s << " end else begin "; // else since
    
    s << format(
            "%s = %s[0]; "
                    "%s = %s; "
                    "tmp = -1;"
                    "for idx = 0 to %s "
                    "begin "
                    "   if %s[idx] <> %s then "
                    "   begin "
                    "      if absvalue(%s - %s[idx]) > tmp then "
                    "      begin "
                    "          tmp = absvalue(%s - %s[idx]);"
                    "          %s = %s[idx]; "
                    "      end; "
                    "   end; "
                    "end; "
    )
         % var("now") % var("val_array")
         % var("before") % limits["min"]
         % var("bars_back")
         % var("val_array") % limits["min"]
         % var("now") % var("val_array")
         % var("now") % var("val_array")
         % var("before") % var("val_array");
    
    s << " end; "; // end if since
    
    return s.str();
}

std::string MovementSignal::get_mql_init()
{
    std::stringstream s;
    s << format("%s = -1; %s = %s;"
                        "%s = %s; %s = %s;"
                        "for(idx = 0; idx < (%s+1); idx++) "
                        "{ "
                        "   %s[idx] = %s; "
                        "} "
    )
         % var("tbef") % var("tnow") % mql_cur_time()
         % var("now") % limits["min"]
         % var("before") % limits["min"]
         % var("bars_back") % var("val_array") % limits["min"];
    
    return s.str();
}

std::string MovementSignal::get_mql_tick()
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
    
    s << format(
            "%s = %s;"
                    "%s = %s;"
                    "if (%s > %s) { %s = -1; }"
    ) % var("tbef") % var("tnow") %
         var("tnow") % mql_cur_time() %
         var("tbef") % var("tnow") % var("tbef");
    
    s << "if (" << var("mode") << " == \"since\") { "; // if since
    
    s << format("if (%s < %s) { %s = %s; }")
         % var("tnow") % var("since") % var("before") % limits["min"];
    
    s << format("if ((%s < %s) && (%s >= %s)) { %s = %s[0]; }")
         % var("tbef") % var("since")
         % var("tnow") % var("since")
         % var("before") % var("val_array");
    
    s << format(
            "if (%s) "
                    "{ "
                    "   if (%s < %s) "
                    "   {    %s = %s; } "
                    "   if ((%s < %s) && (%s >= %s)) "
                    "   {    %s = %s[0]; } "
                    "} "
                    "else "
                    "{ "
                    "   %s = %s[0];"
                    "} "
    )
         % var("use_to")
         % var("tnow") % var("to")
         % var("now") % limits["min"]
         % var("tbef") % var("to")
         % var("tnow") % var("to")
         % var("now") % var("val_array")
         % var("now") % var("val_array");
    
    s << " } else { "; // else since
    
    s << format(
            "%s = %s[%s];"
                    "%s = %s[0];"
    )
         % var("before") % var("val_array") % var("bars_back")
         % var("now") % var("val_array");
    
    s << " } "; // end if since
    
    return s.str();
}

void Up::compute_tick(const Env &env, int idx)
{
    MovementSignal::compute_tick(env, idx);
    
    if ((value_before != std::numeric_limits<double>::min()) && (value_now != std::numeric_limits<double>::min()))
    {
        if ((ABS(value_now - value_before) > min_movement) &&
            (ABS(value_now - value_before) < max_movement))
        {
            if (value_before < value_now)
            {
                m_outp = true;
            }
            else
            {
                m_outp = false;
            }
        }
        else
        {
            m_outp = false;
        }
    }
    else
    {
        m_outp = false;
    }
}

std::string Up::get_el_tick()
{
    std::stringstream s;
    s << MovementSignal::get_el_tick();
    s << var("outp") << " = ";
    
    s << "(";
    s << format("(Absvalue(%s - %s) > %s) and (Absvalue(%s - %s) < %s) ")
         % var("before") % var("now") % var("min")
         % var("before") % var("now") % var("max");
    s << " and ";
    s << format("((%s < %s) and (%s <> %s) and (%s <> %s))")
         % var("before") % var("now")
         % var("before") % limits["min"] % var("now") % limits["min"];
    s << ")";
    
    s << ";";
    
    return s.str();
}

std::string Up::get_mql_tick()
{
    std::stringstream s;
    s << MovementSignal::get_mql_tick();
    s << var("outp") << " = ";
    
    s << "(";
    s << format("(MathAbs(%s - %s) > %s) && (MathAbs(%s - %s) < %s) ")
         % var("before") % var("now") % var("min")
         % var("before") % var("now") % var("max");
    s << " && ";
    s << format("((%s < %s) && (%s != %s) && (%s != %s))")
         % var("before") % var("now")
         % var("before") % limits["min"] % var("now") % limits["min"];
    s << ")";
    s << ";";
    
    return s.str();
}

void Down::compute_tick(const Env &env, int idx)
{
    MovementSignal::compute_tick(env, idx);
    
    if ((value_before != std::numeric_limits<double>::min()) && (value_now != std::numeric_limits<double>::min()))
    {
        if ((ABS(value_now - value_before) > min_movement) &&
            (ABS(value_now - value_before) < max_movement))
        {
            if (value_before > value_now)
            {
                m_outp = true;
            }
            else
            {
                m_outp = false;
            }
        }
        else
        {
            m_outp = false;
        }
    }
    else
    {
        m_outp = false;
    }
}

std::string Down::get_el_tick()
{
    std::stringstream s;
    s << MovementSignal::get_el_tick();
    s << var("outp") << " = ";
    
    s << "(";
    s << format("(Absvalue(%s - %s) > %s) and (Absvalue(%s - %s) < %s) ")
         % var("before") % var("now") % var("min")
         % var("before") % var("now") % var("max");
    s << " and ";
    s << format("((%s > %s) and (%s <> %s) and (%s <> %s))")
         % var("before") % var("now")
         % var("before") % limits["min"] % var("now") % limits["min"];
    s << ");";
    return s.str();
}

std::string Down::get_mql_tick()
{
    std::stringstream s;
    s << MovementSignal::get_mql_tick();
    s << var("outp") << " = ";
    s << "(";
    s << format("(MathAbs(%s - %s) > %s) && (MathAbs(%s - %s) < %s) ")
         % var("before") % var("now") % var("min")
         % var("before") % var("now") % var("max");
    s << " && ";
    s << format("((%s > %s) && (%s != %s) && (%s != %s))")
         % var("before") % var("now")
         % var("before") % limits["min"] % var("now") % limits["min"];
    s << ");";
    return s.str();
}

void PercentUp::compute_tick(const Env &env, int idx)
{
    MovementSignal::compute_tick(env, idx);
    
    if ((value_before != std::numeric_limits<double>::min()) &&
        (value_now != std::numeric_limits<double>::min()) &&
        (value_now != 0))
    {
        if ((ABS(((value_now - value_before) / value_before) * 100.0) > min_movement) &&
            (ABS(((value_now - value_before) / value_before) * 100.0) < max_movement))
        {
            if (value_before < value_now)
            {
                m_outp = true;
            }
            else
            {
                m_outp = false;
            }
        }
        else
        {
            m_outp = false;
        }
    }
    else
    {
        m_outp = false;
    }
}

std::string PercentUp::get_el_tick()
{
    std::stringstream s;
    s << MovementSignal::get_el_tick();
    s << var("outp") << " = ";
    s << format(
            "((absvalue(((%s - %s)/%s)*100.0) > %s) "
                    " and "
                    "(absvalue(((%s - %s)/%s)*100.0) < %s) "
                    " and "
                    "(%s <> %s) and (%s <> %s) "
                    " and "
                    "(%s > %s));"
    )
         % var("now") % var("before") % var("before") % var("min")
         % var("now") % var("before") % var("before") % var("max")
         % var("now") % limits["min"] % var("before") % limits["min"]
         % var("now") % var("before");
    return s.str();
}

std::string PercentUp::get_mql_tick()
{
    std::stringstream s;
    s << MovementSignal::get_mql_tick();
    s << var("outp") << " = ";
    s << format(
            "((MathAbs(((%s - %s)/%s)*100.0) > %s) "
                    " && "
                    "(MathAbs(((%s - %s)/%s)*100.0) < %s) "
                    " && "
                    "(%s != %s) && (%s != %s) "
                    " && "
                    "(%s > %s));"
    )
         % var("now") % var("before") % var("before") % var("min")
         % var("now") % var("before") % var("before") % var("max")
         % var("now") % limits["min"] % var("before") % limits["min"]
         % var("now") % var("before");
    return s.str();
}

void PercentDown::compute_tick(const Env &env, int idx)
{
    MovementSignal::compute_tick(env, idx);
    
    if ((value_before != std::numeric_limits<double>::min()) && (value_now != std::numeric_limits<double>::min()))
    {
        if ((ABS(((value_now - value_before) / value_before) * 100.0) > min_movement) &&
            (ABS(((value_now - value_before) / value_before) * 100.0) < max_movement))
        {
            if (value_before > value_now)
            {
                m_outp = true;
            }
            else
            {
                m_outp = false;
            }
        }
        else
        {
            m_outp = false;
        }
    }
    else
    {
        m_outp = false;
    }
}

std::string PercentDown::get_el_tick()
{
    std::stringstream s;
    s << MovementSignal::get_el_tick();
    s << var("outp") << " = ";
    s << format(
            "((absvalue(((%s - %s)/%s)*100.0) > %s) "
                    " and "
                    "(absvalue(((%s - %s)/%s)*100.0) < %s) "
                    " and "
                    "(%s <> %s) and (%s <> %s) "
                    " and "
                    "(%s < %s));"
    )
         % var("now") % var("before") % var("before") % var("min")
         % var("now") % var("before") % var("before") % var("max")
         % var("now") % limits["min"] % var("before") % limits["min"]
         % var("now") % var("before");
    return s.str();
}

std::string PercentDown::get_mql_tick()
{
    std::stringstream s;
    s << MovementSignal::get_mql_tick();
    s << var("outp") << " = ";
    s << format(
            "((MathAbs(((%s - %s)/%s)*100.0) > %s) "
                    " && "
                    "(MathAbs(((%s - %s)/%s)*100.0) < %s) "
                    " && "
                    "(%s != %s) && (%s != %s) "
                    " && "
                    "(%s < %s));"
    )
         % var("now") % var("before") % var("before") % var("min")
         % var("now") % var("before") % var("before") % var("max")
         % var("now") % limits["min"] % var("before") % limits["min"]
         % var("now") % var("before");
    return s.str();
}

void MovementValBefore::compute_tick(const Env &env, int idx)
{
    MovementSignal::compute_tick(env, idx);
    m_outp = (value_before);
}

std::string MovementValBefore::get_el_tick()
{
    std::stringstream s;
    s << MovementSignal::get_el_tick();
    s << var("outp") << " = ";
    s << format("(%s);") % var("before");
    return s.str();
}

std::string MovementValBefore::get_mql_tick()
{
    std::stringstream s;
    s << MovementSignal::get_mql_tick();
    s << var("outp") << " = ";
    s << format("(%s);") % var("before");
    return s.str();
}

void MovementDiff::compute_tick(const Env &env, int idx)
{
    MovementSignal::compute_tick(env, idx);
    
    m_outp = (value_now - value_before);
}

std::string MovementDiff::get_el_tick()
{
    std::stringstream s;
    s << MovementSignal::get_el_tick();
    s << var("outp") << " = ";
    s << format("(%s - %s);") % var("now") % var("before");
    return s.str();
}

std::string MovementDiff::get_mql_tick()
{
    std::stringstream s;
    s << MovementSignal::get_mql_tick();
    s << var("outp") << " = ";
    s << format("(%s - %s);") % var("now") % var("before");
    return s.str();
}


void MovementPercentDiff::compute_tick(const Env &env, int idx)
{
    MovementSignal::compute_tick(env, idx);
    
    m_outp = ((value_now - value_before) / value_before) * 100.0;
}

std::string MovementPercentDiff::get_el_tick()
{
    std::stringstream s;
    s << MovementSignal::get_el_tick();
    s << var("outp") << " = ";
    s << format("((%s - %s)/%s)*100.0;") % var("now") % var("before") % var("before");
    return s.str();
}

std::string MovementPercentDiff::get_mql_tick()
{
    std::stringstream s;
    s << MovementSignal::get_mql_tick();
    s << var("outp") << " = ";
    s << format("((%s - %s)/%s)*100.0;") % var("now") % var("before") % var("before");
    return s.str();
}


ParameterConstraintMap Price::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_price;
    t_price.usage_mode = CNT_POSSIBLE_VALUES;
    t_price.possible_values.push_back(SP_type(STR("open")));
    t_price.possible_values.push_back(SP_type(STR("high")));
    t_price.possible_values.push_back(SP_type(STR("low")));
    t_price.possible_values.push_back(SP_type(STR("close")));
    t_price.is_mutable = true;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        t_price.default_value = SP_type(STR("close"));
    }
    else
    {
        t_price.default_value = SP_type(STR("open"));
    }
    t_constraints["price"] = t_price;
    
    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 1000000;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 15;
    t_bars_back.is_mutable = true;
    t_bars_back.default_value = 0;
    t_constraints["bars_back"] = t_bars_back;
    
    return t_constraints;
}

void Price::update_data(const DataMatrix &data)
{
    if (get<std::string>(m_parameters["price"]) == STR("open"))
    {
        d = (data.open);
    }
    else if (get<std::string>(m_parameters["price"]) == STR("high"))
    {
        d = (data.high);
    }
    else if (get<std::string>(m_parameters["price"]) == STR("low"))
    {
        d = (data.low);
    }
    else if (get<std::string>(m_parameters["price"]) == STR("close"))
    {
        d = (data.close);
    }
    else
    {
        throw std::runtime_error("price name is wrong, needs to be one of these strings: open/high/low/close");
    }
    
    bb = convert_to_int(m_parameters["bars_back"]);
}

void Price::compute_tick(const Env &env, int idx)
{
    if (((idx - bb) >= 0) && ((idx - bb) < d.size()))
    {
        m_outp = d[idx - bb];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string Price::get_el_tick()
{
    std::stringstream s;
    
    s << var("outp") << " = ";
    
    s << get<std::string>(m_parameters["price"]) << format("[%d]") % var("bars_back");
    if (m_parameters.find("data") != m_parameters.end())
    {
        // key was found
        s << format(" of Data%d ") % (convert_to_int(m_parameters["data"]) + 1);
    }
    
    s << ";";
    
    return s.str();
}

std::string Price::get_mql_tick()
{
    std::stringstream s;
    s << var("outp") << " = ";
    s << capitalize(get<std::string>(m_parameters["price"])) << format("[%d]") % var("bars_back");
    s << ";";
    return s.str();
}


ParameterConstraintMap ValueAtTime::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    // Define the parameter constraints and the variables to be used
    SP_constraints t_time;
    t_time.usage_mode = CNT_MINMAX;
    t_time.min = Time(0, 0);
    t_time.max = Time(23, 59);
    t_time.mut_min = Time(0, 0);
    t_time.mut_max = Time(23, 59);
    t_time.mut_power = Time(1, 0);
    t_time.is_mutable = true;
    t_time.default_value = Time(12, 0);
    t_constraints["time"] = t_time;
    
    SP_constraints t_days_ago;
    t_days_ago.usage_mode = CNT_MINMAX;
    t_days_ago.min = 0;
    t_days_ago.max = 365 * 10;
    t_days_ago.mut_min = 0;
    t_days_ago.mut_max = 14;
    t_days_ago.mut_power = 1;
    t_days_ago.is_mutable = true;
    t_days_ago.default_value = 0;
    t_constraints["days_ago"] = t_days_ago;
    
    SP_constraints t_reset_overnight;
    t_reset_overnight.usage_mode = CNT_POSSIBLE_VALUES;
    t_reset_overnight.possible_values.push_back(SP_type(true));
    t_reset_overnight.possible_values.push_back(SP_type(false));
    t_reset_overnight.is_mutable = false;
    t_reset_overnight.default_value = false;
    t_constraints["reset_overnight"] = t_reset_overnight;
    
    return t_constraints;
}

void ValueAtTime::init_codegen_variables()
{
    m_variables["tbef"] = VAR_TYPE_INT;
    m_variables["tnow"] = VAR_TYPE_INT;
    m_variables["at"] = VAR_TYPE_FLOAT;
    m_variables["next_at"] = VAR_TYPE_ARRAY_FLOAT;
    m_variables["next_at_idx"] = VAR_TYPE_INT;
}

void ValueAtTime::update_data(const DataMatrix &data)
{
    dt = &(data.dt);
    time = (get<Time>(m_parameters["time"])).as_int();
    days_ago = (get<int>(m_parameters["days_ago"]));
    reset_overnight = (get<bool>(m_parameters["reset_overnight"]));
}

void ValueAtTime::compute_init(const Env &env, int idx)
{
    tbef = -1;
    tnow = (*dt)[idx].time().as_int();
    at = -999.0;
}

void ValueAtTime::compute_tick(const Env &env, int idx)
{
    tbef = tnow;
    tnow = (*dt)[idx].time().as_int();
    if (tbef > tnow)
    {
        tbef = -1;
        if (reset_overnight)
        {
            next_at.push_back(-999.0);
        }
    }
    
    if ((((tbef < time) && (tnow >= time)) || (tnow == time)) && (!(tbef > tnow)))
    {
        if (m_children.size() > 0)
        {
            next_at.push_back(m_children[0]->get_output());
        }
        else
        {
            next_at.push_back(false);
        }
    }
    
    if (next_at.size() > 0)
    {
        if (days_ago > (next_at.size() - 1))
        {
            at = next_at[0];
        }
        else
        {
            at = next_at[next_at.size() - 1 - days_ago];
        }
    }
    
    m_outp = at;
    
    /*if (fuckshit)
    {
        std::cout << "outp is " << get<double>(m_outp) << " idx is " << idx << " data is " << get<int>(m_parameters["data"]) << "\n";
    }*/
}

std::string ValueAtTime::get_el_init()
{
    std::stringstream s;
    
    s << fmt(
            "    {tbef} = -1; "
                    "    {tnow} = {curtime}; "
                    "    {at} = -999.0; "
    ) % m_efd;
    
    return s.str();
}

std::string ValueAtTime::get_el_tick()
{
    std::stringstream s;
    
    s << fmt(
            
            "    {tbef} = {tnow}; "
                    "    {tnow} = {curtime}; "
                    "    if ({tbef} > {tnow}) then "
                    "    begin "
                    "        {tbef} = -1; "
                    "        if {reset_overnight} then "
                    "        begin \n"
                    "           Array_SetMaxIndex({next_at}, {next_at_idx}+2); \" // 2 just in case\n"
                    "           {next_at_idx} = {next_at_idx}+1; \"\n"
                    "           {next_at}[{next_at_idx}] = -999;"
                    "        end; \n"
                    "    end; "
                    " "
                    "    if (((({tbef} < {time}) and ({tnow} >= {time})) or ({tnow} = {time}))) and (not ({tbef} > {tnow})) then "
                    "    begin "
                    "          Array_SetMaxIndex({next_at}, {next_at_idx}+2); " // 2 just in case
                    "          {next_at_idx} = {next_at_idx}+1; "
                    "          {next_at}[{next_at_idx}] = {0};"
                    "    end; "
                    " "
                    "    if (({next_at_idx}+1) > 0) then "
                    "    begin "
                    "       if ({days_ago} > ({next_at_idx})) then "
                    "       begin "
                    "               {at} = {next_at}[0]; "
                    "       end "
                    "       else "
                    "       begin "
                    "               {at} = {next_at}[{next_at_idx} - {days_ago}]; "
                    "       end; "
                    "    end; "
                    " "
                    "    {outp} = {at}; "
    ) % m_efd;
    
    return s.str();
}

std::string ValueAtTime::get_mql_init()
{
    std::stringstream s;
    /*s << format(
        "%s = 0;"
        "%s = %s;"
        "%s = %s;") 
        % var("tbef") % var("at") % limits["min"] % var("tnow") 
        % mql_cur_time();*/
    return s.str();
}

std::string ValueAtTime::get_mql_tick()
{
    std::stringstream s;
    /*s << format(
        "%s = %s;"
        "%s = %s;"
        "if (%s > %s) { %s = 0; }"

        "if (TimeDay(Time[0]) != TimeDay(Time[1])) "
        "{ "
        "   %s = %s;"
        "} "

        "if ((%s < %s) && (%s >= %s)) "
        "{ "
        "   %s = %s[0]; "
        "} ") 
        % var("tbef") % var("tnow") 
        % var("tnow") % mql_cur_time() 
        % var("at") % limits["min"] 

        % var("tbef") % var("tnow") % var("tbef") 
        % var("tbef") % var("time") 
        % var("tnow") % var("time") % var("at") 
        % capitalize(get<std::string>(m_parameters["price"]));

    s << var("outp") << " = " << var("at") << ";";*/
    
    return s.str();
}


void AbsValueOf::compute_tick(const Env &env, int idx)
{
    if (m_return_type == RT_INT)
    {
        m_outp = ABS(convert_to_int(m_children[0]->get_output()));
    }
    if (m_return_type == RT_FLOAT)
    {
        m_outp = ABS(convert_to_float(m_children[0]->get_output()));
    }
}

std::string AbsValueOf::get_el_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % ("Absvalue(" + m_children[0]->get_el_output() + ")");
    return s.str();
}

std::string AbsValueOf::get_mql_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % ("MathAbs(" + m_children[0]->get_mql_output() + ")");
    return s.str();
}


void Negative::compute_tick(const Env &env, int idx)
{
    if (m_return_type == RT_INT)
    {
        m_outp = -(convert_to_int(m_children[0]->get_output()));
    }
    if (m_return_type == RT_FLOAT)
    {
        m_outp = -(convert_to_float(m_children[0]->get_output()));
    }
    else
    {
        throw std::runtime_error("Unknown return type passed to NEG()");
    }
}

std::string Negative::get_el_tick()
{
    std::stringstream s;
    s << format("%s = -(%s);") % var("outp") % m_children[0]->get_el_output();
    return s.str();
}

std::string Negative::get_mql_tick()
{
    std::stringstream s;
    s << format("%s = -(%s);") % var("outp") % m_children[0]->get_mql_output();
    return s.str();
}


ParameterConstraintMap PercentOfValue::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_x;
    t_x.usage_mode = CNT_MINMAX;
    t_x.min = -200.0;
    t_x.max = 200.0;
    t_x.mut_min = 0.0;
    t_x.mut_max = 100.0;
    t_x.mut_power = 10.0;
    t_x.is_mutable = true;
    t_x.default_value = 0.0;
    t_constraints["x"] = t_x;
    
    return t_constraints;
}

void PercentOfValue::update_data(const DataMatrix &data)
{
    Signal::update_data(data);
    
    x = convert_to_float(m_parameters["x"]);
}

void PercentOfValue::compute_tick(const Env &env, int idx)
{
    m_outp = (convert_to_float(m_children[0]->get_output()) * x) / 100.0;
}

std::string PercentOfValue::get_el_tick()
{
    std::stringstream s;
    s << var("outp") << " = ";
    s << format("((%s)*(%s)/100.0);")
         % m_children[0]->get_el_output() % var("x");
    return s.str();
}

std::string PercentOfValue::get_mql_tick()
{
    std::stringstream s;
    s << var("outp") << " = ";
    s << format("((%s)*(%s)/100.0);")
         % m_children[0]->get_mql_output() % var("x");
    return s.str();
}


ParameterConstraintMap ValueOf::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 1000000;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 10;
    t_bars_back.is_mutable = true;
    t_bars_back.default_value = 0;
    t_constraints["bars_back"] = t_bars_back;
    return t_constraints;
}

void ValueOf::init_codegen_variables()
{
    if (m_return_type == RT_BOOL)
        m_variables["val_array"] = VAR_TYPE_ARRAY_BOOL;
    if (m_return_type == RT_INT)
        m_variables["val_array"] = VAR_TYPE_ARRAY_INT;
    if (m_return_type == RT_FLOAT)
        m_variables["val_array"] = VAR_TYPE_ARRAY_FLOAT;
}


void ValueOf::update_data(const DataMatrix &data)
{
    bars_back = (unsigned int) convert_to_int(m_parameters["bars_back"]);
}

void ValueOf::compute_init(const Env &env, int idx)
{
    val_array.clear();
    val_array.resize(bars_back + 1);
}

void ValueOf::compute_tick(const Env &env, int idx)
{
    // shift all values with 1 to the right
    for (unsigned long i = val_array.size() - 1; i > 0; i--)
        val_array[i] = val_array[i - 1];
    if (m_return_type == RT_INT)
    {
        val_array[0] = convert_to_int(m_children[0]->get_output());
    }
    if (m_return_type == RT_FLOAT)
    {
        val_array[0] = convert_to_float(m_children[0]->get_output());
    }
    if (m_return_type == RT_BOOL)
    {
        val_array[0] = get<bool>(m_children[0]->get_output());
    }
    
    m_outp = val_array[bars_back];
}

std::string ValueOf::get_el_init()
{
    std::stringstream s;
    s << format(
            "Array_SetMaxIndex(%s, %s+1); "
    ) % var("val_array") % var("bars_back");
    return s.str();
}

std::string ValueOf::get_el_tick()
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
    
    s << format("%s = %s[%s];") % var("outp") % var("val_array") % var("bars_back");
    
    return s.str();
}


std::string ValueOf::get_mql_init()
{
    return ""; // TODO MQL4
}

std::string ValueOf::get_mql_tick()
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
    
    s << format("%s = %s[%s];") % var("outp") % var("val_array") % var("bars_back");
    
    return s.str();
}


ParameterConstraintMap StatValueForPeriod::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    SP_constraints t_period;
    t_period.usage_mode = CNT_MINMAX;
    t_period.min = 2;
    t_period.max = 1200;
    t_period.mut_min = 2;
    t_period.mut_max = 120;
    t_period.mut_power = 20;
    t_period.is_mutable = true;
    t_period.default_value = 1;
    t_constraints["period"] = t_period;
    return t_constraints;
}

void StatValueForPeriod::init_codegen_variables()
{
    m_variables["val_array"] = VAR_TYPE_ARRAY_FLOAT;
    m_variables["min_idx"] = VAR_TYPE_INT;
    m_variables["max_idx"] = VAR_TYPE_INT;
    m_variables["min_v"] = VAR_TYPE_FLOAT;
    m_variables["max_v"] = VAR_TYPE_FLOAT;
    m_variables["avg_v"] = VAR_TYPE_FLOAT;
    m_variables["sum_v"] = VAR_TYPE_FLOAT;
}

void StatValueForPeriod::update_data(const DataMatrix &data)
{
    period = (unsigned int) convert_to_int(m_parameters["period"]);
}

void StatValueForPeriod::compute_init(const Env &env, int idx)
{
    val_array.clear();
    val_array.resize(period + 1);
}

void StatValueForPeriod::compute_tick(const Env &env, int idx)
{
    // shift all values with 1 to the right
    for (unsigned long i = val_array.size() - 1; i > 0; i--)
    {
        val_array[i] = val_array[i - 1];
    }
    val_array[0] = convert_to_float(m_children[0]->get_output());
    
    // Calculate all stats
    min_v = std::numeric_limits<double>::max();
    max_v = std::numeric_limits<double>::min();
    avg_v = 0;
    sum_v = 0;
    min_idx = 0;
    max_idx = 0;
    
    for (unsigned int i = 0; i < val_array.size(); i++)
    {
        if (val_array[i] < min_v)
        {
            min_v = val_array[i];
            min_idx = i;
        }
        if (val_array[i] > max_v)
        {
            max_v = val_array[i];
            max_idx = i;
        }
        sum_v += val_array[i];
    }
    if (val_array.size() > 0)
    {
        avg_v = sum_v / val_array.size();
    }
}

std::string StatValueForPeriod::get_el_init()
{
    std::stringstream s;
    s << format(
            "Array_SetMaxIndex(%s, %s+1); "
    ) % var("val_array") % var("period");
    return s.str();
}

std::string StatValueForPeriod::get_el_tick()
{
    std::stringstream s;
    s << format(
            "for idx = %s downto 1 "
                    "begin "
                    "   %s[idx] = %s[idx-1]; "
                    "end;"
                    "%s[0] = %s;")
         % var("period") % var("val_array") % var("val_array")
         % var("val_array") % m_children[0]->get_el_output();
    
    s << format(
            "%s = %s; "
                    "%s = %s; "
                    "%s = 0; "
                    "%s = 0; "
                    "for idx = 0 to %s "
                    "begin "
                    "   if %s[idx] < %s then "
                    "   begin "
                    "       %s = %s[idx]; "
                    "       %s = idx; "
                    "   end; "
                    "   if %s[idx] > %s then "
                    "   begin "
                    "       %s = %s[idx]; "
                    "       %s = idx; "
                    "   end; "
                    "   %s = %s + %s[idx];"
                    "end; "
                    "if %s > 0 then %s = %s / %s;"
    )
         % var("min_v") % limits["max"]
         % var("max_v") % limits["min"]
         % var("sum_v") % var("avg_v")
         % var("period")
         % var("val_array") % var("min_v")
         % var("min_v") % var("val_array")
         % var("min_idx")
         % var("val_array") % var("max_v")
         % var("max_v") % var("val_array")
         % var("max_idx")
         % var("sum_v") % var("sum_v") % var("val_array")
         % var("period") % var("avg_v") % var("sum_v") % var("period");
    
    return s.str();
}

std::string StatValueForPeriod::get_mql_init()
{
    return ""; // TODO MQL4
}

std::string StatValueForPeriod::get_mql_tick()
{
    std::stringstream s;
    s << format(
            "for(idx = %s; idx > 0; idx--) "
                    "{ "
                    "   %s[idx] = %s[idx-1]; "
                    "} "
                    "%s[0] = %s;")
         % var("period") % var("val_array") % var("val_array")
         % var("val_array") % m_children[0]->get_mql_output();
    
    s << format(
            "%s = %s; "
                    "%s = %s; "
                    "%s = 0; "
                    "%s = 0; "
                    "for(idx = 0; idx < %s; idx++) "
                    "{ "
                    "   if (%s[idx] < %s) "
                    "   { "
                    "       %s = %s[idx]; "
                    "       %s = idx; "
                    "   } "
                    "   if (%s[idx] > %s) "
                    "   { "
                    "       %s = %s[idx]; "
                    "       %s = idx; "
                    "   } "
                    "   %s = %s + %s[idx];"
                    "} "
                    "if (%s > 0) { %s = %s / %s; } "
    )
         % var("min_v") % limits["max"]
         % var("max_v") % limits["min"]
         % var("sum_v") % var("avg_v")
         % var("period")
         % var("val_array") % var("min_v")
         % var("min_v") % var("val_array")
         % var("min_idx")
         % var("val_array") % var("max_v")
         % var("max_v") % var("val_array")
         % var("max_idx")
         % var("sum_v") % var("sum_v") % var("val_array")
         % var("period") % var("avg_v") % var("sum_v") % var("period");
    
    return s.str();
}

void MinValueForPeriod::compute_tick(const Env &env, int idx)
{
    StatValueForPeriod::compute_tick(env, idx);
    
    m_outp = min_v;
}

std::string MinValueForPeriod::get_el_tick()
{
    std::stringstream s;
    s << StatValueForPeriod::get_el_tick();
    s << var("outp") << " = ";
    s << var("min_v") << ";";
    return s.str();
}

std::string MinValueForPeriod::get_mql_tick()
{
    std::stringstream s;
    s << StatValueForPeriod::get_mql_tick();
    s << var("outp") << " = ";
    s << var("min_v") << ";";
    return s.str();
}

void MaxValueForPeriod::compute_tick(const Env &env, int idx)
{
    StatValueForPeriod::compute_tick(env, idx);
    
    m_outp = max_v;
}

std::string MaxValueForPeriod::get_el_tick()
{
    std::stringstream s;
    s << StatValueForPeriod::get_el_tick();
    s << var("outp") << " = ";
    s << var("max_v") << ";";
    return s.str();
}

std::string MaxValueForPeriod::get_mql_tick()
{
    std::stringstream s;
    s << StatValueForPeriod::get_mql_tick();
    s << var("outp") << " = ";
    s << var("max_v") << ";";
    return s.str();
}


void MinValueForPeriodIdx::compute_tick(const Env &env, int idx)
{
    StatValueForPeriod::compute_tick(env, idx);
    
    m_outp = min_idx;
}

std::string MinValueForPeriodIdx::get_el_tick()
{
    std::stringstream s;
    s << StatValueForPeriod::get_el_tick();
    s << var("outp") << " = ";
    s << var("min_idx") << ";";
    return s.str();
}

std::string MinValueForPeriodIdx::get_mql_tick()
{
    std::stringstream s;
    s << StatValueForPeriod::get_mql_tick();
    s << var("outp") << " = ";
    s << var("min_idx") << ";";
    return s.str();
}

void MaxValueForPeriodIdx::compute_tick(const Env &env, int idx)
{
    StatValueForPeriod::compute_tick(env, idx);
    
    m_outp = max_idx;
}

std::string MaxValueForPeriodIdx::get_el_tick()
{
    std::stringstream s;
    s << StatValueForPeriod::get_el_tick();
    s << var("outp") << " = ";
    s << var("max_idx") << ";";
    return s.str();
}

std::string MaxValueForPeriodIdx::get_mql_tick()
{
    std::stringstream s;
    s << StatValueForPeriod::get_mql_tick();
    s << var("outp") << " = ";
    s << var("max_idx") << ";";
    return s.str();
}

void SumValueForPeriod::compute_tick(const Env &env, int idx)
{
    StatValueForPeriod::compute_tick(env, idx);
    
    m_outp = sum_v;
}

std::string SumValueForPeriod::get_el_tick()
{
    std::stringstream s;
    s << StatValueForPeriod::get_el_tick();
    s << var("outp") << " = ";
    s << var("sum_v") << ";";
    return s.str();
}

std::string SumValueForPeriod::get_mql_tick()
{
    std::stringstream s;
    s << StatValueForPeriod::get_mql_tick();
    s << var("outp") << " = ";
    s << var("sum_v") << ";";
    return s.str();
}

void AvgValueForPeriod::compute_tick(const Env &env, int idx)
{
    StatValueForPeriod::compute_tick(env, idx);
    
    m_outp = avg_v;
}

std::string AvgValueForPeriod::get_el_tick()
{
    std::stringstream s;
    s << StatValueForPeriod::get_el_tick();
    s << var("outp") << " = ";
    s << var("avg_v") << ";";
    return s.str();
}

std::string AvgValueForPeriod::get_mql_tick()
{
    std::stringstream s;
    s << StatValueForPeriod::get_mql_tick();
    s << var("outp") << " = ";
    s << var("avg_v") << ";";
    return s.str();
}


ParameterConstraintMap ValueOfWhenFirstHappened::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_timeout;
    t_timeout.usage_mode = CNT_MINMAX;
    t_timeout.min = -1;
    t_timeout.max = 1200;
    t_timeout.mut_min = -1;
    t_timeout.mut_max = 1200;
    t_timeout.mut_power = 50;
    t_timeout.is_mutable = false;
    t_timeout.default_value = -1; // -1 is timeout every new day, 0 is no timeout ever, positive is the timeout
    t_constraints["timeout"] = t_timeout;
    
    return t_constraints;
}


void ValueOfWhenFirstHappened::init_codegen_variables()
{
    if (m_return_type == RT_BOOL)
        m_variables["val"] = VAR_TYPE_BOOL;
    if (m_return_type == RT_INT)
        m_variables["val"] = VAR_TYPE_INT;
    if (m_return_type == RT_FLOAT)
        m_variables["val"] = VAR_TYPE_FLOAT;
    m_variables["happened"] = VAR_TYPE_BOOL;
    m_variables["tcount"] = VAR_TYPE_INT;
}

void ValueOfWhenFirstHappened::update_data(const DataMatrix &data)
{
    dt = &(data.dt);
    timeout = convert_to_int(m_parameters["timeout"]);
}

void ValueOfWhenFirstHappened::compute_init(const Env &env, int idx)
{
    if (m_return_type == RT_INT)
        val = std::numeric_limits<int>::min();
    if (m_return_type == RT_FLOAT)
        val = std::numeric_limits<double>::min();
    if (m_return_type == RT_BOOL)
        val = false;
    happened = false;
    tcount = 0;
}

void ValueOfWhenFirstHappened::compute_tick(const Env &env, int idx)
{
    // reset happened on new days
    if (((timeout == -1) && ((*dt)[idx].date() != (*dt)[prev_idx].date())) ||
        ((timeout > 0) && (tcount > timeout)))
    {
        happened = false;
        if (m_return_type == RT_INT)
            val = std::numeric_limits<int>::min();
        if (m_return_type == RT_FLOAT)
            val = std::numeric_limits<double>::min();
        if (m_return_type == RT_BOOL)
            val = false;
        tcount = 0;
    }
    prev_idx = idx;
    
    if (happened)
    {
        tcount += 1;
    }
    
    if (!happened) // not happened yet?
    {
        if (get<bool>(m_children[1]->get_output()))
        {
            val = m_children[0]->get_output();
            happened = true;
            tcount = 0;
        }
    }
    
    m_outp = val;
}

std::string ValueOfWhenFirstHappened::get_el_init()
{
    std::stringstream s;
    if (m_return_type == RT_BOOL)
    {
        s << format("%s = false;"
                            "%s = false;")
             % var("val")
             % var("happened");
    }
    else
    {
        s << format("%s = %s;"
                            "%s = false;")
             % var("val") % limits["min"]
             % var("happened");
    }
    s << format("%s = 0;") % var("tcount");
    
    return s.str();
}

std::string ValueOfWhenFirstHappened::get_el_tick()
{
    std::stringstream s;
    s << format(
            "if (((%s = -1) and (Date[0] <> Date[1])) or ((%s > 0) and (%s > %s))) then "
                    "begin "
                    "   %s = false; "
                    "   %s = %s;"
                    "   %s = 0;"
                    "end; "
                    "if (%s) then "
                    "begin "
                    "   %s = %s + 1;"
                    "end; "
                    "if (not %s) then "
                    "begin "
                    "if (%s) then begin "
                    "   %s = %s; "
                    "   %s = true; "
                    "   %s = 0; "
                    "end; "
                    "end; "
    )
         % var("timeout") % var("timeout") % var("tcount") % var("timeout")
         % var("happened")
         % var("val") % ((m_return_type == RT_BOOL) ? "false" : limits["min"])
         % var("tcount")
         % var("happened")
         % var("tcount") % var("tcount")
         % var("happened")
         % m_children[1]->get_el_output()
         % var("val") % m_children[0]->get_el_output()
         % var("happened")
         % var("tcount");
    
    s << var("outp") << " = " << var("val") << ";";
    
    return s.str();
}

std::string ValueOfWhenFirstHappened::get_mql_init()
{
    std::stringstream s;
    
    if (m_return_type == RT_BOOL)
    {
        s <<
          format("%s = false;"
                         "%s = false;")
          % var("val")
          % var("happened");
    }
    else
    {
        s <<
          format("%s = %s;"
                         "%s = false;")
          % var("val") % limits["min"]
          % var("happened");
    }
    s << format("%s = 0;") % var("tcount");
    
    return s.str();
}

std::string ValueOfWhenFirstHappened::get_mql_tick()
{
    std::stringstream s;
    s << format(
            "if (((%s == -1) && (TimeDay(Time[0]) != TimeDay(Time[1]))) || ((%s > 0) && (%s > %s))) "
                    "{ "
                    "   %s = false; "
                    "   %s = %s;"
                    "   %s = 0;"
                    "} "
                    "if (%s) {"
                    "   %s = %s + 1;"
                    "} "
                    "if (!(%s)) "
                    "{ "
                    "   if (%s) "
                    "   { "
                    "     %s = %s; "
                    "     %s = true; "
                    "     %s = 0;"
                    "   } "
                    "} "
    )
         % var("timeout") % var("timeout") % var("tcount") % var("timeout")
         % var("happened")
         % var("val") % ((m_return_type == RT_BOOL) ? "false" : limits["min"])
         % var("tcount")
         % var("happened")
         % var("tcount") % var("tcount")
         % var("happened")
         % m_children[1]->get_mql_output()
         % var("val") % m_children[0]->get_mql_output()
         % var("happened")
         % var("tcount");
    
    s << var("outp") << " = " << var("val") << ";";
    
    return s.str();
}


void ValueOfWhenLastHappened::init_codegen_variables()
{
    if (m_return_type == RT_BOOL)
        m_variables["val"] = VAR_TYPE_BOOL;
    if (m_return_type == RT_INT)
        m_variables["val"] = VAR_TYPE_INT;
    if (m_return_type == RT_FLOAT)
        m_variables["val"] = VAR_TYPE_FLOAT;
}

void ValueOfWhenLastHappened::compute_init(const Env &env, int idx)
{
    if (m_return_type == RT_INT)
        val = std::numeric_limits<int>::min();
    if (m_return_type == RT_FLOAT)
        val = std::numeric_limits<double>::min();
    if (m_return_type == RT_BOOL)
        val = false;
}

void ValueOfWhenLastHappened::compute_tick(const Env &env, int idx)
{
    if (get<bool>(m_children[1]->get_output()))
    {
        val = m_children[0]->get_output();
    }
    
    m_outp = val;
}

std::string ValueOfWhenLastHappened::get_el_init()
{
    std::stringstream s;
    if (m_return_type == RT_BOOL)
    {
        s << format("%s = false;") % var("val");
    }
    else
    {
        s << format("%s = %s;") % var("val") % limits["min"];
    }
    
    return s.str();
}

std::string ValueOfWhenLastHappened::get_el_tick()
{
    std::stringstream s;
    s << format(" if (%s) then %s = %s; ")
         % m_children[1]->get_el_output()
         % var("val") % m_children[0]->get_el_output();
    
    s << var("outp") << " = " << var("val") << ";";
    
    return s.str();
}

std::string ValueOfWhenLastHappened::get_mql_init()
{
    std::stringstream s;
    if (m_return_type == RT_BOOL)
    {
        s << format("%s = false;") % var("val");
    }
    else
    {
        s << format("%s = %s;") % var("val") % limits["min"];
    }
    
    return s.str();
}

std::string ValueOfWhenLastHappened::get_mql_tick()
{
    std::stringstream s;
    s << format(" if (%s) { %s = %s; }")
         % m_children[1]->get_mql_output()
         % var("val") % m_children[0]->get_mql_output();
    
    s << var("outp") << " = " << var("val") << ";";
    
    return s.str();
}


ParameterConstraintMap StatValueSince::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_since;
    t_since.usage_mode = CNT_MINMAX;
    t_since.min = Time(0, 0);
    t_since.max = Time(23, 59);
    t_since.mut_min = Time(0, 0);
    t_since.mut_max = Time(23, 59);
    t_since.mut_power = Time(1, 0);
    t_since.is_mutable = false;
    t_since.default_value = Time(0, 0);
    t_constraints["since"] = t_since;
    
    SP_constraints t_to;
    t_to.usage_mode = CNT_MINMAX;
    t_to.min = Time(0, 0);
    t_to.max = Time(23, 59);
    t_to.mut_min = Time(0, 0);
    t_to.mut_max = Time(23, 59);
    t_to.mut_power = Time(1, 0);
    t_to.is_mutable = false;
    t_to.default_value = Time(23, 59, 59);
    t_constraints["to"] = t_to;
    
    SP_constraints t_use_to;
    t_use_to.usage_mode = CNT_POSSIBLE_VALUES;
    t_use_to.possible_values.push_back(SP_type(true));
    t_use_to.possible_values.push_back(SP_type(false));
    t_use_to.is_mutable = false;
    t_use_to.default_value = true;
    t_constraints["use_to"] = t_use_to;
    
    return t_constraints;
}

void StatValueSince::init_codegen_variables()
{
    m_variables["working"] = VAR_TYPE_BOOL;
    m_variables["min_val"] = VAR_TYPE_FLOAT;
    m_variables["max_val"] = VAR_TYPE_FLOAT;
    m_variables["avg_val"] = VAR_TYPE_FLOAT;
    m_variables["sum_val"] = VAR_TYPE_FLOAT;
    m_variables["val"] = VAR_TYPE_FLOAT;
    m_variables["bars_counted"] = VAR_TYPE_INT;
    m_variables["tbef"] = VAR_TYPE_INT;
    m_variables["tnow"] = VAR_TYPE_INT;
}

void StatValueSince::update_data(const DataMatrix &data)
{
    dt = &(data.dt);
    time_since = (get<Time>(m_parameters["since"])).as_int();
    time_to = (get<Time>(m_parameters["to"])).as_int();
    use_to = get<bool>(m_parameters["use_to"]);
}

void StatValueSince::compute_init(const Env &env, int idx)
{
    working = false;
    bars_counted = 0;
    tbef = -1;
    tnow = (*dt)[idx].time().as_int();
}

void StatValueSince::compute_tick(const Env &env, int idx)
{
    tbef = tnow;
    tnow = (*dt)[idx].time().as_int();
    if (tbef > tnow)
        tbef = -1;
    
    // on date change, reset
    /*
    if ((*dt)[idx].date() != (*dt)[prev_idx].date())
    {
        working = false;
        bars_counted = 0;
    }
    prev_idx = idx;
    */
    
    // if not working, clear all stats
    if ((!working) && (bars_counted == 0))
    {
        min_val = std::numeric_limits<double>::max();
        max_val = std::numeric_limits<double>::min();
        avg_val = 0;
        sum_val = 0;
    }
    
    if (working)
    {
        bars_counted++;
        val = convert_to_float(m_children[0]->get_output());
        
        if (val < min_val)
            min_val = val;
        if (val > max_val)
            max_val = val;
        sum_val += val;
        avg_val = sum_val / (double) bars_counted;
    }
    
    // time for since 
    if ((tbef < time_since) && (tnow >= time_since))
    {
        working = true; // start working
        bars_counted = 0;
        
        // ..again
        min_val = std::numeric_limits<double>::max();
        max_val = std::numeric_limits<double>::min();
        avg_val = 0;
        sum_val = 0;
    }
    
    // uses to - end time
    if (use_to)
    {
        if ((tbef < time_to) && (tnow >= time_to))
        {
            working = false;
        }
    }
}

std::string StatValueSince::get_el_init()
{
    std::stringstream s;
    s << format(
            "%s = false;"
                    "%s = -1;"
                    "%s = %s;"
    )
         % var("working")
         % var("tbef")
         % var("tnow") % el_cur_time();
    return s.str();
}

std::string StatValueSince::get_el_tick()
{
    std::stringstream s;
    s << format(
            "%s = %s;"
                    "%s = %s;"
                    "if (%s > %s) then %s = -1;"
                    
                    "{if (Date[0] <> Date[1]) then "
                    "begin "
                    "   %s = false; "
                    "   %s = 0; "
                    "end;} "
                    
                    "if ((not %s) and (%s = 0)) then "
                    "begin "
                    "   %s = %s; "
                    "   %s = %s; "
                    "   %s = 0; "
                    "   %s = 0; "
                    "end; "
                    
                    "if (%s) then "
                    "begin "
                    "   %s = %s + 1;"
                    "   %s = %s;"
                    "   if (%s < %s) then %s = %s;"
                    "   if (%s > %s) then %s = %s;"
                    "   %s = %s + %s;"
                    "   %s = %s / %s;"
                    "end; "
                    
                    "if ((%s < %s) and (%s >= %s)) then "
                    "begin "
                    "   %s = true; "
                    "   %s = 0; "
                    "   %s = %s; "
                    "   %s = %s; "
                    "   %s = 0; "
                    "   %s = 0; "
                    "end; "
                    
                    "if (%s) then "
                    "begin "
                    "   if ((%s < %s) and (%s >= %s)) then "
                    "   begin "
                    "       %s = false; "
                    "   end; "
                    "end;"
    )
         % var("tbef") % var("tnow")
         % var("tnow") % el_cur_time()
         % var("tbef") % var("tnow")
         % var("tbef")
    
         % var("working")
         % var("bars_counted")
    
         % var("working")
         % var("bars_counted")
         % var("min_val") % limits["max"]
         % var("max_val") % limits["min"]
         % var("avg_val")
         % var("sum_val")
    
         % var("working")
         % var("bars_counted") % var("bars_counted")
         % var("val") % (m_children[0]->get_el_output())
         % var("val") % var("min_val") % var("min_val") % var("val")
         % var("val") % var("max_val") % var("max_val") % var("val")
         % var("sum_val") % var("sum_val") % var("val")
         % var("avg_val") % var("sum_val") % var("bars_counted")
    
         % var("tbef") % var("since") % var("tnow") % var("since")
         % var("working")
         % var("bars_counted")
         % var("min_val") % limits["max"]
         % var("max_val") % limits["min"]
         % var("avg_val")
         % var("sum_val")
    
         % var("use_to")
         % var("tbef") % var("to") % var("tnow") % var("to")
         % var("working");
    
    return s.str();
}

std::string StatValueSince::get_mql_init()
{
    std::stringstream s;
    s << format(
            "%s = false;"
                    "%s = -1;"
                    "%s = %s;"
    )
         % var("working")
         % var("tbef")
         % var("tnow") % mql_cur_time();
    return s.str();
}

std::string StatValueSince::get_mql_tick()
{
    std::stringstream s;
    s << format(
            "%s = %s;"
                    "%s = %s;"
                    "if (%s > %s) { %s = -1; }"
                    
                    "/*if (TimeDay(Time[0]) != TimeDay(Time[1])) "
                    "{ "
                    "   %s = false; "
                    "   %s = 0; "
                    "}*/"
                    
                    "if ( (!(%s)) && (%s == 0) ) "
                    "{ "
                    "   %s = %s; "
                    "   %s = %s; "
                    "   %s = 0; "
                    "   %s = 0; "
                    "} "
                    
                    "if (%s) "
                    "{ "
                    "   %s = %s + 1;"
                    "   %s = %s;"
                    "   if (%s < %s) { %s = %s; }"
                    "   if (%s > %s) { %s = %s; }"
                    "   %s = %s + %s;"
                    "   %s = %s / %s;"
                    "} "
                    
                    "if ((%s < %s) && (%s >= %s)) "
                    "{ "
                    "   %s = true; "
                    "   %s = 0; "
                    "   %s = %s; "
                    "   %s = %s; "
                    "   %s = 0; "
                    "   %s = 0; "
                    "} "
                    
                    "if (%s)  "
                    "{ "
                    "   if ((%s < %s) && (%s >= %s)) "
                    "   { "
                    "       %s = false; "
                    "   } "
                    "}"
    )
         % var("tbef") % var("tnow")
         % var("tnow") % mql_cur_time()
         % var("tbef") % var("tnow")
         % var("tbef")
    
         % var("working")
         % var("bars_counted")
    
         % var("working")
         % var("bars_counted")
         % var("min_val") % limits["max"]
         % var("max_val") % limits["min"]
         % var("avg_val")
         % var("sum_val")
    
         % var("working")
         % var("bars_counted") % var("bars_counted")
         % var("val") % (m_children[0]->get_mql_output())
         % var("val") % var("min_val") % var("min_val") % var("val")
         % var("val") % var("max_val") % var("max_val") % var("val")
         % var("sum_val") % var("sum_val") % var("val")
         % var("avg_val") % var("sum_val") % var("bars_counted")
    
         % var("tbef") % var("since") % var("tnow") % var("since")
         % var("working")
         % var("bars_counted")
         % var("min_val") % limits["max"]
         % var("max_val") % limits["min"]
         % var("avg_val")
         % var("sum_val")
    
         % var("use_to")
         % var("tbef") % var("to") % var("tnow") % var("to")
         % var("working");
    
    return s.str();
}

void MinValueSince::compute_tick(const Env &env, int idx)
{
    StatValueSince::compute_tick(env, idx);
    
    m_outp = min_val;
}

std::string MinValueSince::get_el_tick()
{
    std::stringstream s;
    s << StatValueSince::get_el_tick();
    s << format("%s = %s;") % var("outp") % var("min_val");
    return s.str();
}

std::string MinValueSince::get_mql_tick()
{
    std::stringstream s;
    s << StatValueSince::get_mql_tick();
    s << format("%s = %s;") % var("outp") % var("min_val");
    return s.str();
}

void MaxValueSince::compute_tick(const Env &env, int idx)
{
    StatValueSince::compute_tick(env, idx);
    
    m_outp = max_val;
}

std::string MaxValueSince::get_el_tick()
{
    std::stringstream s;
    s << StatValueSince::get_el_tick();
    s << format("%s = %s;") % var("outp") % var("max_val");
    return s.str();
}

std::string MaxValueSince::get_mql_tick()
{
    std::stringstream s;
    s << StatValueSince::get_mql_tick();
    s << format("%s = %s;") % var("outp") % var("max_val");
    return s.str();
}

void AvgValueSince::compute_tick(const Env &env, int idx)
{
    StatValueSince::compute_tick(env, idx);
    
    m_outp = avg_val;
}

std::string AvgValueSince::get_el_tick()
{
    std::stringstream s;
    s << StatValueSince::get_el_tick();
    s << format("%s = %s;") % var("outp") % var("avg_val");
    return s.str();
}

std::string AvgValueSince::get_mql_tick()
{
    std::stringstream s;
    s << StatValueSince::get_mql_tick();
    s << format("%s = %s;") % var("outp") % var("avg_val");
    return s.str();
}

void SumValueSince::compute_tick(const Env &env, int idx)
{
    StatValueSince::compute_tick(env, idx);
    
    m_outp = sum_val;
}

std::string SumValueSince::get_el_tick()
{
    std::stringstream s;
    s << StatValueSince::get_el_tick();
    s << format("%s = %s;") % var("outp") % var("sum_val");
    return s.str();
}

std::string SumValueSince::get_mql_tick()
{
    std::stringstream s;
    s << StatValueSince::get_mql_tick();
    s << format("%s = %s;") % var("outp") % var("sum_val");
    return s.str();
}

void IfThenElse::compute_tick(const Env &env, int idx)
{
    if (get<bool>(m_children[0]->get_output()))
    {
        m_outp = m_children[1]->get_output();
    }
    else
    {
        m_outp = m_children[2]->get_output();
    }
}

std::string IfThenElse::get_el_tick()
{
    std::stringstream s;
    s << fmt("if ({0}) then \n"
                     "    begin \n"
                     "        {outp} = {1}; \n"
                     "    end \n"
                     "    else \n"
                     "    begin \n"
                     "        {outp} = {2}; \n"
                     "    end; ") % m_efd;
    return s.str();
}

std::string IfThenElse::get_mql_tick()
{
    std::stringstream s;
    s << fmt("if ({0})\n"
                     "    {\n"
                     "        {outp} = {1};\n"
                     "    }\n"
                     "    else\n"
                     "    {\n"
                     "        {outp} = {2};\n"
                     "    }") % m_mfd;
    return s.str();
}

ParameterConstraintMap SortedSignal::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_idx;
    t_idx.usage_mode = CNT_MINMAX;
    t_idx.min = 0;
    t_idx.max = 1000000;
    t_idx.mut_min = 0;
    t_idx.mut_max = 120;
    t_idx.mut_power = 1;
    t_idx.is_mutable = true;
    t_idx.default_value = 0;
    t_constraints["idx"] = t_idx;
    
    SP_constraints t_dir;
    t_dir.usage_mode = CNT_POSSIBLE_VALUES;
    t_dir.possible_values.push_back(SP_type(STR("asc")));
    t_dir.possible_values.push_back(SP_type(STR("desc")));
    t_dir.is_mutable = false;
    t_dir.default_value = STR("asc");
    t_constraints["dir"] = t_dir;
    
    return t_constraints;
}

void SortedSignal::init_codegen_variables()
{
    // Define used variables for code generation
    m_variables["arr"] = VAR_TYPE_ARRAY_FLOAT;
    m_variables["temp"] = VAR_TYPE_FLOAT;
}

void SortedSignal::update_data(const DataMatrix &data)
{
    idx = convert_to_int(m_parameters["idx"]);
    dir = get<std::string>(m_parameters["dir"]);
}

void SortedSignal::compute_init(const Env &env, int idx)
{
    arr.clear();
    arr.resize(m_children.size());
    temp = 0;
}

void SortedSignal::compute_tick(const Env &env, int a_idx)
{
    // get all signal outputs into the array
    for (int i = 0; i < m_children.size(); i++)
    {
        arr[i] = get<double>(m_children[i]->get_output());
    }
    
    // bubble sort that array now
    /*if (arr.size() > 0)
    {
        for (int i = 0; i < arr.size(); i++)
        {
            for (int j = arr.size() - 1; j > i; j--)
            {
                if (arr[j] < arr[j - 1])
                {
                    temp = arr[j - 1];
                    arr[j - 1] = arr[j];
                    arr[j] = temp;
                }
            }
        }
    }*/
    std::sort(arr.begin(), arr.end());
    
    if (dir == "asc")
    {
        m_outp = arr[idx];
    }
    else if (dir == "desc")
    {
        m_outp = arr[arr.size() - 1 - idx];
    }
    else
    {
        throw std::runtime_error("Unknown paremeter for SORTED - " + dir);
    }
}

std::string SortedSignal::get_el_init()
{
    return Signal::get_el_init();
}

std::string SortedSignal::get_el_tick()
{
    return Signal::get_el_tick();
}

std::string SortedSignal::get_mql_init()
{
    return Signal::get_mql_init();
}

std::string SortedSignal::get_mql_tick()
{
    return Signal::get_mql_tick();
}

ParameterConstraintMap SortedIdxSignal::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_idx;
    t_idx.usage_mode = CNT_MINMAX;
    t_idx.min = 0;
    t_idx.max = 1000000;
    t_idx.mut_min = 0;
    t_idx.mut_max = 120;
    t_idx.mut_power = 1;
    t_idx.is_mutable = true;
    t_idx.default_value = 0;
    t_constraints["idx"] = t_idx;
    
    SP_constraints t_dir;
    t_dir.usage_mode = CNT_POSSIBLE_VALUES;
    t_dir.possible_values.push_back(SP_type(STR("asc")));
    t_dir.possible_values.push_back(SP_type(STR("desc")));
    t_dir.is_mutable = false;
    t_dir.default_value = STR("asc");
    t_constraints["dir"] = t_dir;
    
    return t_constraints;
}

void SortedIdxSignal::init_codegen_variables()
{
    Signal::init_codegen_variables();
}

void SortedIdxSignal::update_data(const DataMatrix &data)
{
    idx = convert_to_int(m_parameters["idx"]);
    dir = get<std::string>(m_parameters["dir"]);
}

void SortedIdxSignal::compute_init(const Env &env, int idx)
{
    arr.clear();
    arr.resize(m_children.size());
    arridx.resize(m_children.size());
    temp = 0;
}


class MyPair
{
public:
    double a;
    int i;
};

bool pair_comp(const MyPair lhs, const MyPair rhs)
{
    return (lhs.a < rhs.a);
}

void SortedIdxSignal::compute_tick(const Env &env, int a_idx)
{
    std::vector<MyPair> pairs;
    
    // get all signal outputs into the array
    for (int i = 0; i < m_children.size(); i++)
    {
        arr[i] = get<double>(m_children[i]->get_output());
        arridx[i] = i;
        
        MyPair a;
        a.a = arr[i];
        a.i = i;
        pairs.push_back(a);
    }
    
    // bubble sort that array now
    /*if (arr.size() > 0)
    {
        for (int i = 0; i < arr.size(); i++)
        {
            for (int j = arr.size() - 1; j > i; j--)
            {
                if (arr[j] < arr[j - 1])
                {
                    temp = arr[j - 1];
                    arr[j - 1] = arr[j];
                    arr[j] = temp;
                    
                    // also swap the corresponding arridx
                    temp_int = arridx[j-1];
                    arridx[j-1] = arridx[j];
                    arridx[j] = temp_int;
                }
            }
        }
    }*/
    // use quicksort
    std::sort(pairs.begin(), pairs.end(), pair_comp);
    
    // copy the pairs back to the now sorted arrays
    for (int i = 0; i < pairs.size(); i++)
    {
        arr[i] = pairs[i].a;
        arridx[i] = pairs[i].i;
    }
    
    // reverse the arridx array
    std::vector<int> newarridx;
    newarridx.resize(arridx.size());
    for (int i = 0; i < arridx.size(); i++)
    {
        newarridx[arridx[i]] = i;
    }
    
    if (dir == "asc")
    {
        m_outp = newarridx[idx];
    }
    else if (dir == "desc")
    {
        m_outp = newarridx[arr.size() - 1 - idx];
    }
    else
    {
        throw std::runtime_error("Unknown paremeter for SORTED - " + dir);
    }
}

std::string SortedIdxSignal::get_el_init()
{
    return Signal::get_el_init();
}

std::string SortedIdxSignal::get_el_tick()
{
    return Signal::get_el_tick();
}

std::string SortedIdxSignal::get_mql_init()
{
    return Signal::get_mql_init();
}

std::string SortedIdxSignal::get_mql_tick()
{
    return Signal::get_mql_tick();
}
