#include "stops.h"


ParameterConstraintMap PercentStopLoss::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;
    SP_constraints t_size;
    t_size.usage_mode = CNT_MINMAX;
    t_size.min = 0.0;
    t_size.max = 8.0;
    t_size.mut_min = 0.0;
    t_size.mut_max = 3.0;
    t_size.mut_power = 0.2;
    t_size.is_mutable = true;
    t_size.default_value = 1.0;
    t_constraints["size"] = t_size;
    return t_constraints;
}

void PercentStopLoss::init_codegen_variables()
{
    m_variables["stop_price"] = VAR_TYPE_FLOAT;
}

void PercentStopLoss::update_data( const DataMatrix& data )
{
    m_stop_price = 0;
    m_size = convert_to_float(m_parameters["size"]);
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        m_price_data = &(data.close);
    }
    else
    {
        m_price_data = &(data.open);
    }
}

void PercentStopLoss::compute_init_buying( const Env& env, int idx )
{
    // do this only when it's actually going to enter a position
    // so only when not in position or in the opposite one
    // otherwise keep the last stop price
    if ((env.current_position == NO_POSITION) || (env.current_position == SHORT_POSITION))
    {
        // set the stop price for long entry - needs to be below current close
        m_stop_price = (*m_price_data)[idx] - (*m_price_data)[idx] * m_size / 100.0;
    }
}

void PercentStopLoss::compute_init_selling( const Env& env, int idx )
{
    // do this only when it's actually going to enter a position
    // so only when not in position or in the opposite one
    // otherwise keep the last stop price
    if ((env.current_position == NO_POSITION) || (env.current_position == LONG_POSITION))
    {
        // set the stop price for short entry - needs to be above current close
        m_stop_price = (*m_price_data)[idx] + (*m_price_data)[idx] * m_size / 100.0;
    }
}

void PercentStopLoss::compute_tick( const Env& env, int idx )
{
    if ((m_size > 0) && (((env.current_position == 1) && ((*m_price_data)[idx] <= m_stop_price)) ||
        ((env.current_position == -1) && ((*m_price_data)[idx] >= m_stop_price))))
    	m_outp = true;
    else
    	m_outp = false;
}

std::string PercentStopLoss::get_el_init_buying()
{
    std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format(
                "if (MarketPosition = 0) or (MarketPosition = -1) then "
                        "begin "
                        "   %s = Close[0] - Close[0] * %s / 100.0; "
                        "end; "
        )
             % var("stop_price")
             % var("size");
    }
    else
    {
        s << format(
                "if (MarketPosition = 0) or (MarketPosition = -1) then "
                        "begin "
                        "   %s = Open[0] - Open[0] * %s / 100.0; "
                        "end; "
        )
             % var("stop_price")
             % var("size");
    }
    return s.str();
}

std::string PercentStopLoss::get_el_init_selling()
{
    std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format(
                "if (MarketPosition = 0) or (MarketPosition = 1) then "
                        "begin "
                        "   %s = Close[0] + Close[0] * %s / 100.0; "
                        "end; "
        )
             % var("stop_price")
             % var("size");
    }
    else
    {
        s << format(
                "if (MarketPosition = 0) or (MarketPosition = 1) then "
                        "begin "
                        "   %s = Open[0] + Open[0] * %s / 100.0; "
                        "end; "
        )
             % var("stop_price")
             % var("size");
    }
    return s.str();
}

std::string PercentStopLoss::get_el_tick()
{
	std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format("%s = %s;") % var("outp") % (format(
                "((%s > 0) and (((MarketPosition=1) and (Close[0] <= %s)) or "
                        "((MarketPosition=-1) and (Close[0] >= %s))))")
                                                 % var("size")
                                                 % var("stop_price")
                                                 % var("stop_price"));
    }
    else
    {
        s << format("%s = %s;") % var("outp") % (format(
                "((%s > 0) and (((MarketPosition=1) and (Open[0] <= %s)) or "
                        "((MarketPosition=-1) and (Open[0] >= %s))))")
                                                 % var("size")
                                                 % var("stop_price")
                                                 % var("stop_price"));
    }
    return s.str();
}

