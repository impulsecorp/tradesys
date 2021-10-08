#ifndef raw_h__
#define raw_h__

#include "base_signal.h"

enum
{
    BARS_BACK = 0, SINCE
};

class MovementSignal : public Signal
{
public:
    
    // internal variables used
    const std::vector<DateTime> *dt;
    std::vector<double> val_array;
    int bars_back;
    double min_movement;
    double max_movement;
    
    long time_since;
    long time_to;
    int mode;
    bool use_to;
    
    // for computation
    double value_before;
    double value_now;
    long tbef;
    long tnow;
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void init_codegen_variables();
    
    virtual void update_data(const DataMatrix &data);
    
    virtual void compute_init(const Env &env, int idx);
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_init();
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_init();
    
    virtual std::string get_mql_tick();
};

class Up : public MovementSignal
{
public:
    Up(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_BOOL, "UP");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};

class Down : public MovementSignal
{
public:
    Down(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_BOOL, "DOWN");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};

class PercentUp : public MovementSignal
{
public:
    PercentUp(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_BOOL, "P_UP");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};

class PercentDown : public MovementSignal
{
public:
    PercentDown(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_BOOL, "P_DOWN");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};

// (before price)
class MovementValBefore : public MovementSignal
{
public:
    MovementValBefore(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "MOVE_VAL_BEFORE");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};


// (now - before)
class MovementDiff : public MovementSignal
{
public:
    MovementDiff(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "MOVE_DIFF");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};

// (now - before)/before*100.0 
class MovementPercentDiff : public MovementSignal
{
public:
    MovementPercentDiff(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "MOVE_PDIFF");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};

class Price : public Signal
{
    // internal variables used
    int bb;
    std::vector<double> d;

public:
    Price(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "PRICE");
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void update_data(const DataMatrix &data);
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};


class ValueAtTime : public Signal
{
    // internal variables used
    int time;
    ReturnVariant at;
    int days_ago;
    const std::vector<DateTime> *dt;
    std::vector<ReturnVariant> next_at;
    long tbef;
    long tnow;
    bool reset_overnight;

public:
    ValueAtTime(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "VALUE_AT_TIME");
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void init_codegen_variables();
    
    virtual void update_data(const DataMatrix &data);
    
    virtual void compute_init(const Env &env, int idx);
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_init();
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_init();
    
    virtual std::string get_mql_tick();
};


class IfThenElse : public Signal
{
public:
    IfThenElse(ParameterMap &ow, std::vector<Signal *> ch)
    {
        if (ch.size() == 3)
        {
            if (ch[1]->m_return_type == ch[2]->m_return_type)
            {
                if (ch[1]->m_return_type == RT_INT)
                {
                    INIT_SIGNAL(RT_INT, "IFTHENELSE");
                }
                if (ch[1]->m_return_type == RT_BOOL)
                {
                    INIT_SIGNAL(RT_BOOL, "IFTHENELSE");
                }
                if (ch[1]->m_return_type == RT_FLOAT)
                {
                    INIT_SIGNAL(RT_FLOAT, "IFTHENELSE");
                }
            }
            else
            {
                throw std::runtime_error("IF_THEN_ELSE second 2 subtrees type should match");
            }
        }
        else
        {
            throw std::runtime_error("IF_THEN_ELSE needs 3 subtrees");
        }
    }
    
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};


// Returns the value of a sub-tree when the other sub-tree first evaluated to True
// gets reseted after timeout ticks/bars have passed
// if timeout is -1, doesn't reset
class ValueOfWhenFirstHappened : public Signal
{
    const std::vector<DateTime> *dt;
    ReturnVariant val;
    bool happened;
    int tcount;
    int timeout;

public:
    
    ValueOfWhenFirstHappened(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "VALUE_OF_FH");
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void init_codegen_variables();
    
    virtual void update_data(const DataMatrix &data);
    
    virtual void compute_init(const Env &env, int idx);
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_init();
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_init();
    
    virtual std::string get_mql_tick();
};


