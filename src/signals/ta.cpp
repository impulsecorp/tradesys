#ifndef NOTALIB

#include "ta.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

ParameterConstraintMap MA::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_price;
    t_price.usage_mode = CNT_POSSIBLE_VALUES;
    t_price.possible_values.push_back( SP_type( STR("open") ) );
    t_price.possible_values.push_back( SP_type( STR("high") ) );
    t_price.possible_values.push_back( SP_type( STR("low") ) );
    t_price.possible_values.push_back( SP_type( STR("close") ) );
    t_price.is_mutable = false;
    t_price.default_value = SP_type( STR("close") );
    t_constraints["price"] = t_price;
    
    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 120;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 10;
    t_bars_back.is_mutable = false;
    t_bars_back.default_value = 0;
    t_constraints["bars_back"] = t_bars_back;
    
    SP_constraints t_period;
    t_period.usage_mode = CNT_MINMAX;
    t_period.min = 2;
    t_period.max = 240;
    t_period.mut_min = 2;
    t_period.mut_max = 60;
    t_period.mut_power = 20;
    t_period.is_mutable = true;
    t_period.default_value = 3;
    t_constraints["period"] = t_period;
    
    // Define the parameter constraints and the variables to be used
    SP_constraints t_ma_type;
    t_ma_type.usage_mode = CNT_POSSIBLE_VALUES;
    t_ma_type.possible_values.push_back( SP_type( STR("sma") ) );
    t_ma_type.possible_values.push_back( SP_type( STR("wma") ) );
    t_ma_type.possible_values.push_back( SP_type( STR("ema") ) );
    t_ma_type.possible_values.push_back( SP_type( STR("trima") ) );
    t_ma_type.is_mutable = false;
    t_ma_type.default_value = SP_type( STR("sma") );
    t_constraints["ma_type"] = t_ma_type;
    
    return t_constraints;
}

void MA::update_data( const DataMatrix& a_data )
{
    m_period = convert_to_int(m_parameters["period"]);
    m_bars_back = convert_to_int(m_parameters["bars_back"]);
    
    // Compute the data
    m_data.clear();    
    m_data.resize(a_data.len());
    
    TA_MAType mtype = TA_MAType_SMA;
    if (get<std::string>(m_parameters["ma_type"]) == "sma")
        mtype = TA_MAType_SMA;
    if (get<std::string>(m_parameters["ma_type"]) == "wma")
        mtype = TA_MAType_WMA;
    if (get<std::string>(m_parameters["ma_type"]) == "ema")
        mtype = TA_MAType_EMA;
    if (get<std::string>(m_parameters["ma_type"]) == "trima")
        mtype = TA_MAType_TRIMA;
    
    // calculate
    int out_beg = 0;
    int out_nbelement = 0;
    
    if (get<std::string>(m_parameters["price"]) == "open")
    {
        TA_MA(0, (int)m_data.size()-1, 
              &(a_data.open[0]), 
              m_period, mtype, 
              &out_beg, &out_nbelement, 
              &(m_data[0])
        );
    }
    if (get<std::string>(m_parameters["price"]) == "high")
    {
        TA_MA(0, (int)m_data.size()-1, 
              &(a_data.high[0]), 
              m_period, mtype, 
              &out_beg, &out_nbelement, 
              &(m_data[0])
        );
    }
    if (get<std::string>(m_parameters["price"]) == "low")
    {
        TA_MA(0, (int)m_data.size()-1, 
              &(a_data.low[0]), 
              m_period, mtype, 
              &out_beg, &out_nbelement, 
              &(m_data[0])
        );
    }
    if (get<std::string>(m_parameters["price"]) == "close")
    {
        TA_MA(0, (int)m_data.size()-1, 
              &(a_data.close[0]), 
              m_period, mtype, 
              &out_beg, &out_nbelement, 
              &(m_data[0])
        );
    }
    
    // because the TA starts putting the results from index 0, make it aligned
    shift_ta_data(m_data, out_beg);
}

void MA::compute_tick( const Env& env, int idx )
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string MA::get_el_tick()
{
    std::stringstream s;
    
    s << var("outp") << " = ";
    
    if (get<std::string>(m_parameters["ma_type"]) == "sma")
        s << format("Average(%s, %s)[%s]") % get<std::string>(m_parameters["price"])
        % var("period") % var("bars_back");
    if (get<std::string>(m_parameters["ma_type"]) == "wma")
        s << format("WAverage(%s, %s)[%s]") % get<std::string>(m_parameters["price"])
        % var("period") % var("bars_back");
    if (get<std::string>(m_parameters["ma_type"]) == "ema")
        s << format("XAverage(%s, %s)[%s]") % get<std::string>(m_parameters["price"])
        % var("period") % var("bars_back");
    if (get<std::string>(m_parameters["ma_type"]) == "trima")
        s << format("TriAverage(%s, %s)[%s]") % get<std::string>(m_parameters["price"])
        % var("period") % var("bars_back");
    
    s << ";";
    
    return s.str();
}

std::string MA::get_mql_tick()
{
    std::stringstream s;
    
    s << var("outp") << " = ";
    
    if (get<std::string>(m_parameters["ma_type"]) == "sma")
        s << format("iMA(NULL,0, %s, 0, MODE_SMA, PRICE_%s, %s)")
        % var("period")
        % all_upper(get<std::string>(m_parameters["price"]))
        % var("bars_back");
    
    if (get<std::string>(m_parameters["ma_type"]) == "wma")
        s << format("iMA(NULL,0, %s, 0, MODE_LWMA, PRICE_%s, %s)")
        % var("period")
        % all_upper(get<std::string>(m_parameters["price"]))
        % var("bars_back");
    
    
    if (get<std::string>(m_parameters["ma_type"]) == "ema")
        s << format("iMA(NULL,0, %s, 0, MODE_EMA, PRICE_%s, %s)")
        % var("period")
        % all_upper(get<std::string>(m_parameters["price"]))
        % var("bars_back");
    
    s << ";";
    
    return s.str();
}