std::string PercentStopLoss::get_mql_init_buying()
{
    std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format(
                "if ((MarketPosition() == 0) || (MarketPosition() == -1)) "
                        "{ "
                        "   %s = Close[0] - Close[0] * %s / 100.0; "
                        "} "
        )
             % var("stop_price")
             % var("size");
    }
    else
    {
        s << format(
                "if ((MarketPosition() == 0) || (MarketPosition() == -1)) "
                        "{ "
                        "   %s = Open[0] - Open[0] * %s / 100.0; "
                        "} "
        )
             % var("stop_price")
             % var("size");
    }
    return s.str();
}

std::string PercentStopLoss::get_mql_init_selling()
{
    std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format(
                "if ((MarketPosition() == 0) || (MarketPosition() == 1)) "
                        "{ "
                        "   %s = Close[0] + Close[0] * %s / 100.0; "
                        "} "
        )
             % var("stop_price")
             % var("size");
    }
    else
    {
        s << format(
                "if ((MarketPosition() == 0) || (MarketPosition() == 1)) "
                        "{ "
                        "   %s = Open[0] + Open[0] * %s / 100.0; "
                        "} "
        )
             % var("stop_price")
             % var("size");
    }
    return s.str();
}

std::string PercentStopLoss::get_mql_tick()
{
	std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format("%s = %s;") % var("outp") % (format(
                "((%s > 0) && (((MarketPosition()==1) && (Close[0] <= %s)) || "
                        "((MarketPosition()==-1) && (Close[0] >= %s))))")
                                                 % var("size")
                                                 % var("stop_price")
                                                 % var("stop_price"));
    }
    else
    {
        s << format("%s = %s;") % var("outp") % (format(
                "((%s > 0) && (((MarketPosition()==1) && (Open[0] <= %s)) || "
                        "((MarketPosition()==-1) && (Open[0] >= %s))))")
                                                 % var("size")
                                                 % var("stop_price")
                                                 % var("stop_price"));
    }
    return s.str();
}


ParameterConstraintMap TrailingStopLoss::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;
    SP_constraints t_size;
    t_size.usage_mode = CNT_MINMAX;
    t_size.min = 0.0;
    t_size.max = 8.0;
    t_size.mut_min = 0.0;
    t_size.mut_max = 3.0;
    t_size.mut_power = 0.2;
    t_size.is_mutable = true;
    t_size.default_value = 1.0;
    t_constraints["size"] = t_size;
    return t_constraints;
}

void TrailingStopLoss::init_codegen_variables()
{
    m_variables["stop_price"] = VAR_TYPE_FLOAT;
    m_variables["range_price"] = VAR_TYPE_FLOAT;
}

void TrailingStopLoss::update_data( const DataMatrix& data )
{
    m_stop_price = 0;
    m_range_price = 0;
    m_size = convert_to_float(m_parameters["size"]);
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        m_price_data = &(data.close);
    }
    else
    {
        m_price_data = &(data.open);
    }
}

void TrailingStopLoss::compute_init_buying( const Env& env, int idx )
{
    // do this only when it's actually going to enter a position
    // so only when not in position or in the opposite one
    // otherwise keep the last stop price
    if ((env.current_position == NO_POSITION) || (env.current_position == SHORT_POSITION))
    {
        // set the stop price for long entry - needs to be below current close
        m_stop_price = (*m_price_data)[idx] - (*m_price_data)[idx] * m_size / 100.0;

        // set the range price
        m_range_price = (*m_price_data)[idx];
    }
}