// Returns the value of a sub-tree when the other sub-tree last evaluated to True
// if timeout > 0 and timeout ticks/bars have passed, the value becomes undefined again
class ValueOfWhenLastHappened : public Signal
{
    const std::vector<DateTime> *dt;
    ReturnVariant val;
    bool happened;

public:
    
    ValueOfWhenLastHappened(ParameterMap &ow, std::vector<Signal *> ch)
    {
        if (ch.size() > 0)
        {
            INIT_SIGNAL(ch[0]->m_return_type, "VALUE_OF_LH");
        }
        else
        {
            INIT_SIGNAL(RT_FLOAT, "VALUE_OF_LH");
        }
    }
    
    void init_codegen_variables();
    
    virtual void compute_init(const Env &env, int idx);
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_init();
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_init();
    
    virtual std::string get_mql_tick();
};

// Returns the absolute value of a numeric sub-tree 
class AbsValueOf : public Signal
{
public:
    AbsValueOf(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "ABS");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};

// Returns the negated value of a numeric sub-tree
class Negative : public Signal
{
public:
    Negative(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "NEG");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};


// Returns the value of a sub-tree as it was before some bars
class ValueOf : public Signal
{
    unsigned int bars_back;
    std::vector<ReturnVariant> val_array;

public:
    ValueOf(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "VALUE_OF");
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void init_codegen_variables();
    
    virtual void update_data(const DataMatrix &data);
    
    virtual void compute_init(const Env &env, int idx);
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_init();
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_init();
    
    virtual std::string get_mql_tick();
};


// Returns the percent of a numeric value, i.e. 5% from a value of 200 is 10.
class PercentOfValue : public Signal
{
    double x;
public:
    PercentOfValue(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "PERCENT_OF");
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void update_data(const DataMatrix &data);
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};


// Returns the statistical value of a double signal for the last x bars
// Examples are max, min, sum, average, std dev.. 
class StatValueForPeriod : public Signal
{
public:
    unsigned int period;
    double min_v, max_v, avg_v, sum_v;
    int min_idx, max_idx;
    std::vector<double> val_array;
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void init_codegen_variables();
    
    virtual void update_data(const DataMatrix &data);
    
    virtual void compute_init(const Env &env, int idx);
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_init();
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_init();
    
    virtual std::string get_mql_tick();
};

class MinValueForPeriod : public StatValueForPeriod
{
public:
    MinValueForPeriod(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "MIN");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};

class MaxValueForPeriod : public StatValueForPeriod
{
public:
    MaxValueForPeriod(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "MAX");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};

class MinValueForPeriodIdx : public StatValueForPeriod
{
public:
    MinValueForPeriodIdx(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_INT, "MIN_IDX");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};

class MaxValueForPeriodIdx : public StatValueForPeriod
{
public:
    MaxValueForPeriodIdx(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_INT, "MAX_IDX");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};

class SumValueForPeriod : public StatValueForPeriod
{
public:
    SumValueForPeriod(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "SUM");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};

class AvgValueForPeriod : public StatValueForPeriod
{
public:
    AvgValueForPeriod(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "AVG");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};

// Returns the stat (min/max) value of a double signal 
// for time X - present or between times X and Y
class StatValueSince : public Signal
{
public:
    const std::vector<DateTime> *dt;
    
    // params
    unsigned int time_since;
    unsigned int time_to;
    bool use_to;
    
    // vars
    long tbef;
    // vars
    long tnow;
    bool working;
    double min_val, max_val, avg_val, sum_val, val;
    int bars_counted;
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void init_codegen_variables();
    
    virtual void update_data(const DataMatrix &data);
    
    virtual void compute_init(const Env &env, int idx);
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_init();
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_init();
    
    virtual std::string get_mql_tick();
};


class MinValueSince : public StatValueSince
{
public:
    MinValueSince(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "MIN_SINCE");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};


class MaxValueSince : public StatValueSince
{
public:
    MaxValueSince(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "MAX_SINCE");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};


class AvgValueSince : public StatValueSince
{
public:
    AvgValueSince(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "AVG_SINCE");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};