ParameterConstraintMap RSI::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_price;
    t_price.usage_mode = CNT_POSSIBLE_VALUES;
    t_price.possible_values.push_back( SP_type( STR("open") ) );
    t_price.possible_values.push_back( SP_type( STR("high") ) );
    t_price.possible_values.push_back( SP_type( STR("low") ) );
    t_price.possible_values.push_back( SP_type( STR("close") ) );
    t_price.is_mutable = false;
    t_price.default_value = SP_type( STR("close") );
    t_constraints["price"] = t_price;
    
    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 120;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 10;
    t_bars_back.is_mutable = false;
    t_bars_back.default_value = 0;
    t_constraints["bars_back"] = t_bars_back;
    
    SP_constraints t_period;
    t_period.usage_mode = CNT_MINMAX;
    t_period.min = 2;
    t_period.max = 240;
    t_period.mut_min = 2;
    t_period.mut_max = 60;
    t_period.mut_power = 20;
    t_period.is_mutable = true;
    t_period.default_value = 14;
    t_constraints["period"] = t_period;
    
    return t_constraints;
}

void RSI::update_data( const DataMatrix& a_data )
{
    m_period = convert_to_int(m_parameters["period"]);
    m_bars_back = convert_to_int(m_parameters["bars_back"]);
    
    // Compute the data
    m_data.clear();    
    m_data.resize(a_data.len());
    
    // calculate
    int out_beg = 0;
    int out_nbelement = 0;
    
    if (get<std::string>(m_parameters["price"]) == "open")
    {
        TA_RSI(0, (int)m_data.size()-1, 
               &(a_data.open[0]), 
               m_period, 
               &out_beg, &out_nbelement, 
               &(m_data[0])
        );
    }
    if (get<std::string>(m_parameters["price"]) == "high")
    {
        TA_RSI(0, (int)m_data.size()-1, 
               &(a_data.high[0]), 
               m_period, 
               &out_beg, &out_nbelement, 
               &(m_data[0])
        );
    }
    if (get<std::string>(m_parameters["price"]) == "low")
    {
        TA_RSI(0, (int)m_data.size()-1, 
               &(a_data.low[0]), 
               m_period, 
               &out_beg, &out_nbelement, 
               &(m_data[0])
        );
    }
    if (get<std::string>(m_parameters["price"]) == "close")
    {
        TA_RSI(0, (int)m_data.size()-1, 
               &(a_data.close[0]), 
               m_period, 
               &out_beg, &out_nbelement, 
               &(m_data[0])
        );
    }
    
    // because the TA starts putting the results from index 0, make it aligned
    shift_ta_data(m_data, out_beg);
}

void RSI::compute_tick( const Env& env, int idx )
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string RSI::get_el_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("RSI(%s, %s)[%s]")
    % get<std::string>(m_parameters["price"]) 
    % var("period") % var("bars_back"));
    return s.str();
}

std::string RSI::get_mql_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("iRSI(NULL, 0, %s, PRICE_%s, %s)")
    % var("period") 
    % all_upper(get<std::string>(m_parameters["price"]))
    % var("bars_back"));
    return s.str();
}


ParameterConstraintMap ATR::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 120;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 10;
    t_bars_back.is_mutable = false;
    t_bars_back.default_value = 0;
    t_constraints["bars_back"] = t_bars_back;
    
    SP_constraints t_period;
    t_period.usage_mode = CNT_MINMAX;
    t_period.min = 2;
    t_period.max = 240;
    t_period.mut_min = 2;
    t_period.mut_max = 60;
    t_period.mut_power = 20;
    t_period.is_mutable = true;
    t_period.default_value = 9;
    t_constraints["period"] = t_period;
    
    return t_constraints;
}

void ATR::update_data( const DataMatrix& a_data )
{
    m_period = convert_to_int(m_parameters["period"]);
    m_bars_back = convert_to_int(m_parameters["bars_back"]);
    
    // Compute the data
    m_data.clear();    
    m_data.resize(a_data.len());
    
    // calculate
    int out_beg = 0;
    int out_nbelement = 0;
    
    TA_ATR(0, (int)m_data.size()-1, 
           &(a_data.high[0]), 
           &(a_data.low[0]), 
           &(a_data.close[0]), 
           m_period, 
           &out_beg, &out_nbelement, 
           &(m_data[0])
    );
    
    // because the TA starts putting the results from index 0, make it aligned
    shift_ta_data(m_data, out_beg);
}

void ATR::compute_tick( const Env& env, int idx )
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string ATR::get_el_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("AvgTrueRange(%s)[%s]") % var("period") % var("bars_back"));
    return s.str();
}

std::string ATR::get_mql_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("iATR(NULL, 0, %s, %s)") % var("period") % var("bars_back"));
    return s.str();
}


ParameterConstraintMap ADX::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 120;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 10;
    t_bars_back.is_mutable = false;
    t_bars_back.default_value = 0;
    t_constraints["bars_back"] = t_bars_back;
    
    SP_constraints t_period;
    t_period.usage_mode = CNT_MINMAX;
    t_period.min = 2;
    t_period.max = 240;
    t_period.mut_min = 2;
    t_period.mut_max = 60;
    t_period.mut_power = 20;
    t_period.is_mutable = true;
    t_period.default_value = 9;
    t_constraints["period"] = t_period;
    
    return t_constraints;
}

void ADX::update_data( const DataMatrix& a_data )
{
    m_period = convert_to_int(m_parameters["period"]);
    m_bars_back = convert_to_int(m_parameters["bars_back"]);
    
    // Compute the data
    m_data.clear();    
    m_data.resize(a_data.len());
    
    // calculate
    int out_beg = 0;
    int out_nbelement = 0;
    
    TA_ADX(0, (int)m_data.size()-1, 
           &(a_data.high[0]), 
           &(a_data.low[0]), 
           &(a_data.close[0]), 
           m_period, 
           &out_beg, &out_nbelement, 
           &(m_data[0])
    );
    
    // because the TA starts putting the results from index 0, make it aligned
    shift_ta_data(m_data, out_beg);
}

void ADX::compute_tick( const Env& env, int idx )
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string ADX::get_el_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("ADX(%s)[%s]") % var("period") % var("bars_back"));
    return s.str();
}

std::string ADX::get_mql_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("iADX(NULL,0, %s, /* fix this */ PRICE_CLOSE, MODE_MAIN, %s)") % var("period") % var("bars_back"));
    return s.str();
}


ParameterConstraintMap CCI::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 120;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 10;
    t_bars_back.is_mutable = false;
    t_bars_back.default_value = 0;
    t_constraints["bars_back"] = t_bars_back;
    
    SP_constraints t_period;
    t_period.usage_mode = CNT_MINMAX;
    t_period.min = 2;
    t_period.max = 240;
    t_period.mut_min = 2;
    t_period.mut_max = 60;
    t_period.mut_power = 20;
    t_period.is_mutable = true;
    t_period.default_value = 9;
    t_constraints["period"] = t_period;
    
    return t_constraints;
}

