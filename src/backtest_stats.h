#ifndef BACKTEST_STATS_H
#define BACKTEST_STATS_H

#include <vector>
#include <string>
#include <sstream>
#include <boost/format.hpp>
#include "data.h"
#include "signals/common_types.h"

namespace py = boost::python;

class Quantity
{
public:
    int shares;
    double lots;

    Quantity();
    Quantity(int sh);
    Quantity(double lt);

    inline bool operator==(const Quantity& a) const
    {
        return ((shares == a.shares) && (lots == a.lots));
    }

    // Serialization
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & shares;
        ar & lots;
    }

    // Outputs the contents of the quantity (shares, lots, tons of apples, etc)
    void print();
    std::string print_str() const; // to a string
};

class TradeInfo
{
public:
    std::string symbol;
    PositionType position; // -1 : sell, 1 : buy

    DateTime datetime_of_entry;
    DateTime datetime_of_exit;

    Quantity quantity;
    double price_at_entry;
    double price_at_exit;

    double profit;
    double commission_paid;
    double slippage_paid;

    int time_duration; // in seconds
    int bars_duration;
    int bar_index_at_entry;
    int bar_index_at_exit;

    double runup;
    double drawdown;

    std::string comment;

    TradeInfo();

    inline bool operator==(const TradeInfo& a) const
    {
        return 
            ((symbol == a.symbol) && 
            (position == a.position) && 
            (datetime_of_entry == a.datetime_of_entry) && 
            (datetime_of_exit == a.datetime_of_exit) && 
            (quantity == a.quantity) && 
            (price_at_entry == a.price_at_entry) && 
            (price_at_exit == a.price_at_exit) && 
            (profit == a.profit) && 
            (commission_paid == a.commission_paid) && 
            (slippage_paid == a.slippage_paid) && 
            (time_duration == a.time_duration) && 
            (bars_duration == a.bars_duration) && 
            (bar_index_at_entry == a.bar_index_at_entry) && 
            (bar_index_at_exit == a.bar_index_at_exit) && 
            (runup == a.runup) && 
            (drawdown == a.drawdown));
    }

    // Outputs the contents of the TradeInfo
    void print();
    std::string print_str() const; // to a string

    // Serialization
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & symbol;
        ar & position;
        ar & datetime_of_entry;
        ar & datetime_of_exit;
        ar & quantity;
        ar & price_at_entry;
        ar & price_at_exit;
        ar & profit;
        ar & commission_paid;
        ar & slippage_paid;
        ar & time_duration;
        ar & bars_duration;
        ar & bar_index_at_entry;
        ar & bar_index_at_exit;
        ar & runup;
        ar & drawdown;
        ar & comment;
    }
};

class LimitOrderInfo
{
public:
    std::string symbol;
    PositionType position; // -1 : sell, 1 : buy

    DateTime datetime_of_creation;
    DateTime datetime_of_expiry;

    int bar_idx_at_creation;
    int bar_idx_at_expiry;

    bool did_enter;

    Quantity quantity;
    double price_for_entry;

    // Serialization
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & symbol;
        ar & position;
        ar & datetime_of_creation;
        ar & datetime_of_expiry;
        ar & quantity;
        ar & did_enter;
        ar & price_for_entry;
        ar & bar_idx_at_creation;
        ar & bar_idx_at_expiry;
    }

    inline bool operator==(const LimitOrderInfo& a) const
    {
        return  ((symbol == a.symbol) &&
                 (position == a.position) &&
                 (datetime_of_creation == a.datetime_of_creation) &&
                 (datetime_of_expiry == a.datetime_of_expiry) &&
                 (bar_idx_at_creation == a.bar_idx_at_creation) &&
                 (bar_idx_at_expiry == a.bar_idx_at_expiry) &&
                 (quantity == a.quantity) &&
                 (price_for_entry == a.price_for_entry) &&
                 (did_enter == a.did_enter));
    }
};

class BacktestResults
{
public:
    std::vector< TradeInfo > trades;

    // equity/balance history (can be empty)
    std::vector< DateTime > history;
    std::vector< double > equity_history;
    std::vector< double > balance_history;
    std::string strategy_name;

    // Serialization
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & trades;
        ar & history;
        ar & equity_history;
        ar & balance_history;
        ar & strategy_name;
    }
};

enum CommSlipType { SHARE = 0, TRADE = 1};
enum BacktestMode { TRADESTATION = 0, METATRADER = 1};

class BacktestOptions
{
public:
    BacktestMode backtest_mode;
    double initial_capital;
    double commission;
    double slippage;
    CommSlipType commission_type;
    CommSlipType slippage_type;

    Time market_open_time;  // if both are 00:00:00, it trades all the time (24x7).
    Time market_close_time;

    BacktestOptions();

    // Serialization
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & backtest_mode;
        ar & initial_capital;
        ar & commission;
        ar & slippage;
        ar & commission_type;
        ar & slippage_type;
        ar & market_open_time;
        ar & market_close_time;
    }
};



class BacktestStats
{
public:
    // Data members
    std::vector<std::string> symbols;
    double account_balance;
    double initial_capital;
    double net_profit;
    double gross_profit;
    double gross_loss;
    double profit_factor;
    int num_trades;
    int num_long_trades;
    int num_short_trades;
    int num_profit_trades;
    int num_loss_trades;
    int num_even_trades;
    double percent_profit_trades;
    TradeInfo largest_profit_trade;
    TradeInfo largest_loss_trade;
    int max_consecutive_profit_trades;
    int max_consecutive_loss_trades;
    double average_consecutive_profit_trades;
    double average_consecutive_loss_trades;
    double average_trade_net_profit; // (net profit / num trades)