class SumValueSince : public StatValueSince
{
public:
    SumValueSince(ParameterMap &ow, std::vector<Signal *> ch)
    {
        INIT_SIGNAL(RT_FLOAT, "SUM_SINCE");
    }
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_tick();
};

class VariableSignal : public Signal
{
public:
    VariableSignal(ParameterMap &ow, std::vector<Signal *> ch)
    {
        if (ch.size() > 0)
        {
            if (ch[0]->m_return_type == RT_BOOL)
            {
                INIT_SIGNAL(RT_BOOL, "VAR");
            }
            if (ch[0]->m_return_type == RT_INT)
            {
                INIT_SIGNAL(RT_INT, "VAR");
            }
            if (ch[0]->m_return_type == RT_FLOAT)
            {
                INIT_SIGNAL(RT_FLOAT, "VAR");
            }
        }
        else
        {
            INIT_SIGNAL(RT_FLOAT, "VAR");
        }
    }
    
    virtual void compute_tick(const Env &env, int idx)
    {
        if (m_children.size() > 0)
        {
            m_outp = m_children[0]->get_output();
        }
        else
        {
            m_outp = 0.0;
        }
    }
    
    virtual std::string get_el_init()
    {
        return Signal::get_el_init();
    }
    
    virtual std::string get_el_tick()
    {
        return Signal::get_el_tick();
    }
    
    virtual std::string get_mql_init()
    {
        return Signal::get_mql_init();
    }
    
    virtual std::string get_mql_tick()
    {
        return Signal::get_mql_tick();
    }
};


class SortedSignal : public Signal
{
public:
    int idx;
    std::string dir;
    
    // vars
    double temp;
    std::vector<double> arr;
    
    SortedSignal(ParameterMap &ow, std::vector<Signal *> ch)
    {
        // TODO: make sure all children are float, and extend to int
        INIT_SIGNAL(RT_FLOAT, "SORTED");
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void init_codegen_variables();
    
    virtual void update_data(const DataMatrix &data);
    
    virtual void compute_init(const Env &env, int idx);
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_init();
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_init();
    
    virtual std::string get_mql_tick();
};

class SortedIdxSignal : public Signal
{
public:
    int idx;
    std::string dir;
    
    // vars
    double temp;
    int temp_int;
    std::vector<double> arr;
    std::vector<int> arridx;
    double rd;
    
    SortedIdxSignal(ParameterMap &ow, std::vector<Signal *> ch)
    {
        // TODO: make sure all children are float, and extend to int
        INIT_SIGNAL(RT_INT, "SORTED_IDX");
    }
    
    virtual ParameterConstraintMap get_initial_constraints();
    
    virtual void init_codegen_variables();
    
    virtual void update_data(const DataMatrix &data);
    
    virtual void compute_init(const Env &env, int idx);
    
    virtual void compute_tick(const Env &env, int idx);
    
    virtual std::string get_el_init();
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_init();
    
    virtual std::string get_mql_tick();
};

class ProbabilisticSignal : public Signal
{
public:
    double prob;
    
    ProbabilisticSignal(ParameterMap &ow, std::vector<Signal *> ch)
    {
        // TODO: make sure all children are float, and extend to int
        INIT_SIGNAL(RT_BOOL, "PROB");
    }
    
    virtual ParameterConstraintMap get_initial_constraints()
    {
        ParameterConstraintMap t_constraints;
        
        SP_constraints t_prob;
        t_prob.usage_mode = CNT_MINMAX;
        t_prob.min = 0.0;
        t_prob.max = 1.0;
        t_prob.mut_min = 0.0;
        t_prob.mut_max = 0.01;
        t_prob.mut_power = 0.001;
        t_prob.is_mutable = true;
        t_prob.default_value = 1.0;
        t_constraints["prob"] = t_prob;
        
        return t_constraints;
    }
    
    virtual void update_data(const DataMatrix &data)
    {
        prob = convert_to_float(m_parameters["prob"]);
    }
    
    virtual void compute_init(const Env &env, int idx)
    {
        m_outp = false;
        srand(this->m_id * time(0));
    }
    
