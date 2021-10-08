#include <algorithm>
#include <sstream>
#include "expression.h"

using namespace boost;

class OpInfo
{
public:
    std::string op;
    unsigned int pos;
    unsigned int priority;
};

bool op_more_important(OpInfo ls, OpInfo rs)
{
    return (ls.priority > rs.priority);
}

// forward
Signal *ParseSignal(std::string a_expr, std::map<std::string, Signal *> a_variables);
std::string remove_comments(std::string a_expr);

Signal *EvalExpr(std::string a_expr, std::map<std::string, Signal *> a_variables)
{
    std::string t_expr;
    std::string t_n_expr;

    // remove all comments from expression
    t_n_expr = remove_comments(a_expr);

    // remove all whitespace from expression
    for(unsigned int i=0; i<t_n_expr.size(); i++)
    {
        if (!((t_n_expr[i] == ' ') ||
            (t_n_expr[i] == '\n') ||
            (t_n_expr[i] == '\t') ||
            (t_n_expr[i] == '\r')))
            t_expr += t_n_expr[i];
    }
    // now work with t_expr

    // search for parenthesis or square brackets mismatch
    int pl = 0, bl = 0;
    for(unsigned int i=0; i<t_expr.size(); i++)
    {
        // increase parenthesis level
        if (t_expr[i] == '(')
            pl += 1;
        // decrease parenthesis level
        if (t_expr[i] == ')')
            pl -= 1;
        // increase square bracket level
        if (t_expr[i] == '[')
            bl += 1;
        // decrease square bracket level
        if (t_expr[i] == ']')
            bl -= 1;
    }
    if ((pl != 0) || (bl != 0))
    	throw std::runtime_error("Parenthesis or square bracket mismatch.");


    // find the lowest priority operator if any

    // so make a list of found operators, don't include those in parenthesis
    std::vector< OpInfo > ops;
    int p_level = 0;
    for(unsigned int i=0; i<t_expr.size(); i++)
    {
        // increase parenthesis level
        if (t_expr[i] == '(')
        {
            p_level += 1;
        }
        // decrease parenthesis level
        if (t_expr[i] == ')')
        {
            p_level -= 1;
        }

        // only check and count ops on parenthesis level of 0
        if (p_level == 0)
        {
            // check for op signatures
            if (t_expr.substr(i, 3) == "and")
            {
                OpInfo op; op.op = "and"; op.pos = i; op.priority = 4; ops.push_back(op);
            }
            if (t_expr.substr(i, 2) == "or")
            {
                OpInfo op; op.op = "or"; op.pos = i; op.priority = 5; ops.push_back(op);
            }
            if (t_expr.substr(i, 3) == "not")
            {
                OpInfo op; op.op = "not"; op.pos = i; op.priority = 3; ops.push_back(op);
            }
            if (t_expr.substr(i, 1) == "+")
            {
                OpInfo op; op.op = "+"; op.pos = i; op.priority = 1; ops.push_back(op);
            }
            if (t_expr.substr(i, 1) == "-")
            {
                OpInfo op; op.op = "-"; op.pos = i; op.priority = 1; ops.push_back(op);
            }
            if (t_expr.substr(i, 1) == "*")
            {
                OpInfo op; op.op = "*"; op.pos = i; op.priority = 0; ops.push_back(op);
            }
            if (t_expr.substr(i, 1) == "/")
            {
                OpInfo op; op.op = "/"; op.pos = i; op.priority = 0; ops.push_back(op);
            }
            if (t_expr.substr(i, 1) == "%")
            {
                OpInfo op; op.op = "%"; op.pos = i; op.priority = 0; ops.push_back(op);
            }
            if (t_expr.substr(i, 2) == "==")
            {
                OpInfo op; op.op = "=="; op.pos = i; op.priority = 2; ops.push_back(op);
            }
            if (t_expr.substr(i, 2) == "!=")
            {
                OpInfo op; op.op = "!="; op.pos = i; op.priority = 2; ops.push_back(op);
            }
            if (t_expr.substr(i, 2) == "<>")
            {
                OpInfo op; op.op = "!="; op.pos = i; op.priority = 2;  ops.push_back(op);
            }
            if (t_expr.substr(i, 1) == ">")
            {
                OpInfo op; op.op = ">"; op.pos = i; op.priority = 2; ops.push_back(op);
            }
            if (t_expr.substr(i, 1) == "<")
            {
                OpInfo op; op.op = "<"; op.pos = i; op.priority = 2; ops.push_back(op);
            }
            if (t_expr.substr(i, 2) == ">=")
            {
                OpInfo op; op.op = ">="; op.pos = i; op.priority = 2; ops.push_back(op);
            }
            if (t_expr.substr(i, 2) == "<=")
            {
                OpInfo op; op.op = "<="; op.pos = i; op.priority = 2; ops.push_back(op);
            }
        }
    }

    if (ops.size() > 0) // ops found
    {
		// sort the ops by priority
		// current op priorities:
		// 0 : * / %
		// 1 : + -
		// 2 : == != > < >= <=
		// 3 : not
		// 4 : and
		// 5 : or
        if (ops.size() > 1)
            std::sort(ops.begin(), ops.end(), op_more_important);

		// use the last op with lowest priority
		// so that 3 - 2 - 1 is executed as (3 - 2) - 1 = 0 instead of 3 - (2 - 1) = 2
		unsigned int lowest_pr = ops[0].priority;
		unsigned int lowest_last_pr_idx = 0;
        if (ops.size() > 1)
        {
            do 
            {
                if (ops[lowest_last_pr_idx].priority == lowest_pr)
                {
                    lowest_last_pr_idx++;
                }

                if (lowest_last_pr_idx == ops.size())
                {
                    break;
                }
            } while(ops[lowest_last_pr_idx].priority == lowest_pr);
		    lowest_last_pr_idx -= 1;
        }

		// return an op signal connected to the children returned by recursion
		Signal *l = NULL, *r = NULL;
		OpSignal *op = NULL;

		// "not" is the only unary op (syntax is "not SIGNAL_EXPRESSION")
		if (ops[lowest_last_pr_idx].op == "not")
		{
			// just get the right side
			std::string right = t_expr.substr(ops[lowest_last_pr_idx].pos +
					ops[lowest_last_pr_idx].op.length(), std::string::npos);
            
            r = EvalExpr(right, a_variables); // recursion point

			op = new NotOp();
			op->add_child(r);
            op->init_op();
		}
		else
		{
			// binary signals split the string in left and right sides
			std::string left = t_expr.substr(0, ops[lowest_last_pr_idx].pos);
			std::string right = t_expr.substr(ops[lowest_last_pr_idx].pos +
					ops[lowest_last_pr_idx].op.length(), std::string::npos);
            
            l = EvalExpr(left, a_variables); // recursion point
            r = EvalExpr(right, a_variables);

			// create an op depending on type
			if (ops[lowest_last_pr_idx].op == "and")
            {
				op = new AndOp();
            }
			if (ops[lowest_last_pr_idx].op == "or")
            {
				op = new OrOp();
            }
			if (ops[lowest_last_pr_idx].op == "+")
            {
				op = new AddOp();
            }
			if (ops[lowest_last_pr_idx].op == "-")
            {
				op = new SubOp();
            }
			if (ops[lowest_last_pr_idx].op == "*")
            {
				op = new MulOp();
            }
			if (ops[lowest_last_pr_idx].op == "/")
            {
				op = new DivOp();
            }
            if (ops[lowest_last_pr_idx].op == "%")
            {
                op = new MmodOp();
            }
			if (ops[lowest_last_pr_idx].op == ">")
            {
				op = new GrOp();
            }
			if (ops[lowest_last_pr_idx].op == ">=")
            {
                op = new GreOp();
            }
			if (ops[lowest_last_pr_idx].op == "<")
            {
                op = new LeOp();
            }
			if (ops[lowest_last_pr_idx].op == "<=")
            {
                op = new LeeOp();
            }
			if (ops[lowest_last_pr_idx].op == "==")
            {
                op = new EqOp();
            }
			if (ops[lowest_last_pr_idx].op == "!=")
            {
                op = new NeqOp();
            }

			if (op != NULL)
			{
				op->add_child(l);
				op->add_child(r);
                op->init_op();
			}
			else
			{
				throw std::runtime_error("Unknown op type");
			}
		}

		if (op != NULL)
			return op;
		else
		{
			throw std::runtime_error("Error creating op");
		}
    }
    else
    {
    	// no ops found - process a signal (leaf node)

    	// if the entire expression is surrounded by "(" and ")", remove them
        if ((t_expr[0] == '(') && (t_expr[t_expr.size()-1] == ')'))
        {
            return EvalExpr(t_expr.substr(1, t_expr.size() - 2), a_variables); // recursion point
        }
        else
        {
        	// recognize and create signal
            return ParseSignal(t_expr, a_variables); // possible recursion point (for composite signals)
        }
    }

    return NULL;
}

