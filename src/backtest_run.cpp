#include <boost/algorithm/string.hpp>

#include "common.h"
#include "data.h"
#include "strategy.h"
#include "signal_tree.h"
#include "signal_forest.h"


void Env::clear()
{
    current_position = NO_POSITION;
    drawdown = 1;
    position_closed_at_bar_idx = 0;
    position_closed_at_price = 0;
    position_closed_at_time = DateTime();
    position_opened_at_bar_idx = 0;
    position_opened_at_price = 0;
    position_opened_at_time = DateTime();
    potential_profit = 0;
    quantity_traded.shares = 0;
    quantity_traded.lots = 0;
    runup = -1;

    buy_limit_active = false;
    buy_limit_price = 0.0;
    buy_limit_opened_at_time = DateTime();
    buy_limit_opened_at_idx = 0;

    sell_limit_active = false;
    sell_limit_price = 0.0;
    sell_limit_opened_at_time = DateTime();
    sell_limit_opened_at_idx = 0;
}

Env::Env()
{
    clear();
}

Order::Order()
{
    order_type = NO_ORDERS;
    quantity.lots = 0;
    quantity.shares = 0;
}

Order::Order( OrderType type )
{
    order_type = type;
    quantity.lots = 1;
    quantity.shares = 1;
}

Order::Order( OrderType type, Quantity qty )
{
    order_type = type;
    quantity = qty;
}

Order::Order( OrderType type, Quantity qty, std::string comm )
{
    order_type = type;
    quantity = qty;
    comment = comm;
}

// Return the profit that would be acquired if we close the trade just now. 
// Commission/slippage and market type options are accounted for.
double get_potential_commission(const DataMatrix& data, const Env &env, const int idx, const BacktestOptions& options)
{
    double comm_paid = 0;
    // TODO: elaborate more here for FOREX
    if (env.current_position != NO_POSITION)
    {
        if (options.commission_type == TRADE)
        {
            comm_paid = 2 * options.commission;
        }
        if (options.commission_type == SHARE)
        {
            comm_paid = 2 * options.commission * env.quantity_traded.shares;
        }
    }
    return comm_paid;
}

double get_potential_slippage(const DataMatrix& data, const Env &env, const int idx, const BacktestOptions& options)
{
    double slip_paid = 0;
    // TODO: elaborate more here for FOREX
    if (env.current_position != NO_POSITION)
    {
        if (options.slippage_type == TRADE)
        {
            slip_paid = 2 * options.slippage;
        }
        if (options.slippage_type == SHARE)
        {
            slip_paid = 2 * options.slippage * env.quantity_traded.shares;
        }
    }
    return slip_paid;
}

double get_potential_profit(const DataMatrix& data, const Env &env, const int idx, const BacktestOptions& options)
{
    double potential_profit = 0;

    // In Forex, we use the Open for entry/exit
    if (options.backtest_mode == METATRADER)
    {
        if (env.current_position == LONG_POSITION)
        {
        	ASS(env.quantity_traded.shares > 0);
            potential_profit = (data.open[idx] - env.position_opened_at_price) * env.quantity_traded.shares; // TODO: use "lots" in the future
        }
        else
        if (env.current_position == SHORT_POSITION)
        {
        	ASS(env.quantity_traded.shares > 0);
            potential_profit = (env.position_opened_at_price - data.open[idx]) * env.quantity_traded.shares; // TODO: use "lots" in the future
        }
    }
    else 
    if (options.backtest_mode == TRADESTATION)
    {
        if (env.current_position == LONG_POSITION)
        {
            ASS(env.quantity_traded.shares > 0);
            potential_profit = (data.close[idx] - env.position_opened_at_price) * env.quantity_traded.shares;
        }
        else
        if (env.current_position == SHORT_POSITION)
        {
            ASS(env.quantity_traded.shares > 0);
            potential_profit = (env.position_opened_at_price - data.close[idx]) * env.quantity_traded.shares;
        }
    }
    else
    {
        throw std::runtime_error("Unknown market type");
    }

    potential_profit -= get_potential_commission(data, env, idx, options);
    potential_profit -= get_potential_slippage(data, env, idx, options);

    // Write the data to the variables
    return potential_profit;
}