    virtual void compute_tick(const Env &env, int idx)
    {
        m_outp = ((double) rand() / (double) RAND_MAX) < prob;
    }
    
    /*virtual std::string get_el_init();
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_init();
    
    virtual std::string get_mql_tick();*/
};

class CurrentBarSignal : public Signal
{
public:
    int count;
    
    CurrentBarSignal(ParameterMap &ow, std::vector<Signal *> ch)
    {
        // TODO: make sure all children are float, and extend to int
        INIT_SIGNAL(RT_INT, "CURRENT_BAR");
    }
    
    virtual void compute_init(const Env &env, int idx)
    {
        count = 0;
    }
    
    virtual void compute_tick(const Env &env, int idx)
    {
        count++;
        m_outp = count;
    }
    
    /*virtual std::string get_el_init();
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_init();
    
    virtual std::string get_mql_tick();*/
};



class NNSignal : public Signal
{
public:
    //double prob;
    DataMatrix m_data;
    int mbb, winlen;
    double cl;
    std::string modelname;
    std::string nnname;
    
    NNSignal(ParameterMap &ow, std::vector<Signal *> ch)
    {
        // TODO: make sure all children are float, and extend to int
        INIT_SIGNAL(RT_INT, "NNSIGNAL");
    }
    
    virtual ParameterConstraintMap get_initial_constraints()
    {
        ParameterConstraintMap t_constraints;
        
        SP_constraints t_mbb;
        t_mbb.usage_mode = CNT_MINMAX;
        t_mbb.min = 0;
        t_mbb.max = 1000000;
        t_mbb.mut_min = 0;
        t_mbb.mut_max = 120;
        t_mbb.mut_power = 20;
        t_mbb.is_mutable = true;
        t_mbb.default_value = 50;
        t_constraints["mbb"] = t_mbb;
        
        SP_constraints t_winlen;
        t_winlen.usage_mode = CNT_MINMAX;
        t_winlen.min = 0;
        t_winlen.max = 1000000;
        t_winlen.mut_min = 0;
        t_winlen.mut_max = 120;
        t_winlen.mut_power = 20;
        t_winlen.is_mutable = true;
        t_winlen.default_value = 30;
        t_constraints["winlen"] = t_winlen;
    
        SP_constraints t_cl;
        t_cl.usage_mode = CNT_MINMAX;
        t_cl.min = 0.0;
        t_cl.max = 100.0;
        t_cl.mut_min = 0.0;
        t_cl.mut_max = 0.01;
        t_cl.mut_power = 0.001;
        t_cl.is_mutable = true;
        t_cl.default_value = 0.0;
        t_constraints["cl"] = t_cl;
        
        SP_constraints t_modelname;
        t_modelname.usage_mode = CNT_POSSIBLE_VALUES;
        t_modelname.possible_values.push_back(SP_type(STR("model")));
        t_modelname.is_mutable = false;
        t_modelname.default_value = STR("model");
        t_constraints["modelname"] = t_modelname;
        
        SP_constraints t_nnname;
        t_nnname.usage_mode = CNT_POSSIBLE_VALUES;
        t_nnname.possible_values.push_back(SP_type(STR("default")));
        t_nnname.is_mutable = false;
        t_nnname.default_value = STR("default");
        t_constraints["modeltype"] = t_nnname;
        
        return t_constraints;
    }
    
    virtual void update_data(const DataMatrix &data)
    {
        m_data = data;
        
        mbb = convert_to_int(m_parameters["mbb"]);
        winlen = convert_to_int(m_parameters["winlen"]);
        cl = convert_to_float(m_parameters["cl"]);
        modelname = get<std::string>(m_parameters["modelname"]);
        nnname = get<std::string>(m_parameters["modeltype"]);
    }
    
