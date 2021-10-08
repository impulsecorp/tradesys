#ifndef feedback_h__
#define feedback_h__

#include "base_signal.h"
#include "../signal_tree.h"

class EnteredToday : public Signal 
{
    // internal variables used
    bool et;
    PositionType prev_p; // previous position
    PositionType cur_p; // current position
    const std::vector<DateTime> *dt;

public:
    EnteredToday(ParameterMap& ow, std::vector<Signal*> ch)
	{
        INIT_SIGNAL(RT_BOOL, "ENTERED_TODAY");
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

class ExitedToday : public Signal 
{
    // internal variables used
    bool et;
    PositionType prev_p; // previous position
    PositionType cur_p; // current position
    const std::vector<DateTime> *dt;

public:
    ExitedToday(ParameterMap& ow, std::vector<Signal*> ch)
	{
        INIT_SIGNAL(RT_BOOL, "EXITED_TODAY");
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


// Bars since this first happened
class BarsSinceFirstEvent : public Signal 
{
    // internal variables used
    bool prev_p; // previous state
    bool cur_p; // current state
    int counter;
    unsigned int tbef;
    unsigned int tnow;
    bool counting;
    bool happened;

    const std::vector<DateTime>* dt;

public:
    BarsSinceFirstEvent(ParameterMap& ow, std::vector<Signal*> ch)
	{
        INIT_SIGNAL(RT_INT, "BARS_SINCE_FE");
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


// Bars since this last happened
class BarsSinceLastEvent : public Signal
{
    // internal variables used
    bool prev_p; // previous state
    bool cur_p; // current state
    int counter;
    unsigned int tbef;
    unsigned int tnow;
    bool counting;
    bool happened;
    
    const std::vector<DateTime> *dt;

public:
    BarsSinceLastEvent(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_INT, "BARS_SINCE_LE");
    }
    
    virtual void init_codegen_variables();
    
    virtual void update_data(const DataMatrix &data);
    
    virtual void compute_init(const Env &env, int idx);
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_init();
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_init();
    
    virtual std::string get_mql_tick();
};

class BarsSinceEntry : public Signal 
{
    // internal variables used
    PositionType prev_p; // previous position
    PositionType cur_p; // current position
    int counter;
    unsigned int tbef;
    unsigned int tnow;
    bool counting;

public:
    BarsSinceEntry(ParameterMap& ow, std::vector<Signal*> ch)
	{
        INIT_SIGNAL(RT_INT, "BARS_SINCE_ENTRY");
	}

    virtual void init_codegen_variables();

    virtual void compute_init(const Env& env, int idx);
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_init();
    virtual std::string get_el_tick();

    virtual std::string get_mql_init();
    virtual std::string get_mql_tick();
};

class BarsSinceExit : public Signal 
{
    // internal variables used
    PositionType prev_p; // previous position
    PositionType cur_p; // current position
    int counter;
    unsigned int tbef;
    unsigned int tnow;
    bool counting;

public:
    BarsSinceExit(ParameterMap& ow, std::vector<Signal*> ch)
	{
        INIT_SIGNAL(RT_INT, "BARS_SINCE_EXIT");
	}

    virtual void init_codegen_variables();

    virtual void compute_init(const Env& env, int idx);
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_init();
    virtual std::string get_el_tick();

    virtual std::string get_mql_init();
    virtual std::string get_mql_tick();
};

class DaysSinceEntry : public Signal 
{
    // internal variables used
    PositionType prev_p; // previous position
    PositionType cur_p; // current position
    int counter;
    unsigned int tbef;
    unsigned int tnow;
    bool counting;
    const std::vector<DateTime> *dt;

public:
    DaysSinceEntry(ParameterMap& ow, std::vector<Signal*> ch)
	{
        INIT_SIGNAL(RT_INT, "DAYS_SINCE_ENTRY");
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

class DaysSinceExit : public Signal 
{
    // internal variables used
    PositionType prev_p; // previous position
    PositionType cur_p; // current position
    int counter;
    unsigned int tbef;
    unsigned int tnow;
    bool counting;
    const std::vector<DateTime> *dt;

public:
    DaysSinceExit(ParameterMap& ow, std::vector<Signal*> ch)
	{
        INIT_SIGNAL(RT_INT, "DAYS_SINCE_EXIT");
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

class Buying : public Signal 
{
public:
    Buying(ParameterMap& ow, std::vector<Signal*> ch)
	{
        INIT_SIGNAL(RT_BOOL, "BUYING");
	}
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class Selling : public Signal 
{
public:
    Selling(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "SELLING");
	}
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class InsideMarket : public Signal 
{
public:
    InsideMarket(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "INSIDE_MARKET");
	}
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class OutsideMarket : public Signal 
{
public:
    OutsideMarket(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "OUTSIDE_MARKET");
	}
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


///////////////////////////////////////////////////////////////
// The following signals use "global" variables present in
// the parent strategy, or in the EL/MQL code - global variables

class StopLossHit : public Signal 
{
public:
    StopLossHit(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "STOP_LOSS_HIT");
	}
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class TakeProfitHit : public Signal 
{
public:
    TakeProfitHit(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_BOOL, "TAKE_PROFIT_HIT");
	}
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

// Returns -1 for stop loss, 0 for normal exit (ot exit not happened yet), 1 for take profit
class LastExitReason : public Signal 
{
public:
    LastExitReason(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_INT, "LAST_EXIT_REASON");
	}
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class CurrentProfit : public Signal 
{
public:
    CurrentProfit(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_FLOAT, "CURRENT_PROFIT");
	}
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class LastProfit : public Signal 
{
public:
    LastProfit(ParameterMap& ow, std::vector<Signal*> ch)
	{
		INIT_SIGNAL(RT_FLOAT, "LAST_PROFIT");
	}
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};




#endif // feedback_h__