void update_stats(const DataMatrix &data, Env &env, const int idx, const BacktestOptions& options)
{
    double potential_profit = get_potential_profit(data, env, idx, options);
    
    env.potential_profit = potential_profit;
    if (potential_profit > 0)
    {
        if (potential_profit > env.runup)
        {
            env.runup = potential_profit;
        }
    }
    else 
    if (potential_profit < 0)
    {
        if (potential_profit < env.drawdown)
        {
            env.drawdown = potential_profit;
        }
    }
}

void entry( const DataMatrix &data, Env &env, const int idx, const Order &order, const PriceType at_price )
{
    if (at_price == PRICE_OPEN)
    {
        env.position_opened_at_bar_idx = idx;
        env.position_opened_at_time = data.dt[idx];
        env.position_opened_at_price = data.open[idx];
    }
    else 
    if (at_price == PRICE_CLOSE)
    {
        env.position_opened_at_bar_idx = idx;
        env.position_opened_at_time = data.dt[idx];
        env.position_opened_at_price = data.close[idx];
    }
    else
    {
        throw std::runtime_error("at_price is unknown value");
    }
    env.quantity_traded = order.quantity;
}

void buy(const DataMatrix& data, Env& env, const Order& order, const int idx, PriceType at_price)
{
    env.current_position = LONG_POSITION;
    entry(data, env, idx, order, at_price);
}

void sell(const DataMatrix& data, Env& env, const Order& order, const int idx, PriceType at_price)
{
    env.current_position = SHORT_POSITION;
    entry(data, env, idx, order, at_price);
}

TradeInfo close(const DataMatrix& data, Env& env, const Order& order, const int idx,
                PriceType at_price, const BacktestOptions& options)
{
    // Exit
    if (at_price == PRICE_OPEN)
    {
        env.position_closed_at_bar_idx = idx;
        env.position_closed_at_time = data.dt[idx];
        env.position_closed_at_price = data.open[idx];
    }
    else 
    if (at_price == PRICE_CLOSE)
    {
        env.position_closed_at_bar_idx = idx;
        env.position_closed_at_time = data.dt[idx];
        env.position_closed_at_price = data.close[idx];
    }
    else
    {
        throw std::runtime_error("at_price is unknown value");
    }

    // Calculate/set trade stats
    double profit = get_potential_profit(data, env, idx, options);
    TradeInfo trade;
    trade.bar_index_at_entry = env.position_opened_at_bar_idx;
    trade.bar_index_at_exit = env.position_closed_at_bar_idx;
    trade.bars_duration = env.position_closed_at_bar_idx - env.position_opened_at_bar_idx;
    trade.comment = order.comment;
    trade.commission_paid = get_potential_commission(data, env, idx, options);
    trade.datetime_of_entry = env.position_opened_at_time;
    trade.datetime_of_exit = env.position_closed_at_time;
    trade.drawdown = env.drawdown;
    trade.position = env.current_position;
    trade.price_at_entry = env.position_opened_at_price;
    trade.price_at_exit = env.position_closed_at_price;
    trade.profit = profit;
    trade.quantity = env.quantity_traded;
    trade.runup = env.runup;
    trade.slippage_paid = get_potential_slippage(data, env, idx, options);
    trade.symbol = data.name;
    trade.time_duration = 0; // todo: calculate time duration properly

    // clear the Env for next use
    env.clear();
    env.previous_trade_profit = profit;
    env.account_balance += profit;

    return trade;
}