    virtual void compute_init(const Env &env, int idx)
    {
        m_outp = 0;
        
        Py_Initialize();
        
        py::object main_module = py::import("__main__");
        py::object main_namespace = main_module.attr("__dict__");
        
        std::stringstream s;
        
        s << format("import sys\n"
                    "sys.path.insert(0, '/home/peter/Desktop/VBoxShare/model_n')\n"
                    "import nn\n"
                    "from importlib import reload\n"
                    "nn = reload(nn)\n"
                    "nn.init_module(%d, %d, %3.14f, modelname='%s', modeltype='%s', dolog=False)\n") % mbb % winlen % cl % modelname.c_str() % nnname.c_str();
        
        py::object ignored = py::exec(s.str().c_str(), main_namespace);
    }
    
    virtual void compute_tick(const Env &env, int idx)
    {
        if (idx > mbb)
        {
            py::object main_module = py::import("__main__");
            py::object main_namespace = main_module.attr("__dict__");
            
            for (int i = 0; i < mbb; i++)
            {
                std::stringstream s;
                
                s << format("nn.store_data_idx(%d, %3.16f, %3.16f, %3.16f, %3.16f, %d)\n")
                     % i % m_data.open[idx - i] % m_data.high[idx - i] % m_data.low[idx - i] % m_data.close[idx - i] %
                     m_data.volume[idx - i];
                
                py::object ignored = py::exec(s.str().c_str(), main_namespace);
            }
            
            py::object ignored = py::exec("nn.tick()\n"
                                          "x = nn.get_response()\n", main_namespace);
            
            m_outp = py::extract<int>(main_namespace["x"]);

            // determine if this is the last tick
            // if so, write log
            /*if (idx == (m_data.len()-1))
            {
                py::object ignored = py::exec("nn.write_log()\n", main_namespace);
            }*/
        }
        else
        {
            m_outp = 0;
        }
    }
    
    /*virtual std::string get_el_init();
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_init();
    
    virtual std::string get_mql_tick();*/
    
};



class FMLSignal : public Signal
{
public:
    //double prob;
    DataMatrix m_data;
    int mbb, winlen;
    std::string modelname;
    std::string datamodel;
    std::string modeltype;
    std::string weights;
    bool dolog;
    bool noinput;
    
    FMLSignal(ParameterMap &ow, std::vector<Signal *> ch)
    {
        // TODO: make sure all children are float, and extend to int
        INIT_SIGNAL(RT_FLOAT, "FMLSIGNAL");
    }
    
    virtual ParameterConstraintMap get_initial_constraints()
    {
        ParameterConstraintMap t_constraints;
        
        SP_constraints t_mbb;
        t_mbb.usage_mode = CNT_MINMAX;
        t_mbb.min = 0;
        t_mbb.max = 1000000;
        t_mbb.mut_min = 0;
        t_mbb.mut_max = 120;
        t_mbb.mut_power = 20;
        t_mbb.is_mutable = true;
        t_mbb.default_value = 50;
        t_constraints["mbb"] = t_mbb;
        
        SP_constraints t_winlen;
        t_winlen.usage_mode = CNT_MINMAX;
        t_winlen.min = 0;
        t_winlen.max = 1000000;
        t_winlen.mut_min = 0;
        t_winlen.mut_max = 120;
        t_winlen.mut_power = 20;
        t_winlen.is_mutable = true;
        t_winlen.default_value = 30;
        t_constraints["winlen"] = t_winlen;
        
        SP_constraints t_modelname;
        t_modelname.usage_mode = CNT_POSSIBLE_VALUES;
        t_modelname.possible_values.push_back(SP_type(STR("model")));
        t_modelname.is_mutable = false;
        t_modelname.default_value = STR("model");
        t_constraints["modelname"] = t_modelname;
    
        SP_constraints t_datamodel;
        t_datamodel.usage_mode = CNT_POSSIBLE_VALUES;
        t_datamodel.possible_values.push_back(SP_type(STR("dxdxdx")));
        t_datamodel.is_mutable = false;
        t_datamodel.default_value = STR("dxdxdx");
        t_constraints["datamodel"] = t_datamodel;

        SP_constraints t_modeltype;
        t_modeltype.usage_mode = CNT_POSSIBLE_VALUES;
        t_modeltype.possible_values.push_back(SP_type(STR("default")));
        t_modeltype.is_mutable = false;
        t_modeltype.default_value = STR("default");
        t_constraints["modeltype"] = t_modeltype;
    
        SP_constraints t_weights;
        t_weights.usage_mode = CNT_POSSIBLE_VALUES;
        t_weights.possible_values.push_back(SP_type(STR("default")));
        t_weights.is_mutable = false;
        t_weights.default_value = STR("");
        t_constraints["weights"] = t_weights;
    
        SP_constraints t_dolog;
        t_dolog.usage_mode = CNT_POSSIBLE_VALUES;
        t_dolog.possible_values.push_back(SP_type(true));
        t_dolog.possible_values.push_back(SP_type(false));
        t_dolog.is_mutable = false;
        t_dolog.default_value = false;
        t_constraints["dolog"] = t_dolog;
    
        SP_constraints t_noinput;
        t_noinput.usage_mode = CNT_POSSIBLE_VALUES;
        t_noinput.possible_values.push_back(SP_type(true));
        t_noinput.possible_values.push_back(SP_type(false));
        t_noinput.is_mutable = false;
        t_noinput.default_value = false;
        t_constraints["noinput"] = t_noinput;
        
        return t_constraints;
    }
    