Signal *EvalExpr_NoVars(std::string a_expr)
{
    return EvalExpr(a_expr, std::map<std::string, Signal *>());
}

// returns true if it finds an op in the string
bool has_op(std::string s)
{
	for(unsigned int i=0; i<s.length(); i++)
	{
        // check for op signatures
        if ((s.substr(i, 3) == "and") ||
           (s.substr(i, 2) == "or") ||
           (s.substr(i, 3) == "not") ||
           (s.substr(i, 1) == "+") ||
           (s.substr(i, 1) == "-") ||
           (s.substr(i, 1) == "*") ||
           (s.substr(i, 1) == "/") ||
           (s.substr(i, 2) == "==") ||
           (s.substr(i, 2) == "!=") ||
           (s.substr(i, 2) == "<>") ||
           (s.substr(i, 1) == ">") ||
           (s.substr(i, 1) == "<") ||
           (s.substr(i, 2) == ">=") ||
           (s.substr(i, 2) == "<="))
            return true;
	}

	return false;
}

// returns true if it finds parenthesis
bool has_parenthesis(std::string s)
{
	for(unsigned int i=0; i<s.length(); i++)
	{
		if ((s[i] == '(') || (s[i] == ')'))
			return true;
	}

	return false;
}

// returns true if it finds uppercase or underscore
bool has_uppercase(std::string s)
{
    int level = 0;
	for(unsigned int i=0; i<s.length(); i++)
	{
        // don't count if inside brackets, so it's possible to have uppercase inputs []
        if (s[i] == '[')
            level++;
        if (s[i] == ']')
            level--;

        if (level == 0)
        {
		    if (isupper(s[i]))
			    return true;
        }
	}

	return false;
}

// returns true if it finds equality sign
bool has_eq(std::string s)
{
	for(unsigned int i=0; i<s.length(); i++)
	{
		if (s[i] == '=')
			return true;
	}

	return false;
}

// Extracts the parameters from a signal ( .. TIME_IS( ***, ***, *** ... ***) )
void extract_params(const std::string& a_expr, std::vector<std::string>& params)
{
	unsigned int param_idx = 0;
	int sqbr_level = 0;
	int p_level = 0;
	params.push_back("");
	for (unsigned int i = 0; i < a_expr.size(); i++)
	{
		// don't count if inside square brackets [] or parenthesis ()
		if (a_expr[i] == '[')
			sqbr_level += 1;

		if (a_expr[i] == ']')
			sqbr_level -= 1;

		if (a_expr[i] == '(')
			p_level += 1;

		if (a_expr[i] == ')')
			p_level -= 1;

		if ((sqbr_level == 0) && (p_level == 0))
		{
			if (a_expr[i] == ',')
			{
				param_idx += 1;
				params.push_back("");
			}
			else
			{
				params[param_idx] += a_expr[i];
			}
		}
		else
		{
			params[param_idx] += a_expr[i];
		}
	}

	// if that doesn't end up as 0, something is wrong
	if (sqbr_level != 0)
		throw ("Square bracket [] mismatch");
	if (p_level != 0)
		throw ("Parenthesis () mismatch");
}

std::vector<Signal *> ParseSignalParameters(std::string a_expr, std::map<std::string, Signal *> a_variables)
{
	// count how many parameters there are and separate them
	// the separator is a comma

	std::vector<std::string> params;
	extract_params(a_expr, params);

	std::vector<Signal*> results;

	unsigned int num_signal_params = 0;

	// for each parameter
	for(unsigned int i=0;i<params.size();i++)
	{
		// determine if inside is a signal by following signs:
		// * there are uppercase letters or underscores
		if (has_uppercase(params[i]))
		{
			// signal tree inside, this means a parameter with special name
			num_signal_params++;
			char a[64];
			sprintf(a,"%d", num_signal_params);
			results.push_back(EvalExpr(params[i], a_variables)); // recursion point
		}
	}

	return results;
}


// recognize the value between time, time with seconds, int, double, bool or string.
SP_type ParseSPtype(std::string a_val)
{
	SP_type x;

	enum { UNKNOWN = 0,
		   TIME_HHMM_PARAMETER,
		   TIME_HHMMSS_PARAMETER,
		   INT_PARAMETER,
		   DOUBLE_PARAMETER,
		   BOOL_PARAMETER,
		   STRING_PARAMETER };
	int detected_type = UNKNOWN;

    std::string t_val;
    for(unsigned int i=0; i<a_val.size();i++)
    {
        t_val += tolower(a_val[i]);
    }

	// if there is one ':', it's TIME_HHMM, if there are two, it's TIME_HHMMSS
	// count how many
	unsigned int nts = 0;
	for(unsigned int i=0; i<t_val.size(); i++)
	{
		if (t_val[i] == ':')
		{
			nts += 1;
		}
	}

	// Find how many points are found. If it's one, it's a floating point number
	unsigned int dots = 0;
	for(unsigned int i=0; i<t_val.size(); i++)
	{
		if (t_val[i] == '.')
		{
			dots += 1;
		}
	}

	bool all_digits = true;
    // ignore a leading minus sign
    if ((!isdigit(t_val[0])) && (t_val[0] != '-'))
    {
        all_digits = false;
    }
	for(unsigned int i=1; i<t_val.size(); i++)
	{
		if (!isdigit(t_val[i]))
		{
			all_digits = false;
		}
	}

	// if it finds ' or " at the sides, it's a string
	if (((t_val[0] == '"') || (t_val[0] == '\'')) &&
	    ((t_val[t_val.length()-1] == '"') || (t_val[t_val.length()-1] == '\'')))
	{
		detected_type = STRING_PARAMETER;
	}
	else if (nts == 1) // or if there is one :
	{
		detected_type = TIME_HHMM_PARAMETER;
	}
	else if (nts == 2) // or if there are two : :
	{
		detected_type = TIME_HHMMSS_PARAMETER;
	}
	else if (dots == 1)
	{
		detected_type = DOUBLE_PARAMETER;
	}
	else if ((t_val == "true") || (t_val == "false"))
	{
		detected_type = BOOL_PARAMETER;
	}
	else if (all_digits)
	{
		detected_type = INT_PARAMETER;
	}

	int vi=0, h=0, m=0, s=0;
	double vf=0;

	std::istringstream iii(t_val.c_str());

	// OK, now read the parameter
	switch(detected_type)
	{
	case INT_PARAMETER:
		// read int from string
		sscanf(t_val.c_str(), "%d", &vi);
		x = vi;
		break;

	case DOUBLE_PARAMETER:
		// read double from string
        //sscanf(t_val.c_str(), "%lf", &vf);
		if (!(iii >> vf))
			vf = 0;
		x = vf;
		break;

	case BOOL_PARAMETER:
		// read bool from string
		if ((t_val == "true") || (t_val == "True"))
		{
			x = true;
		}
		else if ((t_val == "false") || (t_val == "False"))
		{
			x = false;
		}
		break;

	case TIME_HHMM_PARAMETER:
		// read time from string
		sscanf(t_val.c_str(), "%02d:%02d", &h, &m);
		x = Time(h, m);
		break;

	case TIME_HHMMSS_PARAMETER:
		// read time from string
		sscanf(t_val.c_str(), "%02d:%02d:%02d", &h, &m, &s);
		x = Time(h, m, s);
		break;

	case STRING_PARAMETER:
		// read string from string
		x = t_val.substr(1, t_val.length()-2);
		break;

	default:
		throw std::runtime_error("Unrecognized parameter type - " + t_val);
	}

	return x;
}