void CCI::update_data( const DataMatrix& a_data )
{
    m_period = convert_to_int(m_parameters["period"]);
    m_bars_back = convert_to_int(m_parameters["bars_back"]);
    
    // Compute the data
    m_data.clear();    
    m_data.resize(a_data.len());
    
    // calculate
    int out_beg = 0;
    int out_nbelement = 0;
    
    TA_CCI(0, (int)m_data.size()-1, 
           &(a_data.high[0]), 
           &(a_data.low[0]), 
           &(a_data.close[0]), 
           m_period, 
           &out_beg, &out_nbelement, 
           &(m_data[0])
    );
    
    // because the TA starts putting the results from index 0, make it aligned
    shift_ta_data(m_data, out_beg);
}

void CCI::compute_tick( const Env& env, int idx )
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string CCI::get_el_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("CCI(%s)[%s]") % var("period") % var("bars_back"));
    return s.str(); 
}

std::string CCI::get_mql_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("iCCI(NULL,0, %s, PRICE_TYPICAL, %s)") % var("period") % var("bars_back"));
    return s.str(); 
}


ParameterConstraintMap MACD_BASE::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_price;
    t_price.usage_mode = CNT_POSSIBLE_VALUES;
    t_price.possible_values.push_back( SP_type( STR("open") ) );
    t_price.possible_values.push_back( SP_type( STR("high") ) );
    t_price.possible_values.push_back( SP_type( STR("low") ) );
    t_price.possible_values.push_back( SP_type( STR("close") ) );
    t_price.is_mutable = false;
    t_price.default_value = SP_type( STR("close") );
    t_constraints["price"] = t_price;
    
    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 120;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 10;
    t_bars_back.is_mutable = false;
    t_bars_back.default_value = 0;
    t_constraints["bars_back"] = t_bars_back;
    
    SP_constraints t_fast_period;
    t_fast_period.usage_mode = CNT_MINMAX;
    t_fast_period.min = 2;
    t_fast_period.max = 240;
    t_fast_period.mut_min = 2;
    t_fast_period.mut_max = 60;
    t_fast_period.mut_power = 20;
    t_fast_period.is_mutable = true;
    t_fast_period.default_value = 12;
    t_constraints["fast_period"] = t_fast_period;
    
    SP_constraints t_slow_period;
    t_slow_period.usage_mode = CNT_MINMAX;
    t_slow_period.min = 2;
    t_slow_period.max = 240;
    t_slow_period.mut_min = 2;
    t_slow_period.mut_max = 60;
    t_slow_period.mut_power = 20;
    t_slow_period.is_mutable = true;
    t_slow_period.default_value = 26;
    t_constraints["slow_period"] = t_slow_period;
    
    SP_constraints t_signal_period;
    t_signal_period.usage_mode = CNT_MINMAX;
    t_signal_period.min = 2;
    t_signal_period.max = 240;
    t_signal_period.mut_min = 2;
    t_signal_period.mut_max = 60;
    t_signal_period.mut_power = 20;
    t_signal_period.is_mutable = true;
    t_signal_period.default_value = 18;
    t_constraints["signal_period"] = t_signal_period;
    
    return t_constraints;
}

void MACD_BASE::update_data( const DataMatrix& a_data )
{
    m_fast_period = convert_to_int(m_parameters["fast_period"]);
    m_slow_period = convert_to_int(m_parameters["slow_period"]);
    m_signal_period = convert_to_int(m_parameters["signal_period"]);
    m_bars_back = convert_to_int(m_parameters["bars_back"]);
    
    // Compute the data
    m_data_macd.clear();    
    m_data_macd.resize(a_data.len());
    m_data_signal.clear();    
    m_data_signal.resize(a_data.len());
    m_data_hist.clear();    
    m_data_hist.resize(a_data.len());
    
    // calculate
    int out_beg = 0;
    int out_nbelement = 0;
    
    if (get<std::string>(m_parameters["price"]) == "open")
    {
        TA_MACD(0, (int)a_data.len()-1, 
                &(a_data.open[0]), 
                m_fast_period, 
                m_slow_period, 
                m_signal_period, 
                &out_beg, &out_nbelement, 
                &(m_data_macd[0]),
                &(m_data_signal[0]),
                &(m_data_hist[0])
        );
    }
    if (get<std::string>(m_parameters["price"]) == "high")
    {
        TA_MACD(0, (int)a_data.len()-1, 
                &(a_data.high[0]), 
                m_fast_period, 
                m_slow_period, 
                m_signal_period, 
                &out_beg, &out_nbelement, 
                &(m_data_macd[0]),
                &(m_data_signal[0]),
                &(m_data_hist[0])
        );
    }
    if (get<std::string>(m_parameters["price"]) == "low")
    {
        TA_MACD(0, (int)a_data.len()-1, 
                &(a_data.low[0]), 
                m_fast_period, 
                m_slow_period, 
                m_signal_period, 
                &out_beg, &out_nbelement, 
                &(m_data_macd[0]),
                &(m_data_signal[0]),
                &(m_data_hist[0])
        );
    }
    if (get<std::string>(m_parameters["price"]) == "close")
    {
        TA_MACD(0, (int)a_data.len()-1, 
                &(a_data.close[0]), 
                m_fast_period, 
                m_slow_period, 
                m_signal_period, 
                &out_beg, &out_nbelement, 
                &(m_data_macd[0]),
                &(m_data_signal[0]),
                &(m_data_hist[0])
        );
    }
    
    // because the TA starts putting the results from index 0, make it aligned
    shift_ta_data(m_data_macd, out_beg);
    shift_ta_data(m_data_signal, out_beg);
    shift_ta_data(m_data_hist, out_beg);
}

void MACD::compute_tick( const Env& env, int idx )
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data_macd[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string MACD::get_el_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("MACD(%s, %s, %s)[%s]")
    % get<std::string>(m_parameters["price"]) 
    % var("fast_period") % var("slow_period")
    % var("bars_back"));
    return s.str(); 
}

std::string MACD::get_mql_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("iMACD(NULL, 0, %s, %s, %s, PRICE_%s, MODE_MAIN, %s)")
    % var("fast_period") % var("slow_period") % var("signal_period")
    % all_upper(get<std::string>(m_parameters["price"])) 
    % var("bars_back"));
    return s.str(); 
}

