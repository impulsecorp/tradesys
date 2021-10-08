#include "base_signal.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#define CODEGEN_ARRAY_MAX_SIZE 16384


unsigned long g_global_signal_id = 0;
int g_global_price_preference = PRICE_PREF_CLOSE; // 0 - OPEN, 1 - CLOSE

Signal::Signal()
{
    // Assign a unique ID and increase it
    m_id = g_global_signal_id;
    m_parent_strategy = NULL;
    m_parent_multistrategy = NULL;
    m_return_type = RT_BOOL;
    prev_idx = 0;

    // TODO: make this thread-safe
    g_global_signal_id++;

    // init the EL limits - variables used by the signals to represent 
    // missing value
    limits["min"] = "-999999";
    limits["max"] = "999999";

    init_codegen_outp();
}

Signal::~Signal()
{
    // delete the children as well
    /*for (unsigned int i = 0; i < m_children.size(); i++)
    {
        delete m_children[i];
    }*/
}

void Signal::init_defaults(ParameterMap &a_overridden)
{
    // Go through the map of constraints for this signal and assign everything to be default
    for (ParameterConstraintMap::iterator it = GlobalConstraints[m_name].begin();
         it != GlobalConstraints[m_name].end(); it++)
    {
        m_parameters[it->first] = (it->second).default_value;
    }

    // No go through the map of overrides
    for (ParameterMap::iterator it = a_overridden.begin();
         it != a_overridden.end(); it++)
    {
        // If this is an ELI, record it first
        if ((it->second).contents.type() == typeid(ELI))
        {
            m_ELIs[it->first] = get<ELI>((it->second).contents);
            m_parameters[it->first] = m_ELIs[it->first].value;
        }
        else
        {
            // not an ELI
            // convert from SP to SP type
            m_parameters[it->first] = SP2SPType(it->second);
        }
    }
}

ParameterConstraintMap Signal::get_initial_constraints()
{
    ParameterConstraintMap empty;
    return empty;
}

void Signal::add_child(Signal *a_node)
{
    if (a_node != NULL)
    {
        m_children.push_back(a_node);
    }
    else
    {
        throw std::runtime_error(STR("Tried to add NULL as child"));
    }
}

void Signal::clear_children()
{
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        delete m_children[i];
    }
    m_children.clear();
}

// Fills a linear list of this signal including all children down the tree
void Signal::fill_children_list(std::vector<Signal *> &a_list)
{
    a_list.push_back(this);
    
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        m_children[i]->fill_children_list(a_list);
    }
}

// Returns a linear list of this signal including all children down the tree
std::vector<Signal *> Signal::flatten_tree()
{
    std::vector<Signal *> t;
    fill_children_list(t);
    return t;
}

void Signal::update_data_recursive(const DataMatrix &data)
{
    // if there are any children, just update their data
    // signals will call the derived method
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        m_children[i]->update_data_recursive(data);
    }

    this->update_data(data);
}

void Signal::update_mdata_recursive(const MultiDataMatrix &mdata)
{
    // if there are any children, just update their data
    // signals will call the derived method
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        m_children[i]->update_mdata_recursive(mdata);
    }

    // use the parameter named "data" to retrieve an integer index,
    // then call update_data on mdata.symbols[idx].
    // it corresponds to an index for the mdata.symbols vector.
    int idx = 0;
    if (m_parameters.find("data") == m_parameters.end())
    {
        // key not found
        idx = 0;
    }
    else
    {
        idx = get<int>(m_parameters["data"]);
    }
    // call update_data with the single DataMatrix
    this->update_data(mdata.symbols[idx]);
}

void Signal::update_mdata(const MultiDataMatrix &mdata)
{
    // use the parameter named "data" to retrieve an integer index,
    // then call update_data on mdata.symbols[idx].
    // it corresponds to an index for the mdata.symbols vector.
    int idx = 0;
    if (m_parameters.find("data") == m_parameters.end())
    {
        // key not found
        idx = 0;
    }
    else
    {
        idx = get<int>(m_parameters["data"]);
    }
    // call update_data with the single DataMatrix
    this->update_data(mdata.symbols[idx]);
}