    double average_profit_per_trade; // (gross profit / profit trades)
    double average_loss_per_trade; // (gross loss / loss trades)
    double ratio_of_avg_profit_avg_loss;

    double largest_runup;
    double largest_drawdown;
    TradeInfo largest_runup_trade;
    TradeInfo largest_drawdown_trade;
    double average_runup_per_trade;
    double average_drawdown_per_trade;
    double largest_runup_as_percent_of_initial_balance;
    double largest_drawdown_as_percent_of_initial_balance;
    
    int total_time_in_trades; // in seconds
    int total_bars_in_trades;

    double average_time_in_trades;
    double average_bars_in_trades;
    
    int total_time_between_trades;
    int total_bars_between_trades;

    double average_time_between_trades;
    double average_bars_between_trades;

    double percent_of_time_in_market;

    double account_size_required;
    double total_commissions_paid;
    double total_slippage_paid;
    int max_shares_or_contracts_held;
    int total_shares_or_contracts_held;
    double max_lots_traded;
    double total_lots_traded; // for FOREX

    std::string strategy_name;

    std::vector<TradeInfo> list_of_trades;
    DateTime start_datetime;
    DateTime end_datetime;
    int total_time; // in seconds
    double percent_short_trades;
    double percent_long_trades;
    
    // The logs strategies can set
    std::map<std::string, std::vector<ReturnVariant> > logs;
    std::vector<LimitOrderInfo> limit_orders;

    py::object get_logs()
    {
        py::dict d;
        for (auto it = logs.begin(); it != logs.end(); it++)
        {
            // make a list of values here
            py::list l;
            for (int i = 0; i < it->second.size(); i++)
            {
                if (it->second[i].type() == typeid(int))
                {
                    l.append(get<int>(it->second[i]));
                }
                if (it->second[i].type() == typeid(double))
                {
                    l.append(get<double>(it->second[i]));
                }
                if (it->second[i].type() == typeid(bool))
                {
                    l.append(get<bool>(it->second[i]));
                }
            }
            d[it->first] = l;
        }
        
        return d;
    }

    // Methods
    BacktestStats();
    BacktestStats(const DataMatrix& data, BacktestResults& res, const BacktestOptions& opt);
    BacktestStats(MultiDataMatrix& data, BacktestResults& res, std::vector<BacktestOptions>& opt);
    void compute( std::vector< TradeInfo >& a_trades, double a_init_capital, std::string a_strategy_name );
    void pycompute( py::list a_trades, double a_init_capital, std::string a_strategy_name );
    std::string print_str() const;
    std::string print_str_short() const;
    void print_file(char* filename) const;

    // Serialization
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & symbols;
        ar & account_balance;
        ar & initial_capital;
        ar & net_profit;
        ar & gross_profit;
        ar & gross_loss;
        ar & profit_factor;
        ar & num_trades;
        ar & num_long_trades;
        ar & num_short_trades;
        ar & num_profit_trades;
        ar & num_loss_trades;
        ar & num_even_trades;
        ar & percent_profit_trades;
        ar & largest_profit_trade;
        ar & largest_loss_trade;
        ar & percent_short_trades;
        ar & percent_long_trades;

        ar & max_consecutive_profit_trades;
        ar & max_consecutive_loss_trades;
        ar & average_consecutive_profit_trades;
        ar & average_consecutive_loss_trades;
        ar & average_trade_net_profit;
        ar & average_profit_per_trade;
        ar & average_loss_per_trade;
        ar & ratio_of_avg_profit_avg_loss;
        ar & largest_runup;
        ar & largest_drawdown;
        ar & largest_runup_trade;
        ar & largest_drawdown_trade;
        ar & average_runup_per_trade;
        ar & average_drawdown_per_trade;

        ar & largest_runup_as_percent_of_initial_balance;
        ar & largest_drawdown_as_percent_of_initial_balance;
        ar & total_time_in_trades;
        ar & total_bars_in_trades;
        ar & average_time_in_trades;
        ar & average_bars_in_trades;
        ar & total_time_between_trades;
        ar & total_bars_between_trades;

        ar & average_time_between_trades;
        ar & average_bars_between_trades;
        ar & percent_of_time_in_market;
        ar & account_size_required;

        ar & total_commissions_paid;
        ar & total_slippage_paid;
        ar & max_shares_or_contracts_held;
        ar & total_shares_or_contracts_held;
        ar & max_lots_traded;
        ar & total_lots_traded;

        ar & strategy_name;
        ar & list_of_trades;
        ar & limit_orders;
        ar & start_datetime;
        ar & end_datetime;
        ar & total_time;
    }
};


struct BacktestOptions_pickle_suite : py::pickle_suite
{
    static py::object getstate(const BacktestOptions& a)
    {
        std::ostringstream os;
        boost::archive::text_oarchive oa(os);
        oa << a;
        return py::str (os.str());
    }

    static void setstate(BacktestOptions& a, py::object entries)
    {
        py::str s = py::extract<py::str> (entries)();
        std::string st = py::extract<std::string>(s)();
        std::istringstream is (st);

        boost::archive::text_iarchive ia(is);
        ia >> a;
    }
};


struct BacktestStats_pickle_suite : py::pickle_suite
{
    static py::object getstate(const BacktestStats& a)
    {
        std::ostringstream os;
        boost::archive::text_oarchive oa(os);
        oa << a;
        return py::str (os.str());
    }

    static void setstate(BacktestStats& a, py::object entries)
    {
        py::str s = py::extract<py::str> (entries)();
        std::string st = py::extract<std::string>(s)();
        std::istringstream is (st);

        boost::archive::text_iarchive ia(is);
        ia >> a;
    }
};


#endif // BACKTEST_STATS_H
