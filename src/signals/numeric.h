#ifndef numeric_h__
#define numeric_h__

#include "base_signal.h"

class IntConst : public Signal 
{
    // internal variables used
    int val;

public:
    IntConst(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_INT, "INT");
	}
    virtual ParameterConstraintMap get_initial_constraints();
    void update_data( const DataMatrix& data );
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class FloatConst : public Signal 
{
    // internal variables used
    double val;

public:
    FloatConst(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_FLOAT, "FLOAT");
	}
    virtual ParameterConstraintMap get_initial_constraints();
    void update_data( const DataMatrix& data );
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class BoolConst : public Signal 
{
    // internal variables used
    bool val;

public:
    BoolConst(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "BOOL");
	}
    virtual ParameterConstraintMap get_initial_constraints();
    void update_data( const DataMatrix& data );
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

#endif // numeric_h__