// Execute order also deletes the Order objects
void execute_order(const DataMatrix& data, Env& env, Order* order, int idx,
                   const BacktestOptions& options, BacktestResults& results)
{
	// for NULL orders
	if (order == NULL)
	{
		return;
	}
    TradeInfo trade;
    PriceType ptype;
    ptype = (options.backtest_mode==METATRADER)?PRICE_OPEN:PRICE_CLOSE;
	if (order->order_type == BUY)
	{
		// this can work only if not already in a position
		if (env.current_position == NO_POSITION)
		{
			buy(data, env, *order, idx, ptype);
		}
		if (env.current_position == SHORT_POSITION) // was previously in opposite position - close first
		{
			order->comment = std::string("(switch to buy)");
			results.trades.push_back( close(data, env, *order, idx, ptype, options) );

			buy(data, env, *order, idx, ptype);
		}
	}
	else
	if (order->order_type == SELL)
	{
		// this can work only if not already in a position
		if (env.current_position == NO_POSITION)
		{
			sell(data, env, *order, idx, ptype);
		}
		if (env.current_position == LONG_POSITION) // was previously in opposite position - close first
		{
			order->comment = std::string("(switch to sell)");
			results.trades.push_back( close(data, env, *order, idx, ptype, options) );
			sell(data, env, *order, idx, ptype);
		}
	}
	else
	if (order->order_type == CLOSE)
	{

		if (env.current_position != NO_POSITION)
		{
			results.trades.push_back( close(data, env, *order, idx, ptype, options) );
		}
	}
	else
	{
		throw std::runtime_error("Unknown order type");
	}

	delete order;
}

BacktestStats backtest_single( const DataMatrix& a_data, BaseStrategy& a_strategy, const BacktestOptions& a_options )
{
    // Init variables
    BacktestResults results;
    Env env;
    int idx = 0;
    unsigned int num_bars = a_data.len();
    results.strategy_name = a_strategy.name;
    bool time_limited = false;
    if (!((a_options.market_open_time == Time(0)) &&
    	(a_options.market_close_time == Time(0))))
    {
    	time_limited = true;
    }

    // bind the strategy to the data
    a_strategy.bind(a_data);
    a_strategy.logs.clear();
    a_strategy.limit_orders.clear();
    
    // Backtesting loop
    for (idx=0; idx<(num_bars); idx++)
    {
        // Update Env stats and variables
        update_stats(a_data, env, idx, a_options);

		// All orders are marked with the data name
        //if (idx >= a_strategy.m_max_bars_back)
        {
            // interact with strategy
            Order *order = a_strategy.tick(a_data, env, idx);
        	if (order != NULL)
        	{
        		if (time_limited)
        		{
        			// in market open time?
        			Time t = a_data.dt[idx].time();
        			if ((t > a_options.market_open_time) &&
				        (t <= a_options.market_close_time))
        			{
						execute_order(a_data,
									  env,
									  order,
									  idx,
									  a_options,
									  results);
        			}
        			else
        			{
        				// order exists, but can't execute it now - just delete it.
        				delete order;
        			}
        		}
        		else
        		{
        			// always execute
					execute_order(a_data,
								  env,
								  order,
								  idx,
								  a_options,
								  results);
        		}
        	}
        }
    }

    // Final exit of all open positions
    execute_order(a_data, env, new Order(CLOSE), idx-1, a_options, results);
    update_stats(a_data, env, idx-1, a_options);

    BacktestStats stats(a_data, results, a_options);
    stats.logs = a_strategy.logs;
    stats.limit_orders = a_strategy.limit_orders;
    return stats;
}

