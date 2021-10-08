#include "signal_tree.h"

std::string make_EL(std::string a_strategy)
{
    SignalTreeStrategy st(a_strategy);
    return st.print_EasyLanguage();
}

std::vector<std::string> make_multi_EL(std::string expr, MultiDataMatrix &mdata)
{
    SignalTreeMultiStrategy st(expr, mdata);
    return st.print_EasyLanguage();
}

std::string make_MQL4(std::string expr)
{
    SignalTreeStrategy st(remove_comments(expr));
    return st.print_MQL4();
}

std::string make_indicator_EL(std::string expr)
{
    // Create a signal tree out of it
    SignalTree *buytree = new SignalTree(expr);
    
    std::stringstream s;
    
    // initial boilerplate
#ifndef MULTICHARTS_COMPATIBLE
    s << format("using elsystem; \n"
    );
#else
    //s << format("\n"
    //);
#endif
    
    // prepare the inputs
    // get the ELIs
    std::vector<ELI> all_elis;
    buytree->root->fill_ELI_list(all_elis);
    
    ExplicitParameterMap inps_map;
    for (unsigned int i = 0; i < all_elis.size(); i++)
    {
        // not in map?
        if (inps_map.count(all_elis[i].name) == 0)
        {
            // now it is
            inps_map[all_elis[i].name] = all_elis[i].value;
        } else
        {
            // in map
            
            // the types differ?
            if (inps_map[all_elis[i].name].type() != all_elis[i].value.type())
            {
                throw std::runtime_error("EasyLanguage inputs that have same name but different types: "
                                         + all_elis[i].name);
            }
            
            if (inps_map[all_elis[i].name].type() == typeid(Time))
            {
                if (get<Time>(inps_map[all_elis[i].name]) != get<Time>(all_elis[i].value))
                {
                    throw std::runtime_error("EasyLanguage inputs that have same name/type but different values: "
                                             + all_elis[i].name);
                }
            }
            
            if (inps_map[all_elis[i].name].type() == typeid(std::string))
            {
                if (get<std::string>(inps_map[all_elis[i].name]) != get<std::string>(all_elis[i].value))
                {
                    throw std::runtime_error("EasyLanguage inputs that have same name/type but different values: "
                                             + all_elis[i].name);
                }
            }
            
            if (inps_map[all_elis[i].name].type() == typeid(bool))
            {
                if (get<bool>(inps_map[all_elis[i].name]) != get<bool>(all_elis[i].value))
                {
                    throw std::runtime_error("EasyLanguage inputs that have same name/type but different values: "
                                             + all_elis[i].name);
                }
            }
            
            if (inps_map[all_elis[i].name].type() == typeid(int))
            {
                if (get<int>(inps_map[all_elis[i].name]) != get<int>(all_elis[i].value))
                {
                    throw std::runtime_error("EasyLanguage inputs that have same name/type but different values: "
                                             + all_elis[i].name);
                }
            }
            
            if (inps_map[all_elis[i].name].type() == typeid(double))
            {
                if (get<double>(inps_map[all_elis[i].name]) != get<double>(all_elis[i].value))
                {
                    throw std::runtime_error("EasyLanguage inputs that have same name/type but different values: "
                                             + all_elis[i].name);
                }
            }
        }
    }
    
    // now iterate over the inputs and output the code
    for (ExplicitParameterMap::iterator it = inps_map.begin();
         it != inps_map.end(); it++)
    {
        // time-based inputs are split in 3 integers
        if ((it->second).type() == typeid(Time))
        {
            s << format(
                    "input: %s(%s); \n"
                    "input: %s(%s); \n"
                    "input: %s(%s); \n"
            )
                 % ((it->first) + "_H") % (get<Time>(it->second)).hour
                 % ((it->first) + "_M") % (get<Time>(it->second)).minute
                 % ((it->first) + "_S") % (get<Time>(it->second)).second;
        } else if ((it->second).type() == typeid(std::string))
        {
            s << format("input: %s(\"%s\"); \n") % it->first % get<std::string>(it->second);
        } else if ((it->second).type() == typeid(bool))
        {
            s << format("input: %s(%s); \n") % it->first % ((get<bool>(it->second) == true) ? "true" : "false");
        } else if ((it->second).type() == typeid(int))
        {
            s << format("input: %s(%d); \n") % it->first % get<int>(it->second);
        } else if ((it->second).type() == typeid(double))
        {
            s << format("input: %s(%3.14f); \n") % it->first % get<double>(it->second);
        } else
        {
            throw std::runtime_error("Unknown input type");
        }
    }

#ifndef MULTICHARTS_COMPATIBLE
    // VariableSignal declarations for the main loop
    s << "variable: once_x(true), qty(0), t_buying(false), t_selling(false), t_closing(false); \n"
            "variable: order(0); \n" // 0 - no order, 1 - buy, 2 - sell, 3 - close
            "variable: last_profit(0), last_exit_reason(0); \n"
            "variable: tmp(0.0), idx(0), rev_buysell(false); \n"
            "variable: prev_bar(0); \n"
            "variable: datetime cur_datetime(null); \n";
#else
    // VariableSignal declarations for the main loop
    s << "variable: once_x(true), qty(0); \n"
            "variable: prev_bar(0); \n"
            "variable: tmp(0.0), idx(0); \n";

#endif
    
    // Signal variable declarations
    s << buytree->root->get_el_var_decl();

#ifdef EL_ONCE_PER_BAR
    s << "\n\nif prev_bar <> currentbar then \n"
            "begin \n"
            "   prev_bar = currentbar; \n"
            "\n";
#endif

#ifndef MULTICHARTS_COMPATIBLE
    // Runtime boilerplate code, any precomputations, etc.
    s << "\ncur_datetime = BarDateTime[0]; \n";
#else
    s << "\n\n";
#endif
    
    std::map<Signal *, int> times_inited;
    // initialize the map
    for (auto it = buytree->slist_sorted.begin(); it != buytree->slist_sorted.end(); it++)
    { times_inited[(*it)] = 0; }
    
    // Signal init()s
    // in topological order
    s << "if once_x then \nbegin \n";
    for (auto it = buytree->slist_sorted.begin(); it != buytree->slist_sorted.end(); it++)
    {
        if (times_inited[(*it)] == 0)
        {
            s << (*it)->get_el_init() << "\n";
            times_inited[(*it)]++;
        }
    }
    s << "\nonce_x = false; \nend; \n\n";
    
    
    std::map<Signal *, int> times_ticked;
    // initialize the map
    for (auto it = buytree->slist_sorted.begin(); it != buytree->slist_sorted.end(); it++)
    { times_ticked[(*it)] = 0; }
    
    // tick the signals
    for (auto it = buytree->slist_sorted.begin(); it != buytree->slist_sorted.end(); it++)
    {
        if (times_ticked[(*it)] == 0)
        {
            s << (*it)->get_el_tick() << "\n";
            times_ticked[(*it)]++;
        }
    }
    
    // So, just plot the root of the signal tree or whatever
    s << "Plot1(" << buytree->root->get_el_output() << ");\n";
    
    s << "end; \n\n";
    
    return s.str();
}

SignalTreeStrategy::SignalTreeStrategy(const std::string &a_inp,
                                       const std::string &a_comment)
{
    // defaults
    amttree = NULL;
    rvtree = NULL;

    buylimittree_at_price = NULL;
    selllimittree_at_price = NULL;
    buylimittree_condition = NULL;
    selllimittree_condition = NULL;

    std::string t_ap_inp = remove_comments(a_inp);
    // strip the input from any whitespace
    std::string t_inp;
    for (unsigned int i = 0; i < t_ap_inp.size(); i++)
    {
        if (!((t_ap_inp[i] == ' ') ||
              (t_ap_inp[i] == '\n') ||
              (t_ap_inp[i] == '\t') ||
              (t_ap_inp[i] == '\r')))
            t_inp += t_ap_inp[i];
    }
    // Now we work with t_inp
    
    std::vector<Signal *> buyifs;
    std::vector<Signal *> sellifs;
    std::vector<Signal *> closeifs;
    
    // the variable map
    std::map<std::string, Signal *> vars;
    
    // split into statements
    std::vector<std::string> statements;
    split(statements, t_inp, is_any_of(";"));
    
    if (statements.size() > 0)
    {
        // This is either a variable assignment or BUY_IF/SELL_IF/CLOSE_IF keyword
        for (int i = 0; i < statements.size(); i++)
        {
            // check for keyword
            if (statements[i].find("BUY_IF") == 0)
            {
                // found BUY_IF
                // consider the rest of the string to be a signal
                statements[i].erase(0, std::string("BUY_IF").size());
                Signal *t = EvalExpr(statements[i], vars);
                buyifs.push_back(t);
            }
            else if (statements[i].find("SELL_IF") == 0)
            {
                // found SELL_IF
                // consider the rest of the string to be a signal
                statements[i].erase(0, std::string("SELL_IF").size());
                Signal *t = EvalExpr(statements[i], vars);
                sellifs.push_back(t);
            }
            else if (statements[i].find("CLOSE_IF") == 0)
            {
                // found CLOSE_IF
                // consider the rest of the string to be a signal
                statements[i].erase(0, std::string("CLOSE_IF").size());
                Signal *t = EvalExpr(statements[i], vars);
                closeifs.push_back(t);
            }
            else if (statements[i].find("BUY_LIMIT_AT") == 0)
            {
                // found BUY_LIMIT_AT
                // the rest of the string is two signals separated by _IF_
                statements[i].erase(0, std::string("BUY_LIMIT_AT").size());
                std::vector<std::string> onetwo;
                split(onetwo, statements[i], is_any_of("^"));
                if (onetwo.size() != 2)
                {
                    throw std::runtime_error("Bad form of limit order");
                }

                if ((buylimittree_at_price != NULL) || (buylimittree_condition != NULL))
                {
                    throw std::runtime_error("Only one buy limit order statement per strategy allowed");
                }

                buylimittree_at_price = new SignalTree(EvalExpr(onetwo[0], vars));
                buylimittree_condition = new SignalTree(EvalExpr(onetwo[1], vars));
            }
            else if (statements[i].find("SELL_LIMIT_AT") == 0)
            {
                // found SELL_LIMIT_AT
                // the rest of the string is two signals separated by _IF_
                statements[i].erase(0, std::string("SELL_LIMIT_AT").size());
                std::vector<std::string> onetwo;
                split(onetwo, statements[i], is_any_of("^"));
                if (onetwo.size() != 2)
                {
                    throw std::runtime_error("Bad form of limit order");
                }

                if ((selllimittree_at_price != NULL) || (selllimittree_condition != NULL))
                {
                    throw std::runtime_error("Only one sell limit order statement per strategy allowed");
                }

                selllimittree_at_price = new SignalTree(EvalExpr(onetwo[0], vars));
                selllimittree_condition = new SignalTree(EvalExpr(onetwo[1], vars));
            }
            else
            {
                if (statements[i].size() > 0)
                {
                    // this must be a variable assignment
                    std::vector<std::string> v;
                    std::string ls, rs;
                    split(v, statements[i], is_any_of("="));
                    // in case there are more '=', merge everything after the first one
                    if (v.size() > 1)
                    {
                        ls = v[0];
                        for (int j = 1; j < v.size(); j++)
                        {
                            if (j > 1)
                            {
                                rs += "=";
                            }
        
                            rs += v[j];
                        }
                        
                        // create a new variable with the given name
                        ParameterMap ow;
                        std::vector<Signal *> ch;
                        
                        // if the variable ends with a star, then the variable wants its log enabled
                        if (ls[ls.size()-1] == '*')
                        {
                            ls.erase(ls.size()-1);
                            
                            // so enable the parameter
                            SignalParameter p(ls);
                            ow["log"] = p;
                        }
    
                        // its children are the right side of the assignment
                        ch.push_back(EvalExpr(rs, vars));
                        Signal *vr = new VariableSignal(ow, ch);
                        vr->m_name = ls;
    
                        // Check if a variable with that name exists already.
                        // Rename it if so before we overwrite it (by adding shit to it)
                        // This will prevent memory leaks
                        bool there = false;
                        for (auto it = vars.begin(); it != vars.end(); it++)
                        {
                            if (it->first == ls)
                            {
                                there = true;
                                break;
                            }
                        }
    
                        if (there)
                        {
                            vars[ls + "09385435"] = vars[ls];
                        }
                        
                        
                        vars[ls] = vr;
                    }
                    else
                    {
                        throw std::runtime_error("Bad form of variable assignment or unknown error");
                    }
                }
            }
        }
    }
    else
    {
        throw std::runtime_error("No statements found in strategy");
    }
    
    // can't leave amount_per_trade/reversed_buysell to be NULL
    if (amttree == NULL)
    {
        amttree = new SignalTree("F(['amount_per_trade', 100000.0])");
    }
    if (rvtree == NULL)
    {
        rvtree = new SignalTree("B(['reversed_buysell', false])");
    }
    // the base StopSignal does nothing
    m_stop_loss_tree = new StopSignal();
    // the base StopSignal does nothing
    m_take_profit_tree = new StopSignal();
    
    
    // in case there are multiple entry/exit points, unite them with ORs
    if (buyifs.size() > 1)
    {
        buytree = op_cascade(buyifs);
    }
    else if (buyifs.size() == 1)
    {
        buytree = new SignalTree(buyifs[0]);
    }
    else
    {
        buytree = new SignalTree("B");
    }
    
    if (sellifs.size() > 1)
    {
        selltree = op_cascade(sellifs);
    }
    else if (sellifs.size() == 1)
    {
        selltree = new SignalTree(sellifs[0]);
    }
    else
    {
        selltree = new SignalTree("B");
    }
    
    if (closeifs.size() > 1)
    {
        closetree = op_cascade(closeifs);
    }
    else if (closeifs.size() == 1)
    {
        closetree = new SignalTree(closeifs[0]);
    }
    else
    {
        closetree = new SignalTree("B");
    }
    
    // other variables
    m_once = true;
    m_comment = a_comment;

    m_last_profit = 0;
    m_last_exit_reason = 0;
}