// recognize the value between time, time with seconds, int, double, bool or string.
// also recognize ELIs (in square brackets)
SignalParameter ParseSP(std::string val)
{
    SignalParameter x;

	// determine if that's an ELI
	if ((val[0] == '[') && (val[val.length()-1] == ']'))
	{
		ELI eli;

		// find the separator comma
		unsigned int sep = 0;
		sep = val.find(',');
		if (sep == std::string::npos)
			throw std::runtime_error("Separator comma not found in ELI");

		eli.name = val.substr(2, sep-3); // take ' or " into account
		eli.value = ParseSPtype(val.substr(sep+1, val.length() - sep - 2));
		x.contents = eli;
	}
	else
	{
		// not an ELI
		// convert from SP_type to SP
		x = SPType2SP(ParseSPtype(val));
	}

	return x;
}

// Simple name-value pair. Need to refactor this to use std::pair (TODO)
class NameSP
{
public:
    std::string name;
    SignalParameter value;
};

// Helpful classes
class NSP
{
public:
    int n;
    SignalParameter sp;
};

bool NSP_less(NSP ls, NSP rs)
{
    return ls.n < rs.n;
}



std::vector<NameSP> ParseNormalParameters(std::string a_expr)
{
	std::vector<NameSP> results;
	if (a_expr.size() == 0)
		return results;

	// count how many parameters there are and separate them
	// the separator is a comma
	std::vector<std::string> params;
	extract_params(a_expr, params);
    
    
    unsigned int num_nameless_params = 0;

	// for each parameter
	for(unsigned int i=0;i<params.size();i++)
	{
		// determine if inside is a signal by following signs:
		// * there are uppercase letters or underscores
		if (!(has_uppercase(params[i])))
		{
			// not a signal tree expression
			NameSP nsp;
			std::string val;

			// could still be nameless, so check for '=' inside
			if (has_eq(params[i]))
			{
				// not nameless
				unsigned int eq_pos = params[i].find('=');
				nsp.name = params[i].substr(0, eq_pos);
				val = params[i].substr(eq_pos+1, std::string::npos);
			}
			else
			{
				// nameless, positional
				num_nameless_params++;
				char a[64];
				sprintf(a,"%d", num_nameless_params);
				nsp.name = a;
				val = params[i];
			}

			// recognize the value between time HH:MM, time HH:MM:SS, int, double, bool or string.
			// also recognize ELIs (in square brackets)
			nsp.value = ParseSP(val);
			results.push_back(nsp);

//			std::cout << nsp.name << " " << val << "\n";
		}
	}

	return results;
}


Signal *RecognizeSignal(std::string sn,
                        std::vector<SignalParameter> &nameless_parameters,
                        ParameterMap &ow,
                        std::vector<Signal *> &sps,
                        std::map<std::string, Signal *> a_variables);

Signal *ParseSignal(std::string a_expr, std::map<std::string, Signal *> a_variables)
{
	// check if the signal has any parameters supplied
	bool no_params = false;
	if (a_expr.find('(') == std::string::npos) // no parameters
		no_params = true;

	std::string signal_name;
	std::string parameters;

	if (!no_params) // has parameters
	{
		// find first parenthesis
		unsigned int ppos = a_expr.find('(');

		signal_name = a_expr.substr(0, ppos);
		parameters = a_expr.substr(ppos+1, a_expr.length()-ppos-2);
	}
	else
	{
		signal_name = a_expr;
	}
    
    
    // parse any parameters
    std::vector<NameSP> nvs = ParseNormalParameters(parameters);
    
    // from these, find the nameless ones and separate them
    std::vector<NSP> nameless_with_pos;
    std::vector<SignalParameter> nameless_parameters; // idx is pos
    std::vector<NameSP> named;

    for(unsigned int i=0; i<nvs.size(); i++)
    {
    	bool all_digits = true;
    	for(unsigned int j=0; j<nvs[i].name.size(); j++)
    	{
    		if (!isdigit(nvs[i].name[j]))
    		{
    			all_digits = false;
    			break;
    		}
    	}

    	if (all_digits)
    	{
    		NSP nsp;
    		nsp.n = atoi(nvs[i].name.c_str());
    		nsp.sp = nvs[i].value;
    		nameless_with_pos.push_back(nsp);
    	}
    	else
    	{
    		NameSP nsp;
    		nsp.name = nvs[i].name;
    		nsp.value = nvs[i].value;
    		named.push_back(nsp);
    	}
    }
    std::sort(nameless_with_pos.begin(), nameless_with_pos.end(), NSP_less);
    // todo: check if sorted it correctly
    for(unsigned int i=0; i<nameless_with_pos.size(); i++)
    {
    	nameless_parameters.push_back(nameless_with_pos[i].sp);
    }

    // put the signal tree evaluated parameters here
	std::vector<Signal *> sps = ParseSignalParameters(parameters, a_variables);

	// recognize and create signal
	ParameterMap ow;

	// put the named parameters in the map
	for(unsigned i=0; i<named.size(); i++)
	{
		ow[named[i].name] = named[i].value;
	}
    
    return RecognizeSignal(signal_name, nameless_parameters, ow, sps, a_variables);
}






std::string remove_comments(std::string a_expr)
{
	std::string result;

	int s_level=0;
	int t_level=0;
	int next_step=1;

	// the comments can be the C style /* ... */, and the line ending // and Python's #
	for(unsigned int i=0; i<a_expr.size()-1; i+=next_step)
	{
		next_step = 1; // default

		// 2 char comment delimiters
		std::string t_s1 = a_expr.substr(i, 2);
		if (t_s1 == "/*")
		{
			s_level++;
			next_step = 2;
		}
		if (t_s1 == "*/")
		{
			s_level--;
			next_step = 2;
		}

		if (s_level == 0) // end-of-line comments only work outside of /* .. */
		{
			if (t_s1 == "//")
			{
				t_level++;
				next_step = 2;
			}

			// 1 char comment delimiters
			std::string t_s2 = a_expr.substr(i, 1);
			/*if (t_s2 == "#") // conflicts with # amount_per_trade, etc.
			{
				t_level++;
			}*/
			if (t_s2 == "\n")
			{
				t_level = 0;
			}
		}
		else
		{
			t_level = 0;
		}

		// only add symbols to the result at level 0
		if ((s_level == 0) && (t_level == 0))
		{
			if ((a_expr.substr(i, 2) != "/*") &&
				(a_expr.substr(i, 2) != "*/") &&
				(a_expr.substr(i, 2) != "//") &&
				//(a_expr.substr(i, 1) != "#") &&
				(a_expr.substr(i, 1) != "\n"))
			{
				result += a_expr.substr(i, 1);
			}
		}
	}
	// add the last character (if level is 0)
	if ((s_level == 0) && (t_level == 0))
	{
		result += a_expr.substr(a_expr.size()-1, 1);
	}

	return result;
}




#define AS_STR(tok) #tok
#define CDL_SIGNAL_RECOG(X) if (sn == AS_STR(X)) \
							{ \
								if (nameless_parameters.size() == 1) \
									ow["bars_back"] = nameless_parameters[0]; \
								signal = new X (ow, sps); \
							}


