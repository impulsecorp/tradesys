#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

/* This module defines common types used by the signal tree system
 *
 */

#include <boost/variant.hpp>
#include "../dtime.h"
#include <string>

using namespace boost;


// forward
class SP_constraints;


// Types an internal signal variable can have.
// Must be part of the generated programming language.
enum VariableType { VAR_TYPE_INT = 0, VAR_TYPE_FLOAT, VAR_TYPE_BOOL,
                    VAR_TYPE_ARRAY_INT, VAR_TYPE_ARRAY_FLOAT, VAR_TYPE_ARRAY_BOOL };

// The SP_type - a variant of int, double, bool, Time or std::string
typedef bs::variant<int, double, bool, Time, std::string> SP_type;

// This class maps a unique name to a variant of int/double/bool/string
class ELI
{
public:
    std::string name;
    SP_type value;

    // Handy constructors
    ELI(std::string n, int v) { name = n; value = v; }
    ELI(std::string n, double v) { name = n; value = v; }
    ELI(std::string n, bool v) { name = n; value = v; }
    ELI(std::string n, Time v) { name = n; value = v; }
    ELI(std::string n, std::string v) { name = n; value = v; }
    ELI(){};
};

// Just like SP_type, but can also have an ELI value
class SignalParameter // SP - Signal Parameter, all of them must be wrapped by SP(..) like TimeIs(SP(..)).
{
public:
    bs::variant<int, double, bool, Time, std::string, ELI> contents;

    // handy constructors
    SignalParameter(int v)
    { contents = v; }
    
    SignalParameter(double v)
    { contents = v; }
    
    SignalParameter(bool v)
    { contents = v; }
    
    SignalParameter(Time v)
    { contents = v; }
    
    SignalParameter(std::string v)
    { contents = v; }
    
    SignalParameter(ELI v)
    { contents = v; }
    
    SignalParameter()
    { contents = 0; } // default: int
};

typedef std::map<std::string, SignalParameter> ParameterMap;
typedef std::map< std::string, SP_type > ExplicitParameterMap;
typedef std::map< std::string, SP_constraints > ParameterConstraintMap;
typedef std::map< std::string, ParameterConstraintMap > ParameterConstraintDataBase;

// Signal tree node/op types
enum NodeReturnType { RT_NONE = 0, RT_BOOL, RT_INT, RT_FLOAT };
typedef bs::variant<int, double, bool> ReturnVariant;

extern SignalParameter SPType2SP(SP_type a);

extern SP_type SP2SPType(SignalParameter a);

inline double convert_to_float(SP_type val)
{
	if (val.type() == typeid(int))
	{
		return (double)(bs::get<int>(val));
	}
	else
	if (val.type() == typeid(double))
	{
		return (get<double>(val));
	}
	else
	{
		throw std::runtime_error("Expected float/int, got something different.");
	}
}

inline double convert_to_float(ReturnVariant val)
{
	if (val.type() == typeid(int))
	{
		return (double)(get<int>(val));
	}
	else
	if (val.type() == typeid(double))
	{
		return (get<double>(val));
	}
	else
	{
		throw std::runtime_error("Expected float/int, got something different.");
	}
}

inline int convert_to_int(SP_type val)
{
	if (val.type() == typeid(int))
	{
		return (get<int>(val));
	}
	else
	if (val.type() == typeid(double))
	{
		return (int)(get<double>(val));
	}
	else
	{
		throw std::runtime_error("Expected int/float, got something different.");
	}
}

inline int convert_to_int(ReturnVariant val)
{
	if (val.type() == typeid(int))
	{
		return (get<int>(val));
	}
	else
	if (val.type() == typeid(double))
	{
		return (int)(get<double>(val));
	}
	else
	{
		throw std::runtime_error("Expected int/float, got something different.");
	}
}

// for debugging
extern void display_exp_parameter_map(ExplicitParameterMap& m);

#endif // COMMON_TYPES_H