SignalTree *SignalTreeStrategy::op_cascade(const std::vector<Signal *> &trees)
{
    std::vector<OpSignal *> ops;
    
    // the first op
    OpSignal *op = new OrOp();
    op->add_child(trees[0]);
    op->add_child(trees[1]);
    op->init_op();
    ops.push_back(op);
    
    // now for every next entry in buyifs, make a new op and add the last op as child
    if (trees.size() > 2)
    {
        for (int i = 2; i < trees.size(); i++)
        {
            OpSignal *newop = new OrOp();
            newop->add_child(trees[i]);
            newop->add_child(ops[ops.size() - 1]);
            newop->init_op();
            ops.push_back(newop);
        }
    }
    
    // finally pick the last op
    return new SignalTree(ops[ops.size() - 1]);
}

SignalTreeStrategy::~SignalTreeStrategy()
{
    std::set<Signal *> sset;
    
    // delete all individual signals from flattened tree
    for (int i = 0; i < buytree->slist.size(); i++) sset.insert(buytree->slist[i]);
    for (int i = 0; i < selltree->slist.size(); i++) sset.insert(selltree->slist[i]);
    for (int i = 0; i < closetree->slist.size(); i++) sset.insert(closetree->slist[i]);
    for (int i = 0; i < amttree->slist.size(); i++) sset.insert(amttree->slist[i]);
    for (int i = 0; i < rvtree->slist.size(); i++) sset.insert(rvtree->slist[i]);
    
    for (auto it = sset.begin(); it != sset.end(); it++)
    {
        delete *it;
    }

    if ((buylimittree_at_price != NULL) && (buylimittree_condition != NULL))
    {
        for (int i = 0; i < buylimittree_at_price->slist.size(); i++) sset.insert(buylimittree_at_price->slist[i]);
        for (int i = 0; i < buylimittree_condition->slist.size(); i++) sset.insert(buylimittree_condition->slist[i]);
    }
    if ((selllimittree_at_price != NULL) && (selllimittree_condition != NULL))
    {
        for (int i = 0; i < selllimittree_at_price->slist.size(); i++) sset.insert(selllimittree_at_price->slist[i]);
        for (int i = 0; i < selllimittree_condition->slist.size(); i++) sset.insert(selllimittree_condition->slist[i]);
    }

    delete m_stop_loss_tree;
    delete m_take_profit_tree;
    
    delete buytree;
    delete selltree;
    delete closetree;
    delete amttree;
    delete rvtree;

}

void SignalTreeStrategy::bind(const DataMatrix &data)
{
    buytree->root->update_data_recursive(data);
    selltree->root->update_data_recursive(data);
    closetree->root->update_data_recursive(data);
    amttree->root->update_data_recursive(data);
    rvtree->root->update_data_recursive(data);
    
    m_stop_loss_tree->update_data_recursive(data);
    m_take_profit_tree->update_data_recursive(data);

    if ((buylimittree_at_price != NULL) && (buylimittree_condition != NULL))
    {
        buylimittree_at_price->root->update_data_recursive(data);
        buylimittree_condition->root->update_data_recursive(data);
    }
    if ((selllimittree_at_price != NULL) && (selllimittree_condition != NULL))
    {
        selllimittree_at_price->root->update_data_recursive(data);
        selllimittree_condition->root->update_data_recursive(data);
    }
}


