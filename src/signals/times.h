#ifndef TIMES_H
#define TIMES_H

#include "base_signal.h"

class TimeIs : public Signal 
{
    // internal variables used
    const std::vector<DateTime> *dt;
    int before;
    int now;
    int time;

public:
    TimeIs(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "TIME_IS");
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


class TimeIsBefore : public Signal 
{
    // internal variables used
    const std::vector<DateTime> *dt;
    int time;

public:
    TimeIsBefore(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "TIME_IS_BEFORE");
	}

    virtual ParameterConstraintMap get_initial_constraints();

    virtual void init_codegen_variables();

    virtual void update_data(const DataMatrix& data);
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class TimeIsAfter : public Signal 
{
    // internal variables used
    const std::vector<DateTime> *dt;
    int time;

public:
    TimeIsAfter(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "TIME_IS_AFTER");
	}
    virtual ParameterConstraintMap get_initial_constraints();
    virtual void init_codegen_variables();

    virtual void update_data(const DataMatrix& data);
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class TimeIsBetween : public Signal 
{
    // internal variables used
    const std::vector<DateTime> *dt;
    int st, et;

public:
    TimeIsBetween(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "TIME_IS_BETWEEN");
	}
    virtual ParameterConstraintMap get_initial_constraints();

    virtual void update_data(const DataMatrix& data);
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class NewDay : public Signal
{
    // internal variables used
    const std::vector<DateTime> *dt;

public:
    NewDay(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "NEW_DAY");
	};

    virtual void update_data(const DataMatrix& data);
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};



#endif // TIMES_H