    virtual void update_data(const DataMatrix &data)
    {
        m_data = data;
        
        mbb = convert_to_int(m_parameters["mbb"]);
        winlen = convert_to_int(m_parameters["winlen"]);
        modelname = get<std::string>(m_parameters["modelname"]);
        datamodel = get<std::string>(m_parameters["datamodel"]);
        modeltype = get<std::string>(m_parameters["modeltype"]);
        weights = get<std::string>(m_parameters["weights"]);
        dolog = get<bool>(m_parameters["dolog"]);
        noinput = get<bool>(m_parameters["noinput"]);
    }
    
    virtual void compute_init(const Env &env, int idx)
    {
        m_outp = 0.0;
        
        Py_Initialize();
        
        py::object main_module = py::import("__main__");
        py::object main_namespace = main_module.attr("__dict__");
        py::object ignored;
        
        std::stringstream s;
        
        // self-reference here
        s << "import os, sys\n"
             "sys.path.insert(0, '/home/peter/code/projects/tradesys')\n"
             "sys.path.append('C:/tradesys')\n";
             //"from tradesys.ml import BaseMLModel, BaseDataModel\n"
             //"print('CWD:',os.getcwd())\n";
        ignored = py::exec(s.str().c_str(), main_namespace);
        
        s.clear();
        
        std::string dl;
        if (dolog)
        {
            dl = "True";
        }
        else
        {
            dl = "False";
        }
        
        s << format("from fmlmodel import MLModel as FMLModel\n"
                    //"except: from tradesys.ml import MLModel\n"
                
                    "from %s import DataModel as dmodel\n"
                    //"except: dmodel = BaseDataModel\n"
                
                    //"from importlib import reload\n"
                    //"mlmodel = reload(mlmodel)\n"
                
                    "fmodel = FMLModel()\n"
                    "fmodel.init_module(%d, %d, modelname='%s', datamodel=dmodel, modeltype='%s', weights='%s', dolog=%s)\n")
             % datamodel.c_str() % mbb % winlen % modelname.c_str() % modeltype.c_str() % weights.c_str() % dl.c_str();
        
        ignored = py::exec(s.str().c_str(), main_namespace);
    }
    
    virtual void compute_tick(const Env &env, int idx)
    {
        if (idx > mbb)
        {
            py::object main_module = py::import("__main__");
            py::object main_namespace = main_module.attr("__dict__");
            
            if (!noinput)
            {
                for (int i = 0; i < mbb; i++)
                {
                    std::stringstream s;
        
                    s <<
                      format("fmodel.store_data_idx(%d, %3.16f, %3.16f, %3.16f, %3.16f, %d, %d, %d, %d, %d, %d, %d)\n")
                      % i % m_data.open[idx - i] % m_data.high[idx - i] % m_data.low[idx - i] % m_data.close[idx - i] %
                      m_data.volume[idx - i]
                      % m_data.dt[idx - i].year
                      % m_data.dt[idx - i].month
                      % m_data.dt[idx - i].day
                      % m_data.dt[idx - i].hour
                      % m_data.dt[idx - i].minute
                      % m_data.dt[idx - i].second;
        
                    py::object ignored = py::exec(s.str().c_str(), main_namespace);
                }
            }
            
            py::object ignored = py::exec("fmodel.tick()\n"
                                          "fx = fmodel.get_response()\n", main_namespace);
            
            m_outp = py::extract<double>(main_namespace["fx"]);

            // determine if this is the last tick
            // if so, write log
            if (idx == (m_data.len()-1))
            {
                py::object ignored = py::exec("fmodel.write_log()\n", main_namespace);
            }
        }
        else
        {
            m_outp = 0.0;
        }
    }
    