void Signal::update_parent_strategy_recursive(SignalTreeStrategy *a_parent)
{
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        m_children[i]->update_parent_strategy_recursive(a_parent);
    }

    m_parent_strategy = a_parent;
}

void Signal::update_mparent_strategy_recursive(SignalTreeMultiStrategy *a_parent)
{
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        m_children[i]->update_mparent_strategy_recursive(a_parent);
    }

    m_parent_multistrategy = a_parent;
}

void Signal::reset_parent()
{
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        m_children[i]->reset_parent();
    }

    m_parent_strategy = NULL;
}

int Signal::get_max_bars_back()
{
    int max = 0;

    // if it has children, check them out first
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        int x = m_children[i]->get_max_bars_back();
        if (x > max)
        {
            max = x;
        }
    }

    // now check if the current parameters have something even higher
    for (ExplicitParameterMap::iterator it = m_parameters.begin();
         it != m_parameters.end(); it++)
    {
        // checks for "bars_back", "period" or "N" in the name
        if (((it->first) == "bars_back") ||
            ((it->first) == "bb") ||
            ((it->first).find("period") != std::string::npos) /*||
                                                              ((it->first) == "n") ||
                                                              ((it->first) == "x")*/)
        {
            if ((it->second).type() != typeid(int))
            {
                throw std::runtime_error("bb/bars_back and period parameters must be int");
            }

            if (get<int>(it->second) > (int) max)
            {
                max = get<int>(it->second);
            }
        }
    }

    return max; // worst case - returns 0
}


void Signal::compute_init_recursive(const Env &env, int idx)
{
    // children first
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        m_children[i]->compute_init_recursive(env, idx);
    }

    this->compute_init(env, idx);
}


void Signal::compute_minit(const std::vector<Env> &env, const std::vector<int> &idx)
{
    int i = 0;
    if (m_parameters.find("data") == m_parameters.end())
    {
        // key not found
        i = 0;
    }
    else
    {
        i = get<int>(m_parameters["data"]);
    }
    
    this->compute_init(env[i], idx[i]);
}


void Signal::compute_minit_recursive(const std::vector<Env> &env, const std::vector<int> &idx)
{
    // children first
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        m_children[i]->compute_minit_recursive(env, idx);
    }

    int i = 0;
    if (m_parameters.find("data") == m_parameters.end())
    {
        // key not found
        i = 0;
    }
    else
    {
        i = get<int>(m_parameters["data"]);
    }

    this->compute_init(env[i], idx[i]);
}


void Signal::compute_tick_recursive(const Env &env, int idx)
{
    // children first
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        m_children[i]->compute_tick_recursive(env, idx);
    }

    this->compute_tick(env, idx);
}

void Signal::compute_mtick(const std::vector<Env> &env, const std::vector<int> &idx)
{
    int i = 0;
    if (m_parameters.find("data") == m_parameters.end())
    {
        // key not found
        i = 0;
        //std::cout << "Did not find data parameter in " << m_name << "\n";
    }
    else
    {
        i = get<int>(m_parameters["data"]);
    }
    
    this->compute_tick(env[i], idx[i]);
}

void Signal::compute_mtick_recursive(const std::vector<Env> &env, const std::vector<int> &idx)
{
    // children first
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        m_children[i]->compute_mtick_recursive(env, idx);
    }

    int i = 0;
    if (m_parameters.find("data") == m_parameters.end())
    {
        // key not found
        i = 0;
        //std::cout << "Did not find data parameter in " << m_name << "\n";
    }
    else
    {
        i = get<int>(m_parameters["data"]);
    }

    compute_tick(env[i], idx[i]);
}