Signal *RecognizeSignal(std::string signal_name, std::vector<SignalParameter> &nameless_parameters,
                        ParameterMap &ow, std::vector<Signal *> &sps,
                        std::map<std::string, Signal *> a_variables)
{
    Signal* signal = NULL;
    std::string sn = all_upper(signal_name);
    
    // first check if such variable exists and return it if so
    for (auto it = a_variables.begin(); it != a_variables.end(); it++)
    {
        if (sn == it->first)
        {
            return it->second;
        }
    }
    
    // otherwise check if it's a signal..
    
    ////////////////////////
    // Times
    ////////////////////////
    if (sn == "TIME_IS")
    {
        // can have 1 nameless parameter
        if (nameless_parameters.size() == 1)
            ow["time"] = nameless_parameters[0];
        signal = new TimeIs(ow, sps);
    }
    if (sn == "TIME_IS_BEFORE")
    {
        // can have 1 nameless parameter
        if (nameless_parameters.size() == 1)
            ow["time"] = nameless_parameters[0];
        signal = new TimeIsBefore(ow, sps);
    }
    if (sn == "TIME_IS_AFTER")
    {
        // can have 1 nameless parameter
        if (nameless_parameters.size() == 1)
            ow["time"] = nameless_parameters[0];
        signal = new TimeIsAfter(ow, sps);
    }
    if (sn == "TIME_IS_BETWEEN")
    {
        // can have 2 nameless parameters
        if (nameless_parameters.size() == 2)
        {
            ow["start_time"] = nameless_parameters[0];
            ow["end_time"] = nameless_parameters[1];
        }

        signal = new TimeIsBetween(ow, sps);
    }
    if (sn == "NEW_DAY")
    {
        // no parameters
        signal = new NewDay(ow, sps);
    }

    ////////////////////////
    // Raw price
    ////////////////////////
    if (sn == "UP")
    {
        // can have 1 nameless parameters
        if (nameless_parameters.size() == 1)
        {
            ow["bars_back"] = nameless_parameters[0];
        }
        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new Up(ow, sps);
    }
    if (sn == "DOWN")
    {
        // can have 1 nameless parameters
        if (nameless_parameters.size() == 1)
        {
            ow["bars_back"] = nameless_parameters[0];
        }
        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new Down(ow, sps);
    }
    if ((sn == "PERCENT_UP") || (sn == "P_UP"))
    {
        // can have 1 nameless parameters
        if (nameless_parameters.size() == 1)
        {
            ow["bars_back"] = nameless_parameters[0];
        }
        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new PercentUp(ow, sps);
    }
    if ((sn == "PERCENT_DOWN") || (sn == "P_DOWN"))
    {
        // can have 1 nameless parameters
        if (nameless_parameters.size() == 1)
        {
            ow["bars_back"] = nameless_parameters[0];
        }
        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new PercentDown(ow, sps);
    }
    if ((sn == "MOVE_DIFF")) 
    {
        // can have 1 nameless parameters
        if (nameless_parameters.size() == 1)
        {
            ow["bars_back"] = nameless_parameters[0];
        }
        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new MovementDiff(ow, sps);
    }
    if ((sn == "MOVE_VAL_BEFORE")) 
    {
        // can have 1 nameless parameters
        if (nameless_parameters.size() == 1)
        {
            ow["bars_back"] = nameless_parameters[0];
        }
        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new MovementValBefore(ow, sps);
    }
    if ((sn == "MOVE_PERCENT_DIFF") || (sn == "MOVE_PDIFF")) 
    {
        // can have 1 nameless parameters
        if (nameless_parameters.size() == 1)
        {
            ow["bars_back"] = nameless_parameters[0];
        }
        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new MovementPercentDiff(ow, sps);
    }
    if (sn == "PRICE") // raw price
    {
        // can have 1 or 2 nameless parameters
        if (nameless_parameters.size() == 1)
            ow["price"] = nameless_parameters[0];
        if (nameless_parameters.size() == 2)
        {
            ow["price"] = nameless_parameters[0];
            ow["bars_back"] = nameless_parameters[1];
        }
        signal = new Price(ow, sps);
    }
    if (sn == "VALUE_AT_TIME") // raw value at specified time
    {
        // can have 1 nameless parameters
        if (nameless_parameters.size() == 1)
        {
            ow["time"] = nameless_parameters[0];
        }
        // must have 1 signal parameter
        ASS(sps.size()==1);
        signal = new ValueAtTime(ow, sps);
    }
    if (sn == "ABS") // absolute value of
    {
        // can have no nameless parameters

        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new AbsValueOf(ow, sps);
    }
    if (sn == "NEG") // negative of
    {
        // can have no nameless parameters

        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new Negative(ow, sps);
    }
    if ((sn == "VALUE_OF") || (sn == "VALUE") || (sn == "VALUE_BB")) // value of expression bars back
    {
        // the nameless parameter is bars_back
        if (nameless_parameters.size() == 1)
        {
            ow["bars_back"] = nameless_parameters[0];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new ValueOf(ow, sps);
    }
    if (sn == "PERCENT_OF") // percent of value
    {
        // the nameless parameter is bars_back
        if (nameless_parameters.size() == 1)
        {
            ow["x"] = nameless_parameters[0];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new PercentOfValue(ow, sps);
    }
    if (sn == "MIN") // min value for period bars
    {
        // the nameless parameter is bars_back
        if (nameless_parameters.size() == 1)
        {
            ow["period"] = nameless_parameters[0];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new MinValueForPeriod(ow, sps);
    }
    if (sn == "MIN_IDX") // min value for period bars
    {
        // the nameless parameter is bars_back
        if (nameless_parameters.size() == 1)
        {
            ow["period"] = nameless_parameters[0];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new MinValueForPeriodIdx(ow, sps);
    }
    if (sn == "MAX") // max value for period bars
    {
        // the nameless parameter is bars_back
        if (nameless_parameters.size() == 1)
        {
            ow["period"] = nameless_parameters[0];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new MaxValueForPeriod(ow, sps);
    }
    if (sn == "MAX_IDX") // max value for period bars
    {
        // the nameless parameter is bars_back
        if (nameless_parameters.size() == 1)
        {
            ow["period"] = nameless_parameters[0];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new MaxValueForPeriodIdx(ow, sps);
    }
    if (sn == "SUM") // sum of expression for period
    {
        // the nameless parameter is bars_back
        if (nameless_parameters.size() == 1)
        {
            ow["period"] = nameless_parameters[0];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new SumValueForPeriod(ow, sps);
    }
    if (sn == "AVG") // MA of expression 
    {
        // the nameless parameter is bars_back
        if (nameless_parameters.size() == 1)
        {
            ow["period"] = nameless_parameters[0];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new AvgValueForPeriod(ow, sps);
    }
    if (sn == "MIN_SINCE") 
    {
        if (nameless_parameters.size() == 1)
        {
            ow["since"] = nameless_parameters[0];
        }
        if (nameless_parameters.size() == 2)
        {
            ow["since"] = nameless_parameters[0];
            ow["to"] = nameless_parameters[1];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new MinValueSince(ow, sps);
    }
    if (sn == "MAX_SINCE") 
    {
        if (nameless_parameters.size() == 1)
        {
            ow["since"] = nameless_parameters[0];
        }
        if (nameless_parameters.size() == 2)
        {
            ow["since"] = nameless_parameters[0];
            ow["to"] = nameless_parameters[1];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new MaxValueSince(ow, sps);
    }
    if (sn == "AVG_SINCE") 
    {
        if (nameless_parameters.size() == 1)
        {
            ow["since"] = nameless_parameters[0];
        }
        if (nameless_parameters.size() == 2)
        {
            ow["since"] = nameless_parameters[0];
            ow["to"] = nameless_parameters[1];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new AvgValueSince(ow, sps);
    }
    if (sn == "SUM_SINCE") 
    {
        if (nameless_parameters.size() == 1)
        {
            ow["since"] = nameless_parameters[0];
        }
        if (nameless_parameters.size() == 2)
        {
            ow["since"] = nameless_parameters[0];
            ow["to"] = nameless_parameters[1];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new SumValueSince(ow, sps);
    }
    if ((sn == "VALUE_OF_WHEN_FIRST_HAPPENED") || (sn == "VALUE_OF_FH"))
    {
        if (nameless_parameters.size() == 1)
        {
            ow["timeout"] = nameless_parameters[0];
        }
        // must have 2 signal parameters
        ASS(sps.size()==2);
        signal = new ValueOfWhenFirstHappened(ow, sps);
    }
    if ((sn == "VALUE_OF_WHEN_LAST_HAPPENED") || (sn == "VALUE_OF_LH"))
    {
        /*if (nameless_parameters.size() == 1)
        {
            ow["timeout"] = nameless_parameters[0];
        }*/
        // must have 2 signal parameters
        ASS(sps.size()==2);
        signal = new ValueOfWhenLastHappened(ow, sps);
    }
    if ((sn == "IFTHENELSE") || (sn == "IF_THEN_ELSE"))
    {
        // must have 3 signal parameters
        ASS(sps.size()==3);
        signal = new IfThenElse(ow, sps);
    }
    if (sn == "SORTED")
    {
        if (nameless_parameters.size() == 1)
        {
            ow["dir"] = nameless_parameters[0];
        }
        if (nameless_parameters.size() == 2)
        {
            ow["dir"] = nameless_parameters[0];
            ow["idx"] = nameless_parameters[1];
        }
        
        // can have any number of subsignals
        signal = new SortedSignal(ow, sps);
    }
    if (sn == "SORTED_IDX")
    {
        if (nameless_parameters.size() == 1)
        {
            ow["dir"] = nameless_parameters[0];
        }
        if (nameless_parameters.size() == 2)
        {
            ow["dir"] = nameless_parameters[0];
            ow["idx"] = nameless_parameters[1];
        }
        
        // can have any number of subsignals
        signal = new SortedIdxSignal(ow, sps);
    }
    if (sn == "PROB")
    {
        if (nameless_parameters.size() == 1)
        {
            ow["prob"] = nameless_parameters[0];
        }
		
        signal = new ProbabilisticSignal(ow, sps);
    }
	if (sn == "CURRENT_BAR")
	{
		signal = new CurrentBarSignal(ow, sps);
	}
	if (sn == "NNSIGNAL")
	{
		if (nameless_parameters.size() == 1)
		{
			ow["mbb"] = nameless_parameters[0];
		}
		if (nameless_parameters.size() == 2)
		{
			ow["mbb"] = nameless_parameters[0];
			ow["winlen"] = nameless_parameters[1];
		}
		if (nameless_parameters.size() == 3)
		{
			ow["mbb"] = nameless_parameters[0];
			ow["winlen"] = nameless_parameters[1];
			ow["cl"] = nameless_parameters[2];
		}
		
		signal = new NNSignal(ow, sps);
	}
	if (sn == "MLSIGNAL")
	{
		if (nameless_parameters.size() == 1)
		{
			ow["mbb"] = nameless_parameters[0];
		}
		if (nameless_parameters.size() == 2)
		{
			ow["mbb"] = nameless_parameters[0];
			ow["winlen"] = nameless_parameters[1];
		}
        
		signal = new MLSignal(ow, sps);
	}
	if (sn == "FMLSIGNAL")
	{
		if (nameless_parameters.size() == 1)
		{
			ow["mbb"] = nameless_parameters[0];
		}
		if (nameless_parameters.size() == 2)
		{
			ow["mbb"] = nameless_parameters[0];
			ow["winlen"] = nameless_parameters[1];
		}

		signal = new FMLSignal(ow, sps);
	}


	////////////////////////
    // Constants

    ////////////////////////
    if ((sn == "INT") || (sn == "INT_CONST") || (sn == "I"))
    {
        // can have 1 nameless parameter
        if (nameless_parameters.size() == 1)
        {
            ow["val"] = nameless_parameters[0];
        }
        signal = new IntConst(ow, sps);
    }
    if ((sn == "FLOAT") || (sn == "FLOAT_CONST") || (sn == "F"))
    {
        // can have 1 nameless parameter
        if (nameless_parameters.size() == 1)
        {
            ow["val"] = nameless_parameters[0];
        }
        signal = new FloatConst(ow, sps);
    }
    if ((sn == "BOOL") || (sn == "BOOL_CONST") || (sn == "B"))
    {
        // can have 1 nameless parameter
        if (nameless_parameters.size() == 1)
        {
            ow["val"] = nameless_parameters[0];
        }
        signal = new BoolConst(ow, sps);
    }

    ////////////////////////
    // Happened (composite)
    ////////////////////////
    if (sn == "HAPPENED_TODAY")
    {
        // must have 1 signal tree expression parameter
        ASS(sps.size() == 1);
        signal = new HappenedToday(ow, sps);
    }
    if (sn == "HAPPENED_AT_TIME")
    {
        // can have 1 nameless parameter
        if (nameless_parameters.size() == 1)
        {
            ow["time"] = nameless_parameters[0];
        }

        // must have 1 signal tree expression parameter
        ASS(sps.size() == 1);
        signal = new HappenedAtTime(ow, sps);
    }
    if (sn == "HAPPENED_BEFORE_TIME")
    {
        // can have 1 nameless parameter
        if (nameless_parameters.size() == 1)
        {
            ow["time"] = nameless_parameters[0];
        }

        // must have 1 signal tree expression parameter
        ASS(sps.size() == 1);
        signal = new HappenedBeforeTime(ow, sps);
    }
    if (sn == "HAPPENED_AFTER_TIME")
    {
        // can have 1 nameless parameter
        if (nameless_parameters.size() == 1)
        {
            ow["time"] = nameless_parameters[0];
        }

        // must have 1 signal tree expression parameter
        ASS(sps.size() == 1);
        signal = new HappenedAfterTime(ow, sps);
    }
    if (sn == "HAPPENED_IN_TIME_PERIOD")
    {
        // can have 2 nameless parameters
        if (nameless_parameters.size() == 2)
        {
            ow["start_time"] = nameless_parameters[0];
            ow["end_time"] = nameless_parameters[1];
        }

        // must have 1 signal tree expression parameter
        ASS(sps.size() == 1);
        signal = new HappenedInTimePeriod(ow, sps);
    }
    if (sn == "HAPPENED_BB") // bool expression was true this many bars back
    {
        // the nameless parameter is bars_back
        if (nameless_parameters.size() == 1)
        {
            ow["bars_back"] = nameless_parameters[0];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new HappenedBarsBack(ow, sps);
    }
    if (sn == "HAPPENED_LB") // bool expression was true this many bars back or less
    {
        // the nameless parameter is bars_back
        if (nameless_parameters.size() == 1)
        {
            ow["bars_back"] = nameless_parameters[0];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);
        signal = new HappenedLastBars(ow, sps);
    }
    if ((sn == "SEQUENCE") ||
        (sn == "SEQN") ||
        (sn == "SUCC"))
    {
        // the unnamed parameter is the timeout
        if (nameless_parameters.size() == 1)
        {
            ow["timeout_bars"] = nameless_parameters[0];
        }

        // must have 2 signal tree expression parameters
        ASS(sps.size() == 2);
        signal = new Seqn(ow, sps);
    }


    ////////////////////////
    // Gaps
    ////////////////////////
    if (sn == "GAP_UP")
    {
        // can have no nameless parameters
        signal = new GapUp(ow, sps);
    }
    if (sn == "GAP_DOWN")
    {
        // can have no nameless parameters
        signal = new GapDown(ow, sps);
    }
    if (sn == "FADE_GAP_PERCENT_PRICE")
    {
        // can have no nameless parameters
        signal = new FadeGapPercentPrice(ow, sps);
    }
    if (sn == "GAP_SIZE")
    {
        // can have no nameless parameters
        signal = new GapSize(ow, sps);
    }
    if (sn == "GAP_PRICE_BEFORE")
    {
        // can have no nameless parameters
        signal = new GapPriceBefore(ow, sps);
    }
    if (sn == "GAP_PRICE_AFTER")
    {
        // can have no nameless parameters
        signal = new GapPriceAfter(ow, sps);
    }

    ////////////////////////
    // Feedback
    ////////////////////////
    if (sn == "ENTERED_TODAY")
    {
        signal = new EnteredToday(ow, sps);
    }
    if (sn == "EXITED_TODAY")
    {
        signal = new ExitedToday(ow, sps);
    }
    if ((sn == "BARS_SINCE_FIRST_EVENT") || (sn == "BARS_SINCE_FE"))
    {
        // must have 1 signal parameter
        ASS(sps.size()==1);
        signal = new BarsSinceFirstEvent(ow, sps);
    }
	if ((sn == "BARS_SINCE_LAST_EVENT") || (sn == "BARS_SINCE_LE") || (sn == "BARS_SINCE"))
	{
		// must have 1 signal parameter
		ASS(sps.size() == 1);
		signal = new BarsSinceLastEvent(ow, sps);
	}
    if (sn == "BARS_SINCE_ENTRY")
    {
        signal = new BarsSinceEntry(ow, sps);
    }
    if (sn == "BARS_SINCE_EXIT")
    {
        signal = new BarsSinceExit(ow, sps);
    }
    if (sn == "DAYS_SINCE_ENTRY")
    {
        signal = new DaysSinceEntry(ow, sps);
    }
    if (sn == "DAYS_SINCE_EXIT")
    {
        signal = new DaysSinceExit(ow, sps);
    }
    if (sn == "BUYING")
    {
        signal = new Buying(ow, sps);
    }
    if (sn == "SELLING")
    {
        signal = new Selling(ow, sps);
    }
    if (sn == "INSIDE_MARKET")
    {
        signal = new InsideMarket(ow, sps);
    }
    if (sn == "OUTSIDE_MARKET")
    {
        signal = new OutsideMarket(ow, sps);
    }
    if (sn == "STOP_LOSS_HIT")
    {
        signal = new StopLossHit(ow, sps);
    }
    if (sn == "TAKE_PROFIT_HIT")
    {
        signal = new TakeProfitHit(ow, sps);
    }
    if (sn == "LAST_EXIT_REASON")
    {
        signal = new LastExitReason(ow, sps);
    }
    if (sn == "CURRENT_PROFIT")
    {
        signal = new CurrentProfit(ow, sps);
    }
    if (sn == "LAST_PROFIT")
    {
        signal = new LastProfit(ow, sps);
    }




    ////////////////////////
    // Crosses (composite)
    ////////////////////////
    if ((sn == "CROSS") ||
        (sn == "CROSSES") ||
        (sn == "CROSSED") ||
        (sn == "CROSSING"))
    {
        // must have 2 signal tree expression parameters
        ASS(sps.size() == 2);
        signal = new Crosses(ow, sps);
    }
    if ((sn == "CROSS_ABOVE") ||
        (sn == "CROSSES_ABOVE") ||
        (sn == "CROSSED_ABOVE") ||
        (sn == "CROSSING_ABOVE"))
    {
        // must have 2 signal tree expression parameters
        ASS(sps.size() == 2);
        signal = new CrossesAbove(ow, sps);
    }
    if ((sn == "CROSS_BELOW") ||
        (sn == "CROSSES_BELOW") ||
        (sn == "CROSSED_BELOW") ||
        (sn == "CROSSING_BELOW"))
    {
        // must have 2 signal tree expression parameters
        ASS(sps.size() == 2);
        signal = new CrossesBelow(ow, sps);
    }


    ////////////////////////
    // Times
    ////////////////////////
    if (sn == "TIMES_IN_ROW")
    {
        // must have 1 signal parameter
        ASS(sps.size()==1);
        signal = new TimesInRow(ow, sps);
    }

    if (sn == "TIMES_HAPPENED")
    {
        // must have 1 signal parameter
        ASS(sps.size()==1);
        signal = new TimesHappened(ow, sps);
    }

    if (sn == "TIMES_HAPPENED_SINCE_LH")
    {
        // must have 2 signal parameters
        ASS(sps.size()==2);
        signal = new TimesHappenedSinceLH(ow, sps);
    }

    if (sn == "TIMES_HAPPENED_SINCE_FH")
    {
        // must have 2 signal parameters
        ASS(sps.size()==2);
        signal = new TimesHappenedSinceFH(ow, sps);
    }

    if (sn == "STREAK_ENDED")
    {
        // can have 1 nameless parameter
        // the nameless parameter is N
        if (nameless_parameters.size() == 1)
        {
            ow["n"] = nameless_parameters[0];
        }

        // must have 1 signal parameter
        ASS(sps.size()==1);
        signal = new StreakEnded(ow, sps);
    }

    if (sn == "STREAK_LENGTH")
    {
        // can have 1 nameless parameter
        // the nameless parameter is N
        if (nameless_parameters.size() == 1)
        {
            ow["n"] = nameless_parameters[0];
        }

        // must have 1 signal parameter
        ASS(sps.size()==1);
        signal = new StreakLength(ow, sps, ow, sps);
    }

    if (sn == "STREAK_ENDED_PRICE_DIFF")
    {
        // can have 1 nameless parameter
        // the nameless parameter is N
        if (nameless_parameters.size() == 1)
        {
            ow["n"] = nameless_parameters[0];
        }

        // must have 1 signal parameter
        ASS(sps.size()==1);
        signal = new StreakEndedPriceDiff(ow, sps);
    }


    ////////////////////////////
    // Stop Loss / Take Profit
    ////////////////////////////
    if ((sn == "PERCENT_STOP_LOSS") || (sn == "STOP_LOSS"))
    {
        // can have 1 nameless parameter
        // the nameless parameter is stop size
        if (nameless_parameters.size() == 1)
        {
            ow["size"] = nameless_parameters[0];
        }
        signal = new PercentStopLoss(ow, sps);
    }
    if ((sn == "TRAILING_STOP_LOSS") || (sn == "TRAILING_STOP"))
    {
        // can have 1 nameless parameter
        // the nameless parameter is stop size
        if (nameless_parameters.size() == 1)
        {
            ow["size"] = nameless_parameters[0];
        }
        signal = new TrailingStopLoss(ow, sps);
    }
    if ((sn == "PERCENT_TAKE_PROFIT") || (sn == "TAKE_PROFIT"))
    {
        // can have 1 nameless parameter
        // the nameless parameter is stop size
        if (nameless_parameters.size() == 1)
        {
            ow["size"] = nameless_parameters[0];
        }
        signal = new PercentTakeProfit(ow, sps);
    }


    //////////////////////////////////
    // Technical Analysis
    //////////////////////////////////
	
#ifndef NOTALIB
    	
    if (sn == "MA")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 1)
        {
            ow["period"] = nameless_parameters[0];
        }
        if (nameless_parameters.size() == 2)
        {
            ow["price"]  = nameless_parameters[0];
            ow["period"] = nameless_parameters[1];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);

        signal = new MA(ow, sps);
    }
    if (sn == "RSI")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 1)
        {
            ow["period"] = nameless_parameters[0];
        }
        if (nameless_parameters.size() == 2)
        {
            ow["price"]  = nameless_parameters[0];
            ow["period"] = nameless_parameters[1];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);

        signal = new RSI(ow, sps);
    }
    if (sn == "ATR")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 1)
        {
            ow["period"] = nameless_parameters[0];
        }
        if (nameless_parameters.size() == 2)
        {
            ow["period"]  = nameless_parameters[0];
            ow["bars_back"] = nameless_parameters[1];
        }

        signal = new ATR(ow, sps);
    }
    if (sn == "ADX")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 1)
        {
            ow["period"] = nameless_parameters[0];
        }
        if (nameless_parameters.size() == 2)
        {
            ow["period"]  = nameless_parameters[0];
            ow["bars_back"] = nameless_parameters[1];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);

        signal = new ADX(ow, sps);
    }
    if (sn == "CCI")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 1)
        {
            ow["period"] = nameless_parameters[0];
        }
        if (nameless_parameters.size() == 2)
        {
            ow["period"]  = nameless_parameters[0];
            ow["bars_back"] = nameless_parameters[1];
        }

        signal = new CCI(ow, sps);
    }
    if (sn == "MACD")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 2)
        {
            ow["fast_period"] = nameless_parameters[0];
            ow["slow_period"] = nameless_parameters[1];
        }
        if (nameless_parameters.size() == 3)
        {
            ow["fast_period"] = nameless_parameters[0];
            ow["slow_period"] = nameless_parameters[1];
            ow["signal_period"] = nameless_parameters[2];
        }
        if (nameless_parameters.size() == 4)
        {
            ow["fast_period"] = nameless_parameters[0];
            ow["slow_period"] = nameless_parameters[1];
            ow["signal_period"] = nameless_parameters[2];
            ow["price"] = nameless_parameters[3];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);

        signal = new MACD(ow, sps);
    }
    if (sn == "MACD_SIGNAL")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 2)
        {
            ow["fast_period"] = nameless_parameters[0];
            ow["slow_period"] = nameless_parameters[1];
        }
        if (nameless_parameters.size() == 3)
        {
            ow["fast_period"] = nameless_parameters[0];
            ow["slow_period"] = nameless_parameters[1];
            ow["signal_period"] = nameless_parameters[2];
        }
        if (nameless_parameters.size() == 4)
        {
            ow["fast_period"] = nameless_parameters[0];
            ow["slow_period"] = nameless_parameters[1];
            ow["signal_period"] = nameless_parameters[2];
            ow["price"] = nameless_parameters[3];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);

        signal = new MACD_SIGNAL(ow, sps);
    }
    if (sn == "MACD_HIST")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 2)
        {
            ow["fast_period"] = nameless_parameters[0];
            ow["slow_period"] = nameless_parameters[1];
        }
        if (nameless_parameters.size() == 3)
        {
            ow["fast_period"] = nameless_parameters[0];
            ow["slow_period"] = nameless_parameters[1];
            ow["signal_period"] = nameless_parameters[2];
        }
        if (nameless_parameters.size() == 4)
        {
            ow["fast_period"] = nameless_parameters[0];
            ow["slow_period"] = nameless_parameters[1];
            ow["signal_period"] = nameless_parameters[2];
            ow["price"] = nameless_parameters[3];
        }

        // must have 1 double signal parameter
        ASS(sps.size()==1);

        signal = new MACD_HIST(ow, sps);
    }

    if (sn == "BBANDS_UPPER")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 2)
        {
            ow["period"] = nameless_parameters[0];
            ow["dev_up"] = nameless_parameters[1];
            ow["dev_down"] = nameless_parameters[1];
        }
        if (nameless_parameters.size() == 3)
        {
            ow["price"] = nameless_parameters[0];
            ow["period"] = nameless_parameters[1];
            ow["dev_up"] = nameless_parameters[2];
            ow["dev_down"] = nameless_parameters[2];
        }
        if (nameless_parameters.size() == 4)
        {
            ow["price"] = nameless_parameters[0];
            ow["period"] = nameless_parameters[1];
            ow["dev_up"] = nameless_parameters[2];
            ow["dev_down"] = nameless_parameters[3];
        }

        signal = new BBANDS_UPPER(ow, sps);
    }
    if (sn == "BBANDS_MIDDLE")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 2)
        {
            ow["period"] = nameless_parameters[0];
            ow["dev_up"] = nameless_parameters[1];
            ow["dev_down"] = nameless_parameters[1];
        }
        if (nameless_parameters.size() == 3)
        {
            ow["price"] = nameless_parameters[0];
            ow["period"] = nameless_parameters[1];
            ow["dev_up"] = nameless_parameters[2];
            ow["dev_down"] = nameless_parameters[2];
        }
        if (nameless_parameters.size() == 4)
        {
            ow["price"] = nameless_parameters[0];
            ow["period"] = nameless_parameters[1];
            ow["dev_up"] = nameless_parameters[2];
            ow["dev_down"] = nameless_parameters[3];
        }

        signal = new BBANDS_MIDDLE(ow, sps);
    }
    if (sn == "BBANDS_LOWER")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 2)
        {
            ow["period"] = nameless_parameters[0];
            ow["dev_up"] = nameless_parameters[1];
            ow["dev_down"] = nameless_parameters[1];
        }
        if (nameless_parameters.size() == 3)
        {
            ow["price"] = nameless_parameters[0];
            ow["period"] = nameless_parameters[1];
            ow["dev_up"] = nameless_parameters[2];
            ow["dev_down"] = nameless_parameters[2];
        }
        if (nameless_parameters.size() == 4)
        {
            ow["price"] = nameless_parameters[0];
            ow["period"] = nameless_parameters[1];
            ow["dev_up"] = nameless_parameters[2];
            ow["dev_down"] = nameless_parameters[3];
        }

        signal = new BBANDS_LOWER(ow, sps);
    }
    if (sn == "AROON_UP")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 1)
        {
            ow["period"] = nameless_parameters[0];
        }
        if (nameless_parameters.size() == 2)
        {
            ow["period"]  = nameless_parameters[0];
            ow["bars_back"] = nameless_parameters[1];
        }

        signal = new AROON_UP(ow, sps);
    }
    if (sn == "AROON_DOWN")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 1)
        {
            ow["period"] = nameless_parameters[0];
        }
        if (nameless_parameters.size() == 2)
        {
            ow["period"]  = nameless_parameters[0];
            ow["bars_back"] = nameless_parameters[1];
        }

        signal = new AROON_DOWN(ow, sps);
    }
    if (sn == "AROON_OSC")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 1)
        {
            ow["period"] = nameless_parameters[0];
        }
        if (nameless_parameters.size() == 2)
        {
            ow["period"]  = nameless_parameters[0];
            ow["bars_back"] = nameless_parameters[1];
        }

        signal = new AROON_OSC(ow, sps);
    }
    if (sn == "STOCH_K")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 3)
        {
            ow["fast_k_period"] = nameless_parameters[0];
            ow["slow_k_period"] = nameless_parameters[1];
            ow["slow_d_period"] = nameless_parameters[2];
        }
        if (nameless_parameters.size() == 4)
        {
            ow["fast_k_period"] = nameless_parameters[0];
            ow["slow_k_period"] = nameless_parameters[1];
            ow["slow_d_period"] = nameless_parameters[2];
            ow["bars_back"] = nameless_parameters[3];
        }

        signal = new STOCH_K(ow, sps);
    }
    if (sn == "STOCH_D")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 3)
        {
            ow["fast_k_period"] = nameless_parameters[0];
            ow["slow_k_period"] = nameless_parameters[1];
            ow["slow_d_period"] = nameless_parameters[2];
        }
        if (nameless_parameters.size() == 4)
        {
            ow["fast_k_period"] = nameless_parameters[0];
            ow["slow_k_period"] = nameless_parameters[1];
            ow["slow_d_period"] = nameless_parameters[2];
            ow["bars_back"] = nameless_parameters[3];
        }

        signal = new STOCH_D(ow, sps);
    }
    if (sn == "WILLR")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 1)
        {
            ow["period"] = nameless_parameters[0];
        }
        if (nameless_parameters.size() == 2)
        {
            ow["period"]  = nameless_parameters[0];
            ow["bars_back"] = nameless_parameters[1];
        }

        signal = new WILLR(ow, sps);
    }
    if (sn == "SAR")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 2)
        {
            ow["accel"] = nameless_parameters[0];
        }
        if (nameless_parameters.size() == 2)
        {
            ow["accel"]  = nameless_parameters[0];
            ow["max"] = nameless_parameters[1];
        }
        if (nameless_parameters.size() == 3)
        {
            ow["accel"]  = nameless_parameters[0];
            ow["max"] = nameless_parameters[1];
            ow["bars_back"] = nameless_parameters[2];
        }

        signal = new SAR(ow, sps);
    }
    if (sn == "ULTOSC")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 3)
        {
            ow["period1"] = nameless_parameters[0];
            ow["period2"] = nameless_parameters[1];
            ow["period3"] = nameless_parameters[2];
        }
        if (nameless_parameters.size() == 4)
        {
            ow["period1"] = nameless_parameters[0];
            ow["period2"] = nameless_parameters[1];
            ow["period3"] = nameless_parameters[2];
            ow["bars_back"] = nameless_parameters[3];
        }

        signal = new ULTOSC(ow, sps);
    }
    if (sn == "TSF")
    {
        // the unnamed parameters - price, period or just period
        if (nameless_parameters.size() == 1)
        {
            ow["period"] = nameless_parameters[0];
        }
        if (nameless_parameters.size() == 2)
        {
            ow["period"]  = nameless_parameters[0];
            ow["bars_back"] = nameless_parameters[1];
        }

        signal = new TSF(ow, sps);
    }
    if ((sn == "HEIKIN_ASHI_PRICE") ||
    	(sn == "HA_PRICE"))
	{
    	// can have 1 or 2 nameless parameters
		if (nameless_parameters.size() == 1)
		{
			ow["price"] = nameless_parameters[0];
		}

		signal = new HeikinAshiPrice(ow, sps);
	}

	//////////////////////////////////
	// Others
	//////////////////////////////////
	if ((sn == "SUPERTREND_UP") || (sn == "SUPER_TREND_UP"))
	{
		// the unnamed parameters - price, period or just period
		if (nameless_parameters.size() == 3)
		{
			ow["atr_mul"] = nameless_parameters[0];
			ow["atr_period"] = nameless_parameters[1];
			ow["median_period"] = nameless_parameters[2];
		}

		signal = new SuperTrendUp(ow, sps);
	}
	if ((sn == "SUPERTREND_DOWN") || (sn == "SUPER_TREND_DOWN"))
	{
		// the unnamed parameters - price, period or just period
		if (nameless_parameters.size() == 3)
		{
			ow["atr_mul"] = nameless_parameters[0];
			ow["atr_period"] = nameless_parameters[1];
			ow["median_period"] = nameless_parameters[2];
		}

		signal = new SuperTrendDown(ow, sps);
	}

	/////////////////////////////////////
	// Candlesticks
	CDL_SIGNAL_RECOG(CDL_2CROWS);
	CDL_SIGNAL_RECOG(CDL_3BLACKCROWS);
	CDL_SIGNAL_RECOG(CDL_3INSIDE);
	CDL_SIGNAL_RECOG(CDL_3LINESTRIKE);
	CDL_SIGNAL_RECOG(CDL_3OUTSIDE);
	CDL_SIGNAL_RECOG(CDL_3STARSINSOUTH);
	CDL_SIGNAL_RECOG(CDL_3WHITESOLDIERS);