BacktestStats backtest_single_ticks(TickDataMatrix a_data, BaseStrategy& a_strategy, BacktestOptions a_options)
{
    // Init variables
    BacktestResults results;
    Env env;

    int num_ticks = a_data.len();
    results.strategy_name = a_strategy.name;
    bool time_limited = false;
    if (!((a_options.market_open_time == Time(0)) &&
          (a_options.market_close_time == Time(0))))
    {
        time_limited = true;
    }

    // the data to hold stuff
    DataMatrix data;
    data.name = a_data.name;
    data.dt.resize(a_strategy.m_max_bars_back+1);
    data.open.resize(a_strategy.m_max_bars_back+1);
    data.high.resize(a_strategy.m_max_bars_back+1);
    data.low.resize(a_strategy.m_max_bars_back+1);
    data.close.resize(a_strategy.m_max_bars_back+1);
    data.volume.resize(a_strategy.m_max_bars_back+1);

    // The ticker loop
    DateTime current_time;
    for(int idx=0; idx<num_ticks; idx++)
    {
        // a new tick has just arrived

        // check if crossed the bar boundary
    }

    // bind the strategy to the data
    /*a_strategy.bind(a_data);

    // Backtesting loop
    for (idx=0; idx<(num_ticks); idx++)
    {
        // Update Env stats and variables
        update_stats(a_data, env, idx, a_options);

        // All orders are marked with the data name
        if (idx >= a_strategy.m_max_bars_back)
        {
            // interact with strategy
            Order *order = a_strategy.tick(a_data, env, idx);
            if (order != NULL)
            {
                if (time_limited)
                {
                    // in market open time?
                    Time t = a_data.dt[idx].time();
                    if ((t > a_options.market_open_time) &&
                        (t <= a_options.market_close_time))
                    {
                        execute_order(a_data,
                                      env,
                                      order,
                                      idx,
                                      a_options,
                                      results);
                    }
                    else
                    {
                        // order exists, but can't execute it now - just delete it.
                        delete order;
                    }
                }
                else
                {
                    // always execute
                    execute_order(a_data,
                                  env,
                                  order,
                                  idx,
                                  a_options,
                                  results);
                }
            }
        }
    }

    // Final exit of all open positions
    execute_order(a_data, env, new Order(CLOSE), idx-1, a_options, results);
    update_stats(a_data, env, idx-1, a_options);

    BacktestStats stats(a_data, results, a_options);*/
    return BacktestStats();
}



