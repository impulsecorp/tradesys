#ifndef timesinarow_h__
#define timesinarow_h__

#include "base_signal.h"

// Returns the number of times in a row the bool subtree was true. 
class TimesInRow : public Signal
{
public:
    int count;

    TimesInRow(ParameterMap& ow, std::vector<Signal*> ch);

    virtual void init_codegen_variables();

    virtual void compute_init(const Env& env, int idx);
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_init();
    virtual std::string get_el_tick();

    virtual std::string get_mql_init();
    virtual std::string get_mql_tick();
};

// Returns the number of times the bool subtree was true.
class TimesHappened : public Signal
{
public:
    int count;

    TimesHappened(ParameterMap& ow, std::vector<Signal*> ch)
    {
		INIT_SIGNAL(RT_INT, "TIMES_HAPPENED");
    }

    virtual void init_codegen_variables();

    virtual void compute_init(const Env& env, int idx);
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_init();
    virtual std::string get_el_tick();

    virtual std::string get_mql_init();
    virtual std::string get_mql_tick();
};

// Returns the number of times the first bool subtree was true, since the last time the second bool subtree was true
class TimesHappenedSinceLH : public Signal
{
public:
    int count;

    TimesHappenedSinceLH(ParameterMap& ow, std::vector<Signal*> ch)
    {
		INIT_SIGNAL(RT_INT, "TIMES_HAPPENED_SINCE_LH");
    }

    virtual void init_codegen_variables();

    virtual void compute_init(const Env& env, int idx);
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_init();
    virtual std::string get_el_tick();

    virtual std::string get_mql_init();
    virtual std::string get_mql_tick();
};

// Returns the number of times the first bool subtree was true, since the first time (in a sequence) the second bool subtree was true
class TimesHappenedSinceFH : public Signal
{
public:
    int count;
    bool prev;

    TimesHappenedSinceFH(ParameterMap& ow, std::vector<Signal*> ch)
    {
		INIT_SIGNAL(RT_INT, "TIMES_HAPPENED_SINCE_FH");
    }

    virtual void init_codegen_variables();

    virtual void compute_init(const Env& env, int idx);
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_init();
    virtual std::string get_el_tick();

    virtual std::string get_mql_init();
    virtual std::string get_mql_tick();
};


// When something was true at least N times in a row and is now false.
class StreakEnded : public Signal
{
public:
    int count;
    int prev_count;
    bool ended;
    int n;

    StreakEnded(ParameterMap& ow, std::vector<Signal*> ch)
    {
		INIT_SIGNAL(RT_BOOL, "STREAK_ENDED");
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

// When something was true at least N times in a row and is now false.
// Return how much times it happened.
class StreakLength : public StreakEnded
{
public:
    StreakLength(ParameterMap &ow1, std::vector<Signal *> ch1, ParameterMap &ow, std::vector<Signal *> ch)
            : StreakEnded(ow1, ch1)
    {
		INIT_SIGNAL(RT_INT, "STREAK_LENGTH");
	}
    virtual void compute_tick(const Env& env, int idx);
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


// When something was true at least N times in a row and is now false,
// get the price difference from the start to the end. 
// todo: refactor this to become more general, like the value difference or something. 
class StreakEndedPriceDiff : public Signal
{
public:
    const std::vector<double>* cl;

    int count;
    int prev_count;
    bool ended;

    double price_bef;
    double price_now;
    double diff;
    double prev_price;

    int n;

    StreakEndedPriceDiff(ParameterMap& ow, std::vector<Signal*> ch)
    {
		INIT_SIGNAL(RT_FLOAT, "STREAK_ENDED_PRICE_DIFF");
    }

    ParameterConstraintMap get_initial_constraints();
    void init_codegen_variables();

    virtual void update_data(const DataMatrix& data);
    virtual void compute_init(const Env& env, int idx);
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_init();
    virtual std::string get_el_tick();

    virtual std::string get_mql_init();
    virtual std::string get_mql_tick();
};


#endif // timesinarow_h__
