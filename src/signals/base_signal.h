#ifndef BASE_SIGNAL_H
#define BASE_SIGNAL_H

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <queue>
#include <stack>
#include <boost/variant.hpp>
#include <boost/format.hpp>
#include <typeinfo>

#include "../dtime.h"
#include "../data.h"
#include "../backtest_run.h"
#include "../common.h"
#include "../sformat.h" // for code generation
#include "constraints.h"
#include "common_types.h"


using sformat::fmt;
using sformat::Dict;

//using namespace boost;

// forward
class Signal;

// This is incremented every time a new signal is created
extern unsigned long g_global_signal_id;
extern int g_global_price_preference;

#define PRICE_PREF_OPEN 0
#define PRICE_PREF_CLOSE 1

// Always use the variable names "ow" and "ch" in every derived signal's constructor!
#define INIT_SIGNAL(rtype, name) { m_return_type = rtype; m_name = name; \
	                               if (m_return_type == RT_INT) { m_outp = 0; } \
                                   if (m_return_type == RT_FLOAT) { m_outp = 0.0; } \
                                   if (m_return_type == RT_BOOL) { m_outp = false; } \
	                               if (GlobalConstraints.count(m_name) == 0) \
	                               { GlobalConstraints[m_name] = get_initial_constraints(); } \
	                               init_codegen_variables(); init_codegen_outp(); init_defaults(ow); \
	                               for(unsigned int i=0;i<ch.size();i++) \
                                   { m_children.push_back(ch[i]); }  \
                                   init_fd(); \
                                 }

// forward
class SignalTreeStrategy;
class SignalTreeMultiStrategy;

//  The Signal class
class Signal
{
public:

    /////////////// 
    // Attributes 

    // The unique ID. 
    unsigned long m_id;

    // Name of the string (or op representation)
    std::string m_name;

    // The current parameter values 
    // SP_type is different than SP in that it cannot hold ELIs.
    ExplicitParameterMap m_parameters;
    
    // The used ELIs
    std::map< std::string, ELI > m_ELIs;

    // The variables used by the signal
    // used for code generation
    std::map< std::string, VariableType > m_variables;

    // The return type of this signal (bool/int/float)
    NodeReturnType m_return_type;
    
    // the children
    std::vector<Signal*> m_children;

    // useful for code generation
    std::map<std::string, std::string> limits; 

    // Parent (container) strategy
    const SignalTreeStrategy* m_parent_strategy;
    const SignalTreeMultiStrategy* m_parent_multistrategy;

    // The output of this signal - a ReturnVariant variable
    ReturnVariant m_outp;

    // EL based default variable Dict
    Dict m_efd;
    // MQL based default variable Dict
    Dict m_mfd;

    // for better timekeeping
    int prev_idx;


    ////////////
    // Constructor
    Signal();
    // Destructor
    virtual ~Signal();

    ////////////
    // Methods

    // Populates m_parameters with the default values from the
    // parameter description/constraint database
    void init_defaults(ParameterMap& a_overridden);

    // Specifies what variables will be used for code generation
    virtual void init_codegen_variables()
    {};

    void init_codegen_outp();

    // Call this to init the codegen fd off the variables (after the variables are initialized)
    void init_fd()
    {
        for(auto it = m_variables.begin(); it != m_variables.end(); it++)
        {
            m_efd((*it).first, var((*it).first));
            m_mfd((*it).first, var((*it).first));
        }
        // also run through all parameters
        for(auto it = m_parameters.begin(); it != m_parameters.end(); it++)
        {
            m_efd((*it).first, var((*it).first));
            m_mfd((*it).first, var((*it).first));
        }

        int i=0;
        for(auto it = m_children.begin(); it != m_children.end(); it++)
        {
            std::stringstream s;
            s << format("%d") % i;
            m_efd((std::string)(s.str()), (*it)->get_el_output());
            m_mfd((std::string)(s.str()), (*it)->get_mql_output());
            i++;
        }

        m_efd("curtime", el_cur_time());
        m_mfd("curtime", mql_cur_time());

        m_efd("lmin", limits["min"]);
        m_mfd("lmax", limits["max"]);

        // the MQL needs begin/end delimiters
        m_mfd("bg","{");
        m_mfd("en","}");
    }
    
    void do_log(std::map<std::string, std::vector<ReturnVariant> > &a_logs)
    {
        // check if parameter named "log" exists
        if (m_parameters.find("log") != m_parameters.end())
        {
            // read in the parameter
            std::string logname = get<std::string>(m_parameters["log"]);
            // now check if there is a corresponding place in the map
            if (a_logs.find(logname) != a_logs.end())
            {
                // yes?
                // store it
                a_logs[logname].push_back(m_outp);
            }
            else
            {
                // no?
                // create and store it
                std::vector<ReturnVariant> o;
                a_logs[logname] = o;
                a_logs[logname].push_back(m_outp);
            }
        }
    }

