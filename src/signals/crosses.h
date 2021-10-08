#ifndef crosses_h__
#define crosses_h__

#include "base_signal.h"

class CrossSignal : public Signal
{
public:

    double n1_bef, n1_now;
    double n2_bef, n2_now;

    virtual void init_codegen_variables();
    
    virtual int get_max_bars_back()
    {
        int x = Signal::get_max_bars_back();
        if (x > 2)
        {
            return x;
        }
        else
        {
            return 2;
        }
    }

    virtual void compute_init(const Env& env, int idx);
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_init();
    virtual std::string get_el_tick();

    virtual std::string get_mql_init();
    virtual std::string get_mql_tick();
};

class Crosses : public CrossSignal
{
public:
    Crosses(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_BOOL, "CROSSES");

//        ASS((a->m_return_type == RT_FLOAT) || (a->m_return_type == RT_INT));
//        ASS((b->m_return_type == RT_FLOAT) || (b->m_return_type == RT_INT));
    }
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

// a crosses above b (hits it from the top)
class CrossesAbove : public CrossSignal
{
public:
    CrossesAbove(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_BOOL, "CROSSES_ABOVE")

//        ASS((a->m_return_type == RT_FLOAT) || (a->m_return_type == RT_INT));
//        ASS((b->m_return_type == RT_FLOAT) || (b->m_return_type == RT_INT));
    }
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

// a crosses below b (hits it from the bottom)
class CrossesBelow : public CrossSignal
{
public:
    CrossesBelow(ParameterMap& ow, std::vector<Signal*> ch)
    {
        INIT_SIGNAL(RT_BOOL, "CROSSES_BELOW")

//        ASS((a->m_return_type == RT_FLOAT) || (a->m_return_type == RT_INT));
//        ASS((b->m_return_type == RT_FLOAT) || (b->m_return_type == RT_INT));
    }
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

#endif // crosses_h__
