#ifndef signal_forest_h__
#define signal_forest_h__


#include <vector>
#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace boost;

#include "data.h"
#include "backtest_run.h"
#include "signals/base_signal.h"
#include "signals/stops.h"
#include "strategy.h"
#include "signal_tree.h"
#include "expression.h"

// Signal forest strategy - contains a list of signal tree strategies.
class SignalForestStrategy: public BaseStrategy
{
    std::vector< SignalTreeStrategy* > m_strategies;
    bool m_once;
    int m_who_entered_idx;

public:

    SignalForestStrategy();
    ~SignalForestStrategy();
    SignalForestStrategy(const std::string& a_inp);
    SignalForestStrategy(std::vector< SignalTreeStrategy* >& a_strategies);

    void add_strategy(SignalTreeStrategy& a_strategy);
    void clear_strategies();
    unsigned int num_strategies();

    virtual void bind(const DataMatrix& data);

    virtual Order *tick(const DataMatrix &data, Env &env, const unsigned int idx);
    std::string print_EasyLanguage(bool debug=false, bool readable=true);
    std::string print_MQL4(bool debug=false, bool readable=true);
};



#endif // signal_forest_h__