std::string Signal::print_str()
{
    return m_name + print_parameters();
}


std::string Signal::print_parameters()
{
    std::stringstream s;
    unsigned int count = 0;

    s << "(";

    for (ExplicitParameterMap::iterator it = m_parameters.begin();
         it != m_parameters.end(); it++)
    {
        std::string name = it->first;
        SignalParameter val;
        val = SPType2SP(it->second);

        if (count > 0) // add comma separator
        {
            s << ", ";
        }

        // determine if the key in cparams is also present in the ELIs
        bool eli = false;
        if (m_ELIs.count(it->first) > 0)
        {
            // key exists
            eli = true;
            val = m_ELIs[it->first];
        }

        s << name << "=";

        // it's an ELI
        if (eli)
        {
            s << "['" << (get<ELI>(val.contents)).name << "', ";

            if ((get<ELI>(val.contents)).value.type() == typeid(int))
            {
                s << get<int>((get<ELI>(val.contents)).value);
            }
            if ((get<ELI>(val.contents)).value.type() == typeid(double))
            {
                s << format("%3.14f") % get<double>((get<ELI>(val.contents)).value);
            }
            if ((get<ELI>(val.contents)).value.type() == typeid(bool))
            {
                std::string t;
                if ((get<bool>((get<ELI>(val.contents)).value)) == true)
                    t = "true";
                else
                    t = "false";
                s << t;
            }
            if ((get<ELI>(val.contents)).value.type() == typeid(std::string))
            {
                s << "'" << get<std::string>((get<ELI>(val.contents)).value) << "'";
            }
            if ((get<ELI>(val.contents)).value.type() == typeid(Time))
            {
                Time t = get<Time>((get<ELI>(val.contents)).value);
                if (t.second == 0)
                {
                    s << format("%02d") % t.hour << ":"
                    << format("%02d") % t.minute;
                }
                else
                {
                    s << format("%02d") % t.hour << ":"
                    << format("%02d") % t.minute << ":"
                    << format("%02d") % t.second;
                }
            }

            s << "]";
        }
        else
            // not an ELI
        {
            if (val.contents.type() == typeid(int))
            {
                s << get<int>(val.contents);
            }
            if (val.contents.type() == typeid(double))
            {
                s << format("%3.14f") % get<double>(val.contents);
            }
            if (val.contents.type() == typeid(bool))
            {
                std::string t;
                if ((get<bool>(val.contents)) == true)
                    t = "true";
                else
                    t = "false";
                s << t;
            }
            if (val.contents.type() == typeid(std::string))
            {
                s << "'" << get<std::string>(val.contents) << "'";
            }
            if (val.contents.type() == typeid(Time))
            {
                Time t = get<Time>(val.contents);
                if (t.second == 0)
                {
                    s << format("%02d") % t.hour << ":"
                    << format("%02d") % t.minute;
                }
                else
                {
                    s << format("%02d") % t.hour << ":"
                    << format("%02d") % t.minute << ":"
                    << format("%02d") % t.second;
                }
            }
        }

        count++;
    }

    // Print all children
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        if (count > 0) // add comma separator
        {
            s << ", ";
        }

        // RECURSION POINT
        s << m_children[i]->print_str();

        count++;
    }

    s << ")";

    // print the children signals first, if any
    return s.str();
}

py::dict Signal::get_parameters(bool a_only_mutable = true)
{
    py::dict d;
    for (ExplicitParameterMap::iterator it = m_parameters.begin();
         it != m_parameters.end(); it++)
    {
        std::string name = it->first;
        SP_type val = it->second;

        if ((a_only_mutable && GlobalConstraints[m_name][it->first].is_mutable) || (!a_only_mutable))
        {
            if (val.type() == typeid(int))
            {
                d[name] = get<int>(val);
            }
            if (val.type() == typeid(double))
            {
                d[name] = get<double>(val);
            }
            if (val.type() == typeid(bool))
            {
                d[name] = get<bool>(val);
            }
            if (val.type() == typeid(std::string))
            {
                d[name] = get<std::string>(val);
            }
            if (val.type() == typeid(Time))
            {
                d[name] = get<Time>(val);
            }
        }
    }

    return d;
}