void MACD_SIGNAL::compute_tick(const Env& env, int idx)
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data_signal[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string MACD_SIGNAL::get_el_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("XAverage(MACD(%s, %s, %s), %s)[%s]")
    % get<std::string>(m_parameters["price"])
    % var("fast_period") % var("slow_period") % var("signal_period")
    % var("bars_back"));
    return s.str();
}

std::string MACD_SIGNAL::get_mql_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("iMACD(NULL, 0, %s, %s, %s, PRICE_%s, MODE_SIGNAL, %s)")
    % var("fast_period") % var("slow_period") % var("signal_period")
    % all_upper(get<std::string>(m_parameters["price"]))
    % var("bars_back"));
    return s.str();
}

void MACD_HIST::compute_tick(const Env& env, int idx)
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data_hist[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string MACD_HIST::get_el_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("(MACD(%s, %s, %s) - XAverage(MACD(%s, %s, %s), %s))[%s]")
    % get<std::string>(m_parameters["price"])
    % var("fast_period") % var("slow_period")
    % get<std::string>(m_parameters["price"])
    % var("fast_period") % var("slow_period") % var("signal_period")
    % var("bars_back"));
    return s.str();
}

std::string MACD_HIST::get_mql_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("iMACD(NULL, 0, %s, %s, %s, PRICE_%s, MODE_MAIN, %s) - iMACD(NULL, 0, %s, %s, %s, PRICE_%s, MODE_SIGNAL, %s)")
    % var("fast_period") % var("slow_period") % var("signal_period")
    % all_upper(get<std::string>(m_parameters["price"]))
    % var("bars_back")
    % var("fast_period") % var("slow_period") % var("signal_period")
    % all_upper(get<std::string>(m_parameters["price"]))
    % var("bars_back"));
    return s.str();
    
}

ParameterConstraintMap BBANDS::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_price;
    t_price.usage_mode = CNT_POSSIBLE_VALUES;
    t_price.possible_values.push_back( SP_type( STR("open") ) );
    t_price.possible_values.push_back( SP_type( STR("high") ) );
    t_price.possible_values.push_back( SP_type( STR("low") ) );
    t_price.possible_values.push_back( SP_type( STR("close") ) );
    t_price.is_mutable = false;
    t_price.default_value = SP_type( STR("close") );
    t_constraints["price"] = t_price;
    
    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 120;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 10;
    t_bars_back.is_mutable = false;
    t_bars_back.default_value = 0;
    t_constraints["bars_back"] = t_bars_back;
    
    SP_constraints t_period;
    t_period.usage_mode = CNT_MINMAX;
    t_period.min = 2;
    t_period.max = 240;
    t_period.mut_min = 2;
    t_period.mut_max = 60;
    t_period.mut_power = 20;
    t_period.is_mutable = true;
    t_period.default_value = 12;
    t_constraints["period"] = t_period;
    
    // Define the parameter constraints and the variables to be used
    SP_constraints t_ma_type;
    t_ma_type.usage_mode = CNT_POSSIBLE_VALUES;
    t_ma_type.possible_values.push_back( SP_type( STR("sma") ) );
    t_ma_type.possible_values.push_back( SP_type( STR("wma") ) );
    t_ma_type.possible_values.push_back( SP_type( STR("ema") ) );
    t_ma_type.possible_values.push_back( SP_type( STR("trima") ) );
    t_ma_type.is_mutable = false;
    t_ma_type.default_value = SP_type( STR("sma") );
    t_constraints["ma_type"] = t_ma_type;
    
    SP_constraints t_dev_up;
    t_dev_up.usage_mode = CNT_MINMAX;
    t_dev_up.min = 0.1;
    t_dev_up.max = 10.0;
    t_dev_up.mut_min = 0.1;
    t_dev_up.mut_max = 10.0;
    t_dev_up.mut_power = 1.0;
    t_dev_up.is_mutable = false;
    t_dev_up.default_value = 2.0;
    t_constraints["dev_up"] = t_dev_up;
    
    SP_constraints t_dev_down;
    t_dev_down.usage_mode = CNT_MINMAX;
    t_dev_down.min = 0.1;
    t_dev_down.max = 10.0;
    t_dev_down.mut_min = 0.1;
    t_dev_down.mut_max = 10.0;
    t_dev_down.mut_power = 1.0;
    t_dev_down.is_mutable = false;
    t_dev_down.default_value = 2.0;
    t_constraints["dev_down"] = t_dev_down;
    
    return t_constraints;
}

void BBANDS::update_data(const DataMatrix& a_data)
{
    m_period = convert_to_int(m_parameters["period"]);
    m_bars_back = convert_to_int(m_parameters["bars_back"]);
    m_dev_up = convert_to_float(m_parameters["dev_up"]);
    m_dev_down = convert_to_float(m_parameters["dev_down"]);
    
    TA_MAType mtype = TA_MAType_SMA;
    if (get<std::string>(m_parameters["ma_type"]) == "sma")
        mtype = TA_MAType_SMA;
    if (get<std::string>(m_parameters["ma_type"]) == "wma")
        mtype = TA_MAType_WMA;
    if (get<std::string>(m_parameters["ma_type"]) == "ema")
        mtype = TA_MAType_EMA;
    if (get<std::string>(m_parameters["ma_type"]) == "trima")
        mtype = TA_MAType_TRIMA;
    
    // Compute the data
    m_data_upper.clear();
    m_data_upper.resize(a_data.len());
    m_data_middle.clear();
    m_data_middle.resize(a_data.len());
    m_data_lower.clear();
    m_data_lower.resize(a_data.len());
    
    // calculate
    int out_beg = 0;
    int out_nbelement = 0;
    
    if (get<std::string>(m_parameters["price"]) == "open")
    {
        TA_BBANDS(0, (int)a_data.len()-1,
                  &(a_data.open[0]),
                  m_period,
                  m_dev_up,
                  m_dev_down,
                  mtype,
                  &out_beg, &out_nbelement,
                  &(m_data_upper[0]),
                  &(m_data_middle[0]),
                  &(m_data_lower[0])
        );
    }
    if (get<std::string>(m_parameters["price"]) == "high")
    {
        TA_BBANDS(0, (int)a_data.len()-1,
                  &(a_data.high[0]),
                  m_period,
                  m_dev_up,
                  m_dev_down,
                  mtype,
                  &out_beg, &out_nbelement,
                  &(m_data_upper[0]),
                  &(m_data_middle[0]),
                  &(m_data_lower[0])
        );
    }
    if (get<std::string>(m_parameters["price"]) == "low")
    {
        TA_BBANDS(0, (int)a_data.len()-1,
                  &(a_data.low[0]),
                  m_period,
                  m_dev_up,
                  m_dev_down,
                  mtype,
                  &out_beg, &out_nbelement,
                  &(m_data_upper[0]),
                  &(m_data_middle[0]),
                  &(m_data_lower[0])
        );
    }
    if (get<std::string>(m_parameters["price"]) == "close")
    {
        TA_BBANDS(0, (int)a_data.len()-1,
                  &(a_data.close[0]),
                  m_period,
                  m_dev_up,
                  m_dev_down,
                  mtype,
                  &out_beg, &out_nbelement,
                  &(m_data_upper[0]),
                  &(m_data_middle[0]),
                  &(m_data_lower[0])
        );
    }
    
    // because the TA starts putting the results from index 0, make it aligned
    shift_ta_data(m_data_upper, out_beg);
    shift_ta_data(m_data_middle, out_beg);
    shift_ta_data(m_data_lower, out_beg);
}