void TrailingStopLoss::compute_init_selling( const Env& env, int idx )
{
    // do this only when it's actually going to enter a position
    // so only when not in position or in the opposite one
    // otherwise keep the last stop price
    if ((env.current_position == NO_POSITION) || (env.current_position == LONG_POSITION))
    {
        // set the stop price for short entry - needs to be above current close
        m_stop_price = (*m_price_data)[idx] + (*m_price_data)[idx] * m_size / 100.0;

        // set the range price
        m_range_price = (*m_price_data)[idx];
    }
}

void TrailingStopLoss::compute_tick( const Env& env, int idx )
{
    // if in position
    if (env.current_position != NO_POSITION)
    {
        // to move the range up, the price needs to be outside the range price
        // and the position must be long
        if (((*m_price_data)[idx] > m_range_price) && (env.current_position == LONG_POSITION))
        {
            double j = ABS((*m_price_data)[idx] - m_range_price); // the distance to move
            m_stop_price += j;
            m_range_price += j;
        }

        // to move the range down, the price needs to be outside the range price
        // and the position must be short
        if (((*m_price_data)[idx] < m_range_price) && (env.current_position == SHORT_POSITION))
        {
            double j = ABS((*m_price_data)[idx] - m_range_price); // the distance to move
            m_stop_price -= j;
            m_range_price -= j;
        }
    }

    if ((m_size > 0) && (((env.current_position == 1) && ((*m_price_data)[idx] <= m_stop_price)) ||
        ((env.current_position == -1) && ((*m_price_data)[idx] >= m_stop_price))))
    	m_outp = true;
    else
    	m_outp = false;
}

std::string TrailingStopLoss::get_el_init_buying()
{
    std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format(
                "if (MarketPosition = 0) or (MarketPosition = -1) then "
                        "begin "
                        "   %s = Close[0] - Close[0] * %s / 100.0; "
                        "   %s = Close[0]; "
                        "end; "
        )
             % var("stop_price")
             % var("size")
             % var("range_price");
    }
    else
    {
        s << format(
                "if (MarketPosition = 0) or (MarketPosition = -1) then "
                        "begin "
                        "   %s = Open[0] - Open[0] * %s / 100.0; "
                        "   %s = Open[0]; "
                        "end; "
        )
             % var("stop_price")
             % var("size")
             % var("range_price");
    }
    return s.str();
}

std::string TrailingStopLoss::get_el_init_selling()
{
    std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format(
                "if (MarketPosition = 0) or (MarketPosition = 1) then "
                        "begin "
                        "   %s = Close[0] + Close[0] * %s / 100.0; "
                        "   %s = Close[0]; "
                        "end; "
        )
             % var("stop_price")
             % var("size")
             % var("range_price");
    }
    else
    {
        s << format(
                "if (MarketPosition = 0) or (MarketPosition = 1) then "
                        "begin "
                        "   %s = Open[0] + Open[0] * %s / 100.0; "
                        "   %s = Open[0]; "
                        "end; "
        )
             % var("stop_price")
             % var("size")
             % var("range_price");
    }
    return s.str();
}