Order *SignalTreeStrategy::tick(const DataMatrix &data, Env &env, const int idx)
{
    int qty = 0;
    bool bu = false, se = false, cl = false, sl = false, tp = false;
    Order *order = NULL;
    
    if (m_once)
    {
        std::map<Signal *, int> times_inited;
        // initialize the map
        for (auto it = buytree->slist_sorted.begin(); it != buytree->slist_sorted.end(); it++)
        { times_inited[(*it)] = 0; }
        for (auto it = selltree->slist_sorted.begin(); it != selltree->slist_sorted.end(); it++)
        { times_inited[(*it)] = 0; }
        for (auto it = closetree->slist_sorted.begin(); it != closetree->slist_sorted.end(); it++)
        { times_inited[(*it)] = 0; }
        for (auto it = amttree->slist_sorted.begin(); it != amttree->slist_sorted.end(); it++)
        { times_inited[(*it)] = 0; }
        for (auto it = rvtree->slist_sorted.begin(); it != rvtree->slist_sorted.end(); it++)
        { times_inited[(*it)] = 0; }

        if ((buylimittree_at_price != NULL) && (buylimittree_condition != NULL))
        {
            for (auto it = buylimittree_at_price->slist_sorted.begin(); it != buylimittree_at_price->slist_sorted.end(); it++)
            { times_inited[(*it)] = 0; }
            for (auto it = buylimittree_condition->slist_sorted.begin(); it != buylimittree_condition->slist_sorted.end(); it++)
            { times_inited[(*it)] = 0; }
        }
        if ((selllimittree_at_price != NULL) && (selllimittree_condition != NULL))
        {
            for (auto it = selllimittree_at_price->slist_sorted.begin(); it != selllimittree_at_price->slist_sorted.end(); it++)
            { times_inited[(*it)] = 0; }
            for (auto it = selllimittree_condition->slist_sorted.begin(); it != selllimittree_condition->slist_sorted.end(); it++)
            { times_inited[(*it)] = 0; }
        }

        // init
        // in topological order
        for (auto it = buytree->slist_sorted.begin(); it != buytree->slist_sorted.end(); it++)
        {
            if (times_inited[(*it)] == 0)
            {
                (*it)->compute_init(env, idx);
                times_inited[(*it)]++;
            }
        }
        for (auto it = selltree->slist_sorted.begin(); it != selltree->slist_sorted.end(); it++)
        {
            if (times_inited[(*it)] == 0)
            {
                (*it)->compute_init(env, idx);
                times_inited[(*it)]++;
            }
        }
        for (auto it = closetree->slist_sorted.begin(); it != closetree->slist_sorted.end(); it++)
        {
            if (times_inited[(*it)] == 0)
            {
                (*it)->compute_init(env, idx);
                times_inited[(*it)]++;
            }
        }
        for (auto it = amttree->slist_sorted.begin(); it != amttree->slist_sorted.end(); it++)
        {
            if (times_inited[(*it)] == 0)
            {
                (*it)->compute_init(env, idx);
                times_inited[(*it)]++;
            }
        }
        for (auto it = rvtree->slist_sorted.begin(); it != rvtree->slist_sorted.end(); it++)
        {
            if (times_inited[(*it)] == 0)
            {
                (*it)->compute_init(env, idx);
                times_inited[(*it)]++;
            }
        }

        if ((buylimittree_at_price != NULL) && (buylimittree_condition != NULL))
        {
            for (auto it = buylimittree_at_price->slist_sorted.begin(); it != buylimittree_at_price->slist_sorted.end(); it++)
            {
                if (times_inited[(*it)] == 0)
                {
                    (*it)->compute_init(env, idx);
                    times_inited[(*it)]++;
                }
            }
            for (auto it = buylimittree_condition->slist_sorted.begin(); it != buylimittree_condition->slist_sorted.end(); it++)
            {
                if (times_inited[(*it)] == 0)
                {
                    (*it)->compute_init(env, idx);
                    times_inited[(*it)]++;
                }
            }
        }
        if ((selllimittree_at_price != NULL) && (selllimittree_condition != NULL))
        {
            for (auto it = selllimittree_at_price->slist_sorted.begin(); it != selllimittree_at_price->slist_sorted.end(); it++)
            {
                if (times_inited[(*it)] == 0)
                {
                    (*it)->compute_init(env, idx);
                    times_inited[(*it)]++;
                }
            }
            for (auto it = selllimittree_condition->slist_sorted.begin(); it != selllimittree_condition->slist_sorted.end(); it++)
            {
                if (times_inited[(*it)] == 0)
                {
                    (*it)->compute_init(env, idx);
                    times_inited[(*it)]++;
                }
            }
        }

        m_once = false;
    }

    // compute the internal variables all signals can see
    if (env.current_position != NO_POSITION)
    {
        m_last_profit = env.potential_profit;
    }

    // always tick stops (before signals)
    m_stop_loss_tree->compute_tick(env, idx);
    m_take_profit_tree->compute_tick(env, idx);
    
    std::map<Signal *, int> times_ticked;
    // initialize the map
    for (auto it = buytree->slist_sorted.begin(); it != buytree->slist_sorted.end(); it++)
    { times_ticked[(*it)] = 0; }
    for (auto it = selltree->slist_sorted.begin(); it != selltree->slist_sorted.end(); it++)
    { times_ticked[(*it)] = 0; }
    for (auto it = closetree->slist_sorted.begin(); it != closetree->slist_sorted.end(); it++)
    { times_ticked[(*it)] = 0; }
    for (auto it = amttree->slist_sorted.begin(); it != amttree->slist_sorted.end(); it++)
    { times_ticked[(*it)] = 0; }
    for (auto it = rvtree->slist_sorted.begin(); it != rvtree->slist_sorted.end(); it++)
    { times_ticked[(*it)] = 0; }

    if ((buylimittree_at_price != NULL) && (buylimittree_condition != NULL))
    {
        for (auto it = buylimittree_at_price->slist_sorted.begin(); it != buylimittree_at_price->slist_sorted.end(); it++)
        { times_ticked[(*it)] = 0; }
        for (auto it = buylimittree_condition->slist_sorted.begin(); it != buylimittree_condition->slist_sorted.end(); it++)
        { times_ticked[(*it)] = 0; }
    }
    if ((selllimittree_at_price != NULL) && (selllimittree_condition != NULL))
    {
        for (auto it = selllimittree_at_price->slist_sorted.begin(); it != selllimittree_at_price->slist_sorted.end(); it++)
        { times_ticked[(*it)] = 0; }
        for (auto it = selllimittree_condition->slist_sorted.begin(); it != selllimittree_condition->slist_sorted.end(); it++)
        { times_ticked[(*it)] = 0; }
    }

    // tick signals
    // in topological order
    for (auto it = buytree->slist_sorted.begin(); it != buytree->slist_sorted.end(); it++)
    {
        if (times_ticked[(*it)] == 0)
        {
            (*it)->compute_tick(env, idx);
            (*it)->do_log(logs);
            times_ticked[(*it)]++;
        }
    }
    for (auto it = selltree->slist_sorted.begin(); it != selltree->slist_sorted.end(); it++)
    {
        if (times_ticked[(*it)] == 0)
        {
            (*it)->compute_tick(env, idx);
            (*it)->do_log(logs);
            times_ticked[(*it)]++;
        }
    }
    for (auto it = closetree->slist_sorted.begin(); it != closetree->slist_sorted.end(); it++)
    {
        if (times_ticked[(*it)] == 0)
        {
            (*it)->compute_tick(env, idx);
            (*it)->do_log(logs);
            times_ticked[(*it)]++;
        }
    }
    for (auto it = amttree->slist_sorted.begin(); it != amttree->slist_sorted.end(); it++)
    {
        if (times_ticked[(*it)] == 0)
        {
            (*it)->compute_tick(env, idx);
            (*it)->do_log(logs);
            times_ticked[(*it)]++;
        }
    }
    for (auto it = rvtree->slist_sorted.begin(); it != rvtree->slist_sorted.end(); it++)
    {
        if (times_ticked[(*it)] == 0)
        {
            (*it)->compute_tick(env, idx);
            (*it)->do_log(logs);
            times_ticked[(*it)]++;
        }
    }

    if ((buylimittree_at_price != NULL) && (buylimittree_condition != NULL))
    {
        for (auto it = buylimittree_at_price->slist_sorted.begin(); it != buylimittree_at_price->slist_sorted.end(); it++)
        {
            if (times_ticked[(*it)] == 0)
            {
                (*it)->compute_tick(env, idx);
                (*it)->do_log(logs);
                times_ticked[(*it)]++;
            }
        }
        for (auto it = buylimittree_condition->slist_sorted.begin(); it != buylimittree_condition->slist_sorted.end(); it++)
        {
            if (times_ticked[(*it)] == 0)
            {
                (*it)->compute_tick(env, idx);
                (*it)->do_log(logs);
                times_ticked[(*it)]++;
            }
        }
    }
    if ((selllimittree_at_price != NULL) && (selllimittree_condition != NULL))
    {
        for (auto it = selllimittree_at_price->slist_sorted.begin(); it != selllimittree_at_price->slist_sorted.end(); it++)
        {
            if (times_ticked[(*it)] == 0)
            {
                (*it)->compute_tick(env, idx);
                (*it)->do_log(logs);
                times_ticked[(*it)]++;
            }
        }
        for (auto it = selllimittree_condition->slist_sorted.begin(); it != selllimittree_condition->slist_sorted.end(); it++)
        {
            if (times_ticked[(*it)] == 0)
            {
                (*it)->compute_tick(env, idx);
                (*it)->do_log(logs);
                times_ticked[(*it)]++;
            }
        }
    }

    // get orders
    bu = get<bool>(buytree->root->get_output());
    se = get<bool>(selltree->root->get_output());
    cl = get<bool>(closetree->root->get_output());

    // SL/TP orders are always on.
    sl = get<bool>(m_stop_loss_tree->get_output());
    tp = get<bool>(m_take_profit_tree->get_output());
    
    if (get<bool>(rvtree->root->get_output()))
    {
        bool tmp = bu;
        bu = se;
        se = tmp;
    }

    // check if there is already a limit order active
    // if so, check if the current price crossed the limit order price and make an order if so (and delete the limit order)
    // otherwise, check the limit order condition and if it's met, read the target and either create a new limit order or update
    // the price of the existing one with the target.
    bool is_limit_buy = false;
    bool is_limit_sell = false;
    if ((buylimittree_condition != NULL) && (buylimittree_at_price != NULL))
    {
        bool condition = get<bool>(buylimittree_condition->root->get_output());
        if (condition)
        {
            // if currently in active limit order, close it
            // update logs
            if (env.buy_limit_active)
            {
                LimitOrderInfo linfo;
                linfo.symbol = data.name;
                linfo.position = PositionType::LONG_POSITION;
                linfo.datetime_of_creation = env.buy_limit_opened_at_time;
                linfo.datetime_of_expiry = data.dt[idx];
                linfo.bar_idx_at_creation = env.buy_limit_opened_at_idx;
                linfo.bar_idx_at_expiry = idx;
                linfo.price_for_entry = env.buy_limit_price;
                linfo.did_enter = false;

                limit_orders.push_back(linfo);
            }

            // every time the condition is true, the price is updated
            env.buy_limit_active = true;
            env.buy_limit_price = get<double>(buylimittree_at_price->root->get_output());
            env.buy_limit_opened_at_time = data.dt[idx];
            env.buy_limit_opened_at_idx = idx;
        }

        // check if it's time to cancel the limit order
        if ((env.buy_limit_active) && (env.buy_limit_opened_at_time.date() != data.dt[idx].date())) // next day already?
        {
            LimitOrderInfo linfo;
            linfo.symbol = data.name;
            linfo.position = PositionType::LONG_POSITION;
            linfo.datetime_of_creation = env.buy_limit_opened_at_time;
            linfo.datetime_of_expiry = data.dt[idx];
            linfo.bar_idx_at_creation = env.buy_limit_opened_at_idx;
            linfo.bar_idx_at_expiry = idx;
            linfo.price_for_entry = env.buy_limit_price;
            linfo.did_enter = false;

            limit_orders.push_back(linfo);

            // clear the buy limit order from the env
            env.buy_limit_active = false;
            env.buy_limit_price = 0;
            env.buy_limit_opened_at_idx = 0;
            env.buy_limit_opened_at_time = DateTime();
        }

        if (env.buy_limit_active)
        {
            if (data.close[idx] <= env.buy_limit_price) // for buying, less is better
            {
                // condition is met, create an order
                bu = true;
                is_limit_buy = true;

                LimitOrderInfo linfo;
                linfo.symbol = data.name;
                linfo.position = PositionType::LONG_POSITION;
                linfo.datetime_of_creation = env.buy_limit_opened_at_time;
                linfo.datetime_of_expiry = data.dt[idx];
                linfo.bar_idx_at_creation = env.buy_limit_opened_at_idx;
                linfo.bar_idx_at_expiry = idx;
                linfo.price_for_entry = env.buy_limit_price;
                linfo.did_enter = true;

                limit_orders.push_back(linfo);

                // clear the buy limit order from the env
                env.buy_limit_active = false;
                env.buy_limit_price = 0;
                env.buy_limit_opened_at_idx = 0;
                env.buy_limit_opened_at_time = DateTime();
            }
        }
    }

    if ((selllimittree_condition != NULL) && (selllimittree_at_price != NULL))
    {
        bool condition = get<bool>(selllimittree_condition->root->get_output());
        if (condition)
        {
            if (env.sell_limit_active)
            {
                LimitOrderInfo linfo;
                linfo.symbol = data.name;
                linfo.position = PositionType::SHORT_POSITION;
                linfo.datetime_of_creation = env.sell_limit_opened_at_time;
                linfo.datetime_of_expiry = data.dt[idx];
                linfo.bar_idx_at_creation = env.sell_limit_opened_at_idx;
                linfo.bar_idx_at_expiry = idx;
                linfo.price_for_entry = env.sell_limit_price;
                linfo.did_enter = false;

                limit_orders.push_back(linfo);
            }
            // every time the condition is true, the price is updated
            env.sell_limit_active = true;
            env.sell_limit_price = get<double>(selllimittree_at_price->root->get_output());
            env.sell_limit_opened_at_time = data.dt[idx];
            env.sell_limit_opened_at_idx = idx;
        }

        // check if it's time to cancel the limit order
        if ((env.sell_limit_active) && (env.sell_limit_opened_at_time.date() != data.dt[idx].date())) // next day already?
        {
            LimitOrderInfo linfo;
            linfo.symbol = data.name;
            linfo.position = PositionType::SHORT_POSITION;
            linfo.datetime_of_creation = env.sell_limit_opened_at_time;
            linfo.datetime_of_expiry = data.dt[idx];
            linfo.bar_idx_at_creation = env.sell_limit_opened_at_idx;
            linfo.bar_idx_at_expiry = idx;
            linfo.price_for_entry = env.sell_limit_price;
            linfo.did_enter = false;

            limit_orders.push_back(linfo);

            // clear the buy limit order from the env
            env.sell_limit_active = false;
            env.sell_limit_price = 0;
            env.sell_limit_opened_at_idx = 0;
            env.sell_limit_opened_at_time = DateTime();
        }

        if (env.sell_limit_active)
        {
            if (data.close[idx] >= env.sell_limit_price) // for selling, more is better
            {
                // condition is met, create an order
                se = true;
                is_limit_sell = true;

                LimitOrderInfo linfo;
                linfo.symbol = data.name;
                linfo.position = PositionType::SHORT_POSITION;
                linfo.datetime_of_creation = env.sell_limit_opened_at_time;
                linfo.datetime_of_expiry = data.dt[idx];
                linfo.bar_idx_at_creation = env.sell_limit_opened_at_idx;
                linfo.bar_idx_at_expiry = idx;
                linfo.price_for_entry = env.sell_limit_price;
                linfo.did_enter = true;

                limit_orders.push_back(linfo);

                // clear the buy limit order from the env
                env.sell_limit_active = false;
                env.sell_limit_price = 0;
                env.sell_limit_opened_at_idx = 0;
                env.sell_limit_opened_at_time = DateTime();
            }
        }
    }

    // number of shares for a fixed amount of dollars
    qty = (int) (get<double>(amttree->root->get_output()) / data.close[idx]);

    if ((cl || sl || tp) && (env.current_position != NO_POSITION))
    {
        if (sl)
        {
            order = new Order(CLOSE, 0, "stop loss");
            m_last_exit_reason = -1;
        }
        else if (tp)
        {
            order = new Order(CLOSE, 0, "take profit");
            m_last_exit_reason = 1;
        }
        else
        {
            order = new Order(CLOSE);
            m_last_exit_reason = 0;
        }
    }
    else
    {
        if (bu && (!se))
        {
            if (!is_limit_buy)
            {
                order = new Order(BUY, qty);
            }
            else
            {
                order = new Order(BUY, qty, "limit buy");
            }

            // stop loss
            m_stop_loss_tree->compute_init_buying(env, idx);
            // take profit
            m_take_profit_tree->compute_init_buying(env, idx);
        }
    
        if (se && (!bu))
        {
            if (!is_limit_sell)
            {
                order = new Order(SELL, qty);
            }
            else
            {
                order = new Order(SELL, qty, "limit sell");
            }

            // stop loss
            m_stop_loss_tree->compute_init_selling(env, idx);
            // take profit
            m_take_profit_tree->compute_init_selling(env, idx);
        }
    }

    return order;
}