void Signal::set_parameters(py::dict a_parameters)
{
    py::list keys = a_parameters.keys();
    ssize_t lk = len(keys);
    for (unsigned int i = 0; i < lk; i++)
    {
        std::string key = py::extract<std::string>(keys[i]);

        // check if that key exists in the parameters, skip if not
        if (m_parameters.count(key) > 0)
        {
            // get the object
            py::object o = py::extract<py::object>(a_parameters[key]);
            // get its class name
            std::string obj_classname = py::extract<std::string>(o.attr("__class__").attr("__name__"));

            // TODO: convert int/float types

            // this object is an int
            if (obj_classname == "int")
            {
                int v = py::extract<int>(o);
                // clip to min/max
                if (v < get<int>(GlobalConstraints[m_name][key].min))
                    v = get<int>(GlobalConstraints[m_name][key].min);
                if (v > get<int>(GlobalConstraints[m_name][key].max))
                    v = get<int>(GlobalConstraints[m_name][key].max);
                m_parameters[key] = v;
            }
            if (obj_classname == "float")
            {
                double v = py::extract<double>(o);
                // clip to min/max
                if (v < get<double>(GlobalConstraints[m_name][key].min))
                    v = get<double>(GlobalConstraints[m_name][key].min);
                if (v > get<double>(GlobalConstraints[m_name][key].max))
                    v = get<double>(GlobalConstraints[m_name][key].max);
                m_parameters[key] = v;
            }
            if (obj_classname == "bool")
            {
                m_parameters[key] = py::extract<bool>(o);
            }
            if (obj_classname == "str")
            {
                std::string v = py::extract<std::string>(o);
                // make sure the new value is permitted
                // todo
                m_parameters[key] = v;
            }
            if (obj_classname == "Time")
            {
                Time v;
                v = py::extract<Time>(o);
                // clip to min/max
                if (v < get<Time>(GlobalConstraints[m_name][key].min))
                    v = get<Time>(GlobalConstraints[m_name][key].min);
                if (v > get<Time>(GlobalConstraints[m_name][key].max))
                    v = get<Time>(GlobalConstraints[m_name][key].max);
                m_parameters[key] = v;
            }
        }
    }
}