BacktestStats backtest_multi( MultiDataMatrix& data,
                              BaseStrategy& strategy, 
                              std::vector<BacktestOptions>& options )
{
    // Init variables
    BacktestResults results;
    results.strategy_name = strategy.name;
    std::vector<bool> time_limited;
    time_limited.resize(options.size());
    for(unsigned int i=0; i<options.size(); i++)
    {
        time_limited[i] = !((options[i].market_open_time == Time(0)) &&
                            (options[i].market_close_time == Time(0)));
    }

    // bind the strategy to the data
    strategy.mbind(data);
    strategy.logs.clear();

    // envs
    std::vector< Env > envs;
    envs.resize(data.symbols.size());
    // indices
    std::vector<int> idxd;
    for(int i=0; i<data.symbols.size(); i++)
    {
        idxd.push_back(0);
    }

    // Set the current time to min[start time of DataMatrix[..]]
    DateTime current_time(3000); // 3000 will set it to definite future
    for(unsigned int i=0; i<data.symbols.size(); i++)
    {
        if (current_time > data.symbols[i].dt[0])
        {
            current_time = data.symbols[i].dt[0];
        }
    }

    // The backtesting loop
    bool all_finished = false;
    while(!all_finished)
    {
        // Update the Envs
    	for(unsigned int i=0; i<data.symbols.size(); i++)
        {
            update_stats(data.symbols[i], envs[i], idxd[i], options[i]);
        }
        // TODO: Add stats (like equity)

        // Tick
        std::vector< OrderPair > orders;
        // Only tick when all indices are >= max_bars_back
        bool all_ok = true;
        if (all_ok)
        {
            orders = strategy.mtick(data, envs, idxd);
        }

        // Process the orders
        for(unsigned int i=0; i<orders.size(); i++)
        {
        	if (orders[i].second != NULL)
            {
        		if (time_limited[orders[i].first])
        		{
        			// in market open time?
					Time t = data.symbols[orders[i].first].dt[idxd[orders[i].first]].time();
					if ((t > options[orders[i].first].market_open_time) &&
						(t <= options[orders[i].first].market_close_time))
					{
						execute_order(data.symbols[orders[i].first],
									  envs[orders[i].first],
									  orders[i].second,
									  idxd[orders[i].first],
									  options[orders[i].first],
									  results);
					}
					else
					{
						delete orders[i].second;
					}
        		}
        		else
        		{
					execute_order(data.symbols[orders[i].first],
								   envs[orders[i].first],
								   orders[i].second,
								   idxd[orders[i].first],
								   options[orders[i].first],
								   results);
        		}
            }
        }

        if (all_ok)
        {
            // Advance the indices where necessary
            std::vector<DateTime> next_dts;
            std::vector<int> ended_dts;
            for (int i = 0; i < data.symbols.size(); i++)
            {
                if (idxd[i] == (data.symbols[i].dt.size() - 1))
                {
                    next_dts.push_back(data.symbols[i].dt[idxd[i]]);
                    ended_dts.push_back(i); // remember the ones who ended
                }
                else
                {
                    next_dts.push_back(data.symbols[i].dt[idxd[i] + 1]);
                }
            }

            // Find the closest one(s)
            std::vector<unsigned int> to_add;
            DateTime closest_time(3000); // 3000 will set it to definite future
            for (unsigned int i = 0; i < next_dts.size(); i++)
            {
                // skip over the ended ones
                bool in_ended = false;
                for (int j = 0; j < ended_dts.size(); j++)
                {
                    if (ended_dts[j] == i)
                    {
                        in_ended = true;
                        break;
                    }
                }
                if (!in_ended)
                {
                    if (closest_time > next_dts[i])
                    {
                        closest_time = next_dts[i];
                    }
                }
            }
            for (unsigned int i = 0; i < data.symbols.size(); i++)
            {
                if (data.symbols[i].dt[idxd[i] + 1] == closest_time)
                {
                    // add to list
                    to_add.push_back(i);
                }
            }

            bool no_idx_updated = true;
            for (unsigned int i = 0; i < to_add.size(); i++)
            {
                // don't increment if already at the end.
                if (idxd[to_add[i]] < (data.symbols[to_add[i]].len() - 1))
                {
                    idxd[to_add[i]]++;
                    no_idx_updated = false;
                }
            }

            // update current time
            current_time = closest_time;
            // check if all data is at the end
            if (no_idx_updated)
            {
                all_finished = true;
            }
        }
        else
        {
            for (unsigned int i = 0; i < data.symbols.size(); i++)
            {
                idxd[i]++;
            }
        }
    }

    BacktestStats stats(data, results, options);
    stats.logs = strategy.logs;
    stats.limit_orders = strategy.limit_orders;
    return stats;
}


// Use a signal tree string
BacktestStats backtest_single_stree( const DataMatrix& a_data,
                               const std::string& a_inp,
                               const BacktestOptions& a_options )
{
    BacktestStats t_stats;
    SignalTreeStrategy t_st(a_inp);

    // Backtest strategy
    try
    {
        t_stats = backtest_single(a_data, t_st, a_options);
    }
    catch(std::runtime_error& e)
    {
        std::cout << "Error! " << e.what() << "\n\n";
    }

    return t_stats;
}


// Use a signal tree string
BacktestStats backtest_multi_stree( MultiDataMatrix& a_data,
                               const std::string& a_inp,
                               const BacktestOptions& a_options )
{
    BacktestStats t_stats;
    SignalTreeMultiStrategy t_st(a_inp, a_data);
    std::vector<BacktestOptions> opts;
    opts.resize(a_data.symbols.size());
    for(unsigned int i=0; i<a_data.symbols.size(); i++)
    {
    	opts[i] = a_options;
    }

    // Backtest strategy
    try
    {
        t_stats = backtest_multi(a_data, t_st, opts);
    }
    catch(std::runtime_error& e)
    {
        std::cout << "Error! " << e.what() << "\n\n";
    }

    return t_stats;
}