std::string SignalTreeStrategy::print_EasyLanguage(bool debug/*=false*/, bool readable/*=true*/ )
{
    std::stringstream s;
    
    // initial boilerplate
#ifndef MULTICHARTS_COMPATIBLE
    s << format("using elsystem; \n"
    );
#else
    //s << format("\n"
    //);
#endif
    
    // prepare the inputs
    // get the ELIs
    std::vector<ELI> all_elis;
    buytree->root->fill_ELI_list(all_elis);
    selltree->root->fill_ELI_list(all_elis);
    closetree->root->fill_ELI_list(all_elis);
    amttree->root->fill_ELI_list(all_elis);
    rvtree->root->fill_ELI_list(all_elis);
    
    m_stop_loss_tree->fill_ELI_list(all_elis);
    m_take_profit_tree->fill_ELI_list(all_elis);

    ExplicitParameterMap inps_map;
    for (unsigned int i = 0; i < all_elis.size(); i++)
    {
        // not in map?
        if (inps_map.count(all_elis[i].name) == 0)
        {
            // now it is
            inps_map[all_elis[i].name] = all_elis[i].value;
        }
        else
        {
            // in map
    
            // the types differ?
            if (inps_map[all_elis[i].name].type() != all_elis[i].value.type())
            {
                throw std::runtime_error("EasyLanguage inputs that have same name but different types: "
                                         + all_elis[i].name);
            }
    
            if (inps_map[all_elis[i].name].type() == typeid(Time))
            {
                if (get<Time>(inps_map[all_elis[i].name]) != get<Time>(all_elis[i].value))
                {
                    throw std::runtime_error("EasyLanguage inputs that have same name/type but different values: "
                                             + all_elis[i].name);
                }
            }
    
            if (inps_map[all_elis[i].name].type() == typeid(std::string))
            {
                if (get<std::string>(inps_map[all_elis[i].name]) != get<std::string>(all_elis[i].value))
                {
                    throw std::runtime_error("EasyLanguage inputs that have same name/type but different values: "
                                             + all_elis[i].name);
                }
            }
    
            if (inps_map[all_elis[i].name].type() == typeid(bool))
            {
                if (get<bool>(inps_map[all_elis[i].name]) != get<bool>(all_elis[i].value))
                {
                    throw std::runtime_error("EasyLanguage inputs that have same name/type but different values: "
                                             + all_elis[i].name);
                }
            }
    
            if (inps_map[all_elis[i].name].type() == typeid(int))
            {
                if (get<int>(inps_map[all_elis[i].name]) != get<int>(all_elis[i].value))
                {
                    throw std::runtime_error("EasyLanguage inputs that have same name/type but different values: "
                                             + all_elis[i].name);
                }
            }
    
            if (inps_map[all_elis[i].name].type() == typeid(double))
            {
                if (get<double>(inps_map[all_elis[i].name]) != get<double>(all_elis[i].value))
                {
                    throw std::runtime_error("EasyLanguage inputs that have same name/type but different values: "
                                             + all_elis[i].name);
                }
            }
        }
    }
    
    // now iterate over the inputs and output the code
    for (ExplicitParameterMap::iterator it = inps_map.begin();
         it != inps_map.end(); it++)
    {
        // time-based inputs are split in 3 integers
        if ((it->second).type() == typeid(Time))
        {
            s << format(
                    "input: %s(%s); \n"
                            "input: %s(%s); \n"
                            "input: %s(%s); \n"
            )
                 % ((it->first) + "_H") % (get<Time>(it->second)).hour
                 % ((it->first) + "_M") % (get<Time>(it->second)).minute
                 % ((it->first) + "_S") % (get<Time>(it->second)).second;
        }
        else if ((it->second).type() == typeid(std::string))
        {
            s << format("input: %s(\"%s\"); \n") % it->first % get<std::string>(it->second);
        }
        else if ((it->second).type() == typeid(bool))
        {
            s << format("input: %s(%s); \n") % it->first % ((get<bool>(it->second) == true) ? "true" : "false");
        }
        else if ((it->second).type() == typeid(int))
        {
            s << format("input: %s(%d); \n") % it->first % get<int>(it->second);
        }
        else if ((it->second).type() == typeid(double))
        {
            s << format("input: %s(%3.14f); \n") % it->first % get<double>(it->second);
        }
        else
        {
            throw std::runtime_error("Unknown input type");
        }
    }

#ifndef MULTICHARTS_COMPATIBLE
    // VariableSignal declarations for the main loop
    s << "variable: once_x(true), qty(0), t_buying(false), t_selling(false), t_closing(false); \n"
            "variable: order(0); \n" // 0 - no order, 1 - buy, 2 - sell, 3 - close
            "variable: last_profit(0), last_exit_reason(0); \n"
            "variable: tmp(0.0), idx(0), rev_buysell(false); \n"
    		"variable: prev_bar(0); \n"
            "variable: datetime cur_datetime(null); \n";
#else
    // VariableSignal declarations for the main loop
    s << "variable: once_x(true), qty(0), t_buying(false), t_selling(false), t_closing(false); \n"
            "variable: order(0); \n" // 0 - no order, 1 - buy, 2 - sell, 3 - close
            "variable: last_profit(0), last_exit_reason(0); \n"
            "variable: prev_bar(0); \n"
            "variable: tmp(0.0), idx(0), rev_buysell(false); \n";

#endif
    
    // Signal variable declarations
    s << buytree->root->get_el_var_decl() <<
      selltree->root->get_el_var_decl() <<
      closetree->root->get_el_var_decl() <<
      m_stop_loss_tree->get_el_var_decl() <<
      m_take_profit_tree->get_el_var_decl() <<
      amttree->root->get_el_var_decl() <<
      rvtree->root->get_el_var_decl();

#ifdef EL_ONCE_PER_BAR
    s << "\n\nif prev_bar <> currentbar then \n"
            "begin \n"
            "   prev_bar = currentbar; \n"
            "\n";
#endif

#ifndef MULTICHARTS_COMPATIBLE
    // Runtime boilerplate code, any precomputations, etc.
    s << "\ncur_datetime = BarDateTime[0]; \n";
#else
    s << "\n\n";
#endif
    
    std::map<Signal *, int> times_inited;
    // initialize the map
    for (auto it = buytree->slist_sorted.begin(); it != buytree->slist_sorted.end(); it++)
    { times_inited[(*it)] = 0; }
    for (auto it = selltree->slist_sorted.begin(); it != selltree->slist_sorted.end(); it++)
    { times_inited[(*it)] = 0; }
    for (auto it = closetree->slist_sorted.begin(); it != closetree->slist_sorted.end(); it++)
    { times_inited[(*it)] = 0; }
    for (auto it = amttree->slist_sorted.begin(); it != amttree->slist_sorted.end(); it++)
    { times_inited[(*it)] = 0; }
    for (auto it = rvtree->slist_sorted.begin(); it != rvtree->slist_sorted.end(); it++)
    { times_inited[(*it)] = 0; }
    
    // Signal init()s
    // in topological order
    s << "if once_x then \nbegin \n";
    for (auto it = buytree->slist_sorted.begin(); it != buytree->slist_sorted.end(); it++)
    {
        if (times_inited[(*it)] == 0)
        {
            s << (*it)->get_el_init() << "\n";
            times_inited[(*it)]++;
        }
    }
    for (auto it = selltree->slist_sorted.begin(); it != selltree->slist_sorted.end(); it++)
    {
        if (times_inited[(*it)] == 0)
        {
            s << (*it)->get_el_init() << "\n";
            times_inited[(*it)]++;
        }
    }
    for (auto it = closetree->slist_sorted.begin(); it != closetree->slist_sorted.end(); it++)
    {
        if (times_inited[(*it)] == 0)
        {
            s << (*it)->get_el_init() << "\n";
            times_inited[(*it)]++;
        }
    }
    for (auto it = amttree->slist_sorted.begin(); it != amttree->slist_sorted.end(); it++)
    {
        if (times_inited[(*it)] == 0)
        {
            s << (*it)->get_el_init() << "\n";
            times_inited[(*it)]++;
        }
    }
    for (auto it = rvtree->slist_sorted.begin(); it != rvtree->slist_sorted.end(); it++)
    {
        if (times_inited[(*it)] == 0)
        {
            s << (*it)->get_el_init() << "\n";
            times_inited[(*it)]++;
        }
    }
    s << "\nonce_x = false; \nend; \n\n";
    
    s << "if MarketPosition <> 0 then \n" <<
      "begin \n" <<
      "   last_profit = PositionProfit; \n" <<
      "end; \n";
    
    // Tick the stops first
    s << m_stop_loss_tree->get_el_tick_recursive() <<
      m_take_profit_tree->get_el_tick_recursive();
    
    s << "t_buying = false; t_selling = false; t_closing = false; order = 0; \n";
    
    std::map<Signal *, int> times_ticked;
    // initialize the map
    for (auto it = buytree->slist_sorted.begin(); it != buytree->slist_sorted.end(); it++)
    { times_ticked[(*it)] = 0; }
    for (auto it = selltree->slist_sorted.begin(); it != selltree->slist_sorted.end(); it++)
    { times_ticked[(*it)] = 0; }
    for (auto it = closetree->slist_sorted.begin(); it != closetree->slist_sorted.end(); it++)
    { times_ticked[(*it)] = 0; }
    for (auto it = amttree->slist_sorted.begin(); it != amttree->slist_sorted.end(); it++)
    { times_ticked[(*it)] = 0; }
    for (auto it = rvtree->slist_sorted.begin(); it != rvtree->slist_sorted.end(); it++)
    { times_ticked[(*it)] = 0; }
    
    // tick the signals
    for (auto it = buytree->slist_sorted.begin(); it != buytree->slist_sorted.end(); it++)
    {
        if (times_ticked[(*it)] == 0)
        {
            s << (*it)->get_el_tick() << "\n";
            times_ticked[(*it)]++;
        }
    }
    for (auto it = selltree->slist_sorted.begin(); it != selltree->slist_sorted.end(); it++)
    {
        if (times_ticked[(*it)] == 0)
        {
            s << (*it)->get_el_tick() << "\n";
            times_ticked[(*it)]++;
        }
    }
    for (auto it = closetree->slist_sorted.begin(); it != closetree->slist_sorted.end(); it++)
    {
        if (times_ticked[(*it)] == 0)
        {
            s << (*it)->get_el_tick() << "\n";
            times_ticked[(*it)]++;
        }
    }
    for (auto it = amttree->slist_sorted.begin(); it != amttree->slist_sorted.end(); it++)
    {
        if (times_ticked[(*it)] == 0)
        {
            s << (*it)->get_el_tick() << "\n";
            times_ticked[(*it)]++;
        }
    }
    for (auto it = rvtree->slist_sorted.begin(); it != rvtree->slist_sorted.end(); it++)
    {
        if (times_ticked[(*it)] == 0)
        {
            s << (*it)->get_el_tick() << "\n";
            times_ticked[(*it)]++;
        }
    }
    
    
    s << "t_buying  = " << buytree->root->get_el_output() << "; \n";
    s << "t_selling = " << selltree->root->get_el_output() << "; \n";
    s << "t_closing = " << closetree->root->get_el_output() << "; \n";
    s << "qty = " << "round((" << amttree->root->get_el_output() << ") / close, 0); \n";
    s << "rev_buysell = " << rvtree->root->get_el_output() << "; \n";
    
    
    // determine order type
    s << format("if (t_closing or %s or %s) and (Marketposition <> 0) then \n"
                        "begin \n"
                        "   if %s then order = 4 \n"
                        "   else if %s then order = 5 \n"
                        "   else order = 3; \n"
                        "end \n"
                        "else \n"
                        "begin \n"
                        "   if t_buying and (not t_selling) {and (Marketposition = 0)} then \n"
                        "   begin \n"
                        "      order = 1; \n"
                        "   end; \n"
                        "   if t_selling and (not t_buying) {and (Marketposition = 0)} then \n"
                        "   begin \n"
                        "      order = 2; \n"
                        "   end; \n"
                        "end; \n")
         % m_stop_loss_tree->get_el_output() % m_take_profit_tree->get_el_output()
         % m_stop_loss_tree->get_el_output() % m_take_profit_tree->get_el_output();
    
    
    // Buying template
    std::stringstream ssbuy;
    ssbuy << format(
            "if rev_buysell = false then \n"
                    "begin \n"
                    "   %s %s \n"
                    "   buy qty shares this bar; \n"
                    "end \n"
                    "else \n"
                    "begin \n"
                    "   %s %s \n"
                    "   sellshort qty shares this bar; \n"
                    "end; \n")
             % m_stop_loss_tree->get_el_init_buying() % m_take_profit_tree->get_el_init_buying()
             % m_stop_loss_tree->get_el_init_selling() % m_take_profit_tree->get_el_init_selling();
    
    // Selling template
    std::stringstream sssell;
    sssell << format(
            "if rev_buysell = false then \n"
                    "begin \n"
                    "   %s %s \n"
                    "   sellshort qty shares this bar; \n"
                    "end \n"
                    "else \n"
                    "begin \n"
                    "   %s %s \n"
                    "   buy qty shares this bar; \n"
                    "end; \n")
              % m_stop_loss_tree->get_el_init_selling() % m_take_profit_tree->get_el_init_selling()
              % m_stop_loss_tree->get_el_init_buying() % m_take_profit_tree->get_el_init_buying();
    
    std::string sclose = "sell this bar; buytocover this bar; \n";
    std::string sclose_sl = "sell (\"Stop Loss LX\") this bar; buytocover (\"Stop Loss SX\") this bar; \n";
    std::string sclose_tp = "sell (\"Take Profit LX\") this bar; buytocover (\"Take Profit SX\") this bar; \n";
    
    // add to master string stream
    s << format("if (order>=3) then \n"
                        "begin \n"
                        "   if (order=4) then \n"
                        "   begin \n"
                        "      last_exit_reason = -1; \n"
                        "      %s \n"
                        "   end \n"
                        "   else if (order=5) then \n"
                        "   begin \n"
                        "      last_exit_reason = 1; \n"
                        "      %s \n"
                        "   end \n"
                        "   else \n"
                        "   begin \n"
                        "      last_exit_reason = 0; \n"
                        "      %s; \n"
                        "   end; \n"
                        "end \n"
                        "else \n"
                        "begin \n"
                        "   if (order = 1) then \n"
                        "   begin \n"
                        "      %s; \n"
                        "   end; \n"
                        "   if (order = 2) then \n"
                        "   begin \n"
                        "      %s; \n"
                        "   end; \n"
                        "end; \n")
         % sclose_sl
         % sclose_tp
         % sclose
         % ssbuy.str()
         % sssell.str();

#ifdef EL_ONCE_PER_BAR
    s << "\n\n end; \n\n";
#endif
    
    return s.str();
}