std::string TrailingStopLoss::get_el_tick()
{
    std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format(
                "if (MarketPosition <> 0) then "
                        "begin "
                        "   if ((Close[0] > %s) and (MarketPosition = 1)) then "
                        "   begin "
                        "       tmp = Absvalue(Close[0] - %s);"
                        "       %s = %s + tmp;"
                        "       %s = %s + tmp;"
                        "   end; "

                        "   if ((Close[0] < %s) and (MarketPosition = -1)) then "
                        "   begin "
                        "       tmp = Absvalue(Close[0] - %s);"
                        "       %s = %s - tmp;"
                        "       %s = %s - tmp;"
                        "   end; "

                        "end; "
        )
             % var("range_price")
             % var("range_price")
             % var("stop_price")
             % var("stop_price")
             % var("range_price")
             % var("range_price")

             % var("range_price")
             % var("range_price")
             % var("stop_price")
             % var("stop_price")
             % var("range_price")
             % var("range_price");

        s << format("%s = %s;") % var("outp") % (format(
                "((%s > 0) and (((MarketPosition=1) and (Close[0] <= %s)) or "
                        "((MarketPosition=-1) and (Close[0] >= %s))))")
                                                 % var("size")
                                                 % var("stop_price")
                                                 % var("stop_price"));
    }
    else
    {
        s << format(
                "if (MarketPosition <> 0) then "
                        "begin "
                        "   if ((Open[0] > %s) and (MarketPosition = 1)) then "
                        "   begin "
                        "       tmp = Absvalue(Open[0] - %s);"
                        "       %s = %s + tmp;"
                        "       %s = %s + tmp;"
                        "   end; "

                        "   if ((Open[0] < %s) and (MarketPosition = -1)) then "
                        "   begin "
                        "       tmp = Absvalue(Open[0] - %s);"
                        "       %s = %s - tmp;"
                        "       %s = %s - tmp;"
                        "   end; "

                        "end; "
        )
             % var("range_price")
             % var("range_price")
             % var("stop_price")
             % var("stop_price")
             % var("range_price")
             % var("range_price")

             % var("range_price")
             % var("range_price")
             % var("stop_price")
             % var("stop_price")
             % var("range_price")
             % var("range_price");

        s << format("%s = %s;") % var("outp") % (format(
                "((%s > 0) and (((MarketPosition=1) and (Open[0] <= %s)) or "
                        "((MarketPosition=-1) and (Open[0] >= %s))))")
                                                 % var("size")
                                                 % var("stop_price")
                                                 % var("stop_price"));
    }

    return s.str();
}


std::string TrailingStopLoss::get_mql_init_buying()
{
    std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format(
                "if ((MarketPosition() == 0) || (MarketPosition() == -1)) "
                        "{ "
                        "   %s = Close[0] - Close[0] * %s / 100.0; "
                        "   %s = Close[0]; "
                        "} "
        )
             % var("stop_price")
             % var("size")
             % var("range_price");
    }
    else
    {
        s << format(
                "if ((MarketPosition() == 0) || (MarketPosition() == -1)) "
                        "{ "
                        "   %s = Open[0] - Open[0] * %s / 100.0; "
                        "   %s = Open[0]; "
                        "} "
        )
             % var("stop_price")
             % var("size")
             % var("range_price");
    }
    return s.str();
}

std::string TrailingStopLoss::get_mql_init_selling()
{
    std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format(
                "if ((MarketPosition() == 0) || (MarketPosition() == 1)) "
                        "{ "
                        "   %s = Close[0] + Close[0] * %s / 100.0; "
                        "   %s = Close[0]; "
                        "} "
        )
             % var("stop_price")
             % var("size")
             % var("range_price");
    }
    else
    {
        s << format(
                "if ((MarketPosition() == 0) || (MarketPosition() == 1)) "
                        "{ "
                        "   %s = Open[0] + Open[0] * %s / 100.0; "
                        "   %s = Open[0]; "
                        "} "
        )
             % var("stop_price")
             % var("size")
             % var("range_price");
    }
    return s.str();
}