BacktestStats backtest_single_sforest( const DataMatrix& a_data,
                                       const std::string& a_inp,
                                       const BacktestOptions& a_options )
{
    BacktestStats t_stats;
    SignalForestStrategy t_forest(a_inp);

    // Backtest strategy
    try
    {
        t_stats = backtest_single(a_data, t_forest, a_options);
    }
    catch(std::runtime_error& e)
    {
        std::cout << "Error! " << e.what() << "\n\n";
    }

    return t_stats;
}



// get the max bars back down the tree
int get_stree_max_bars_back(const std::string &a_stree)
{
    int t_mbb = 0;
    Signal *t_tree = EvalExpr(a_stree, std::map<std::string, Signal *>());
    t_mbb = t_tree->get_max_bars_back();
    delete t_tree;
    return t_mbb;
}




std::vector<int> get_int_stree_log(const std::string& a_stree, const DataMatrix& a_data)
{
	BacktestOptions opts;
    opts.market_open_time = Time(0,0);
    opts.market_close_time = Time(0,0);
	std::vector<int> ret;
	SignalTreeLoggerStrategy* st = new SignalTreeLoggerStrategy(a_stree);
	if ((st->m_tree == NULL) || (a_data.len() == 0))
	{
		delete st;
		return ret;
	}
	if (st->m_tree->m_return_type != RT_INT)
	{
		throw std::runtime_error("Requested int log from a signal tree returning non-int");
	}

    // Backtest strategy
    try
    {
        backtest_single(a_data, *st, opts);
        ret = st->m_int_result;
    }
    catch(std::runtime_error& e)
    {
        std::cout << "Error! " << e.what() << "\n\n";
    }

    delete st;
    return ret;
}

std::vector<double> get_float_stree_log(const std::string& a_stree, const DataMatrix& a_data)
{
	BacktestOptions opts;
    opts.market_open_time = Time(0,0);
    opts.market_close_time = Time(0,0);
	std::vector<double> ret;
	SignalTreeLoggerStrategy* st = new SignalTreeLoggerStrategy(a_stree);
	if ((st->m_tree == NULL) || (a_data.len() == 0))
	{
		delete st;
		return ret;
	}
	if (st->m_tree->m_return_type != RT_FLOAT)
	{
		throw std::runtime_error("Requested float log from a signal tree returning non-float");
	}

    // Backtest strategy
    try
    {
        backtest_single(a_data, *st, opts);
        ret = st->m_float_result;
    }
    catch(std::runtime_error& e)
    {
        std::cout << "Error! " << e.what() << "\n\n";
    }

    delete st;
    return ret;
}

std::vector<bool> get_bool_stree_log(const std::string& a_stree, const DataMatrix& a_data)
{
	BacktestOptions opts;
    opts.market_open_time = Time(0,0);
    opts.market_close_time = Time(0,0);
	std::vector<bool> ret;
	SignalTreeLoggerStrategy* st = new SignalTreeLoggerStrategy(a_stree);

	if ((st->m_tree == NULL) || (a_data.len() == 0))
	{
		delete st;
		return ret;
	}

	if (st->m_tree->m_return_type != RT_BOOL)
	{
		throw std::runtime_error("Requested bool log from a signal tree returning non-bool");
	}

    // Backtest strategy
    try
    {
        backtest_single(a_data, *st, opts);
        ret = st->m_bool_result;
    }
    catch(std::runtime_error& e)
    {
        std::cout << "Error! " << e.what() << "\n\n";
    }

    delete st;
    return ret;
}

