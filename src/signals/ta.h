#ifndef ta_h__
#define ta_h__

#ifndef NOTALIB

#include <ta-lib/ta_func.h>
#include <ta-lib/ta_common.h>
#include "base_signal.h"

// TA functions needed:

// MA, RSI, ATR, ADX, CCI, MACD, BBANDS, AROON, AROON_OSC, STOCH

class MA : public Signal 
{
public:
    
    // internal variables used
    
    // the computed data
    std::vector<double> m_data;
    
    int m_bars_back;
    int m_period;
    
    std::string m_price; // on which price
    std::string m_ma_type;
    
    MA(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "MA");
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void update_data(const DataMatrix& data);
    virtual void compute_tick(const Env& env, int idx);
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class RSI : public Signal 
{
public:
    
    // internal variables used
    
    // the computed data
    std::vector<double> m_data;
    
    int m_bars_back;
    int m_period;
    
    std::string m_price; // on which price
    
    RSI(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "RSI");
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void update_data(const DataMatrix& data);
    virtual void compute_tick(const Env& env, int idx);
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class ATR : public Signal 
{
public:
    
    // internal variables used
    
    // the computed data
    std::vector<double> m_data;
    
    int m_bars_back;
    int m_period;
    
    ATR(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "ATR");
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void update_data(const DataMatrix& data);
    virtual void compute_tick(const Env& env, int idx);
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class ADX : public Signal 
{
public:
    
    // internal variables used
    
    // the computed data
    std::vector<double> m_data;
    
    int m_bars_back;
    int m_period;
    
    ADX(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "ADX");
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void update_data(const DataMatrix& data);
    virtual void compute_tick(const Env& env, int idx);
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};



class CCI : public Signal 
{
public:
    
    // internal variables used
    
    // the computed data
    std::vector<double> m_data;
    
    int m_bars_back;
    int m_period;
    
    CCI(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "CCI");
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void update_data(const DataMatrix& data);
    virtual void compute_tick(const Env& env, int idx);
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class MACD_BASE : public Signal
{
public:
    
    // internal variables used
    
    // the computed data
    std::vector<double> m_data_macd;
    std::vector<double> m_data_signal;
    std::vector<double> m_data_hist;
    
    int m_bars_back;
    int m_fast_period;
    int m_slow_period;
    int m_signal_period;
    
    std::string price;
    
    virtual ParameterConstraintMap get_initial_constraints();
    virtual void update_data(const DataMatrix& data);
};

class MACD : public MACD_BASE
{
public:
    MACD(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "MACD");
    }
    virtual void compute_tick(const Env& env, int idx);
    
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class MACD_SIGNAL : public MACD_BASE
{
public:
    MACD_SIGNAL(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "MACD_SIGNAL");
    }
    virtual void compute_tick(const Env& env, int idx);
    
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class MACD_HIST : public MACD_BASE
{
public:
    MACD_HIST(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "MACD_HIST");
    }
    virtual void compute_tick(const Env& env, int idx);
    
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class BBANDS : public Signal
{
public:
    // internal variables used
    
    // the computed data
    std::vector<double> m_data_upper;
    std::vector<double> m_data_middle;
    std::vector<double> m_data_lower;
    
    // parameters
    std::string price;
    int m_bars_back;
    int m_period;
    std::string m_ma_type;
    double m_dev_up;
    double m_dev_down;
    
    virtual ParameterConstraintMap get_initial_constraints();
    virtual void update_data(const DataMatrix& a_data);
};

class BBANDS_UPPER : public BBANDS
{
public:
    BBANDS_UPPER(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "BBANDS_UPPER");
    }
    virtual void compute_tick(const Env& env, int idx);
    
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class BBANDS_MIDDLE : public BBANDS
{
public:
    BBANDS_MIDDLE(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "BBANDS_MIDDLE");
    }
    virtual void compute_tick(const Env& env, int idx);
    
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class BBANDS_LOWER : public BBANDS
{
public:
    BBANDS_LOWER(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "BBANDS_LOWER");
    }
    virtual void compute_tick(const Env& env, int idx);
    
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class AROON : public Signal
{
public:
    // internal variables used
    
    // the computed data
    std::vector<double> m_data_up;
    std::vector<double> m_data_down;
    
    // parameters
    int m_bars_back;
    int m_period;
    
    virtual ParameterConstraintMap get_initial_constraints();
    virtual void update_data(const DataMatrix& a_data);
};

class AROON_UP : public AROON
{
public:
    AROON_UP(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "AROON_UP");
    }
    virtual void compute_tick(const Env& env, int idx);
    
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class AROON_DOWN : public AROON
{
public:
    AROON_DOWN(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "AROON_DOWN");
    }
    virtual void compute_tick(const Env& env, int idx);
    
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class AROON_OSC : public Signal
{
public:
    
    // internal variables used
    
    // the computed data
    std::vector<double> m_data;
    
    int m_bars_back;
    int m_period;
    
    AROON_OSC(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "AROON_OSC");
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void update_data(const DataMatrix& data);
    virtual void compute_tick(const Env& env, int idx);
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class STOCH : public Signal
{
public:
    // internal variables used
    
    // the computed data
    std::vector<double> m_data_k;
    std::vector<double> m_data_d;
    
    // parameters
    int m_bars_back;
    int m_fast_k_period;
    int m_slow_k_period;
    int m_slow_d_period;
    // todo: the MA types are also configurable
    
    virtual ParameterConstraintMap get_initial_constraints();
    virtual void update_data(const DataMatrix& a_data);
};

class STOCH_K : public STOCH
{
public:
    STOCH_K(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "STOCH_K");
    }
    virtual void compute_tick(const Env& env, int idx);
    
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class STOCH_D : public STOCH
{
public:
    STOCH_D(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "STOCH_D");
    }
    virtual void compute_tick(const Env& env, int idx);
    
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class WILLR : public Signal
{
public:
    
    // internal variables used
    
    // the computed data
    std::vector<double> m_data;
    
    int m_bars_back;
    int m_period;
    
    WILLR(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "WILLR");
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void update_data(const DataMatrix& data);
    virtual void compute_tick(const Env& env, int idx);
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class SAR : public Signal
{
public:
    
    // internal variables used
    
    // the computed data
    std::vector<double> m_data;
    
    int m_bars_back;
    double m_accel;
    double m_max;
    
    SAR(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "SAR");
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void update_data(const DataMatrix& data);
    virtual void compute_tick(const Env& env, int idx);
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class ULTOSC : public Signal
{
public:
    
    // internal variables used
    
    // the computed data
    std::vector<double> m_data;
    
    int m_bars_back;
    int m_period1;
    int m_period2;
    int m_period3;
    
    ULTOSC(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "ULTOSC");
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void update_data(const DataMatrix& data);
    virtual void compute_tick(const Env& env, int idx);
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class TSF : public Signal
{
public:
    
    // internal variables used
    
    // the computed data
    std::vector<double> m_data;
    
    int m_bars_back;
    int m_period;
    
    TSF(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "TSF");
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void update_data(const DataMatrix& data);
    virtual void compute_tick(const Env& env, int idx);
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class HeikinAshiPrice : public Signal
{
    // internal variables used
    
    // the computed data
    std::vector<double> m_open;
    std::vector<double> m_high;
    std::vector<double> m_low;
    std::vector<double> m_close;
    
    double xopen, prev_xopen;
    double xclose, prev_xclose;
    double xhigh, xlow;
    
public:
    HeikinAshiPrice(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "HA_PRICE");
    }
    
    virtual int get_max_bars_back()
    {
        return 4;
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    virtual void init_codegen_variables();
    virtual void update_data(const DataMatrix& data);
    virtual void compute_tick(const Env& env, int idx);
    
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};






#endif // NOTALIB

#endif // ta_h__