std::string SignalTreeStrategy::print_MQL4(bool debug/*=false*/, bool readable/*=true*/ )
{
    std::stringstream s;
#if 0
    // initial boilerplate
    s << "";

    // Use 0.25 lots instead of the default 100000.0
    Signal *t_save = m_amount_per_trade;
    m_amount_per_trade = EvalExpr("F(['amount_per_trade', 1.0])");

    // prepare the inputs
    // get the ELIs
    std::vector<ELI> all_elis;
    m_buy_tree->fill_ELI_list(all_elis);
    m_sell_tree->fill_ELI_list(all_elis);
    m_close_tree->fill_ELI_list(all_elis);
    m_stop_loss_tree->fill_ELI_list(all_elis);
    m_take_profit_tree->fill_ELI_list(all_elis);
    m_amount_per_trade->fill_ELI_list(all_elis);
    m_reversed_buysell->fill_ELI_list(all_elis);

    // restore the m_amount_per_trade
    delete m_amount_per_trade;
    m_amount_per_trade = t_save;

    ExplicitParameterMap inps_map;
    for (unsigned int i = 0; i < all_elis.size(); i++)
    {
        // not in map?
        if (inps_map.count(all_elis[i].name) == 0)
        {
            // now it is
            inps_map[all_elis[i].name] = all_elis[i].value;
        }
        else
        {
            // in map

            // the types differ?
            if (inps_map[all_elis[i].name].type() != all_elis[i].value.type())
            {
                throw std::runtime_error("MQL4 inputs that have same name but different types: "
                                         + all_elis[i].name);
            }

            if (inps_map[all_elis[i].name].type() == typeid(Time))
            {
                if (get<Time>(inps_map[all_elis[i].name]) != get<Time>(all_elis[i].value))
                {
                    throw std::runtime_error("MQL4 inputs that have same name/type but different values: "
                                             + all_elis[i].name);
                }
            }

            if (inps_map[all_elis[i].name].type() == typeid(std::string))
            {
                if (get<std::string>(inps_map[all_elis[i].name]) != get<std::string>(all_elis[i].value))
                {
                    throw std::runtime_error("MQL4 inputs that have same name/type but different values: "
                                             + all_elis[i].name);
                }
            }

            if (inps_map[all_elis[i].name].type() == typeid(bool))
            {
                if (get<bool>(inps_map[all_elis[i].name]) != get<bool>(all_elis[i].value))
                {
                    throw std::runtime_error("MQL4 inputs that have same name/type but different values: "
                                             + all_elis[i].name);
                }
            }

            if (inps_map[all_elis[i].name].type() == typeid(int))
            {
                if (get<int>(inps_map[all_elis[i].name]) != get<int>(all_elis[i].value))
                {
                    throw std::runtime_error("MQL4 inputs that have same name/type but different values: "
                                             + all_elis[i].name);
                }
            }

            if (inps_map[all_elis[i].name].type() == typeid(double))
            {
                if (get<double>(inps_map[all_elis[i].name]) != get<double>(all_elis[i].value))
                {
                    throw std::runtime_error("MQL4 inputs that have same name/type but different values: "
                                             + all_elis[i].name);
                }
            }
        }
    }

    // now iterate over the inputs and output the code
    for (ExplicitParameterMap::iterator it = inps_map.begin();
         it != inps_map.end(); it++)
    {
        // time-based inputs are split in 3 integers
        if ((it->second).type() == typeid(Time))
        {
            s << format(
                    "extern int %s = %s; \n"
                            "extern int %s = %s; \n"
                            "extern int %s = %s; \n"
            )
                 % ((it->first) + "_H") % (get<Time>(it->second)).hour
                 % ((it->first) + "_M") % (get<Time>(it->second)).minute
                 % ((it->first) + "_S") % (get<Time>(it->second)).second;
        }
        else
        {
            if ((it->second).type() == typeid(std::string))
            {
                s << format("extern string %s = \"%s\"; \n") % it->first % get<std::string>(it->second);
            }
            else
            {
                if ((it->second).type() == typeid(bool))
                {
                    s << format("extern bool %s = %s; \n") % it->first %
                         ((get<bool>(it->second) == true) ? "true" : "false");
                }
                else
                {
                    if ((it->second).type() == typeid(int))
                    {
                        s << format("extern int %s = %d; \n") % it->first % get<int>(it->second);
                    }
                    else
                    {
                        if ((it->second).type() == typeid(double))
                        {
                            s << format("extern double %s = %3.6f; \n") % it->first % get<double>(it->second);
                        }
                        else
                        {
                            throw std::runtime_error("Unknown input type");
                        }
                    }
                }
            }
        }
    }

    // VariableSignal declarations for the main loop
    s << "bool once_x = true; \ndouble qty = 0; \n"
            "bool t_buying = false; \nbool t_selling = false; \nbool t_closing = false; \n"
            "int order = 0; \n" // 0 - no order, 1 - buy, 2 - sell, 3 - close
            "double last_profit = 0; \nint last_exit_reason = 0; \n"
            "bool rev_buysell = false; \n"
            "bool IsNewBar; \n"
            "datetime LastBarOpenAt; \n"
            "int ticks = 0; \n"
            "int idx = 0; "
            "double tmpd = 0; \n"
            "int tmpi = 0; \n"
            "int ticket, handle; \n"
            "extern bool m_LogData = false; \n";

    s << "int MagicNumber = " << rand() % 10000 << "; \n";


    // Signal variable declarations
    s << m_buy_tree->get_mql_var_decl() <<
    m_sell_tree->get_mql_var_decl() <<
    m_close_tree->get_mql_var_decl() <<
    m_stop_loss_tree->get_mql_var_decl() <<
    m_take_profit_tree->get_mql_var_decl() <<
    m_amount_per_trade->get_mql_var_decl() <<
    m_reversed_buysell->get_mql_var_decl();

    // The init function
    s <<
    "\n\nint init() \n"
            "{ \n"
            "   int h; \n" // clear the log
            "   h = FileOpen(\"trader_log.csv\", FILE_CSV|FILE_WRITE, ','); \n"
            "   FileClose(h); \n"
            "   return(0); \n"
            "   MathSrand(GetTickCount());"
            "   MagicNumber = MathRand(); \n"
            " } \n\n";


    // Runtime boilerplate code, any precomputations, functions, etc.
    s <<
    "void Log(string err) \n"
            "{  \n"
            "   string fname = \"trader_log.csv\"; \n"
            "   handle = FileOpen(fname, \n "
            "   FILE_CSV | FILE_WRITE | FILE_READ,',');  \n"
            "   if(handle < 1)  \n"
            "   {  \n"
            "       Print(\"Log file error, the last error is \", GetLastError());  \n"
            "       return;  \n"
            "   }  \n"
            "   else  \n"
            "   {  \n"
            "       FileSeek( handle, 0, SEEK_END);  \n"
            "       FileWrite(handle, TimeToStr(CurTime(),TIME_DATE|TIME_MINUTES|TIME_SECONDS), err, GetLastError(), Bid, Ask);  \n"
            "       FileClose(handle);  \n"
            "   }  \n"
            "}  \n"
            "int MarketPosition()  \n"
            "{  \n"
            "   int i = 0;"
            "   int orderstotal = OrdersTotal();  \n"
            "   if (orderstotal == 0)  \n"
            "   {  \n"
            "       return (0);  \n"
            "   }  \n"
            "   int minetotal = 0; \n"
            "   for(i=0; i<orderstotal; i++) \n"
            "   { \n"
            "       if (!OrderSelect( i, SELECT_BY_POS, MODE_TRADES ))  \n"
            "       {  \n"
            "           Log(\"OrderSelect error in MarketPosition\");  \n"
            "       }  \n"
            "       \n"
            "       if ((OrderSymbol() == Symbol()) && (OrderMagicNumber() == MagicNumber)) \n"
            "       { \n"
            "           minetotal++; \n"
            "       } \n"
            "   }  \n"
            "   if (minetotal > 0)  \n"
            "   {  \n"
            "       \n"
            "       for(i = 0; i < orderstotal; i++) \n"
            "       { \n"
            "           if (!OrderSelect( i, SELECT_BY_POS, MODE_TRADES ))  \n"
            "           {  \n"
            "               Log(\"OrderSelect error in MarketPosition\");  \n"
            "           }  \n"
            "           ticket = OrderTicket();  \n"
            "           if ((OrderSymbol() == Symbol()) && (OrderMagicNumber() == MagicNumber)) \n"
            "           { \n"
            "              int type = OrderType();  \n"
            "              if (type == OP_BUY)  \n"
            "              {  \n"
            "                  return (1);  \n"
            "              }  \n"
            "              if (type == OP_SELL)  \n"
            "              {  \n"
            "                  return (-1);  \n"
            "              }  \n"
            "          } \n"
            "      }  \n"
            "   } \n"
            "   return (0); \n"
            "}  \n"
            "  \n"
            "double PositionProfit()  \n"
            "{  \n"
            "   int  i = 0; \n"
            "   int orderstotal = OrdersTotal();  \n"
            "   if (orderstotal == 0)  \n"
            "   {  \n"
            "       return (0);  \n"
            "   }  \n"
            "   int minetotal = 0; \n"
            "   for(i=0; i<orderstotal; i++) \n"
            "   { \n"
            "       if (!OrderSelect( i, SELECT_BY_POS, MODE_TRADES ))  \n"
            "       {  \n"
            "           Log(\"OrderSelect error in MarketPosition\");  \n"
            "       }  \n"
            "       "
            "       if ((OrderSymbol() == Symbol()) && (OrderMagicNumber() == MagicNumber)) \n"
            "       { \n"
            "           minetotal++; \n"
            "       } \n"
            "   }  \n"
            "   if (minetotal > 0)  \n"
            "   {  \n"
            "       for(i = 0; i < orderstotal; i++) \n"
            "       { \n"
            "          if (!OrderSelect( i, SELECT_BY_POS, MODE_TRADES ))  \n"
            "          {  \n"
            "              Log(\"OrderSelect error in PositionProfit\");  \n"
            "          }  \n"
            "          \n"
            "          ticket = OrderTicket();  \n"
            "          if ((OrderSymbol() == Symbol()) && (OrderMagicNumber() == MagicNumber)) \n"
            "          { \n"
            "              int type = OrderType();  \n"
            "              double profit = OrderProfit();  \n"
            "              return (profit);  \n"
            "          } \n"
            "       } \n"
            "   }  \n"
            "   return (0); \n"
            "} \n"
            "  \n"
            "double Array_Sum(double a[], int start_idx, int end_idx)  \n"
            "{  \n"
            "   double total = 0; \n"
            "   int i = 0; \n"
            "   for(i = start_idx; i< end_idx; i++) \n"
            "   { \n"
            "       total = total + a[i]; \n"
            "   } \n"
            " \n"
            "   return (total);  \n"
            "} \n"
            "  \n"
            "double Array_Avg(double a[], int start_idx, int end_idx)  \n"
            "{  \n"
            "   double total = 0; \n"
            "   int i = 0; \n"
            "   for(i = start_idx; i< end_idx; i++) \n"
            "   { \n"
            "       total = total + a[i]; \n"
            "   } \n"
            " \n"
            "    return (total / (end_idx - start_idx));  \n"
            "}  \n"
            "  \n"
            "void CloseOrder(double q, string s)  \n"
            "{  \n"
            "   int i = 0;"
            "   int orderstotal = OrdersTotal();  \n"
            "   int minetotal = 0; \n"
            "   for(i=0; i<orderstotal; i++) \n"
            "   {   \n"
            "       if (!OrderSelect( i, SELECT_BY_POS, MODE_TRADES ))  \n"
            "       {  \n"
            "           Log(\"OrderSelect error in CloseOrder\");  \n"
            "       }  \n"
            "       \n"
            "       if ((OrderSymbol() == Symbol()) && (OrderMagicNumber() == MagicNumber)) \n"
            "       { \n"
            "           minetotal++; \n"
            "       } \n"
            "   }  \n"
            "   if (minetotal >= 1)  \n"
            "   {  \n"
            "       for(i = 0; i < orderstotal; i++) \n"
            "       { \n"
            "          if (!OrderSelect( i, SELECT_BY_POS, MODE_TRADES ))  \n"
            "          {  \n"
            "              Log(\"OrderSelect error in CloseOrder\");  \n"
            "          }  \n"
            "           \n"
            "          ticket = OrderTicket();  \n"
            "          if ((OrderSymbol() == Symbol()) && (OrderMagicNumber() == MagicNumber)) \n"
            "          { \n"
            "              ticket = OrderTicket();  \n"
            "              int type = OrderType();  \n"
            "              if (type == OP_BUY)  \n"
            "              {  \n"
            "                  if (!OrderClose(ticket, q, Bid, 3, Green))  \n"
            "                  {  \n"
            "                      Log(\"OrderClose error\");  \n"
            "                  }  \n"
            "              }  \n"
            "              if (type == OP_SELL)  \n"
            "              {  \n"
            "                  if (!OrderClose(ticket, q, Ask, 3, Green))  \n"
            "                  {  \n"
            "                      Log(\"OrderClose error\");  \n"
            "                  }  \n"
            "              }  \n"
            "                 \n"
            "              Log(\"CLOSE success\" + s);  \n"
            "          }  \n"
            "       } \n"
            "   } \n"
            "}  \n"
            "void BuyOrder(double q)  \n"
            "{  \n"
            "   /*int orderstotal = OrdersTotal();  \n"
            "   int minetotal = 0; \n"
            "   for(int i=0; i<orderstotal; i++) \n"
            "   { \n"
            "       if (!OrderSelect( i, SELECT_BY_POS, MODE_TRADES ))  \n"
            "       {  \n"
            "           Log(\"OrderSelect error in BuyOrder\");  \n"
            "       }  \n"
            "       \n"
            "       if ((OrderSymbol() == Symbol()) && (OrderMagicNumber() == MagicNumber)) \n"
            "       { \n"
            "           minetotal++; \n"
            "       } \n"
            "   } */ \n"
            "   int mp = MarketPosition(); \n"
            "   /*if (mp == -1) { CloseOrder(q, \"\"); mp = 0; }*/ \n"
            "   if (mp == 0)  \n"
            "   {  \n"
            "      \n"
            "      ticket = OrderSend(Symbol(), OP_BUY, q, Ask, 3, 0, 0, \"LE\", MagicNumber, 0, Blue);  \n"
            "     \n"
            "      if(ticket < 0)   \n"
            "      {  \n"
            "          Log(\"LONG_OPEN failed\");  \n"
            "      }  \n"
            "     \n"
            "      Log(\"LONG_OPEN success\");  \n"
            "     \n"
            "   }  \n"
            "}  \n"
            "  \n"
            "void SellOrder(double q)  \n"
            "{  \n"
            "   /*int orderstotal = OrdersTotal();  \n"
            "   int minetotal = 0; \n"
            "   for(int i=0; i<orderstotal; i++) \n"
            "   {   \n"
            "       if (!OrderSelect( i, SELECT_BY_POS, MODE_TRADES ))  \n"
            "       {  \n"
            "           Log(\"OrderSelect error in SellOrder\");  \n"
            "       }  \n"
            "       \n "
            "       if ((OrderSymbol() == Symbol()) && (OrderMagicNumber() == MagicNumber)) \n"
            "       { \n"
            "           minetotal++; \n"
            "       } \n"
            "   } */ \n"
            "   int mp = MarketPosition(); \n"
            "   /*if (mp == 1) { CloseOrder(q, \"\"); mp = 0; }*/ \n"
            "   if (mp == 0)  \n"
            "   {  \n"
            "      \n"
            "      ticket = OrderSend(Symbol(), OP_SELL, q, Bid, 3, 0, 0, \"SE\", MagicNumber, 0, Red);  \n"
            "      \n"
            "      if(ticket < 0)  \n"
            "      {  \n"
            "          Log(\"SHORT_OPEN failed\");  \n"
            "      }  \n"
            "      \n"
            "      Log(\"SHORT_OPEN success\");  \n"
            "      \n"
            "   }  \n"
            "}  \n"
            "  \n"
            "";

    // We must be inside the start() function.
    // Do initial timing stuff. We need to execute each bar, not each tick!
    s << "\n\nint start() \n"
            "{ \n"
            "  if (LastBarOpenAt == Time[0]) \n"
            "  { \n"
            "      IsNewBar = false; \n"
            "  } \n"
            "  else \n"
            "  { \n"
            "      LastBarOpenAt = Time[0]; \n"
            "      IsNewBar = true; \n"
            "  } \n";

    s << "  if (IsNewBar) { \n"; // execute on new bars only

    // Signal init()s
    // TODO: Move this to the init() function and drop the once_x variable
    s << "if (once_x) { \n" <<
    m_buy_tree->get_mql_init_recursive() <<
    m_sell_tree->get_mql_init_recursive() <<
    m_close_tree->get_mql_init_recursive() <<
    m_amount_per_trade->get_mql_init_recursive() <<
    m_reversed_buysell->get_mql_init_recursive() <<
    ";\nonce_x = false; \n } \n\n";

    s << "if (MarketPosition() != 0) \n" <<
    "{ \n" <<
    "   last_profit = PositionProfit(); \n" <<
    "} \n";

    // Tick the stops first
    s << m_stop_loss_tree->get_mql_tick_recursive() <<
    m_take_profit_tree->get_mql_tick_recursive() << ";\n";

    s << "t_buying = false; t_selling = false; t_closing = false; order = 0;\n";

    s << m_buy_tree->get_mql_tick_recursive() <<
    m_sell_tree->get_mql_tick_recursive() <<
    m_close_tree->get_mql_tick_recursive() <<
    m_amount_per_trade->get_mql_tick_recursive() <<
    m_reversed_buysell->get_mql_tick_recursive() << ";\n";

    s << "t_buying  = " << m_buy_tree->get_mql_output() << "; \n";
    s << "t_selling = " << m_sell_tree->get_mql_output() << "; \n";
    s << "t_closing = " << m_close_tree->get_mql_output() << "; \n";
    s << "qty = " << m_amount_per_trade->get_mql_output() << "; \n";
    s << "rev_buysell = " << m_reversed_buysell->get_mql_output() << "; \n";

    // determine order type
    s << format(
            "if (((t_closing) || (%s) || (%s)) && (MarketPosition() != 0)) \n"
                    "{ \n"
                    "   if (%s) { order = 4; } \n"
                    "   else if (%s) { order = 5; } \n"
                    "   else { order = 3; } \n"
                    "} \n"
                    "else \n"
                    "{ \n"
                    "   if (t_buying && (! t_selling) && (MarketPosition() == 0)) \n"
                    "   { \n"
                    "      order = 1; \n"
                    "   } \n"
                    "   if (t_selling && (! t_buying) && (MarketPosition() == 0)) \n"
                    "   { \n"
                    "      order = 2; \n"
                    "   } \n"
                    "} \n")
         % m_stop_loss_tree->get_mql_output() % m_take_profit_tree->get_mql_output()
         % m_stop_loss_tree->get_mql_output() % m_take_profit_tree->get_mql_output();

    // Buying template
    std::stringstream ssbuy;
    ssbuy << format(
            "if (rev_buysell == false) \n"
                    "{ \n"
                    "   %s;  %s;        \n"
                    "   BuyOrder(qty);  \n"
                    "}                  \n"
                    "else               \n"
                    "{                  \n"
                    "   %s;  %s;        \n"
                    "   SellOrder(qty); \n"
                    "} \n")
             % m_stop_loss_tree->get_mql_init_buying() % m_take_profit_tree->get_mql_init_buying()
             % m_stop_loss_tree->get_mql_init_selling() % m_take_profit_tree->get_mql_init_selling();

    // Selling template
    std::stringstream sssell;
    sssell << format(
            "if (rev_buysell == false) \n"
                    "{ \n"
                    "   %s;  %s;        \n"
                    "   SellOrder(qty); \n"
                    "}                  \n"
                    "else               \n"
                    "{                  \n"
                    "   %s;  %s;        \n"
                    "   BuyOrder(qty);  \n"
                    "} \n")
              % m_stop_loss_tree->get_mql_init_selling() % m_take_profit_tree->get_mql_init_selling()
              % m_stop_loss_tree->get_mql_init_buying() % m_take_profit_tree->get_mql_init_buying();

    std::string sclose = "CloseOrder(qty, \"\"); \n";
    std::string sclose_sl = "CloseOrder(qty, \"Stop Loss\"); \n";
    std::string sclose_tp = "CloseOrder(qty, \"Take Profit\"); \n";

    // add to master string stream
    s << format(
            "if (order >= 3) \n"
                    "{ \n"
                    "   if (order == 4) \n"
                    "   { \n"
                    "      last_exit_reason = -1; \n"
                    "      %s; \n"
                    "   } \n"
                    "   else if (order == 5) \n"
                    "   { \n"
                    "      last_exit_reason = 1; \n"
                    "      %s; \n"
                    "   } \n"
                    "   else \n"
                    "   { \n"
                    "      last_exit_reason = 0; \n"
                    "      %s;  \n"
                    "   } \n"
                    "} \n"
                    "else \n"
                    "{ \n"
                    "   if (order == 1) \n"
                    "   { \n"
                    "      %s; \n"
                    "   } \n"
                    "   if (order == 2) \n"
                    "   { \n"
                    "      %s;  \n"
                    "   } \n"
                    "} \n")
         % sclose_sl
         % sclose_tp
         % sclose
         % ssbuy.str()
         % sssell.str();

    s << " } \n"; // end execute at new bars only
    s << " return (0); } \n"; // end of start()
#endif
    return s.str();
}