std::string TrailingStopLoss::get_mql_tick()
{
    std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format(
                "if (MarketPosition() != 0) "
                        "{ "
                        "   if ((Close[0] > %s) && (MarketPosition() == 1)) "
                        "   { "
                        "       tmpd = MathAbs(Close[0] - %s);"
                        "       %s = %s + tmpd;"
                        "       %s = %s + tmpd;"
                        "   } "

                        "   if ((Close[0] < %s) && (MarketPosition() == -1)) "
                        "   { "
                        "       tmpd = MathAbs(Close[0] - %s);"
                        "       %s = %s - tmpd;"
                        "       %s = %s - tmpd;"
                        "   } "
                        "} "
        )
             % var("range_price")
             % var("range_price")
             % var("stop_price")
             % var("stop_price")
             % var("range_price")
             % var("range_price")

             % var("range_price")
             % var("range_price")
             % var("stop_price")
             % var("stop_price")
             % var("range_price")
             % var("range_price");

        s << format("%s = %s;") % var("outp") % (format(
                "((%s > 0) && (((MarketPosition()==1) && (Close[0] <= %s)) || "
                        "((MarketPosition()==-1) && (Close[0] >= %s))))")
                                                 % var("size")
                                                 % var("stop_price")
                                                 % var("stop_price"));
    }
    else
    {
        s << format(
                "if (MarketPosition() != 0) "
                        "{ "
                        "   if ((Open[0] > %s) && (MarketPosition() == 1)) "
                        "   { "
                        "       tmpd = MathAbs(Open[0] - %s);"
                        "       %s = %s + tmpd;"
                        "       %s = %s + tmpd;"
                        "   } "

                        "   if ((Open[0] < %s) && (MarketPosition() == -1)) "
                        "   { "
                        "       tmpd = MathAbs(Open[0] - %s);"
                        "       %s = %s - tmpd;"
                        "       %s = %s - tmpd;"
                        "   } "
                        "} "
        )
             % var("range_price")
             % var("range_price")
             % var("stop_price")
             % var("stop_price")
             % var("range_price")
             % var("range_price")

             % var("range_price")
             % var("range_price")
             % var("stop_price")
             % var("stop_price")
             % var("range_price")
             % var("range_price");

        s << format("%s = %s;") % var("outp") % (format(
                "((%s > 0) && (((MarketPosition()==1) && (Open[0] <= %s)) || "
                        "((MarketPosition()==-1) && (Open[0] >= %s))))")
                                                 % var("size")
                                                 % var("stop_price")
                                                 % var("stop_price"));
    }

    return s.str();
}


ParameterConstraintMap PercentTakeProfit::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;
    SP_constraints t_size;
    t_size.usage_mode = CNT_MINMAX;
    t_size.min = 0.0;
    t_size.max = 8.0;
    t_size.mut_min = 0.0;
    t_size.mut_max = 3.0;
    t_size.mut_power = 0.2;
    t_size.is_mutable = true;
    t_size.default_value = 1.0;
    t_constraints["size"] = t_size;
    return t_constraints;
}

void PercentTakeProfit::init_codegen_variables()
{
    m_variables["stop_price"] = VAR_TYPE_FLOAT;
}

void PercentTakeProfit::update_data( const DataMatrix& data )
{
    m_stop_price = 0;
    m_size = convert_to_float(m_parameters["size"]);
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        m_price_data = &(data.close);
    }
    else
    {
        m_price_data = &(data.open);
    }
}

void PercentTakeProfit::compute_init_buying( const Env& env, int idx )
{
    // do this only when it's actually going to enter a position
    // so only when not in position or in the opposite one
    // otherwise keep the last stop price
    if ((env.current_position == NO_POSITION) || (env.current_position == SHORT_POSITION))
    {
        // set the stop price for long entry - needs to be above current close
        m_stop_price = (*m_price_data)[idx] + (*m_price_data)[idx] * m_size / 100.0;
    }
}

void PercentTakeProfit::compute_init_selling( const Env& env, int idx )
{
    // do this only when it's actually going to enter a position
    // so only when not in position or in the opposite one
    // otherwise keep the last stop price
    if ((env.current_position == NO_POSITION) || (env.current_position == LONG_POSITION))
    {
        // set the stop price for short entry - needs to be below current close
        m_stop_price = (*m_price_data)[idx] - (*m_price_data)[idx] * m_size / 100.0;
    }
}

void PercentTakeProfit::compute_tick( const Env& env, int idx )
{
    if ((m_size > 0) && (((env.current_position == 1) && ((*m_price_data)[idx] >= m_stop_price)) ||
        ((env.current_position == -1) && ((*m_price_data)[idx] <= m_stop_price))))
    	m_outp = true;
    else
    	m_outp = false;
}

