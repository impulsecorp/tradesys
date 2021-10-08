#ifndef happened_h__
#define happened_h__

#include "base_signal.h"

class HappenedToday : public Signal
{
    const std::vector<DateTime> *dt;
    bool ht;

public:

    HappenedToday(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "HAPPENED_TODAY");
	}

    virtual void init_codegen_variables();

    virtual void update_data(const DataMatrix& data);
    virtual void compute_init(const Env& env, int idx);
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_init();
    virtual std::string get_el_tick();

    virtual std::string get_mql_init();
    virtual std::string get_mql_tick();
};


class HappenedAtTimeSignal : public Signal
{
public:

    const std::vector<DateTime> *dt;
    bool ht;
    int tbef, tnow, time;

    virtual void init_codegen_variables();
    virtual ParameterConstraintMap get_initial_constraints();

    virtual void update_data(const DataMatrix& data);
    virtual void compute_init(const Env& env, int idx);

    virtual std::string get_el_init();
    //virtual std::string get_el_tick();

    virtual std::string get_mql_init();
    //virtual std::string get_mql_tick();
};

// subtree was true at this time
class HappenedAtTime : public HappenedAtTimeSignal
{
public:
    HappenedAtTime(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "HAPPENED_AT_TIME");
	}
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

// subtree was true before this time
class HappenedBeforeTime : public HappenedAtTimeSignal
{
public:
    HappenedBeforeTime(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "HAPPENED_BEFORE_TIME");
	}
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

// subtree was true after this time
class HappenedAfterTime : public HappenedAtTimeSignal
{
public:
    HappenedAfterTime(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "HAPPENED_AFTER_TIME");
	}
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

// subtree was true between start/end time
class HappenedInTimePeriod : public Signal
{
public:

    const std::vector<DateTime> *dt;
    bool ht;
    int tbef, tnow, start_time, end_time;

    HappenedInTimePeriod(ParameterMap& ow, std::vector<Signal*> ch)
    {
		INIT_SIGNAL(RT_BOOL, "HAPPENED_IN_TIME_PERIOD");
    }

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

// subtree was true this many bars back
class HappenedBarsBack : public Signal
{
public:
    std::vector<bool> val_array;
    int bars_back;

    HappenedBarsBack(ParameterMap& ow, std::vector<Signal*> ch)
    {
		INIT_SIGNAL(RT_BOOL, "HAPPENED_BB");
    }

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

// subtree was true anywhere in the last bars 
class HappenedLastBars : public Signal
{
public:
    std::vector<bool> val_array;
    int bars_back;
    bool hp;

    HappenedLastBars(ParameterMap& ow, std::vector<Signal*> ch)
    {
		INIT_SIGNAL(RT_BOOL, "HAPPENED_LB");
    }

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



// Subtrees light up in sequence (size 2)
class Seqn : public Signal
{
    bool ht1, ht2;
    bool happened;
    int timeout;
    int timeout_counter;

public:

    // timeout_bars can be -1 for infinite
    Seqn(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "SEQN");
	}
    
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

#endif // happened_h__