SignalTreeMultiStrategy::SignalTreeMultiStrategy(const std::string &a_inp,
                                                 const MultiDataMatrix &mdata,
                                                 const std::string &a_comment /*= ""*/ )
{

    std::string t_ap_inp = remove_comments(a_inp);

    // first strip the input from any whitespace
    std::string t_inp;
    for (unsigned int i = 0; i < t_ap_inp.size(); i++)
    {
        if (!((t_ap_inp[i] == ' ') ||
              (t_ap_inp[i] == '\n') ||
              (t_ap_inp[i] == '\t') ||
              (t_ap_inp[i] == '\r')))
            t_inp += t_ap_inp[i];
    }

    // Split the input by strategy (with |)
    std::vector<std::string> master_inputs;
    split(master_inputs, t_inp, is_any_of("|"));
    
    // the variable map
    std::map<std::string, Signal *> vars;
    
    // Now for each master input..
    for (unsigned int minp = 0; minp < master_inputs.size(); minp++)
    {
        // split into statements
        std::vector<std::string> statements;
        split(statements, master_inputs[minp], is_any_of(";"));
    
        std::vector<Signal *> buyifs;
        std::vector<Signal *> sellifs;
        std::vector<Signal *> closeifs;
    
        if (statements.size() > 0)
        {
            // This is either a variable assignment or BUY_IF/SELL_IF/CLOSE_IF keyword
            for (int i = 0; i < statements.size(); i++)
            {
                // check for keyword
                if (statements[i].find("BUY_IF") == 0)
                {
                    // found BUY_IF
                    // consider the rest of the string to be a signal
                    statements[i].erase(0, std::string("BUY_IF").size());
                    Signal *t = EvalExpr(statements[i], vars);
                    buyifs.push_back(t);
                }
                else if (statements[i].find("SELL_IF") == 0)
                {
                    // found SELL_IF
                    // consider the rest of the string to be a signal
                    statements[i].erase(0, std::string("SELL_IF").size());
                    Signal *t = EvalExpr(statements[i], vars);
                    sellifs.push_back(t);
                }
                else if (statements[i].find("CLOSE_IF") == 0)
                {
                    // found CLOSE_IF
                    // consider the rest of the string to be a signal
                    statements[i].erase(0, std::string("CLOSE_IF").size());
                    Signal *t = EvalExpr(statements[i], vars);
                    closeifs.push_back(t);
                }
                else
                {
                    if (statements[i].size() > 0)
                    {
                        // this must be a variable assignment
                        std::vector<std::string> v;
                        split(v, statements[i], is_any_of("="));
                        // in case there are more '=', merge everything after the first one
                        std::string ls, rs;
                        if (v.size() > 1)
                        {
                            ls = v[0];
                            for (int j = 1; j < v.size(); j++)
                            {
                                if (j > 1)
                                {
                                    rs += "=";
                                }
        
                                rs += v[j];
                            }
    
                            // create a new variable with the given name
                            ParameterMap ow;
                            std::vector<Signal *> ch;
    
                            // if the variable ends with a star, then the variable wants its log enabled
                            if (ls[ls.size()-1] == '*')
                            {
                                ls.erase(ls.size()-1);
        
                                // so enable the parameter
                                SignalParameter p(ls);
                                ow["log"] = p;
                            }
                            
                            // its children are the right side of the assignment
                            Signal *s = EvalExpr(rs, vars);
                            ch.push_back(s);
                            Signal *vr = new VariableSignal(ow, ch);
                            vr->m_name = ls;
    
                            // Check if a variable with that name exists already.
                            // Rename it if so before we overwrite it (by adding shit to it)
                            // This will prevent memory leaks
                            bool there = false;
                            for (auto it = vars.begin(); it != vars.end(); it++)
                            {
                                if (it->first == ls)
                                {
                                    there = true;
                                    break;
                                }
                            }
    
                            if (there)
                            {
                                vars[ls + "09385435"] = vars[ls];
                            }
                            
                            vars[ls] = vr;
    
                        }
                        else
                        {
                            throw std::runtime_error("Bad form of variable assignment or unknown error");
                        }
                    }
                }
            }
        }
        else
        {
            throw std::runtime_error("No statements found in strategy");
        }
    
        // add the trees
        m_buy_tree.push_back(new SignalTree(buyifs[0]));
        m_sell_tree.push_back(new SignalTree(sellifs[0]));
        m_close_tree.push_back(new SignalTree(closeifs[0]));
        m_stop_loss_tree.push_back(new StopSignal());
        m_take_profit_tree.push_back(new StopSignal());
    }

    ASS(m_buy_tree.size() == mdata.symbols.size());
    ASS(m_sell_tree.size() == mdata.symbols.size());
    ASS(m_close_tree.size() == mdata.symbols.size());
    ASS(m_stop_loss_tree.size() == mdata.symbols.size());
    ASS(m_take_profit_tree.size() == mdata.symbols.size());

    m_num_symbols = (int) mdata.symbols.size();

    // set the parent of all signals to self
    unsigned int t_mbb = 0;
    m_max_bars_back = t_mbb;
    for (unsigned int i = 0; i < mdata.symbols.size(); i++)
    {
        m_buy_tree[i]->root->update_mparent_strategy_recursive(this);
        m_sell_tree[i]->root->update_mparent_strategy_recursive(this);
        m_close_tree[i]->root->update_mparent_strategy_recursive(this);
        m_stop_loss_tree[i]->update_mparent_strategy_recursive(this);
        m_take_profit_tree[i]->update_mparent_strategy_recursive(this);
    }

    m_once = true;
    m_amount_per_trade = 100000.0;
}

