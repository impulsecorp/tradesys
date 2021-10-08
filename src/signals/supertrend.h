#ifndef supertrend_h__
#define supertrend_h__

#include "base_signal.h"

#ifndef NOTALIB

class SuperTrendSignal : public Signal 
{
public:
    // internal variables used
    std::vector<double> atr;
    std::vector<double> amedprice;
	std::vector<double> close;

	// the parameters
	double atr_multiplier;
	int atr_period;
	int median_period;

	// variables
	double up, dn;
	int trend;
	double prev_up, prev_dn;
	int prev_trend;
	int flag, flagh;

    virtual ParameterConstraintMap get_initial_constraints();
    virtual void init_codegen_variables();

    virtual void update_data(const DataMatrix& data);
    virtual void compute_init(const Env& env, int idx);
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_init();
    virtual std::string get_el_tick();

    virtual std::string get_mql_init();
    virtual std::string get_mql_tick();
};

class SuperTrendUp : public SuperTrendSignal 
{
public:

	SuperTrendUp(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "SUPERTREND_UP");
	}

	virtual void compute_tick(const Env& env, int idx);
	virtual std::string get_el_tick();
	virtual std::string get_mql_tick();
};

class SuperTrendDown : public SuperTrendSignal 
{
public:

	SuperTrendDown(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "SUPERTREND_DOWN");
	}

	virtual void compute_tick(const Env& env, int idx);
	virtual std::string get_el_tick();
	virtual std::string get_mql_tick();
};

#endif

#endif // supertrend_h__