py::list get_stree_log(const std::string& a_stree, const DataMatrix& a_data)
{
	py::list l;

	BacktestOptions opts;
    opts.market_open_time = Time(0,0);
    opts.market_close_time = Time(0,0);
	SignalTreeLoggerStrategy* st = new SignalTreeLoggerStrategy(a_stree);

	if ((st->m_tree == NULL) || (a_data.len() == 0))
	{
		delete st;
		return l;
	}

    // Backtest strategy
    try
    {
        backtest_single(a_data, *st, opts);
        if (st->m_tree->m_return_type == RT_INT)
        {
        	 for(unsigned int i=0; i<(st->m_int_result.size()); i++)
        	 {
        		 l.append((int)(st->m_int_result[i]));
        	 }
        }
        else
        if (st->m_tree->m_return_type == RT_FLOAT)
        {
        	 for(unsigned int i=0; i<(st->m_float_result.size()); i++)
        	 {
        		 l.append((double)(st->m_float_result[i]));
        	 }
        }
        else
        if (st->m_tree->m_return_type == RT_BOOL)
        {
        	 for(unsigned int i=0; i<(st->m_bool_result.size()); i++)
        	 {
        		 l.append((bool)(st->m_bool_result[i]));
        	 }
        }
    }
    catch(std::runtime_error& e)
    {
        std::cout << "Error! " << e.what() << "\n\n";
    }

    delete st;
    return l;
}


std::vector< std::vector<int> > get_int_multi_stree_log(const std::string& a_stree, MultiDataMatrix& a_mdata)
{
    std::vector< std::vector<int> > l;

    std::vector<BacktestOptions> opts;
    opts.resize(a_mdata.symbols.size());
    for(int o=0; o<opts.size(); o++)
    {
        opts[o].market_open_time = Time(0,0);
        opts[o].market_close_time = Time(0,0);
    }
    SignalTreeMultiLoggerStrategy* st = new SignalTreeMultiLoggerStrategy(a_stree, a_mdata);

    bool bad=false;
    for(int idx=0; idx<a_mdata.symbols.size(); idx++)
    {
        if ((st->m_tree[idx] == NULL) || (a_mdata.symbols[idx].len() == 0))
        {
            bad=true;
            break;
        }
    }
    if (bad)
    {
        delete st;
        return l;
    }

    try
    {
        // Backtest strategy
        backtest_multi(a_mdata, *st, opts);
        for (int idx = 0; idx < a_mdata.symbols.size(); idx++)
        {
            std::vector<int> ml;

            if (st->m_tree[idx]->m_return_type == RT_INT)
            {
                for (unsigned int i = 0; i < (st->m_int_result[idx].size()); i++)
                {
                    ml.push_back((int) (st->m_int_result[idx][i]));
                }
            }

            l.push_back(ml);
        }
    }
    catch(std::runtime_error& e)
    {
        std::cout << "Error! " << e.what() << "\n\n";
    }

    delete st;
    return l;
}



std::vector< std::vector<double> > get_float_multi_stree_log(const std::string& a_stree, MultiDataMatrix& a_mdata)
{
    std::vector< std::vector<double> > l;
    std::vector<double> ml;
    std::vector<BacktestOptions> opts;
    opts.resize(a_mdata.symbols.size());
    for(int o=0; o<opts.size(); o++)
    {
        opts[o].market_open_time = Time(0,0);
        opts[o].market_close_time = Time(0,0);
    }
    SignalTreeMultiLoggerStrategy* st = new SignalTreeMultiLoggerStrategy(a_stree, a_mdata);

    bool bad=false;
    for(int idx=0; idx<a_mdata.symbols.size(); idx++)
    {
        if ((st->m_tree[idx] == NULL) || (a_mdata.symbols[idx].len() == 0))
        {
            bad=true;
            break;
        }
    }
    if (bad)
    {
        delete st;
        return l;
    }

    try
    {
        // Backtest strategy
        backtest_multi(a_mdata, *st, opts);
        for (int idx = 0; idx < a_mdata.symbols.size(); idx++)
        {
            ml.clear();

            if (st->m_tree[idx]->m_return_type == RT_FLOAT)
            {
                for (unsigned int i = 0; i < (st->m_float_result[idx].size()); i++)
                {
                    ml.push_back((double) (st->m_float_result[idx][i]));
                }
            }

            l.push_back(ml);
        }
    }
    catch(std::runtime_error& e)
    {
        std::cout << "Error! " << e.what() << "\n\n";
    }

    delete st;
    return l;
}



