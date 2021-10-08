//
// Created by peter on 8/28/16.
//
#include "embedded.h"

class EmbeddedPythonSignal : public Signal
{
public:
    
    // internal variables used
    const std::vector<DateTime> *dt;
    
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