std::string PercentTakeProfit::get_el_init_buying()
{
    std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format(
                "if (MarketPosition = 0) or (MarketPosition = -1) then "
                        "begin "
                        "   %s = Close[0] + Close[0] * %s / 100.0; "
                        "end; "
        )
             % var("stop_price")
             % var("size");
    }
    else
    {
        s << format(
                "if (MarketPosition = 0) or (MarketPosition = -1) then "
                        "begin "
                        "   %s = Open[0] + Open[0] * %s / 100.0; "
                        "end; "
        )
             % var("stop_price")
             % var("size");
    }
    return s.str();
}

std::string PercentTakeProfit::get_el_init_selling()
{
    std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format(
                "if (MarketPosition = 0) or (MarketPosition = 1) then "
                        "begin "
                        "   %s = Close[0] - Close[0] * %s / 100.0; "
                        "end; "
        )
             % var("stop_price")
             % var("size");
    }
    else
    {
        s << format(
                "if (MarketPosition = 0) or (MarketPosition = 1) then "
                        "begin "
                        "   %s = Open[0] - Open[0] * %s / 100.0; "
                        "end; "
        )
             % var("stop_price")
             % var("size");
    }
    return s.str();
}

std::string PercentTakeProfit::get_el_tick()
{
    std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format("%s = %s;") % var("outp") % (format(
                "((%s > 0) and (((MarketPosition=1) and (Close[0] >= %s)) or "
                        "((MarketPosition=-1) and (Close[0] <= %s))))")
                                                 % var("size")
                                                 % var("stop_price")
                                                 % var("stop_price"));
    }
    else
    {
        s << format("%s = %s;") % var("outp") % (format(
                "((%s > 0) and (((MarketPosition=1) and (Open[0] >= %s)) or "
                        "((MarketPosition=-1) and (Open[0] <= %s))))")
                                                 % var("size")
                                                 % var("stop_price")
                                                 % var("stop_price"));
    }
    return s.str();
}

std::string PercentTakeProfit::get_mql_init_buying()
{
    std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format(
                "if ((MarketPosition() == 0) || (MarketPosition() == -1)) "
                        "{ "
                        "   %s = Close[0] + Close[0] * %s / 100.0; "
                        "} "
        )
             % var("stop_price")
             % var("size");
    }
    else
    {
        s << format(
                "if ((MarketPosition() == 0) || (MarketPosition() == -1)) "
                        "{ "
                        "   %s = Open[0] + Open[0] * %s / 100.0; "
                        "} "
        )
             % var("stop_price")
             % var("size");
    }
    return s.str();
}

std::string PercentTakeProfit::get_mql_init_selling()
{
    std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format(
                "if ((MarketPosition() == 0) || (MarketPosition() == 1)) "
                        "{ "
                        "   %s = Close[0] - Close[0] * %s / 100.0; "
                        "} "
        )
             % var("stop_price")
             % var("size");
    }
    else
    {
        s << format(
                "if ((MarketPosition() == 0) || (MarketPosition() == 1)) "
                        "{ "
                        "   %s = Open[0] - Open[0] * %s / 100.0; "
                        "} "
        )
             % var("stop_price")
             % var("size");
    }
    return s.str();
}

std::string PercentTakeProfit::get_mql_tick()
{
    std::stringstream s;
    if (g_global_price_preference == PRICE_PREF_CLOSE)
    {
        s << format("%s = %s;") % var("outp") % (format(
                "((%s > 0) && (((MarketPosition()==1) && (Close[0] >= %s)) || "
                        "((MarketPosition()==-1) && (Close[0] <= %s))))")
                                                 % var("size")
                                                 % var("stop_price")
                                                 % var("stop_price"));
    }
    else
    {
        s << format("%s = %s;") % var("outp") % (format(
                "((%s > 0) && (((MarketPosition()==1) && (Open[0] >= %s)) || "
                        "((MarketPosition()==-1) && (Open[0] <= %s))))")
                                                 % var("size")
                                                 % var("stop_price")
                                                 % var("stop_price"));
    }
    return s.str();
}