//	CDL_SIGNAL_RECOG(CDL_ABANDONEDBABY);
	CDL_SIGNAL_RECOG(CDL_ADVANCEBLOCK);
	CDL_SIGNAL_RECOG(CDL_BELTHOLD);
	CDL_SIGNAL_RECOG(CDL_BREAKAWAY);
	CDL_SIGNAL_RECOG(CDL_CLOSINGMARUBOZU);
	CDL_SIGNAL_RECOG(CDL_CONCEALBABYSWALL);
	CDL_SIGNAL_RECOG(CDL_COUNTERATTACK);
//	CDL_SIGNAL_RECOG(CDL_DARKCLOUDCOVER);
	CDL_SIGNAL_RECOG(CDL_DOJI);
	CDL_SIGNAL_RECOG(CDL_DOJISTAR);
	CDL_SIGNAL_RECOG(CDL_DRAGONFLYDOJI);
	CDL_SIGNAL_RECOG(CDL_ENGULFING);
//	CDL_SIGNAL_RECOG(CDL_EVENINGDOJISTAR);
//	CDL_SIGNAL_RECOG(CDL_EVENINGSTAR);
	CDL_SIGNAL_RECOG(CDL_GAPSIDESIDEWHITE);
	CDL_SIGNAL_RECOG(CDL_GRAVESTONEDOJI);
	CDL_SIGNAL_RECOG(CDL_HAMMER);
	CDL_SIGNAL_RECOG(CDL_HANGINGMAN);
	CDL_SIGNAL_RECOG(CDL_HARAMI);
	CDL_SIGNAL_RECOG(CDL_HARAMICROSS);
	CDL_SIGNAL_RECOG(CDL_HIGHWAVE);
	CDL_SIGNAL_RECOG(CDL_HIKKAKE);
	CDL_SIGNAL_RECOG(CDL_HIKKAKEMOD);
	CDL_SIGNAL_RECOG(CDL_HOMINGPIGEON);
	CDL_SIGNAL_RECOG(CDL_IDENTICAL3CROWS);
	CDL_SIGNAL_RECOG(CDL_INNECK);
	CDL_SIGNAL_RECOG(CDL_INVERTEDHAMMER);
	CDL_SIGNAL_RECOG(CDL_KICKING);
	CDL_SIGNAL_RECOG(CDL_KICKINGBYLENGTH);
	CDL_SIGNAL_RECOG(CDL_LADDERBOTTOM);
	CDL_SIGNAL_RECOG(CDL_LONGLEGGEDDOJI);
	CDL_SIGNAL_RECOG(CDL_LONGLINE);
	CDL_SIGNAL_RECOG(CDL_MARUBOZU);
	CDL_SIGNAL_RECOG(CDL_MATCHINGLOW);
