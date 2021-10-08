#ifndef python_bindings_h__
#define python_bindings_h__

#include <boost/python.hpp>
//#include <boost/python/numeric.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
namespace py = boost::python;
using namespace py;

#include "data.h"
#include "backtest_run.h"
#include "strategy.h"

#include "signals/base_signal.h"
#include "signals/times.h"
#include "signals/numeric.h"
#include "signals/feedback.h"
#include "signals/raw.h"
#include "signals/gaps.h"
#include "signals/crosses.h"
#include "signals/timesinarow.h"
#include "signals/happened.h"
#include "signals/ops.h"
#include "signals/constraints.h"
#include "signal_tree.h"

#include "expression.h"

#include <datetime.h> // compile with -I/path/to/python/include

BOOST_PYTHON_MODULE(_tradesys)
{
    enum_<PositionType>("PositionType")
        .value("SHORT_POSITION", SHORT_POSITION)
        .value("NO_POSITION", NO_POSITION)
        .value("LONG_POSITION", LONG_POSITION)
        ;

    enum_<CommSlipType>("CommSlipType")
        .value("SHARE", SHARE)
        .value("TRADE", TRADE)
        ;

    enum_<BacktestMode>("BacktestMode")
        .value("TRADESTATION", TRADESTATION)
        .value("METATRADER", METATRADER)
        ;

    enum_<ConstraintUsageMode>("ConstraintUsageMode")
        .value("CNT_MINMAX", CNT_MINMAX)
        .value("CNT_POSSIBLE_VALUES", CNT_POSSIBLE_VALUES)
        ;

    class_<DateTime>("DateTime", init<>())
		.def(init<int>())
		.def(init<int, int>())
		.def(init<int, int, int>())
    	.def(init<int, int, int, int, int>())
    	.def(init<int, int, int, int, int, int>())
        .def(init<int, int, int, int, int, int, int>())
        .def_readonly("day", &DateTime::day)
        .def_readonly("month", &DateTime::month)
        .def_readonly("year", &DateTime::year)
        .def_readonly("hour", &DateTime::hour)
        .def_readonly("minute", &DateTime::minute)
        .def_readonly("second", &DateTime::second)
        .def_readonly("millisecond", &DateTime::millisecond)

        .def("__repr__", &DateTime::print_str)
            .def(self < self)
            .def(self <= self)
            .def(self > self)
            .def(self >= self)
            .def(self == self)
            .def(self != self)
        .def_pickle(DateTime_pickle_suite());

    class_<Time>("Time", init<>())
		.def(init<int>())
		.def(init<int, int>())
		.def(init<int, int, int>())
        .def(init<int, int, int, int>())
        .def_readonly("hour", &Time::hour)
        .def_readonly("minute", &Time::minute)
        .def_readonly("second", &Time::second)
        .def_readonly("millisecond", &Time::millisecond)

        .def("__repr__", &Time::print_str)
            .def(self < self)
            .def(self <= self)
            .def(self > self)
            .def(self >= self)
            .def(self == self)
            .def(self != self)
        .def_pickle(Time_pickle_suite());

    class_<DataMatrix>("DataMatrix", init<>())
		.def_readwrite("name", &DataMatrix::name)
        .def_readwrite("open", &DataMatrix::open)
        .def_readwrite("high", &DataMatrix::high)
        .def_readwrite("low", &DataMatrix::low)
        .def_readwrite("close", &DataMatrix::close)
        .def_readwrite("volume", &DataMatrix::volume)
        .def_readwrite("dt", &DataMatrix::dt)

        .def("__len__",
        &DataMatrix::len)
        .def("len",
        &DataMatrix::len)

        .def("load_tsys",
        &DataMatrix::load_tsys)
        .def("dump_tsys",
        &DataMatrix::dump_tsys)
        .def("load_tradestation",
        &DataMatrix::load_tradestation)
        .def("load_metatrader4",
        &DataMatrix::load_metatrader4)
        .def("load_multicharts",
             &DataMatrix::load_multicharts)

        .def("clear",
        &DataMatrix::clear)
        .def("add_row",
        &DataMatrix::add_row)
        .def("clip_dt",
        &DataMatrix::clip_dt)
        .def("clip_idx",
        &DataMatrix::clip_idx)
        .def("slice_dt",
        &DataMatrix::slice_dt)
        .def("slice_idx",
        &DataMatrix::slice_idx)

        .def("get_dt_list",
        &DataMatrix::get_dt_list)

        //.def("make_ticks",
        //     &DataMatrix::make_ticks)

        .def_pickle(DataMatrix_pickle_suite());

    class_<MultiDataMatrix>("MultiDataMatrix", init<>())
		.def_readwrite("symbols", &MultiDataMatrix::symbols)
		.def("len", &MultiDataMatrix::len)
		.def("__len__", &MultiDataMatrix::len)
        .def("clip_dt",
        &MultiDataMatrix::clip_dt)
        .def("clip_idx",
        &MultiDataMatrix::clip_idx)
        .def("slice_dt",
        &MultiDataMatrix::slice_dt)
        .def("slice_idx",
        &MultiDataMatrix::slice_idx)
		.def_pickle(MultiDataMatrix_pickle_suite());

    class_<TickDataMatrix>("TickDataMatrix", init<>())
            .def_readwrite("name", &TickDataMatrix::name)
            .def_readwrite("price", &TickDataMatrix::price)
            .def_readwrite("dt", &TickDataMatrix::dt)

            .def("__len__",
                 &TickDataMatrix::len)
            .def("len",
                 &TickDataMatrix::len)
            .def("clear",
                 &TickDataMatrix::clear)
            .def("add_row",
                 &TickDataMatrix::add_tick)
            .def("clip_dt",
                 &TickDataMatrix::clip_dt)
            .def("clip_idx",
                 &TickDataMatrix::clip_idx)
            .def("slice_dt",
                 &TickDataMatrix::slice_dt)
            .def("slice_idx",
                 &TickDataMatrix::slice_idx)
            .def("get_dt_list",
                 &TickDataMatrix::get_dt_list)

            .def_pickle(TickDataMatrix_pickle_suite());

    class_<Quantity>("Quantity", init<>())
        .def_readwrite("shares", &Quantity::shares)
        .def_readwrite("lots", &Quantity::lots)
		.def("__repr__", &Quantity::print_str);

    class_<TradeInfo>("TradeInfo", init<>())
        .def_readwrite("symbol", &TradeInfo::symbol)
        .def_readwrite("position", &TradeInfo::position)
        .def_readwrite("datetime_of_entry", &TradeInfo::datetime_of_entry)
        .def_readwrite("datetime_of_exit", &TradeInfo::datetime_of_exit)
        .def_readwrite("quantity", &TradeInfo::quantity)
        .def_readwrite("price_at_entry", &TradeInfo::price_at_entry)
        .def_readwrite("price_at_exit", &TradeInfo::price_at_exit)
        .def_readwrite("profit", &TradeInfo::profit)
        .def_readwrite("commission_paid", &TradeInfo::commission_paid)
        .def_readwrite("slippage_paid", &TradeInfo::slippage_paid)
        .def_readwrite("time_duration", &TradeInfo::time_duration)
        .def_readwrite("bars_duration", &TradeInfo::bars_duration)
        .def_readwrite("bar_index_at_entry", &TradeInfo::bar_index_at_entry)
        .def_readwrite("bar_index_at_exit", &TradeInfo::bar_index_at_exit)
        .def_readwrite("runup", &TradeInfo::runup)
        .def_readwrite("drawdown", &TradeInfo::drawdown)
        .def_readwrite("comment", &TradeInfo::comment)

        .def("__repr__", &TradeInfo::print_str);

    class_<LimitOrderInfo>("LimitOrderInfo", init<>())
            .def_readwrite("symbol", &LimitOrderInfo::symbol)
            .def_readwrite("position", &LimitOrderInfo::position)
            .def_readwrite("datetime_of_creation", &LimitOrderInfo::datetime_of_creation)
            .def_readwrite("datetime_of_expiry", &LimitOrderInfo::datetime_of_expiry)
            .def_readwrite("bar_idx_at_creation", &LimitOrderInfo::bar_idx_at_creation)
            .def_readwrite("bar_idx_at_expiry", &LimitOrderInfo::bar_idx_at_expiry)
            .def_readwrite("quantity", &LimitOrderInfo::quantity)
            .def_readwrite("price_for_entry", &LimitOrderInfo::price_for_entry)
            .def_readwrite("did_enter", &LimitOrderInfo::did_enter);

    class_<BacktestResults>("BacktestResults", init<>())
        .def_readonly("trades", &BacktestResults::trades)
        .def_readonly("history", &BacktestResults::history)
        .def_readonly("equity_history", &BacktestResults::equity_history)
        .def_readonly("balance_history", &BacktestResults::balance_history)
        .def_readonly("strategy_name", &BacktestResults::strategy_name);


    class_<BacktestOptions>("BacktestOptions", init<>())
        .def_readwrite("backtest_mode", &BacktestOptions::backtest_mode)
        .def_readwrite("initial_capital", &BacktestOptions::initial_capital)
        .def_readwrite("commission", &BacktestOptions::commission)
        .def_readwrite("slippage", &BacktestOptions::slippage)
        .def_readwrite("commission_type", &BacktestOptions::commission_type)
        .def_readwrite("slippage_type", &BacktestOptions::slippage_type)
        .def_readwrite("market_open_time", &BacktestOptions::market_open_time)
        .def_readwrite("market_close_time", &BacktestOptions::market_close_time)
        .def_pickle(BacktestOptions_pickle_suite());
  
    class_<BacktestStats>("BacktestStats", init<>())
        .def_readonly("symbols", &BacktestStats::symbols)
        .def_readonly("account_balance", &BacktestStats::account_balance)
        .def_readonly("initial_capital", &BacktestStats::initial_capital)
        .def_readonly("net_profit", &BacktestStats::net_profit)
        .def_readonly("gross_profit", &BacktestStats::gross_profit)
        .def_readonly("gross_loss", &BacktestStats::gross_loss)
        .def_readonly("profit_factor", &BacktestStats::profit_factor)
        .def_readonly("num_trades", &BacktestStats::num_trades)
        .def_readonly("num_long_trades", &BacktestStats::num_long_trades)
        .def_readonly("num_short_trades", &BacktestStats::num_short_trades)
        .def_readonly("num_profit_trades", &BacktestStats::num_profit_trades)
        .def_readonly("num_loss_trades", &BacktestStats::num_loss_trades)
        .def_readonly("num_even_trades", &BacktestStats::num_even_trades)
        .def_readonly("percent_profit_trades", &BacktestStats::percent_profit_trades)
        .def_readonly("largest_profit_trade", &BacktestStats::largest_profit_trade)
        .def_readonly("largest_loss_trade", &BacktestStats::largest_loss_trade)
        .def_readonly("max_consecutive_profit_trades", &BacktestStats::max_consecutive_profit_trades)
        .def_readonly("max_consecutive_loss_trades", &BacktestStats::max_consecutive_loss_trades)
        .def_readonly("average_consecutive_profit_trades", &BacktestStats::average_consecutive_profit_trades)
        .def_readonly("average_consecutive_loss_trades", &BacktestStats::average_consecutive_loss_trades)
        .def_readonly("average_trade_net_profit", &BacktestStats::average_trade_net_profit)
        .def_readonly("average_profit_per_trade", &BacktestStats::average_profit_per_trade)
        .def_readonly("average_loss_per_trade", &BacktestStats::average_loss_per_trade)
        .def_readonly("ratio_of_avg_profit_avg_loss", &BacktestStats::ratio_of_avg_profit_avg_loss)
        .def_readonly("largest_runup", &BacktestStats::largest_runup)
        .def_readonly("largest_drawdown", &BacktestStats::largest_drawdown)
        .def_readonly("largest_runup_trade", &BacktestStats::largest_runup_trade)
        .def_readonly("largest_drawdown_trade", &BacktestStats::largest_drawdown_trade)
        .def_readonly("average_runup_per_trade", &BacktestStats::average_runup_per_trade)
        .def_readonly("average_drawdown_per_trade", &BacktestStats::average_drawdown_per_trade)
        .def_readonly("largest_runup_as_percent_of_initial_balance", &BacktestStats::largest_runup_as_percent_of_initial_balance)
        .def_readonly("largest_drawdown_as_percent_of_initial_balance", &BacktestStats::largest_drawdown_as_percent_of_initial_balance)
        .def_readonly("total_time_in_trades", &BacktestStats::total_time_in_trades)
        .def_readonly("total_bars_in_trades", &BacktestStats::total_bars_in_trades)
        .def_readonly("average_time_in_trades", &BacktestStats::average_time_in_trades)
        .def_readonly("average_bars_in_trades", &BacktestStats::average_bars_in_trades)
        .def_readonly("total_time_between_trades", &BacktestStats::total_time_between_trades)
        .def_readonly("total_bars_between_trades", &BacktestStats::total_bars_between_trades)
        .def_readonly("average_time_between_trades", &BacktestStats::average_time_between_trades)
        .def_readonly("average_bars_between_trades", &BacktestStats::average_bars_between_trades)
        .def_readonly("percent_of_time_in_market", &BacktestStats::percent_of_time_in_market)
        .def_readonly("account_size_required", &BacktestStats::account_size_required)
        .def_readonly("total_commissions_paid", &BacktestStats::total_commissions_paid)
        .def_readonly("total_slippage_paid", &BacktestStats::total_slippage_paid)
        .def_readonly("max_shares_or_contracts_held", &BacktestStats::max_shares_or_contracts_held)
        .def_readonly("total_shares_or_contracts_held", &BacktestStats::total_shares_or_contracts_held)
        .def_readonly("max_lots_traded", &BacktestStats::max_lots_traded)
        .def_readonly("total_lots_traded", &BacktestStats::total_lots_traded)
        .def_readonly("strategy_name", &BacktestStats::strategy_name)
        .def_readonly("list_of_trades", &BacktestStats::list_of_trades)
        .def_readonly("limit_orders", &BacktestStats::limit_orders)
        .def_readonly("start_datetime", &BacktestStats::start_datetime)
        .def_readonly("end_datetime", &BacktestStats::end_datetime)
        .def_readonly("total_time", &BacktestStats::total_time)
        .def_readonly("percent_short_trades", &BacktestStats::percent_short_trades)
        .def_readonly("percent_long_trades", &BacktestStats::percent_long_trades)
        
        .def("get_logs", &BacktestStats::get_logs)
        .def("__repr__", &BacktestStats::print_str_short)
        .def("print", &BacktestStats::print_str)
        .def("compute", &BacktestStats::pycompute)
        .def("print_file", &BacktestStats::print_file)
        .def_pickle(BacktestStats_pickle_suite());

    // Base signal tree handle
    class_<Signal, Signal*>("Signal", init<>())
    	// variables
		.def_readwrite("children", &Signal::m_children)
		.def_readonly("name", &Signal::m_name)

		// methods
		.def("add_child", &Signal::add_child)
		.def("clear_children", &Signal::clear_children)
		.def("clear_ELIs", &Signal::clear_ELIs)
		.def("fill_children_list", &Signal::fill_children_list)
		.def("flatten_tree", &Signal::flatten_tree)
		.def("get_parameters", &Signal::get_parameters)
		.def("set_parameters", &Signal::set_parameters)
		.def("get_max_bars_back", &Signal::get_max_bars_back)
		.def("get_parameter_constraints", &Signal::get_parameter_constraints)
		.def("set_parameter_constraints", &Signal::set_parameter_constraints)
		.def("get_all_constraints", &Signal::get_all_constraints)
		.def("set_all_constraints", &Signal::set_all_constraints)

		// Make an expression out of signal tree
		.def("print_str", &Signal::print_str)
		.def("__repr__", &Signal::print_str)
		;

    // Make a signal tree out of expression
    def("eval_stree_expr", &EvalExpr_NoVars, return_value_policy<manage_new_object>());

    // Backtester
    def("backtest_single_stree", &backtest_single_stree);
    def("backtest_single_sforest", &backtest_single_sforest);
    def("backtest_multi_stree", &backtest_multi_stree);

    def("get_stree_log", &get_stree_log);
    def("get_multi_stree_log", &get_multi_stree_log);
    def("get_stree_max_bars_back", &get_stree_max_bars_back);
    def("remove_comments", &remove_comments);
    def("init_system_constraints", &init_system_constraints);

    def("make_EL", &make_EL);
    def("make_indicator_EL", &make_indicator_EL);
    def("make_multi_EL", &make_multi_EL);
    def("make_forest_EL", &make_forest_EL);
    def("make_MQL4", &make_MQL4);
    def("make_forest_MQL4", &make_forest_MQL4);

    // Vectors of various objects
    class_< std::vector<int> >("IntsList")
        .def(vector_indexing_suite< std::vector<int> >() )
        ;

    class_< std::vector<float> >("FloatsList")
        .def(vector_indexing_suite< std::vector<float> >() )
        ;

    class_< std::vector<double> >("DoublesList")
        .def(vector_indexing_suite< std::vector<double> >() )
        ;

    class_< std::vector<bool> >("BoolsList")
        .def(vector_indexing_suite< std::vector<bool> >() )
        ;

    class_< std::vector<DateTime> >("DateTimeList")
        .def(vector_indexing_suite< std::vector<DateTime> >() )
        ;

    class_< std::vector<DataMatrix> >("DataMatrixList")
        .def(vector_indexing_suite< std::vector<DataMatrix> >() )
        ;

    class_< std::vector<TradeInfo> >("TradeInfoList")
        .def(vector_indexing_suite< std::vector<TradeInfo> >() )
        ;

    class_< std::vector<LimitOrderInfo> >("LimitOrderInfoList")
        .def(vector_indexing_suite< std::vector<LimitOrderInfo> >() )
        ;

    class_< std::vector<std::string> >("StringList")
        .def(vector_indexing_suite< std::vector<std::string> >() )
        ;

    class_< std::vector<Signal*> >("SignalList")
        .def(vector_indexing_suite< std::vector<Signal*> >() )
        ;
};



#endif // python_bindings_h__