py::dict Signal::get_parameter_constraints(std::string a_parameter_name)
{
    py::dict d;
    SP_constraints cts = GlobalConstraints[m_name][a_parameter_name];

    // fill the dict with the info
    d["usage_mode"] = cts.usage_mode;
    d["is_mutable"] = cts.is_mutable;

    // extract and put the appropriate values

    if (cts.min.type() == typeid(int))
        d["min"] = get<int>(cts.min);
    if (cts.min.type() == typeid(double))
        d["min"] = get<double>(cts.min);
    if (cts.min.type() == typeid(Time))
        d["min"] = get<Time>(cts.min);
    if (cts.max.type() == typeid(int))
        d["max"] = get<int>(cts.max);
    if (cts.max.type() == typeid(double))
        d["max"] = get<double>(cts.max);
    if (cts.max.type() == typeid(Time))
        d["max"] = get<Time>(cts.max);

    if (cts.mut_min.type() == typeid(int))
        d["mut_min"] = get<int>(cts.mut_min);
    if (cts.mut_min.type() == typeid(double))
        d["mut_min"] = get<double>(cts.mut_min);
    if (cts.mut_min.type() == typeid(Time))
        d["mut_min"] = get<Time>(cts.mut_min);
    if (cts.mut_max.type() == typeid(int))
        d["mut_max"] = get<int>(cts.mut_max);
    if (cts.mut_max.type() == typeid(double))
        d["mut_max"] = get<double>(cts.mut_max);
    if (cts.mut_max.type() == typeid(Time))
        d["mut_max"] = get<Time>(cts.mut_max);

    if (cts.mut_power.type() == typeid(int))
        d["mut_power"] = get<int>(cts.mut_power);
    if (cts.mut_power.type() == typeid(double))
        d["mut_power"] = get<double>(cts.mut_power);
    if (cts.mut_power.type() == typeid(Time))
        d["mut_power"] = get<Time>(cts.mut_power);

    if (cts.default_value.type() == typeid(bool))
        d["default_value"] = get<bool>(cts.default_value);
    if (cts.default_value.type() == typeid(int))
        d["default_value"] = get<int>(cts.default_value);
    if (cts.default_value.type() == typeid(double))
        d["default_value"] = get<double>(cts.default_value);
    if (cts.default_value.type() == typeid(Time))
        d["default_value"] = get<Time>(cts.default_value);
    if (cts.default_value.type() == typeid(std::string))
        d["default_value"] = get<std::string>(cts.default_value);

    py::list pv;
    for (unsigned int i = 0; i < cts.possible_values.size(); i++)
    {
        if (cts.possible_values[i].type() == typeid(bool))
            pv.append(get<bool>(cts.possible_values[i]));
        if (cts.possible_values[i].type() == typeid(int))
            pv.append(get<int>(cts.possible_values[i]));
        if (cts.possible_values[i].type() == typeid(double))
            pv.append(get<double>(cts.possible_values[i]));
        if (cts.possible_values[i].type() == typeid(Time))
            pv.append(get<Time>(cts.possible_values[i]));
        if (cts.possible_values[i].type() == typeid(std::string))
            pv.append(get<std::string>(cts.possible_values[i]));
    }
    d["possible_values"] = pv;

    return d;
}

// TODO: Implement those functions below
void Signal::set_parameter_constraints(std::string a_parameter_name,
                                       py::dict a_parameter_constraints)
{
}

py::dict Signal::get_all_constraints()
{
    return py::dict();
}

void Signal::set_all_constraints(py::dict a_all_constraints)
{
}

void Signal::randomize_parameters()
{
}

void Signal::mutate_parameters()
{
}

std::string Signal::var(std::string a_name)
{
    // check if the variable exists either in the variables or cparams maps
    bool problem = true;
    if (m_variables.count(a_name) > 0)
    {
        problem = false;
    }
    if (m_parameters.count(a_name) > 0)
    {
        problem = false;
    }

    if (problem)
    {
        throw std::runtime_error("VariableSignal \"" + a_name + "\" not found in signal " + m_name);
    }

    std::string name = var_name(a_name);

    // when the variable referred to is a time-based parameter
    if (m_parameters.count(a_name) > 0)
    {
        if (m_parameters[a_name].type() == typeid(Time))
        {
            std::stringstream t;
            t << format("(%s_H * 3600 + %s_M*60 + %s_S)") % name % name % name;
            name = t.str();
        }
    }

    return name;
}

std::string Signal::var_name(std::string a_name)
{
    std::stringstream vname;
    std::string name;

    vname << a_name + "_" << m_id;
    name = vname.str();
    return name;
}

std::vector<ELI> Signal::get_ELIs()
{
    std::vector<ELI> elis;

    for (std::map<std::string, ELI>::iterator it = m_ELIs.begin();
         it != m_ELIs.end();
         it++)
    {
        elis.push_back(it->second);
    }
    return elis;
}

void Signal::fill_ELI_list(std::vector<ELI> &a_eli_list)
{
    // if there are children, add them first
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        m_children[i]->fill_ELI_list(a_eli_list);
    }

    // add self
    std::vector<ELI> elis;
    elis = get_ELIs();

    for (unsigned int i = 0; i < elis.size(); i++)
    {
        a_eli_list.push_back(elis[i]);
    }
}

