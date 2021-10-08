#ifndef gaps_h__
#define gaps_h__

#include "base_signal.h"

class GapSignal : public Signal 
{
public:
    // internal variables used
    const std::vector<DateTime> *dt;
    const std::vector<double> *data_before;
    const std::vector<double> *data_after;

    int gap_dir;
    double gap_size;
    double pr_bef;
    double pr_aft;
    double fpct; // price of the face_pct percents towards fading the gap
    int tbef;
    int tnow;

    int time_st, time_en;
    double min_size, max_size, price_before, price_after, fade_pct;
    bool in_percents;

    virtual void init_codegen_variables();
    virtual ParameterConstraintMap get_initial_constraints();

    virtual void update_data(const DataMatrix& data);
    virtual void compute_init(const Env& env, int idx);
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_init();
    virtual std::string get_el_tick();

    virtual std::string get_mql_init();
    virtual std::string get_mql_tick();
};

class GapUp : public GapSignal
{
public:

    GapUp(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "GAP_UP");
	}

    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class GapDown : public GapSignal
{
public:

    GapDown(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "GAP_DOWN");
	}

    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class FadeGapPercentPrice : public GapSignal
{
public:

    FadeGapPercentPrice(ParameterMap& ow, std::vector<Signal*> ch)
    {
		INIT_SIGNAL(RT_FLOAT, "FADE_GAP_PERCENT_PRICE");
    }

    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class GapSize : public GapSignal
{
public:

    GapSize(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_FLOAT, "GAP_SIZE");
	}

    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class GapPriceBefore : public GapSignal
{
public:

    GapPriceBefore(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_FLOAT, "GAP_PRICE_BEFORE");
	}

    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class GapPriceAfter : public GapSignal
{
public:

    GapPriceAfter(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_FLOAT, "GAP_PRICE_AFTER");
	}

    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};
#endif // gaps_h__