//	CDL_SIGNAL_RECOG(CDL_MATHOLD);
//	CDL_SIGNAL_RECOG(CDL_MORNINGDOJISTAR);
//	CDL_SIGNAL_RECOG(CDL_MORNINGSTAR);
	CDL_SIGNAL_RECOG(CDL_ONNECK);
	CDL_SIGNAL_RECOG(CDL_PIERCING);
	CDL_SIGNAL_RECOG(CDL_RICKSHAWMAN);
	CDL_SIGNAL_RECOG(CDL_RISEFALL3METHODS);
	CDL_SIGNAL_RECOG(CDL_SEPARATINGLINES);
	CDL_SIGNAL_RECOG(CDL_SHOOTINGSTAR);
	CDL_SIGNAL_RECOG(CDL_SHORTLINE);
	CDL_SIGNAL_RECOG(CDL_SPINNINGTOP);
	CDL_SIGNAL_RECOG(CDL_STALLEDPATTERN);
	CDL_SIGNAL_RECOG(CDL_STICKSANDWICH);
	CDL_SIGNAL_RECOG(CDL_TAKURI);
	CDL_SIGNAL_RECOG(CDL_TASUKIGAP);
	CDL_SIGNAL_RECOG(CDL_THRUSTING);
	CDL_SIGNAL_RECOG(CDL_TRISTAR);
	CDL_SIGNAL_RECOG(CDL_UNIQUE3RIVER);
	CDL_SIGNAL_RECOG(CDL_UPSIDEGAP2CROWS);
	CDL_SIGNAL_RECOG(CDL_XSIDEGAP3METHODS);

#endif
    
	// END
	///////////////////////////////////////////

    if (signal == NULL)
    {
        // no such signal or variable was found
        throw std::runtime_error("Signal name \"" + sn + "\" not recognized and no such variable exists.");
    }

    return signal;
}




