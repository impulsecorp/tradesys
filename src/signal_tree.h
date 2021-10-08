#ifndef SIGNAL_TREE_H
#define SIGNAL_TREE_H

#include <vector>
#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <utility> // std::pair
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>

#include "expression.h"
#include "data.h"
#include "backtest_run.h"
#include "signals/base_signal.h"
#include "signals/stops.h"
#include "strategy.h"


std::string make_EL(std::string a_strategy);

std::string make_indicator_EL(std::string expr);
std::vector<std::string> make_multi_EL( std::string expr, MultiDataMatrix& mdata );
std::string make_forest_EL(std::string a_exprs);

std::string make_MQL4(std::string expr);
std::string make_forest_MQL4(std::string a_exprs);

// forward
Signal *EvalExpr(std::string a_expr, std::map<std::string, Signal *> a_variables);

// Boost adjacency list tree representation
typedef std::pair<int, int> Edge;
typedef property<vertex_name_t, std::string> VertexProperty;
typedef adjacency_list<vecS, vecS, bidirectionalS, VertexProperty> Graph;
typedef graph_traits<Graph>::vertex_descriptor Vertex;

class SignalTree
{
public:
    Signal *root; // points to the root of the tree
    std::vector<Signal *> slist; // flat list of signals
    std::vector<Signal *> slist_sorted; // sorted topologically
    
    std::map<Signal *, int> smap; // maps signals to integer indices
    std::map<int, Signal *> smap_inv; // reverse mapping (from indices to signals)
    Graph graph;
    
    void init()
    {
        slist = root->flatten_tree();
        slist_sorted.clear();
        smap.clear();
        smap_inv.clear();
        graph.clear();
        
        update_maps();
        make_graph();
        topo_sort();
    }
    
    SignalTree(Signal *a_root)
    {
        root = a_root;
        init();
    }
    
    SignalTree(std::string a_exp)
    {
        root = EvalExpr(a_exp, std::map<std::string, Signal *>());
        init();
    }
    
    ~SignalTree()
    {
    }
    
    void topo_sort();
    
    void make_graph();
    
    void update_maps();
};

// Signal tree strategy
class SignalTreeStrategy : public BaseStrategy
{
public:
    SignalTree *buytree;
    SignalTree *selltree;
    SignalTree *closetree;
    SignalTree *amttree; // must return float
    SignalTree *rvtree; // must return bool

    // for limit orders
    SignalTree *buylimittree_at_price;
    SignalTree *selllimittree_at_price;
    SignalTree *buylimittree_condition;
    SignalTree *selllimittree_condition;

    std::string m_comment;
    bool m_once;

    // These can be accessed by all the signals
    // The stop loss/take profit signals
    StopSignal* m_stop_loss_tree;
    StopSignal* m_take_profit_tree;

    // The last closed position's profit. 
    // Equal to current profit if still in position.
    double m_last_profit;
    // The reason for the last exit - -1 is stop loss, 0 is normal exit, 1 is take profit. 
    int m_last_exit_reason;

    // Can be constructed by string only for now (it's enough for everything).
    SignalTreeStrategy(const std::string& a_inp,
                       const std::string& a_comment = "");
    ~SignalTreeStrategy();

    virtual void bind(const DataMatrix& data);

    virtual Order *tick(const DataMatrix &data, Env &env, const int idx);
    std::string print_EasyLanguage(bool debug=false, bool readable=true);
    std::string print_MQL4(bool debug=false, bool readable=true);
    
    SignalTree * op_cascade(const std::vector<Signal *> &trees);
};


// Signal tree multi strategy
class SignalTreeMultiStrategy: public BaseStrategy
{
public:
    // Each symbol gets its own set of 5 signal trees
    std::vector<SignalTree *> m_buy_tree;
    std::vector<SignalTree *> m_sell_tree;
    std::vector<SignalTree *> m_close_tree;
    std::vector<StopSignal*> m_stop_loss_tree;
    std::vector<StopSignal*> m_take_profit_tree;

    // for sync
    std::vector<int> m_prev_idxd;

    double m_amount_per_trade;
    std::string m_comment;
    bool m_once;
    int m_num_symbols;

    // The last closed position's profit. 
    // Equal to current profit if still in position.
    std::vector<double> m_last_profit;
    // The reason for the last exit - -1 is stop loss, 0 is normal exit, 1 is take profit. 
    std::vector<int> m_last_exit_reason;

    // Needs a string and the data. 
    SignalTreeMultiStrategy(const std::string& a_inp, const MultiDataMatrix& mdata, 
                            const std::string& a_comment = "");
    ~SignalTreeMultiStrategy();

    virtual void mbind(const MultiDataMatrix& mdata);

    virtual std::vector<OrderPair>
            mtick(MultiDataMatrix &data, std::vector<Env> &env, std::vector<int> &idxd);

    std::vector<std::string> print_EasyLanguage(bool debug=false, bool readable=true);
    std::vector<std::string> print_MQL4(bool debug=false, bool readable=true);
};


// This strategy just runs through the data set and logs whetever the
// signal tree outputs. The log can be of ints, floats or bools.
// Very useful for debugging and visualizing strategies
class SignalTreeLoggerStrategy: public BaseStrategy
{
public:
    Signal* m_tree;

    std::vector<int>    m_int_result;
    std::vector<double> m_float_result;
    std::vector<bool>   m_bool_result;

    bool m_once;

    // Can be constructed by string only for now (it's enough for everything).
    SignalTreeLoggerStrategy(const std::string& a_inp);
    ~SignalTreeLoggerStrategy();

    virtual void bind(const DataMatrix& data);

    virtual Order *tick(const DataMatrix &data, Env &env, const int idx);
};


class SignalTreeMultiLoggerStrategy: public BaseStrategy
{
public:

    std::vector<Signal*> m_tree;

    std::vector< std::vector<int> >   m_int_result;
    std::vector< std::vector<double> > m_float_result;
    std::vector< std::vector<bool> >  m_bool_result;

    std::vector< std::vector<DateTime> > m_dts;

    bool m_once;
    // for sync
    std::vector<int> m_prev_idxd;
    std::vector<ReturnVariant> m_prev_outp;

    // Can be constructed by string only for now (it's enough for everything).
    SignalTreeMultiLoggerStrategy(const std::string &a_inp, MultiDataMatrix &a_mdata);
    ~SignalTreeMultiLoggerStrategy();

    virtual void mbind(const MultiDataMatrix& mdata);

    virtual std::vector<OrderPair>
            mtick(MultiDataMatrix &data, std::vector<Env> &env, std::vector<int> &idxd);
};




#endif // SIGNAL_TREE_H