void Signal::clear_ELIs()
{
    // if there are children, clear them first
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        m_children[i]->clear_ELIs();
    }

    m_ELIs.clear();
}

std::string Signal::el_cur_time()
{
#ifndef MULTICHARTS_COMPATIBLE
    return "(cur_datetime.Hour*3600 + cur_datetime.Minute*60 + cur_datetime.Second)";
#else
    return "((Time_s-(Mod(Time_s , 10000)))/10000)*60*60 + (((Mod(Time_s , 10000)) - (Mod(Time_s , 100)))/100)*60 + (Mod(Time_s , 100))";
#endif
}


std::string Signal::get_el_init()
{
    return "";
}

std::string Signal::get_el_init_recursive()
{
    // merge all children's output
    std::string s;
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        std::string r;
        r = m_children[i]->get_el_init_recursive();
        if (r != "")
        {
            s += r + "\n";
        }
    }
    return s + this->get_el_init();
}

// default
std::string Signal::get_el_tick()
{
    return "";
}

std::string Signal::get_el_tick_recursive()
{
    // merge all children's output
    std::string s;
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        std::string r;
        r = m_children[i]->get_el_tick_recursive();
        if (r != "")
        {
            s += r + "\n";
        }
    }

    return s + this->get_el_tick();
}


std::string Signal::get_el_var_decl()
{
    std::stringstream s;

    // get all children's variables first
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        s << m_children[i]->get_el_var_decl();
    }

    // iterate through the variable map
    for (std::map<std::string, VariableType>::iterator it = m_variables.begin();
         it != m_variables.end(); it++)
    {
        // the variable type is an array 
        if (((it->second) == VAR_TYPE_ARRAY_INT) ||
            ((it->second) == VAR_TYPE_ARRAY_FLOAT) ||
            ((it->second) == VAR_TYPE_ARRAY_BOOL))
        {
            s << "array: " << var(it->first) << "[]";

            if (((it->second) == VAR_TYPE_ARRAY_INT) ||
                ((it->second) == VAR_TYPE_ARRAY_FLOAT))
            {
                s << "(0); ";
            }
            else
            {
                s << "(false); ";
            }
        }
        else
        {
            // not an array
            s << "variable: " << var(it->first);
            if ((it->second) == VAR_TYPE_BOOL)
            {
                s << "(false); ";
            }
            else
            {
                s << "(0); ";
            }
        }
    }

    // now add the existing values in cparams as additional variables
    // every parameter must be inside a variable
    for (ExplicitParameterMap::iterator it = m_parameters.begin();
         it != m_parameters.end(); it++)
    {
        std::string vname = var_name(it->first);
        SignalParameter val;
        val = SPType2SP(it->second);

        // determine if the key in cparams is also present in the ELIs
        bool eli = false;
        if (m_ELIs.count(it->first) > 0)
        {
            // key exists
            eli = true;
            val = m_ELIs[it->first];
        }

        // it's an ELI 
        if (eli)
        {
            if ((get<ELI>(val.contents)).value.type() == typeid(Time))
            {
                // times are split to H,M,S integers
                s << format("variable: %s(%s); "
                                    "variable: %s(%s); "
                                    "variable: %s(%s); ") %
                     (vname + "_H") % ((get<ELI>(val.contents)).name + "_H") %
                     (vname + "_M") % ((get<ELI>(val.contents)).name + "_M") %
                     (vname + "_S") % ((get<ELI>(val.contents)).name + "_S");
            }
            else
            {
                s << format("variable: %s(%s); ") % vname % (get<ELI>(val.contents)).name;
            }
        }
        else
        {
            // a regular variable

            if (val.contents.type() == typeid(Time))
            {
                // times are split to H,M,S integers
                s << format(
                        "variable: %s(%d); "
                                "variable: %s(%d); "
                                "variable: %s(%d); ") %
                     (vname + "_H") % (get<Time>(val.contents).hour) %
                     (vname + "_M") % (get<Time>(val.contents).minute) %
                     (vname + "_S") % (get<Time>(val.contents).second);
            }
            else
            {
                if (val.contents.type() == typeid(std::string))
                {
                    s << format("variable: %s(\"%s\"); ") % vname % get<std::string>(val.contents);
                }
                else
                {
                    if (val.contents.type() == typeid(bool))
                    {
                        s <<
                        format("variable: %s(%s); ") % vname % ((get<bool>(val.contents) == true) ? "true" : "false");
                    }
                    else
                    {
                        if (val.contents.type() == typeid(int))
                        {
                            s << format("variable: %s(%d); ") % vname % get<int>(val.contents);
                        }
                        else
                        {
                            if (val.contents.type() == typeid(double))
                            {
                                s << format("variable: %s(%3.14f); ") % vname % get<double>(val.contents);
                            }
                            else
                            {
                                throw std::runtime_error("Unrecognized parameter/variable type - '" + vname + "'");
                            }
                        }
                    }
                }
            }
        }
    }

    return s.str();
}