    // Initializes the parameter description and default constraints for the first time
    // and returns them. Used to initialize the database at system startup.
    virtual ParameterConstraintMap get_initial_constraints();

    // Add new child
    void add_child(Signal* a_node);
    void clear_children();

    // Returns a list of pointers to all signals down the tree,
    // including this one
    std::vector<Signal*> flatten_tree();
    void fill_children_list(std::vector<Signal*>& a_list);

    // Randomizes the parameters, overwrites cparams
    void randomize_parameters();
    void mutate_parameters();

    // To play well with Python
    py::dict get_parameters(bool a_only_mutable);
    void set_parameters(py::dict a_parameters);

    // get/set a specific parameter's constraints
    py::dict get_parameter_constraints(std::string a_parameter_name);
    void set_parameter_constraints(std::string a_parameter_name,
    		                       py::dict a_parameter_constraints);

    // get/set all constraints,
    // these methods use dict of dicts, the outer dict's keys are the parameter names
    py::dict get_all_constraints();
    // of course, set only the ones specified in the given dict's keys
    void set_all_constraints(py::dict a_all_constraints);


    // Updates the data used
    virtual void update_data(const DataMatrix& data){};
    virtual void update_data_recursive(const DataMatrix& data);
    virtual void update_mdata_recursive(const MultiDataMatrix& mdata);
    
    void update_parent_strategy_recursive(SignalTreeStrategy *a_parent);
    
    void update_mparent_strategy_recursive(SignalTreeMultiStrategy *a_parent);
    void reset_parent();

    // Multiply all scalable parameters (in m_cparams)
    // by the time scale
    // This needs to be called after init_defaults() and before each update_data()
    // LEGACY
    //void time_scale(unsigned int a_timescale);
    
    virtual int get_max_bars_back();

    // Inits the signal
    virtual void compute_init(const Env& env, int idx){};
    
    virtual void compute_minit(const std::vector<Env> &env, const std::vector<int> &idx);
    virtual void compute_init_recursive(const Env& env, int idx);
    virtual void compute_minit_recursive(const std::vector<Env>& env,
    						   	   	   	 const std::vector<int>& idx);

    // Ticks the signal
    virtual void compute_tick(const Env& env, int idx){};
    
    virtual void compute_mtick(const std::vector<Env> &env, const std::vector<int> &idx);
    virtual void compute_tick_recursive(const Env& env, int idx);
    virtual void compute_mtick_recursive(const std::vector<Env>& env,
    						   	   	   	 const std::vector<int>& idx);

    ReturnVariant get_output()
    {
    	return m_outp;
    }

    // prints the parameters like "(start_time=12:30, min=0.03, bars_back=4)"
    std::string print_parameters();

    // prints recursively the TradeSys code like "TIME_IS(time=12:30)"
    // or UP(PRICE(bars_back=0, price='close'), bars_back=1, ... )
    virtual std::string print_str();

    //////////////////////////////////
    // Code Generation
    //////////////////////////////////
    // get variable name
    std::string var(std::string a_name);
    std::string var_name(std::string a_name);

    std::vector<ELI> get_ELIs();
    // fills a list of all ELIs down the tree
    void fill_ELI_list(std::vector<ELI>& a_eli_list);
    void clear_ELIs(); // it's recursive


    //////////////////////////////////
    // EasyLanguage Code Generation
    //////////////////////////////////

    // returns the compute_init_recursive equivalent EL code
    virtual std::string get_el_init();
    virtual std::string get_el_init_recursive();
    // returns the compute_tick_recursive equivalent EL code
    virtual std::string get_el_tick();
    virtual std::string get_el_tick_recursive();
    std::string get_el_output()
    {
    	return var("outp");
    }

    // get current time (as int)
    std::string el_cur_time();

    // get EasyLanguage variable declarations
    std::string get_el_var_decl();

    //////////////////////////////////
    // MQL4 Code Generation
    //////////////////////////////////

    // returns the compute_init_recursive equivalent EL code
    virtual std::string get_mql_init();
    virtual std::string get_mql_init_recursive();
    // returns the compute_tick_recursive equivalent EL code
    virtual std::string get_mql_tick();
    virtual std::string get_mql_tick_recursive();
    std::string get_mql_output()
    {
    	return var("outp");
    }

    // get current time (as int)
    std::string mql_cur_time();

    // get MQL4 variable declarations
    std::string get_mql_var_decl();
    
    void update_mdata(const MultiDataMatrix &mdata);
};


#endif // BASE_SIGNAL_H

#pragma clang diagnostic pop