    ~FMLSignal()
    {
    }
    
    /*virtual std::string get_el_init();
    
    virtual std::string get_el_tick();
    
    virtual std::string get_mql_init();
    
    virtual std::string get_mql_tick();*/
    
};





class MLSignal : public Signal
{
public:
    //double prob;
    DataMatrix m_data;
    int mbb, winlen;
    std::string modelname;
    std::string datamodel;
    std::string modeltype;
    std::string weights;
    bool dolog;
    bool noinput;

    MLSignal(ParameterMap &ow, std::vector<Signal *> ch)
    {
        // TODO: make sure all children are float, and extend to int
        INIT_SIGNAL(RT_INT, "MLSIGNAL");
    }

    virtual ParameterConstraintMap get_initial_constraints()
    {
        ParameterConstraintMap t_constraints;

        SP_constraints t_mbb;
        t_mbb.usage_mode = CNT_MINMAX;
        t_mbb.min = 0;
        t_mbb.max = 1000000;
        t_mbb.mut_min = 0;
        t_mbb.mut_max = 120;
        t_mbb.mut_power = 20;
        t_mbb.is_mutable = true;
        t_mbb.default_value = 50;
        t_constraints["mbb"] = t_mbb;

        SP_constraints t_winlen;
        t_winlen.usage_mode = CNT_MINMAX;
        t_winlen.min = 0;
        t_winlen.max = 1000000;
        t_winlen.mut_min = 0;
        t_winlen.mut_max = 120;
        t_winlen.mut_power = 20;
        t_winlen.is_mutable = true;
        t_winlen.default_value = 30;
        t_constraints["winlen"] = t_winlen;

        SP_constraints t_modelname;
        t_modelname.usage_mode = CNT_POSSIBLE_VALUES;
        t_modelname.possible_values.push_back(SP_type(STR("model")));
        t_modelname.is_mutable = false;
        t_modelname.default_value = STR("model");
        t_constraints["modelname"] = t_modelname;

        SP_constraints t_datamodel;
        t_datamodel.usage_mode = CNT_POSSIBLE_VALUES;
        t_datamodel.possible_values.push_back(SP_type(STR("dxdxdx")));
        t_datamodel.is_mutable = false;
        t_datamodel.default_value = STR("dxdxdx");
        t_constraints["datamodel"] = t_datamodel;

        SP_constraints t_modeltype;
        t_modeltype.usage_mode = CNT_POSSIBLE_VALUES;
        t_modeltype.possible_values.push_back(SP_type(STR("default")));
        t_modeltype.is_mutable = false;
        t_modeltype.default_value = STR("default");
        t_constraints["modeltype"] = t_modeltype;

        SP_constraints t_weights;
        t_weights.usage_mode = CNT_POSSIBLE_VALUES;
        t_weights.possible_values.push_back(SP_type(STR("default")));
        t_weights.is_mutable = false;
        t_weights.default_value = STR("");
        t_constraints["weights"] = t_weights;

        SP_constraints t_dolog;
        t_dolog.usage_mode = CNT_POSSIBLE_VALUES;
        t_dolog.possible_values.push_back(SP_type(true));
        t_dolog.possible_values.push_back(SP_type(false));
        t_dolog.is_mutable = false;
        t_dolog.default_value = false;
        t_constraints["dolog"] = t_dolog;

        SP_constraints t_noinput;
        t_noinput.usage_mode = CNT_POSSIBLE_VALUES;
        t_noinput.possible_values.push_back(SP_type(true));
        t_noinput.possible_values.push_back(SP_type(false));
        t_noinput.is_mutable = false;
        t_noinput.default_value = false;
        t_constraints["noinput"] = t_noinput;

        return t_constraints;
    }