void BBANDS_UPPER::compute_tick(const Env& env, int idx)
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data_upper[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string BBANDS_UPPER::get_el_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("BollingerBand(%s, %s, %s)[%s]")
    % get<std::string>(m_parameters["price"])
    % var("period") % var("dev_up")
    % var("bars_back"));
    return s.str();
}

std::string BBANDS_UPPER::get_mql_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("iBands(NULL, 0, %s, %s, 0, PRICE_%s, MODE_UPPER, %s)")
    % var("period") % var("dev_up") % all_upper(get<std::string>(m_parameters["price"]))
    % var("bars_back"));
    return s.str();
}

void BBANDS_MIDDLE::compute_tick(const Env& env, int idx)
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data_middle[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string BBANDS_MIDDLE::get_el_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("BollingerBand(%s, %s, 0)[%s]")
    % get<std::string>(m_parameters["price"])
    % var("period") % var("bars_back"));
    return s.str();
}

std::string BBANDS_MIDDLE::get_mql_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("iBands(NULL, 0, %s, %s, 0, PRICE_%s, MODE_MAIN, %s)")
    % var("period") % var("dev_up") % all_upper(get<std::string>(m_parameters["price"])) // FIXME: probably shouldn't be dev_up
    % var("bars_back"));
    return s.str();
}

void BBANDS_LOWER::compute_tick(const Env& env, int idx)
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data_lower[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string BBANDS_LOWER::get_el_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("BollingerBand(%s, %s, -%s)[%s]")
    % get<std::string>(m_parameters["price"])
    % var("period") % var("dev_down")
    % var("bars_back"));
    return s.str();
}

std::string BBANDS_LOWER::get_mql_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("iBands(NULL, 0, %s, %s, 0, PRICE_%s, MODE_LOWER, %s)")
    % var("period") % var("dev_down") % all_upper(get<std::string>(m_parameters["price"]))
    % var("bars_back"));
    return s.str();
}

ParameterConstraintMap AROON::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 120;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 10;
    t_bars_back.is_mutable = false;
    t_bars_back.default_value = 0;
    t_constraints["bars_back"] = t_bars_back;
    
    SP_constraints t_period;
    t_period.usage_mode = CNT_MINMAX;
    t_period.min = 2;
    t_period.max = 240;
    t_period.mut_min = 2;
    t_period.mut_max = 60;
    t_period.mut_power = 20;
    t_period.is_mutable = true;
    t_period.default_value = 14;
    t_constraints["period"] = t_period;
    
    return t_constraints;
}

void AROON::update_data(const DataMatrix& a_data)
{
    m_period = convert_to_int(m_parameters["period"]);
    m_bars_back = convert_to_int(m_parameters["bars_back"]);
    
    // Compute the data
    m_data_up.clear();
    m_data_up.resize(a_data.len());
    m_data_down.clear();
    m_data_down.resize(a_data.len());
    
    // calculate
    int out_beg = 0;
    int out_nbelement = 0;
    
    TA_AROON(0, (int)m_data_up.size()-1,
             &(a_data.high[0]),
             &(a_data.low[0]),
             m_period,
             &out_beg, &out_nbelement,
             &(m_data_down[0]),
             &(m_data_up[0])
    );
    
    // because the TA starts putting the results from index 0, make it aligned
    shift_ta_data(m_data_up, out_beg);
    shift_ta_data(m_data_down, out_beg);
}

void AROON_UP::compute_tick(const Env& env, int idx)
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data_up[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string AROON_UP::get_el_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("AroonUp(%s)[%s]")
    % var("period")
    % var("bars_back"));
    return s.str();
}

std::string AROON_UP::get_mql_tick()
{
    //TODO: implement this in MQL4
    return "";
}

void AROON_DOWN::compute_tick(const Env& env, int idx)
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data_down[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string AROON_DOWN::get_el_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("AroonDown(%s)[%s]")
    % var("period")
    % var("bars_back"));
    return s.str();
}

std::string AROON_DOWN::get_mql_tick()
{
    //TODO: implement this in MQL4
    return "";
}

ParameterConstraintMap AROON_OSC::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 120;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 10;
    t_bars_back.is_mutable = false;
    t_bars_back.default_value = 0;
    t_constraints["bars_back"] = t_bars_back;
    
    SP_constraints t_period;
    t_period.usage_mode = CNT_MINMAX;
    t_period.min = 2;
    t_period.max = 240;
    t_period.mut_min = 2;
    t_period.mut_max = 60;
    t_period.mut_power = 20;
    t_period.is_mutable = true;
    t_period.default_value = 14;
    t_constraints["period"] = t_period;
    
    return t_constraints;
}

void AROON_OSC::update_data(const DataMatrix& a_data)
{
    m_period = convert_to_int(m_parameters["period"]);
    m_bars_back = convert_to_int(m_parameters["bars_back"]);
    
    // Compute the data
    m_data.clear();
    m_data.resize(a_data.len());
    
    // calculate
    int out_beg = 0;
    int out_nbelement = 0;
    
    TA_AROONOSC(0, (int)m_data.size()-1,
                &(a_data.high[0]),
                &(a_data.low[0]),
                m_period,
                &out_beg, &out_nbelement,
                &(m_data[0])
    );
    
    // because the TA starts putting the results from index 0, make it aligned
    shift_ta_data(m_data, out_beg);
}