SignalTreeMultiStrategy::~SignalTreeMultiStrategy()
{
    std::set<Signal *> signals;
    
    for (int i = 0; i < m_buy_tree.size(); i++)
    {
        for (int j = 0; j < m_buy_tree[i]->slist.size(); j++)
        {
            signals.insert(m_buy_tree[i]->slist[j]);
        }
    }
    for (int i = 0; i < m_sell_tree.size(); i++)
    {
        for (int j = 0; j < m_sell_tree[i]->slist.size(); j++)
        {
            signals.insert(m_sell_tree[i]->slist[j]);
        }
    }
    for (int i = 0; i < m_close_tree.size(); i++)
    {
        for (int j = 0; j < m_close_tree[i]->slist.size(); j++)
        {
            signals.insert(m_close_tree[i]->slist[j]);
        }
    }
    
    for (auto it = signals.begin(); it != signals.end(); it++)
    {
        delete (*it);
    }
    
    for (unsigned int i = 0; i < m_stop_loss_tree.size(); i++)
    {
        delete m_stop_loss_tree[i]; // because it's old fashion
    }
    for (unsigned int i = 0; i < m_take_profit_tree.size(); i++)
    {
        delete m_take_profit_tree[i]; // because it's old fashion
    }
}

void SignalTreeMultiStrategy::mbind(const MultiDataMatrix &mdata)
{
    ASS(m_buy_tree.size() == mdata.symbols.size());
    ASS(m_sell_tree.size() == mdata.symbols.size());
    ASS(m_close_tree.size() == mdata.symbols.size());
    ASS(m_stop_loss_tree.size() == mdata.symbols.size());
    ASS(m_take_profit_tree.size() == mdata.symbols.size());

    // update data
   
    std::map<Signal *, int> times_updated;
    // initialize the map
    for (unsigned int i = 0; i < mdata.symbols.size(); i++)
    {
        for (auto it = m_buy_tree[i]->slist_sorted.begin(); it != m_buy_tree[i]->slist_sorted.end(); it++)
        { times_updated[(*it)] = 0; }
        for (auto it = m_sell_tree[i]->slist_sorted.begin(); it != m_sell_tree[i]->slist_sorted.end(); it++)
        { times_updated[(*it)] = 0; }
        for (auto it = m_close_tree[i]->slist_sorted.begin(); it != m_close_tree[i]->slist_sorted.end(); it++)
        { times_updated[(*it)] = 0; }
    }
    
    for (unsigned int i = 0; i < mdata.symbols.size(); i++)
    {
        // init
        // in topological order
        for (auto it = m_buy_tree[i]->slist_sorted.begin(); it != m_buy_tree[i]->slist_sorted.end(); it++)
        {
            if (times_updated[(*it)] == 0)
            {
                (*it)->update_mdata(mdata);
                times_updated[(*it)]++;
            }
        }
        for (auto it = m_sell_tree[i]->slist_sorted.begin(); it != m_sell_tree[i]->slist_sorted.end(); it++)
        {
            if (times_updated[(*it)] == 0)
            {
                (*it)->update_mdata(mdata);
                times_updated[(*it)]++;
            }
        }
        for (auto it = m_close_tree[i]->slist_sorted.begin(); it != m_close_tree[i]->slist_sorted.end(); it++)
        {
            if (times_updated[(*it)] == 0)
            {
                (*it)->update_mdata(mdata);
                times_updated[(*it)]++;
            }
        }
    }
    
    m_prev_idxd.clear();
    m_prev_idxd.resize(mdata.symbols.size());

    m_once = true;
}

std::vector<OrderPair> SignalTreeMultiStrategy::mtick(MultiDataMatrix &mdata, std::vector<Env> &env,
                                                      std::vector<int> &idxd)
{
    std::vector<OrderPair> orders;
    if (m_once)
    {
        std::map<Signal *, int> times_inited;
        // initialize the map
        for (unsigned int i = 0; i < mdata.symbols.size(); i++)
        {
            for (auto it = m_buy_tree[i]->slist_sorted.begin(); it != m_buy_tree[i]->slist_sorted.end(); it++)
            { times_inited[(*it)] = 0; }
            for (auto it = m_sell_tree[i]->slist_sorted.begin(); it != m_sell_tree[i]->slist_sorted.end(); it++)
            { times_inited[(*it)] = 0; }
            for (auto it = m_close_tree[i]->slist_sorted.begin(); it != m_close_tree[i]->slist_sorted.end(); it++)
            { times_inited[(*it)] = 0; }
        }
        
        for (unsigned int i = 0; i < mdata.symbols.size(); i++)
        {
            // init
            // in topological order
            for (auto it = m_buy_tree[i]->slist_sorted.begin(); it != m_buy_tree[i]->slist_sorted.end(); it++)
            {
                if (times_inited[(*it)] == 0)
                {
                    (*it)->compute_minit(env, idxd);
                    times_inited[(*it)]++;
                }
            }
            for (auto it = m_sell_tree[i]->slist_sorted.begin(); it != m_sell_tree[i]->slist_sorted.end(); it++)
            {
                if (times_inited[(*it)] == 0)
                {
                    (*it)->compute_minit(env, idxd);
                    times_inited[(*it)]++;
                }
            }
            for (auto it = m_close_tree[i]->slist_sorted.begin(); it != m_close_tree[i]->slist_sorted.end(); it++)
            {
                if (times_inited[(*it)] == 0)
                {
                    (*it)->compute_minit(env, idxd);
                    times_inited[(*it)]++;
                }
            }
        }

        m_once = false;
    }

    std::vector<bool> ticked;
    ticked.resize(mdata.symbols.size());
    
    std::map<Signal *, int> times_ticked;
    // initialize the map
    for (unsigned int i = 0; i < mdata.symbols.size(); i++)
    {
        for (auto it = m_buy_tree[i]->slist_sorted.begin(); it != m_buy_tree[i]->slist_sorted.end(); it++)
        { times_ticked[(*it)] = 0; }
        for (auto it = m_sell_tree[i]->slist_sorted.begin(); it != m_sell_tree[i]->slist_sorted.end(); it++)
        { times_ticked[(*it)] = 0; }
        for (auto it = m_close_tree[i]->slist_sorted.begin(); it != m_close_tree[i]->slist_sorted.end(); it++)
        { times_ticked[(*it)] = 0; }
    }

    for (unsigned int i = 0; i < mdata.symbols.size(); i++)
    {
        // only if not "stuck" in time, waiting for others to catch up
        //if (idxd[i] != m_prev_idxd[i])
            // TODO: Solve this problem. Maybe the lagging signals are OK,
            // or the idx can be used instead of internal counters
        {
            // in topological order
            for (auto it = m_buy_tree[i]->slist_sorted.begin(); it != m_buy_tree[i]->slist_sorted.end(); it++)
            {
                if (times_ticked[(*it)] == 0)
                {
                    (*it)->compute_mtick(env, idxd);
                    (*it)->do_log(logs);
                    times_ticked[(*it)]++;
                }
            }
            for (auto it = m_sell_tree[i]->slist_sorted.begin(); it != m_sell_tree[i]->slist_sorted.end(); it++)
            {
                if (times_ticked[(*it)] == 0)
                {
                    (*it)->compute_mtick(env, idxd);
                    (*it)->do_log(logs);
                    times_ticked[(*it)]++;
                }
            }
            for (auto it = m_close_tree[i]->slist_sorted.begin(); it != m_close_tree[i]->slist_sorted.end(); it++)
            {
                if (times_ticked[(*it)] == 0)
                {
                    (*it)->compute_mtick(env, idxd);
                    (*it)->do_log(logs);
                    times_ticked[(*it)]++;
                }
            }

            ticked[i] = true;
        }

        //m_prev_idxd[i] = idxd[i];
    }

    for (int i = 0; i < (int) (mdata.symbols.size()); i++)
    {
        if (ticked[i])
        {
            bool bu = get<bool>(m_buy_tree[i]->root->get_output());
            bool se = get<bool>(m_sell_tree[i]->root->get_output());
            bool cl = get<bool>(m_close_tree[i]->root->get_output());
            int qty = (int) (100000.0 / mdata.symbols[i].close[idxd[i]]);

            if (cl)
            {
                orders.push_back(OrderPair(i, new Order(CLOSE)));
            }
            else
            {
                if (!(bu && se))
                {
                    if (bu)
                    {
                        orders.push_back(OrderPair(i, new Order(BUY, qty)));
                    }
                    else if (se)
                    {
                        orders.push_back(OrderPair(i, new Order(SELL, qty)));
                    }
                }
            }
        }
    }

    return orders;
}

