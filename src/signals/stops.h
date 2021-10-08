#ifndef stops_h__
#define stops_h__

#include "base_signal.h"


// Main class doing sl/tp stuff
// Doesn't work like the rest of the signals in the system. 
// It gets initialized every time there is a new order.
class StopSignal : public Signal 
{
public:
    const std::vector < double >* m_price_data;
    StopSignal()
    {
    	m_return_type = RT_BOOL;
        init_codegen_outp();
    }

    virtual void update_data(const DataMatrix& data)
    {
        m_price_data = &(data.close); // use the close price by default
    }

    // init the stop according to the new position
    virtual void compute_init_buying(const Env& env, int idx)
    {};
    virtual void compute_init_selling(const Env& env, int idx)
    {};
    virtual void compute_tick(const Env& env, int idx)
    { m_outp = false; }

    virtual std::string get_el_init_buying()
    { return ""; };
    virtual std::string get_el_init_selling()
    { return ""; };

    virtual std::string get_mql_init_buying()
    { return ""; };
    virtual std::string get_mql_init_selling()
    { return ""; };

    virtual std::string get_el_tick()
    { 
    	std::stringstream s;
    	s << format("%s = %s;") % var("outp") % "(false)";
    	return s.str();
    };

    virtual std::string get_mql_tick()
    { 
    	std::stringstream s;
    	s << format("%s = %s;") % var("outp") % "(false)";
    	return s.str();
    };
};


class PercentStopLoss : public StopSignal
{
public:
    double m_stop_price;
    double m_size;

    PercentStopLoss(ParameterMap& ow, std::vector<Signal*> ch)
    {
		INIT_SIGNAL(RT_BOOL, "STOP_LOSS");
    }

    virtual ParameterConstraintMap get_initial_constraints();
    virtual void init_codegen_variables();

    virtual void update_data(const DataMatrix& data);

    // init the stop according to the new position
    virtual void compute_init_buying(const Env& env, int idx);;
    virtual void compute_init_selling(const Env& env, int idx);;

    // no tick, the expression is applicable everywhere
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_init_buying();
    virtual std::string get_el_init_selling();
    virtual std::string get_el_tick();

    virtual std::string get_mql_init_buying();
    virtual std::string get_mql_init_selling();
    virtual std::string get_mql_tick();
};



class TrailingStopLoss : public StopSignal
{
    double m_stop_price;
    double m_range_price;
    double m_size;

public:
    TrailingStopLoss(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "TRAILING_STOP");
	}

    virtual ParameterConstraintMap get_initial_constraints();
    virtual void init_codegen_variables();

    virtual void update_data(const DataMatrix& data);

    // init the stop according to the new position
    virtual void compute_init_buying(const Env& env, int idx);;
    virtual void compute_init_selling(const Env& env, int idx);;

    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_init_buying();
    virtual std::string get_el_init_selling();
    virtual std::string get_el_tick();

    virtual std::string get_mql_init_buying();
    virtual std::string get_mql_init_selling();
    virtual std::string get_mql_tick();
};


class PercentTakeProfit : public StopSignal
{
    double m_stop_price;
    double m_size;

public:

    PercentTakeProfit(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "TAKE_PROFIT");
	}

    virtual ParameterConstraintMap get_initial_constraints();
    virtual void init_codegen_variables();

    virtual void update_data(const DataMatrix& data);

    // init the stop according to the new position
    virtual void compute_init_buying(const Env& env, int idx);;
    virtual void compute_init_selling(const Env& env, int idx);;

    // no tick, the expression is applicable everywhere
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_init_buying();
    virtual std::string get_el_init_selling();
    virtual std::string get_el_tick();

    virtual std::string get_mql_init_buying();
    virtual std::string get_mql_init_selling();
    virtual std::string get_mql_tick();
};



#endif // stops_h__