void AROON_OSC::compute_tick(const Env& env, int idx)
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string AROON_OSC::get_el_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % (format("AroonOsc(%s)[%s]")
    % var("period")
    % var("bars_back"));
    return s.str();
    return "";
    return "";
}

std::string AROON_OSC::get_mql_tick()
{
    //TODO: implement this in MQL4
    return "";
}

ParameterConstraintMap STOCH::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 120;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 10;
    t_bars_back.is_mutable = false;
    t_bars_back.default_value = 0;
    t_constraints["bars_back"] = t_bars_back;
    
    SP_constraints t_fast_k_period;
    t_fast_k_period.usage_mode = CNT_MINMAX;
    t_fast_k_period.min = 2;
    t_fast_k_period.max = 240;
    t_fast_k_period.mut_min = 2;
    t_fast_k_period.mut_max = 60;
    t_fast_k_period.mut_power = 20;
    t_fast_k_period.is_mutable = true;
    t_fast_k_period.default_value = 9;
    t_constraints["fast_k_period"] = t_fast_k_period;
    
    SP_constraints t_slow_k_period;
    t_slow_k_period.usage_mode = CNT_MINMAX;
    t_slow_k_period.min = 2;
    t_slow_k_period.max = 240;
    t_slow_k_period.mut_min = 2;
    t_slow_k_period.mut_max = 60;
    t_slow_k_period.mut_power = 20;
    t_slow_k_period.is_mutable = true;
    t_slow_k_period.default_value = 14;
    t_constraints["slow_k_period"] = t_slow_k_period;
    
    SP_constraints t_slow_d_period;
    t_slow_d_period.usage_mode = CNT_MINMAX;
    t_slow_d_period.min = 2;
    t_slow_d_period.max = 240;
    t_slow_d_period.mut_min = 2;
    t_slow_d_period.mut_max = 60;
    t_slow_d_period.mut_power = 20;
    t_slow_d_period.is_mutable = true;
    t_slow_d_period.default_value = 26;
    t_constraints["slow_d_period"] = t_slow_d_period;
    
    return t_constraints;
    
}

void STOCH::update_data(const DataMatrix& a_data)
{
    m_fast_k_period = convert_to_int(m_parameters["fast_k_period"]);
    m_slow_k_period = convert_to_int(m_parameters["slow_k_period"]);
    m_slow_d_period = convert_to_int(m_parameters["slow_d_period"]);
    m_bars_back = convert_to_int(m_parameters["bars_back"]);
    
    // Compute the data
    m_data_k.clear();
    m_data_k.resize(a_data.len());
    m_data_d.clear();
    m_data_d.resize(a_data.len());
    
    // calculate
    int out_beg = 0;
    int out_nbelement = 0;
    
    TA_STOCH(0, (int)m_data_k.size()-1,
             &(a_data.high[0]),
             &(a_data.low[0]),
             &(a_data.close[0]),
             m_fast_k_period,
             m_slow_k_period,
             TA_MAType_SMA, // TODO: Make these MAs optional
             m_slow_d_period,
             TA_MAType_SMA,
             &out_beg, &out_nbelement,
             &(m_data_k[0]),
             &(m_data_d[0])
    );
    
    // because the TA starts putting the results from index 0, make it aligned
    shift_ta_data(m_data_k, out_beg);
    shift_ta_data(m_data_d, out_beg);
}

void STOCH_K::compute_tick(const Env& env, int idx)
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data_k[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string STOCH_K::get_el_tick()
{
    //TODO: implement this in EL
    return "";
}

std::string STOCH_K::get_mql_tick()
{
    //TODO: implement this in MQL4
    return "";
}

void STOCH_D::compute_tick(const Env& env, int idx)
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data_d[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string STOCH_D::get_el_tick()
{
    //TODO: implement this in EL
    return "";
}

std::string STOCH_D::get_mql_tick()
{
    //TODO: implement this in MQL4
    return "";
}

ParameterConstraintMap WILLR::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 120;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 10;
    t_bars_back.is_mutable = false;
    t_bars_back.default_value = 0;
    t_constraints["bars_back"] = t_bars_back;
    
    SP_constraints t_period;
    t_period.usage_mode = CNT_MINMAX;
    t_period.min = 2;
    t_period.max = 240;
    t_period.mut_min = 2;
    t_period.mut_max = 60;
    t_period.mut_power = 20;
    t_period.is_mutable = true;
    t_period.default_value = 9;
    t_constraints["period"] = t_period;
    
    return t_constraints;
}

void WILLR::update_data( const DataMatrix& a_data )
{
    m_period = convert_to_int(m_parameters["period"]);
    m_bars_back = convert_to_int(m_parameters["bars_back"]);
    
    // Compute the data
    m_data.clear();
    m_data.resize(a_data.len());
    
    // calculate
    int out_beg = 0;
    int out_nbelement = 0;
    
    TA_WILLR(0, (int)m_data.size()-1,
             &(a_data.high[0]),
             &(a_data.low[0]),
             &(a_data.close[0]),
             m_period,
             &out_beg, &out_nbelement,
             &(m_data[0])
    );
    
    // because the TA starts putting the results from index 0, make it aligned
    shift_ta_data(m_data, out_beg);
}

void WILLR::compute_tick( const Env& env, int idx )
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string WILLR::get_el_tick()
{
    std::stringstream s;
    // s << format("%s = %s;") % var("outp") % (format("ADX(%s)[%s]") % var("period") % var("bars_back"));
    return s.str();
}

std::string WILLR::get_mql_tick()
{
    std::stringstream s;
    //  s << format("%s = %s;") % var("outp") % (format("iADX(NULL,0, %s, /* fix this */ PRICE_CLOSE, MODE_MAIN, %s)") % var("period") % var("bars_back"));
    return s.str();
}

ParameterConstraintMap SAR::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 120;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 10;
    t_bars_back.is_mutable = false;
    t_bars_back.default_value = 0;
    t_constraints["bars_back"] = t_bars_back;
    
    SP_constraints t_accel;
    t_accel.usage_mode = CNT_MINMAX;
    t_accel.min = 0.1;
    t_accel.max = 10.0;
    t_accel.mut_min = 0.1;
    t_accel.mut_max = 10.0;
    t_accel.mut_power = 1.0;
    t_accel.is_mutable = true;
    t_accel.default_value = 1.0;
    t_constraints["accel"] = t_accel;
    
    SP_constraints t_max;
    t_max.usage_mode = CNT_MINMAX;
    t_max.min = 0.1;
    t_max.max = 10.0;
    t_max.mut_min = 0.1;
    t_max.mut_max = 10.0;
    t_max.mut_power = 1.0;
    t_max.is_mutable = true;
    t_max.default_value = 2.0;
    t_constraints["max"] = t_max;
    
    return t_constraints;
}