std::vector< std::vector<bool> > get_bool_multi_stree_log(const std::string& a_stree, MultiDataMatrix& a_mdata)
{
    std::vector< std::vector<bool> > l;

    std::vector<BacktestOptions> opts;
    opts.resize(a_mdata.symbols.size());
    for(int o=0; o<opts.size(); o++)
    {
        opts[o].market_open_time = Time(0,0);
        opts[o].market_close_time = Time(0,0);
    }
    SignalTreeMultiLoggerStrategy* st = new SignalTreeMultiLoggerStrategy(a_stree, a_mdata);

    bool bad=false;
    for(int idx=0; idx<a_mdata.symbols.size(); idx++)
    {
        if ((st->m_tree[idx] == NULL) || (a_mdata.symbols[idx].len() == 0))
        {
            bad=true;
            break;
        }
    }
    if (bad)
    {
        delete st;
        return l;
    }

    try
    {
        // Backtest strategy
        backtest_multi(a_mdata, *st, opts);
        for (int idx = 0; idx < a_mdata.symbols.size(); idx++)
        {
            std::vector<bool> ml;

            if (st->m_tree[idx]->m_return_type == RT_INT)
            {
                for (unsigned int i = 0; i < (st->m_bool_result[idx].size()); i++)
                {
                    ml.push_back((bool) (st->m_bool_result[idx][i]));
                }
            }

            l.push_back(ml);
        }
    }
    catch(std::runtime_error& e)
    {
        std::cout << "Error! " << e.what() << "\n\n";
    }

    delete st;
    return l;
}




py::list get_multi_stree_log(const std::string& a_stree, MultiDataMatrix& a_mdata)
{
    py::list l;

    std::vector<BacktestOptions> opts;
    opts.resize(a_mdata.symbols.size());
    for(int o=0; o<opts.size(); o++)
    {
        opts[o].market_open_time = Time(0,0);
        opts[o].market_close_time = Time(0,0);
        opts[o].backtest_mode = TRADESTATION;
    }

    //g_global_price_preference = PRICE_PREF_CLOSE; // TODO: solve that global_price_preference issue

    SignalTreeMultiLoggerStrategy* st = new SignalTreeMultiLoggerStrategy(a_stree, a_mdata);


    bool bad=false;
    for(int idx=0; idx<a_mdata.symbols.size(); idx++)
    {
        if ((st->m_tree[idx] == NULL) || (a_mdata.symbols[idx].len() == 0))
        {
            bad=true;
            break;
        }
    }
    if (bad)
    {
        delete st;
        return l;
    }

    try
    {
        // Backtest strategy
        backtest_multi(a_mdata, *st, opts);
        for (int idx = 0; idx < a_mdata.symbols.size(); idx++)
        {
            py::list ml;

            if (st->m_tree[idx]->m_return_type == RT_INT)
            {
                for (unsigned int i = 0; i < (st->m_int_result[idx].size()); i++)
                {
                    ml.append((int) (st->m_int_result[idx][i]));
                }
            }
            else if (st->m_tree[idx]->m_return_type == RT_FLOAT)
            {
                for (unsigned int i = 0; i < (st->m_float_result[idx].size()); i++)
                {
                    ml.append((double) (st->m_float_result[idx][i]));
                }
            }
            else if (st->m_tree[idx]->m_return_type == RT_BOOL)
            {
                for (unsigned int i = 0; i < (st->m_bool_result[idx].size()); i++)
                {
                    ml.append((bool) (st->m_bool_result[idx][i]));
                }
            }

            l.append(ml);
        }
    }
    catch(std::runtime_error& e)
    {
        std::cout << "Error! " << e.what() << "\n\n";
    }

    delete st;
    return l;
}