    virtual void update_data(const DataMatrix &data)
    {
        m_data = data;

        mbb = convert_to_int(m_parameters["mbb"]);
        winlen = convert_to_int(m_parameters["winlen"]);
        modelname = get<std::string>(m_parameters["modelname"]);
        datamodel = get<std::string>(m_parameters["datamodel"]);
        modeltype = get<std::string>(m_parameters["modeltype"]);
        weights = get<std::string>(m_parameters["weights"]);
        dolog = get<bool>(m_parameters["dolog"]);
        noinput = get<bool>(m_parameters["noinput"]);
    }

    virtual void compute_init(const Env &env, int idx)
    {
        m_outp = 0;

        Py_Initialize();

        py::object main_module = py::import("__main__");
        py::object main_namespace = main_module.attr("__dict__");
        py::object ignored;

        std::stringstream s;

        // self-reference here
        s << "import os, sys\n"
                "sys.path.insert(0, '/home/peter/code/projects/tradesys')\n"
                "sys.path.append('C:/tradesys')\n";
        //"from tradesys.ml import BaseMLModel, BaseDataModel\n"
        //"print('CWD:',os.getcwd())\n";
        ignored = py::exec(s.str().c_str(), main_namespace);

        s.clear();

        std::string dl;
        if (dolog)
        {
            dl = "True";
        }
        else
        {
            dl = "False";
        }

        s << format("from mlmodel import MLModel\n"
                            //"except: from tradesys.ml import MLModel\n"

                            "from %s import DataModel as dmodel\n"
                            //"except: dmodel = BaseDataModel\n"

                            //"from importlib import reload\n"
                            //"mlmodel = reload(mlmodel)\n"

                            "model = MLModel()\n"
                            "model.init_module(%d, %d, modelname='%s', datamodel=dmodel, modeltype='%s', weights='%s', dolog=%s)\n")
             % datamodel.c_str() % mbb % winlen % modelname.c_str() % modeltype.c_str() % weights.c_str() % dl.c_str();

        ignored = py::exec(s.str().c_str(), main_namespace);
    }

    virtual void compute_tick(const Env &env, int idx)
    {
        if (idx > mbb)
        {
            py::object main_module = py::import("__main__");
            py::object main_namespace = main_module.attr("__dict__");

            if (!noinput)
            {
                for (int i = 0; i < mbb; i++)
                {
                    std::stringstream s;

                    s <<
                      format("model.store_data_idx(%d, %3.16f, %3.16f, %3.16f, %3.16f, %d, %d, %d, %d, %d, %d, %d)\n")
                      % i % m_data.open[idx - i] % m_data.high[idx - i] % m_data.low[idx - i] % m_data.close[idx - i] %
                      m_data.volume[idx - i]
                      % m_data.dt[idx - i].year
                      % m_data.dt[idx - i].month
                      % m_data.dt[idx - i].day
                      % m_data.dt[idx - i].hour
                      % m_data.dt[idx - i].minute
                      % m_data.dt[idx - i].second;

                    py::object ignored = py::exec(s.str().c_str(), main_namespace);
                }
            }

            py::object ignored = py::exec("model.tick()\n"
                                                  "x = model.get_response()\n", main_namespace);

            m_outp = py::extract<int>(main_namespace["x"]);

            // determine if this is the last tick
            // if so, write log
            if (idx == (m_data.len()-1))
            {
                py::object ignored = py::exec("model.write_log()\n", main_namespace);
            }
        }
        else
        {
            m_outp = 0;
        }
    }

    ~MLSignal()
    {
    }

    /*virtual std::string get_el_init();

    virtual std::string get_el_tick();

    virtual std::string get_mql_init();

    virtual std::string get_mql_tick();*/

};




#endif // raw_h__