std::string Signal::mql_cur_time()
{
    return "(TimeHour(Time[0])*3600 + TimeMinute(Time[0])*60)";
}

std::string Signal::get_mql_init()
{
    return "";
}

std::string Signal::get_mql_init_recursive()
{
    // merge all children's output
    std::string s;
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        std::string r;
        r = m_children[i]->get_mql_init_recursive();
        if (r != "")
        {
            s += r + "\n";
        }
    }
    return s + this->get_mql_init();
}

// default
std::string Signal::get_mql_tick()
{
    return "";
}

std::string Signal::get_mql_tick_recursive()
{
    // merge all children's output
    std::string s;
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        std::string r;
        r = m_children[i]->get_mql_tick_recursive();
        if (r != "")
        {
            s += r + "\n";
        }
    }
    return s + this->get_mql_tick();
}

std::string Signal::get_mql_var_decl()
{
    std::stringstream s;

    // get all children's variables first
    for (unsigned int i = 0; i < m_children.size(); i++)
    {
        s << m_children[i]->get_mql_var_decl();
    }

    // iterate through the variable map
    for (std::map<std::string, VariableType>::iterator it = m_variables.begin();
         it != m_variables.end(); it++)
    {
        // the variable type is an array 
        if (((it->second) == VAR_TYPE_ARRAY_INT) ||
            ((it->second) == VAR_TYPE_ARRAY_FLOAT) ||
            ((it->second) == VAR_TYPE_ARRAY_BOOL))
        {
            if ((it->second) == VAR_TYPE_ARRAY_INT)
            {
                s << "int " << var(it->first) << "[" << /*get_max_bars_back() + 5*/ CODEGEN_ARRAY_MAX_SIZE << "]; ";
            }
            if ((it->second) == VAR_TYPE_ARRAY_FLOAT)
            {
                s << "double " << var(it->first) << "[" << /*get_max_bars_back() + 5*/ CODEGEN_ARRAY_MAX_SIZE << "]; ";
            }
            if ((it->second) == VAR_TYPE_ARRAY_BOOL)
            {
                s << "bool " << var(it->first) << "[" << /*get_max_bars_back() + 5*/ CODEGEN_ARRAY_MAX_SIZE << "]; ";
            }

            // no good init for MQL4 arrays yet
            // TODO
        }
        else
        {
            // not an array
            if ((it->second) == VAR_TYPE_INT)
            {
                s << "int " << var(it->first) << " = 0; ";
            }
            if ((it->second) == VAR_TYPE_FLOAT)
            {
                s << "double " << var(it->first) << " = 0.0; ";
            }
            if ((it->second) == VAR_TYPE_BOOL)
            {
                s << "bool " << var(it->first) << " = false; ";
            }
        }
    }

    // now add the existing values in cparams as additional variables
    // every parameter must be inside a variable
    for (ExplicitParameterMap::iterator it = m_parameters.begin();
         it != m_parameters.end(); it++)
    {
        std::string vname = var_name(it->first);
        SignalParameter val;
        val = SPType2SP(it->second);

        // determine if the key in cparams is also present in the ELIs
        bool eli = false;
        if (m_ELIs.count(it->first) > 0)
        {
            // key exists
            eli = true;
            val = m_ELIs[it->first];
        }

        // it's an ELI 
        if (eli)
        {
            if ((get<ELI>(val.contents)).value.type() == typeid(Time))
            {
                // times are split to H,M,S integers
                s << format(
                        "int %s = %s; "
                                "int %s = %s; "
                                "int %s = %s; ") %
                     (vname + "_H") % ((get<ELI>(val.contents)).name + "_H") %
                     (vname + "_M") % ((get<ELI>(val.contents)).name + "_M") %
                     (vname + "_S") % ((get<ELI>(val.contents)).name + "_S");
            }
            else
            {
                if ((get<ELI>(val.contents)).value.type() == typeid(std::string))
                {
                    s << format("string %s = %s; ") % vname % (get<ELI>(val.contents)).name;
                }
                else
                {
                    if ((get<ELI>(val.contents)).value.type() == typeid(bool))
                    {
                        s << format("bool %s = %s; ") % vname % (get<ELI>(val.contents)).name;
                    }
                    else
                    {
                        if ((get<ELI>(val.contents)).value.type() == typeid(int))
                        {
                            s << format("int %s = %s; ") % vname % (get<ELI>(val.contents)).name;
                        }
                        else
                        {
                            if ((get<ELI>(val.contents)).value.type() == typeid(double))
                            {
                                s << format("double %s = %s; ") % vname % (get<ELI>(val.contents)).name;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            // a regular variable

            if (val.contents.type() == typeid(Time))
            {
                // times are split to H,M,S integers
                s << format(
                        "int %s = %d; "
                                "int %s = %d; "
                                "int %s = %d; ") %
                     (vname + "_H") % (get<Time>(val.contents).hour) %
                     (vname + "_M") % (get<Time>(val.contents).minute) %
                     (vname + "_S") % (get<Time>(val.contents).second);
            }
            else
            {
                if (val.contents.type() == typeid(std::string))
                {
                    s << format("string %s =\"%s\"; ") % vname % get<std::string>(val.contents);
                }
                else
                {
                    if (val.contents.type() == typeid(bool))
                    {
                        s << format("bool %s = %s; ") % vname % ((get<bool>(val.contents) == true) ? "true" : "false");
                    }
                    else
                    {
                        if (val.contents.type() == typeid(int))
                        {
                            s << format("int %s = %d; ") % vname % get<int>(val.contents);
                        }
                        else
                        {
                            if (val.contents.type() == typeid(double))
                            {
                                s << format("double %s = %3.6f; ") % vname % get<double>(val.contents);
                            }
                            else
                            {
                                throw std::runtime_error("Unrecognized parameter/variable type - '" + vname + "'");
                            }
                        }
                    }
                }
            }
        }
    }

    return s.str();
}


#pragma clang diagnostic pop

void Signal::init_codegen_outp()
{
    /*for(unsigned int i=0; i<m_children.size(); i++)
    {
        m_children[i]->init_codegen_outp();
    }*/

    // make the variable outp
    if (m_return_type == RT_BOOL)
    {
        m_variables["outp"] = VAR_TYPE_BOOL;
    }
    else
    if (m_return_type == RT_INT)
    {
        m_variables["outp"] = VAR_TYPE_INT;
    }
    else
    if (m_return_type == RT_FLOAT)
    {
        m_variables["outp"] = VAR_TYPE_FLOAT;
    }
    else
    {
        throw std::runtime_error("Signal " + m_name + " doesn't have a return type.");
    }
}