void SAR::update_data( const DataMatrix& a_data )
{
    m_bars_back = convert_to_int(m_parameters["bars_back"]);
    m_accel = convert_to_float(m_parameters["accel"]);
    m_max = convert_to_float(m_parameters["max"]);
    
    // Compute the data
    m_data.clear();
    m_data.resize(a_data.len());
    
    // calculate
    int out_beg = 0;
    int out_nbelement = 0;
    
    TA_SAR(0, (int)m_data.size()-1,
           &(a_data.high[0]),
           &(a_data.low[0]),
           m_accel,
           m_max,
           &out_beg, &out_nbelement,
           &(m_data[0])
    );
    
    // because the TA starts putting the results from index 0, make it aligned
    shift_ta_data(m_data, out_beg);
}

void SAR::compute_tick( const Env& env, int idx )
{
    if (!((idx - m_bars_back) < 0))
    {
        m_outp = m_data[idx - m_bars_back];
    }
    else
    {
        m_outp = 0.0;
    }
}

std::string SAR::get_el_tick()
{
    std::stringstream s;
    // s << format("%s = %s;") % var("outp") % (format("ADX(%s)[%s]") % var("period") % var("bars_back"));
    return s.str();
}

std::string SAR::get_mql_tick()
{
    std::stringstream s;
    //  s << format("%s = %s;") % var("outp") % (format("iADX(NULL,0, %s, /* fix this */ PRICE_CLOSE, MODE_MAIN, %s)") % var("period") % var("bars_back"));
    return s.str();
}



ParameterConstraintMap ULTOSC::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 120;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 10;
    t_bars_back.is_mutable = false;
    t_bars_back.default_value = 0;
    t_constraints["bars_back"] = t_bars_back;
    
    SP_constraints t_period1;
    t_period1.usage_mode = CNT_MINMAX;
    t_period1.min = 2;
    t_period1.max = 240;
    t_period1.mut_min = 2;
    t_period1.mut_max = 60;
    t_period1.mut_power = 20;
    t_period1.is_mutable = true;
    t_period1.default_value = 12;
    t_constraints["period1"] = t_period1;
    
    SP_constraints t_period2;
    t_period2.usage_mode = CNT_MINMAX;
    t_period2.min = 2;
    t_period2.max = 240;
    t_period2.mut_min = 2;
    t_period2.mut_max = 60;
    t_period2.mut_power = 20;
    t_period2.is_mutable = true;
    t_period2.default_value = 24;
    t_constraints["period2"] = t_period2;
    
    SP_constraints t_period3;
    t_period3.usage_mode = CNT_MINMAX;
    t_period3.min = 2;
    t_period3.max = 240;
    t_period3.mut_min = 2;
    t_period3.mut_max = 60;
    t_period3.mut_power = 20;
    t_period3.is_mutable = true;
    t_period3.default_value = 32;
    t_constraints["period3"] = t_period3;
    
    return t_constraints;
}

void ULTOSC::update_data( const DataMatrix& a_data )
{
    m_bars_back = convert_to_int(m_parameters["bars_back"]);
    m_period1 = convert_to_int(m_parameters["period1"]);
    m_period2 = convert_to_int(m_parameters["period2"]);
    m_period3 = convert_to_int(m_parameters["period3"]);
    
    // Compute the data
    m_data.clear();
    m_data.resize(a_data.len());
    
    // calculate
    int out_beg = 0;
    int out_nbelement = 0;
    
    TA_ULTOSC(0, (int)m_data.size()-1,
              &(a_data.high[0]),
              &(a_data.low[0]),
              &(a_data.close[0]),
              m_period1, m_period2, m_period3,
              &out_beg, &out_nbelement,
              &(m_data[0])
    );
    
    // because the TA starts putting the results from index 0, make it aligned
    shift_ta_data(m_data, out_beg);
}

void ULTOSC::compute_tick( const Env& env, int idx )
{
    m_outp = m_data[idx - m_bars_back];
}

std::string ULTOSC::get_el_tick()
{
    std::stringstream s;
    // s << format("%s = %s;") % var("outp") % (format("ADX(%s)[%s]") % var("period") % var("bars_back"));
    return s.str();
}

std::string ULTOSC::get_mql_tick()
{
    std::stringstream s;
    //  s << format("%s = %s;") % var("outp") % (format("iADX(NULL,0, %s, /* fix this */ PRICE_CLOSE, MODE_MAIN, %s)") % var("period") % var("bars_back"));
    return s.str();
}


ParameterConstraintMap TSF::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_bars_back;
    t_bars_back.usage_mode = CNT_MINMAX;
    t_bars_back.min = 0;
    t_bars_back.max = 120;
    t_bars_back.mut_min = 0;
    t_bars_back.mut_max = 120;
    t_bars_back.mut_power = 10;
    t_bars_back.is_mutable = false;
    t_bars_back.default_value = 0;
    t_constraints["bars_back"] = t_bars_back;
    
    SP_constraints t_period;
    t_period.usage_mode = CNT_MINMAX;
    t_period.min = 2;
    t_period.max = 240;
    t_period.mut_min = 2;
    t_period.mut_max = 60;
    t_period.mut_power = 20;
    t_period.is_mutable = true;
    t_period.default_value = 9;
    t_constraints["period"] = t_period;
    
    return t_constraints;
}

void TSF::update_data( const DataMatrix& a_data )
{
    m_period = convert_to_int(m_parameters["period"]);
    m_bars_back = convert_to_int(m_parameters["bars_back"]);
    
    // Compute the data
    m_data.clear();
    m_data.resize(a_data.len());
    
    // calculate
    int out_beg = 0;
    int out_nbelement = 0;
    
    // TODO: make it support any price or inner signal
    
    TA_TSF(0, (int)m_data.size()-1,
           &(a_data.close[0]),
           m_period,
           &out_beg, &out_nbelement,
           &(m_data[0])
    );
    
    // because the TA starts putting the results from index 0, make it aligned
    shift_ta_data(m_data, out_beg);
}

void TSF::compute_tick( const Env& env, int idx )
{
    m_outp = m_data[idx - m_bars_back];
}

std::string TSF::get_el_tick()
{
    std::stringstream s;
    // s << format("%s = %s;") % var("outp") % (format("ADX(%s)[%s]") % var("period") % var("bars_back"));
    return s.str();
}

