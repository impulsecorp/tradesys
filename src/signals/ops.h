#ifndef ops_h__
#define ops_h__

#include "base_signal.h"

/////////////////////////////////////
// Ops
/////////////////////////////////////

//////////////////
// Basic boolean

class OpSignal : public Signal 
{
public:
    virtual void init_op()
    {
    };
};

// Ops don't need to call init_defaults() or init_codegen_variables(),
// as they can't have any parameters

class AndOp : public OpSignal 
{
public:
    AndOp()
    {
        m_name = "AND";
    };
    virtual void init_op();
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string print_str();
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class OrOp : public OpSignal 
{
public:
    OrOp()
    {
        m_name = "OR";
    }
    virtual void init_op();
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string print_str();
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class NotOp : public OpSignal 
{
public:
    NotOp()
    {
        m_name = "NOT";
    }
    virtual void init_op();
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string print_str();
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

///////////////
// Arithmetic


class AddOp : public OpSignal 
{
public:
    AddOp()
    {
        m_name = "+";
    }
    virtual void init_op();
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string print_str();
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class SubOp : public OpSignal 
{
public:
    SubOp()
    {
        m_name = "-";
    }
    virtual void init_op();
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string print_str();
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class MulOp : public OpSignal 
{
public:
    MulOp()
    {
        m_name = "*";
    }
    virtual void init_op();
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string print_str();
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class DivOp : public OpSignal 
{
public:
    DivOp()
    {
        m_name = "/";
    }
    virtual void init_op();
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string print_str();
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class MmodOp : public OpSignal
{
public:
    MmodOp()
    {
        m_name = "%";
    }
    virtual void init_op();
    virtual void compute_tick(const Env& env, int idx);
    
    virtual std::string print_str();
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

///////////////
// Comparison

class EqOp : public OpSignal 
{
public:
    EqOp()
    {
        m_name = "==";
    }
    virtual void init_op();
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string print_str();
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class NeqOp : public OpSignal 
{
public:
    NeqOp()
    {
        m_name = "!=";
    }
    virtual void init_op();
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string print_str();
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class GrOp : public OpSignal 
{
public:
    GrOp()
    {
        m_name = ">";
    }
    virtual void init_op();
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string print_str();
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class GreOp : public OpSignal 
{
public:
    GreOp()
    {
        m_name = ">=";
    }
    virtual void init_op();
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string print_str();
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};

class LeOp : public OpSignal 
{
public:
    LeOp() 
    {
        m_name = "<";
    }
    virtual void init_op();
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string print_str();
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


class LeeOp : public OpSignal 
{
public:
    LeeOp()
    {
        m_name = "<=";
    }
    virtual void init_op();
    virtual void compute_tick(const Env& env, int idx);

    virtual std::string print_str();
    virtual std::string get_el_tick();
    virtual std::string get_mql_tick();
};


#endif // ops_h__
