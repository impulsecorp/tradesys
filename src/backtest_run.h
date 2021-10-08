#ifndef BACKTEST_RUN_H
#define BACKTEST_RUN_H

#include <string>
#include <vector>

#include "data.h"
#include "backtest_stats.h"

class Env
{
public:
    PositionType current_position; // -1 : selling, 0 : out of market, 1 : buying

    double     position_opened_at_price;
    DateTime  position_opened_at_time;
    int       position_opened_at_bar_idx;

    double     position_closed_at_price;
    DateTime  position_closed_at_time;
    int       position_closed_at_bar_idx;

    Quantity quantity_traded; 

    double previous_trade_profit;
    double runup;
    double drawdown;

    double potential_profit;
    double account_balance;

	BacktestMode backtest_mode;

    ////////////////////////
    // limit order variables
    ////////////////////////

    bool buy_limit_active;
    double buy_limit_price;
    DateTime buy_limit_opened_at_time; // for good until end of day
    int buy_limit_opened_at_idx; // for good until # of bars

    bool sell_limit_active;
    double sell_limit_price;
    DateTime sell_limit_opened_at_time; // for good until end of day
    int sell_limit_opened_at_idx; // for good until # of bars

    ///////////////////////
    // methods
    ///////////////////////

    void clear();

    // Initializes with zeroes
    Env();
};


enum OrderType { BUY, SELL, CLOSE, NO_ORDERS};

class Order
{
public:
    OrderType order_type; 
    Quantity quantity;
    std::string comment;

    Order();
    Order(OrderType type);
    Order(OrderType type, Quantity qty);
    Order(OrderType type, Quantity qty, std::string comm);
};

typedef std::pair<int, Order*> OrderPair;

// Functions

// forward
class BaseStrategy;

enum PriceType { PRICE_OPEN = 0, PRICE_CLOSE = 1};

// Backtests a tick() strategy on a single symbol and returns the results 
BacktestStats backtest_single(const DataMatrix& a_data, BaseStrategy& a_strategy, const BacktestOptions& a_options);

// Signal tree variant
BacktestStats backtest_single_stree( const DataMatrix& a_data, 
                               const std::string& a_strategy, 
                               const BacktestOptions& options );

// Signal forest variant
BacktestStats backtest_single_sforest( const DataMatrix& a_data, 
                                      const std::string& a_forest, 
                                      const BacktestOptions& a_options );

// Ticks
BacktestStats backtest_single_ticks(TickDataMatrix matrix, BaseStrategy& strategy, BacktestOptions options);


// Backtests a mtick() strategy on multiple unaligned symbols and merges the results.
BacktestStats backtest_multi(MultiDataMatrix& a_data,
                             BaseStrategy& a_strategy,
                             std::vector<BacktestOptions>& a_options);
// the signal tree variant
BacktestStats backtest_multi_stree( MultiDataMatrix& a_data,
                                    const std::string& a_strategy,
                                    const BacktestOptions& options );

// return the max_bars_back in the given signal tree
int get_stree_max_bars_back(const std::string &a_stree);

// These functions return data aligned logs of a single signal tree activity
// will raise std::runtime_error if the return type doesn't match the signal's return type
std::vector<int> get_int_stree_log(const std::string& a_stree,
		                           const DataMatrix& a_data);
std::vector<double> get_float_stree_log(const std::string& a_stree,
		                           const DataMatrix& a_data);
std::vector<bool> get_bool_stree_log(const std::string& a_stree,
		                           const DataMatrix& a_data);
// returns a python list
py::list get_stree_log(const std::string& a_stree, const DataMatrix& a_data);


// These functions return data aligned logs of a multi signal tree activity
// will raise std::runtime_error if the return type doesn't match the signal's return type
std::vector< std::vector<int> > get_int_multi_stree_log(const std::string& a_stree, MultiDataMatrix& a_mdata);
std::vector< std::vector<double> > get_float_multi_stree_log(const std::string& a_stree, MultiDataMatrix& a_mdata);
std::vector< std::vector<bool> > get_bool_multi_stree_log(const std::string& a_stree, MultiDataMatrix& a_mdata);

// returns a python list
py::list get_multi_stree_log(const std::string& a_stree, MultiDataMatrix& a_mdata);


#endif