std::string TSF::get_mql_tick()
{
    std::stringstream s;
    //  s << format("%s = %s;") % var("outp") % (format("iADX(NULL,0, %s, /* fix this */ PRICE_CLOSE, MODE_MAIN, %s)") % var("period") % var("bars_back"));
    return s.str();
}




ParameterConstraintMap HeikinAshiPrice::get_initial_constraints()
{
    ParameterConstraintMap t_constraints;
    
    SP_constraints t_price;
    t_price.usage_mode = CNT_POSSIBLE_VALUES;
    t_price.possible_values.push_back( SP_type( STR("open") ) );
    t_price.possible_values.push_back( SP_type( STR("high") ) );
    t_price.possible_values.push_back( SP_type( STR("low") ) );
    t_price.possible_values.push_back( SP_type( STR("close") ) );
    t_price.is_mutable = true;
    t_price.default_value = SP_type( STR("close") );
    t_constraints["price"] = t_price;
    
    return t_constraints;
}

void HeikinAshiPrice::update_data( const DataMatrix& a_data )
{
    m_open.clear();
    m_open.resize(a_data.len());
    m_high.clear();
    m_high.resize(a_data.len());
    m_low.clear();
    m_low.resize(a_data.len());
    m_close.clear();
    m_close.resize(a_data.len());
    
    for(unsigned int i=0; i<a_data.len(); i++)
    {
        m_open[i] = a_data.open[i];
        m_high[i] = a_data.high[i];
        m_low[i] = a_data.low[i];
        m_close[i] = a_data.close[i];
    }
    
    prev_xclose = xclose = 0;
    prev_xopen = xopen = 0;
    xhigh = 0;
    xlow = 0;
}

void HeikinAshiPrice::init_codegen_variables()
{
    // Define used variables for code generation
    m_variables["xopen"] = VAR_TYPE_FLOAT;
    m_variables["xclose"]  = VAR_TYPE_FLOAT;
    m_variables["prev_xopen"] = VAR_TYPE_FLOAT;
    m_variables["prev_xclose"]  = VAR_TYPE_FLOAT;
    m_variables["xhigh"] = VAR_TYPE_FLOAT;
    m_variables["xlow"] = VAR_TYPE_FLOAT;
}

void HeikinAshiPrice::compute_tick( const Env& env, int idx )
{
    if (idx > 2)
    {
        idx -= 1;
        
        prev_xclose = xclose;
        prev_xopen = xopen;
        
        std::vector<double> t;
        xclose = (m_open[idx] +
        m_high[idx] +
        m_low[idx] +
        m_close[idx])/4.0;
        xopen = (prev_xopen + m_close[idx-1])/2.0;
        
        t.clear();
        t.push_back(m_high[idx]);
        t.push_back(xopen);
        t.push_back(xclose);
        xhigh = *(std::max_element(t.begin(), t.end()));
        t.clear();
        t.push_back(m_low[idx]);
        t.push_back(xopen);
        t.push_back(xclose);
        xlow = *(std::min_element(t.begin(), t.end()));
        
        if (get<std::string>(m_parameters["price"]) == STR("open"))
        {
            m_outp = (xopen);
        }
        else if (get<std::string>(m_parameters["price"]) == STR("high"))
        {
            m_outp = (xhigh);
        }
        else if (get<std::string>(m_parameters["price"]) == STR("low"))
        {
            m_outp = (xlow);
        }
        else if (get<std::string>(m_parameters["price"]) == STR("close"))
        {
            m_outp = (xclose);
        }
        else
        {
            throw std::runtime_error("price name is wrong, needs to be one of these strings: open/high/low/close");
        }
    }
    else
    {
        m_outp = 0.0;
    }
}


std::string HeikinAshiPrice::get_el_tick()
{
    std::stringstream s;

    s << var("prev_xclose") << " = " << var("xclose") << ";";
    s << var("prev_xopen") << " = " << var("xopen") << ";";

    s << var("xclose") << " = " << "(Open[1] + High[1] + Low[1] + Close[1]) / 4.0;";
    s << var("xopen") << " = " << "(" << var("prev_xopen") << " + Close[2]) / 2.0;";

    s << var("xhigh") << " = MaxList(MaxList(High[1], " << var("xopen") << "), " << var("xclose") << ");";
    s << var("xlow") << " = MaxList(MaxList(Low[1], " << var("xopen") << "), " << var("xclose") << ");";

    if (get<std::string>(m_parameters["price"]) == STR("open"))
    {
        s << var("outp") << " = " << var("xopen");
    }
    else if (get<std::string>(m_parameters["price"]) == STR("high"))
    {
        s << var("outp") << " = " << var("xhigh");
    }
    else if (get<std::string>(m_parameters["price"]) == STR("low"))
    {
        s << var("outp") << " = " << var("xlow");
    }
    else if (get<std::string>(m_parameters["price"]) == STR("close"))
    {
        s << var("outp") << " = " << var("xclose");
    }
    s << ";";

    return s.str();
}

std::string HeikinAshiPrice::get_mql_tick()
{
    std::stringstream s;

    s << var("prev_xclose") << " = " << var("xclose") << ";";
    s << var("prev_xopen") << " = " << var("xopen") << ";";

    s << var("xclose") << " = " << "(Open[1] + High[1] + Low[1] + Close[1]) / 4.0;";
    s << var("xopen") << " = " << "(" << var("prev_xopen") << " + Close[2]) / 2.0;";

    s << var("xhigh") << " = MathMax(MathMax(High[1], " << var("xopen") << "), " << var("xclose") << ");";
    s << var("xlow") << " = MathMax(MathMax(Low[1], " << var("xopen") << "), " << var("xclose") << ");";

    if (get<std::string>(m_parameters["price"]) == STR("open"))
    {
        s << var("outp") << " = " << var("xopen");
    }
    else if (get<std::string>(m_parameters["price"]) == STR("high"))
    {
        s << var("outp") << " = " << var("xhigh");
    }
    else if (get<std::string>(m_parameters["price"]) == STR("low"))
    {
        s << var("outp") << " = " << var("xlow");
    }
    else if (get<std::string>(m_parameters["price"]) == STR("close"))
    {
        s << var("outp") << " = " << var("xclose");
    }
    s << ";";

    return s.str();
}

#endif






#pragma clang diagnostic pop
#pragma clang diagnostic pop
#pragma clang diagnostic pop