std::vector<std::string> SignalTreeMultiStrategy::print_EasyLanguage(bool debug/*=false*/,
                                                                     bool readable/*=true*/ )
{
    std::vector<std::string> all;

    for (int mdt_idx = 0; mdt_idx < m_num_symbols; mdt_idx++)
    {
        std::stringstream s;

        // initial boilerplate
        s << format("using elsystem; \n"
                            "input: amount_per_trade(%3.14f); \n"
                            "input: reversed_buysell(false); \n")
             % m_amount_per_trade;

        // prepare the inputs
        // get the ELIs
        std::vector<ELI> all_elis;
        m_buy_tree[mdt_idx]->root->fill_ELI_list(all_elis);
        m_sell_tree[mdt_idx]->root->fill_ELI_list(all_elis);
        m_close_tree[mdt_idx]->root->fill_ELI_list(all_elis);
        m_stop_loss_tree[mdt_idx]->fill_ELI_list(all_elis);
        m_take_profit_tree[mdt_idx]->fill_ELI_list(all_elis);

        ExplicitParameterMap inps_map;
        for (unsigned int i = 0; i < all_elis.size(); i++)
        {
            // not in map?
            if (inps_map.count(all_elis[i].name) == 0)
            {
                // now it is
                inps_map[all_elis[i].name] = all_elis[i].value;
            }
            else
            {
                // in map

                // the types differ?
                if (inps_map[all_elis[i].name].type() != all_elis[i].value.type())
                {
                    throw std::runtime_error("EasyLanguage inputs that have same name but different types: "
                                             + all_elis[i].name);
                }

                if (inps_map[all_elis[i].name].type() == typeid(Time))
                {
                    if (get<Time>(inps_map[all_elis[i].name]) != get<Time>(all_elis[i].value))
                    {
                        throw std::runtime_error("EasyLanguage inputs that have same name/type but different values: "
                                                 + all_elis[i].name);
                    }
                }

                if (inps_map[all_elis[i].name].type() == typeid(std::string))
                {
                    if (get<std::string>(inps_map[all_elis[i].name]) != get<std::string>(all_elis[i].value))
                    {
                        throw std::runtime_error("EasyLanguage inputs that have same name/type but different values: "
                                                 + all_elis[i].name);
                    }
                }

                if (inps_map[all_elis[i].name].type() == typeid(bool))
                {
                    if (get<bool>(inps_map[all_elis[i].name]) != get<bool>(all_elis[i].value))
                    {
                        throw std::runtime_error("EasyLanguage inputs that have same name/type but different values: "
                                                 + all_elis[i].name);
                    }
                }

                if (inps_map[all_elis[i].name].type() == typeid(int))
                {
                    if (get<int>(inps_map[all_elis[i].name]) != get<int>(all_elis[i].value))
                    {
                        throw std::runtime_error("EasyLanguage inputs that have same name/type but different values: "
                                                 + all_elis[i].name);
                    }
                }

                if (inps_map[all_elis[i].name].type() == typeid(double))
                {
                    if (get<double>(inps_map[all_elis[i].name]) != get<double>(all_elis[i].value))
                    {
                        throw std::runtime_error("EasyLanguage inputs that have same name/type but different values: "
                                                 + all_elis[i].name);
                    }
                }
            }
        }

        // now iterate over the inputs and output the code
        for (ExplicitParameterMap::iterator it = inps_map.begin();
             it != inps_map.end(); it++)
        {
            // time-based inputs are split in 3 integers
            if ((it->second).type() == typeid(Time))
            {
                s << format(
                        "input: %s(%s); \n"
                                "input: %s(%s); \n"
                                "input: %s(%s); \n"
                )
                     % ((it->first) + "_H") % (get<Time>(it->second)).hour
                     % ((it->first) + "_M") % (get<Time>(it->second)).minute
                     % ((it->first) + "_S") % (get<Time>(it->second)).second;
            }
            else if ((it->second).type() == typeid(std::string))
            {
                s << format("input: %s(\"%s\"); \n") % it->first % get<std::string>(it->second);
            }
            else if ((it->second).type() == typeid(bool))
            {
                s << format("input: %s(%s); \n") % it->first % ((get<bool>(it->second) == true) ? "true" : "false");
            }
            else if ((it->second).type() == typeid(int))
            {
                s << format("input: %s(%d); \n") % it->first % get<int>(it->second);
            }
            else if ((it->second).type() == typeid(double))
            {
                s << format("input: %s(%3.14f); \n") % it->first % get<double>(it->second);
            }
            else
            {
                throw std::runtime_error("Unknown input type");
            }
        }
    
        // VariableSignal declarations for the main loop
        s << "variable: once_x(true), qty(0), t_buying(false), t_selling(false), t_closing(false); \n"
                "variable: order(0); \n" // 0 - no order, 1 - buy, 2 - sell, 3 - close
                "variable: last_profit(0), last_exit_reason(0); \n"
                "variable: tmp(0.0), idx(0); \n"
                "variable: datetime cur_datetime(null); \n";


        // Signal variable declarations
        s << m_buy_tree[mdt_idx]->root->get_el_var_decl() <<
          m_sell_tree[mdt_idx]->root->get_el_var_decl() <<
          m_close_tree[mdt_idx]->root->get_el_var_decl() <<
          m_stop_loss_tree[mdt_idx]->get_el_var_decl() <<
          m_take_profit_tree[mdt_idx]->get_el_var_decl();

        // Runtime boilerplate code, any precomputations, etc.
        s << "cur_datetime = BarDateTime[0]; \n";

        // Signal init()s
        s << "if once_x then \nbegin \n" <<
          m_buy_tree[mdt_idx]->root->get_el_init_recursive() <<
          m_sell_tree[mdt_idx]->root->get_el_init_recursive() <<
          m_close_tree[mdt_idx]->root->get_el_init_recursive() <<
          "\nonce_x = false; \nend; \n\n";

        s << "if MarketPosition <> 0 then " <<
        "begin " <<
        "   last_profit = PositionProfit; " <<
        "end; ";

        // Tick the stops first
        s << m_stop_loss_tree[mdt_idx]->get_el_tick_recursive() <<
        m_take_profit_tree[mdt_idx]->get_el_tick_recursive();

        s << "\nqty = round(amount_per_trade / close, 0);\n"
                "t_buying = false; t_selling = false; t_closing = false; order = 0;\n";
    
        s << m_buy_tree[mdt_idx]->root->get_el_tick_recursive() <<
          m_sell_tree[mdt_idx]->root->get_el_tick_recursive() <<
          m_close_tree[mdt_idx]->root->get_el_tick_recursive();
    
        s << "t_buying  = " << m_buy_tree[mdt_idx]->root->get_el_output() << "; \n";
        s << "t_selling = " << m_sell_tree[mdt_idx]->root->get_el_output() << "; \n";
        s << "t_closing = " << m_close_tree[mdt_idx]->root->get_el_output() << "; \n";

        // determine order type
        s << format("if t_closing or %s or %s then "
                            "begin "
                            "   if %s then order = 4 "
                            "   else if %s then order = 5 "
                            "   else order = 3; "
                            "end "
                            "else "
                            "begin "
                            "   if t_buying and (not t_selling) then \n"
                            "   begin\n"
                            "      order = 1;\n"
                            "   end;  \n"
                            "   if t_selling and (not t_buying) then  \n"
                            "   begin \n"
                            "      order = 2;  \n"
                            "   end; \n"
                            "end;")
             % m_stop_loss_tree[mdt_idx]->get_el_output() % m_take_profit_tree[mdt_idx]->get_el_output()
             % m_stop_loss_tree[mdt_idx]->get_el_output() % m_take_profit_tree[mdt_idx]->get_el_output();

        // Buying template
        std::stringstream ssbuy;
        ssbuy << format(
                "if reversed_buysell = false then  \n"
                        "begin \n"
                        "   %s  %s    \n"
                        "   buy qty shares this bar; \n"
                        "end         \n"
                        "else        \n"
                        "begin       \n"
                        "   %s  %s    \n"
                        "   sellshort qty shares this bar; \n"
                        "end; \n")
                 % m_stop_loss_tree[mdt_idx]->get_el_init_buying() % m_take_profit_tree[mdt_idx]->get_el_init_buying()
                 % m_stop_loss_tree[mdt_idx]->get_el_init_selling() %
                 m_take_profit_tree[mdt_idx]->get_el_init_selling();

        // Selling template
        std::stringstream sssell;
        sssell << format(
                "if reversed_buysell = false then  \n"
                        "begin \n"
                        "   %s  %s    \n"
                        "   sellshort qty shares this bar; \n"
                        "end         \n"
                        "else        \n"
                        "begin       \n"
                        "   %s  %s    \n"
                        "   buy qty shares this bar; \n"
                        "end; \n")
                  % m_stop_loss_tree[mdt_idx]->get_el_init_selling() %
                  m_take_profit_tree[mdt_idx]->get_el_init_selling()
                  % m_stop_loss_tree[mdt_idx]->get_el_init_buying() % m_take_profit_tree[mdt_idx]->get_el_init_buying();

        std::string sclose = "sell this bar; buytocover this bar; \n";
        std::string sclose_sl = "sell (\"Stop Loss LX\") this bar; buytocover (\"Stop Loss SX\") this bar; \n";
        std::string sclose_tp = "sell (\"Take Profit LX\") this bar; buytocover (\"Take Profit SX\") this bar; \n";

        // add to master string stream
        s << format("if (order>=3) then  \n"
                            "begin \n"
                            "   if (order=4) then "
                            "   begin "
                            "      last_exit_reason = -1;"
                            "      %s "
                            "   end "
                            "   else if (order=5) then "
                            "   begin "
                            "      last_exit_reason = 1;"
                            "      %s "
                            "   end "
                            "   else "
                            "   begin "
                            "      last_exit_reason = 0;"
                            "      %s;  \n"
                            "   end; "
                            "end \n"
                            "else \n"
                            "begin \n"
                            "   if (order = 1) then \n"
                            "   begin\n"
                            "      %s;\n"
                            "   end;  \n"
                            "   if (order = 2) then  \n"
                            "   begin \n"
                            "      %s;  \n"
                            "   end; \n"
                            "end; \n")
             % sclose_sl
             % sclose_tp
             % sclose
             % ssbuy.str()
             % sssell.str();

        all.push_back(s.str());
    }

    // possibly needs switching of Data1 <-> Data0 for the second string
    // when outputting pair strategies

    return all;
}

std::vector<std::string> SignalTreeMultiStrategy::print_MQL4(bool debug/*=false*/,
                                                             bool readable/*=true*/ )
{
    return std::vector<std::string>();
}


SignalTreeLoggerStrategy::SignalTreeLoggerStrategy(const std::string &a_inp)
{
    // first strip the input from any whitespace
    std::string t_inp;
    for (unsigned int i = 0; i < a_inp.size(); i++)
    {
        if (!((a_inp[i] == ' ') ||
              (a_inp[i] == '\n') ||
              (a_inp[i] == '\t') ||
              (a_inp[i] == '\r')))
            t_inp += a_inp[i];
    }
    
    m_tree = EvalExpr(t_inp, std::map<std::string, Signal *>());
    m_once = true;
}

SignalTreeLoggerStrategy::~SignalTreeLoggerStrategy()
{
    delete m_tree;
}

void SignalTreeLoggerStrategy::bind(const DataMatrix &data)
{
    m_tree->update_data_recursive(data);
}

Order *SignalTreeLoggerStrategy::tick(const DataMatrix &data, Env &env, const int idx)
{
    if (m_once)
    {
        m_tree->compute_init_recursive(env, idx);
        m_once = false;
    }

    m_tree->compute_tick_recursive(env, idx);

    ReturnVariant outp = m_tree->get_output();

    if (m_tree->m_return_type == RT_INT)
    {
        m_int_result.push_back(get<int>(outp));
    }
    if (m_tree->m_return_type == RT_FLOAT)
    {
        m_float_result.push_back(get<double>(outp));
    }
    if (m_tree->m_return_type == RT_BOOL)
    {
        m_bool_result.push_back(get<bool>(outp));
    }

    return NULL;
}


SignalTreeMultiLoggerStrategy::SignalTreeMultiLoggerStrategy(const std::string &a_inp, MultiDataMatrix &a_mdata)
{
    // first strip the input from any whitespace
    std::string t_inp;
    for (unsigned int i = 0; i < a_inp.size(); i++)
    {
        if (!((a_inp[i] == ' ') ||
              (a_inp[i] == '\n') ||
              (a_inp[i] == '\t') ||
              (a_inp[i] == '\r')))
            t_inp += a_inp[i];
    }
    t_inp = remove_comments(t_inp);

    for (int i = 0; i < a_mdata.symbols.size(); i++)
    {
        m_tree.push_back(EvalExpr(t_inp, std::map<std::string, Signal *>()));
    }
    m_once = true;

    m_int_result.resize(a_mdata.symbols.size());
    m_float_result.resize(a_mdata.symbols.size());
    m_bool_result.resize(a_mdata.symbols.size());
}

SignalTreeMultiLoggerStrategy::~SignalTreeMultiLoggerStrategy()
{
    for (int i = 0; i < m_tree.size(); i++) delete m_tree[i];
}

void SignalTreeMultiLoggerStrategy::mbind(const MultiDataMatrix &mdata)
{
    // update data
    m_once = true;
    for (int i = 0; i < m_tree.size(); i++) m_tree[i]->update_mdata_recursive(mdata);
    m_dts.resize(mdata.symbols.size());
}

std::vector<OrderPair> SignalTreeMultiLoggerStrategy::mtick(MultiDataMatrix &mdata, std::vector<Env> &env,
                                                            std::vector<int> &idxd)
{
    std::vector<OrderPair> orders;
    if (m_once)
    {
        for (unsigned int i = 0; i < mdata.symbols.size(); i++)
        {
            m_tree[i]->compute_minit_recursive(env, idxd);
        }

        m_once = false;
    }

    for (int i = 0; i < (int) (mdata.symbols.size()); i++)
    {
        ReturnVariant outp;

        m_tree[i]->compute_mtick_recursive(env, idxd);
        outp = m_tree[i]->get_output();

        if (m_tree[i]->m_return_type == RT_INT)
        {
            m_int_result[i].push_back(get<int>(outp));
        }
        if (m_tree[i]->m_return_type == RT_FLOAT)
        {
            m_float_result[i].push_back(get<double>(outp));
        }
        if (m_tree[i]->m_return_type == RT_BOOL)
        {
            m_bool_result[i].push_back(get<bool>(outp));
        }

        m_dts[i].push_back(mdata.symbols[i].dt[idxd[i]]);
    }

    return orders;
}


void SignalTree::topo_sort()
{
    slist_sorted.clear();
    std::vector<Vertex> vlist;
    topological_sort(graph, back_inserter(vlist));
    for (auto it = vlist.begin(); it != vlist.end(); it++)
    {
        slist_sorted.push_back(smap_inv[(*it)]);
    }
}

void SignalTree::make_graph()
{
    std::vector<Edge> edge_array;
    for (auto it = slist.begin(); it != slist.end(); it++)
    {
        for (auto nit = slist.begin(); nit != slist.end(); nit++)
        {
            if (it != nit)
            {
                for (auto s = (*it)->m_children.begin(); s != (*it)->m_children.end(); s++)
                {
                    if ((*nit) == (*s))
                    {
                        // there is an edge, nit is a child of it
                        edge_array.push_back(Edge(smap[(*it)], smap[(*nit)]));
                    }
                }
            }
        }
    }
    
    // declare a graph object
    graph = Graph(slist.size());
    
    // add the edges to the graph object
    for (int i = 0; i < edge_array.size(); ++i)
    {
        add_edge(edge_array[i].first, edge_array[i].second, graph);
    }
}

void SignalTree::update_maps()
{
    int idx = 0;
    smap.clear();
    smap_inv.clear();
    for (auto it = slist.begin(); it != slist.end(); it++)
    {
        smap[*it] = idx;
        smap_inv[idx] = *it;
        idx++;
    }
}

#pragma clang diagnostic pop