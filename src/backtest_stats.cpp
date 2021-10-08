#include "backtest_stats.h"

void TradeInfo::print()
{
    std::cout << print_str();
}

std::string TradeInfo::print_str() const
{
    std::stringstream s;

    s << "---------------------------------------\n";
    s << boost::format("Symbol: %s\n") % symbol.c_str();
    s << boost::format("Position: %s\n") % (position == LONG_POSITION ? "Buy" : "Sell");
    s << boost::format("Date/Time of entry: %s\n") % datetime_of_entry.print_str().c_str();
    s << boost::format("Date/Time of exit:  %s\n") % datetime_of_exit.print_str().c_str();
    s << boost::format("Price at entry: %3.5f\n") % price_at_entry;
    s << boost::format("Price at exit:  %3.5f\n") % price_at_exit;
    s << boost::format("Quantity: %d\n") % quantity.shares;
    s << boost::format("Profit: %3.5f\n") % profit;
    if (comment != std::string(""))
    {
        s << boost::format("Comment: %s\n") % comment.c_str();
    }
    s << "---------------------------------------\n";

    return s.str();
}

TradeInfo::TradeInfo()
{
    bar_index_at_entry = 0;
    bar_index_at_exit = 0;
    bars_duration = 0;
    comment = "";
    commission_paid = 0;
    datetime_of_entry = DateTime();
    datetime_of_exit = DateTime();
    drawdown = 0;
    position = NO_POSITION;
    price_at_entry = 0;
    price_at_exit = 0;
    profit = 0;
    quantity = 0;
    runup = 0;
    slippage_paid = 0;
    symbol = "";
    time_duration = 0;
}


std::string BacktestStats::print_str() const
{
    std::stringstream s;

    s << "====================================================\n";
    s << "Strategy Performance Report\n";
    s << "====================================================\n";
    s << "Strategy name: " << strategy_name << "\n";
    s << "--------------------\n";
    s << "Strategy inputs:\n";
    s << "--------------------\n";
    s << "<under construction>\n"; //TODO: Add support for optimization-enabled strategy inputs
    // or maybe ELIs
    s << "--------------------\n";
    s << "Backtest stats:\n\n";

    // Output all stats in order of importance
    s << "Profit factor: " << boost::format("%3.2f") % profit_factor << "\n";
    s << "Trades: " << num_trades << "\n";
    s << "Profit / Loss trades: " << num_profit_trades << " (" << 
        boost::format("%3.1f") % percent_profit_trades 
        << "%) / " << num_loss_trades << " (" << 
        boost::format("%3.1f") % (100.0 - percent_profit_trades) << "%)\n";
    s << "Gross profit: " << boost::format("%3.2f") % gross_profit << "\n";
    s << "Gross loss:  " << boost::format("%3.2f") % gross_loss << "\n\n";

    s << "Initial capital: " << boost::format("%3.2f") % initial_capital << "\n";
    s << "Final balance:   " << boost::format("%3.2f") % account_balance << "\n";
    s << "Account size required: " << boost::format("%3.2f") % account_size_required << "\n\n";
    s << "Net profit: " << boost::format("%3.2f") % net_profit << "\n";

    s << "Long / Short positions: " << num_long_trades << " (" << 
        boost::format("%3.1f") % percent_long_trades 
        << "%) / " << num_short_trades << " (" << 
        boost::format("%3.1f") % (100.0 - percent_long_trades) << "%)\n";
    s << "Largest profit trade:\n" << largest_profit_trade.print_str();
    s << "Largest loss trade:\n" << largest_loss_trade.print_str() << "\n";

    s << "Average gross profit per trade: " << boost::format("%3.2f") % average_profit_per_trade << "\n";
    s << "Average gross loss per trade:  " << boost::format("%3.2f") % average_loss_per_trade << "\n";
    s << "Average trade net profit: " << boost::format("%3.2f") % average_trade_net_profit << "\n";
    s << "Ratio of avg profit / avg loss: " << boost::format("%3.2f") % ratio_of_avg_profit_avg_loss << "\n\n";

    s << "Largest runup:     " << boost::format("%3.2f") % largest_runup << "\n";
    s << "Largest drawdown: " << boost::format("%3.2f") % largest_drawdown << "\n";
    s << "Largest runup as percent of initial balance:     " << 
        boost::format("%3.2f") % largest_runup_as_percent_of_initial_balance << "\n";
    s << "Largest drawdown as percent of initial balance: " << 
        boost::format("%3.2f") % largest_drawdown_as_percent_of_initial_balance << "\n\n";

    s << "Total commissions paid: " << boost::format("%3.2f") % total_commissions_paid << "\n";
    s << "Total slippage paid:    " << boost::format("%3.2f") % total_slippage_paid << "\n\n";

    s << "Max shares or contracts held:   " << boost::format("%d") % max_shares_or_contracts_held << "\n";
    s << "Total shares or contracts held: " << boost::format("%d") % total_shares_or_contracts_held << "\n";
    s << "Max lots traded:   " << boost::format("%3.2f") % max_lots_traded << "\n";
    s << "Total lots traded: " << boost::format("%3.2f") % total_lots_traded << "\n\n";

    s << "Start date/time: " << start_datetime.print_str() << "\n";
    s << "End date/time:   " << end_datetime.print_str() << "\n\n";

    s << "--------------------\n";
    s << "List of trades:\n";
    for(unsigned int i=0; i<list_of_trades.size(); i++)
        s << list_of_trades[i].print_str();

    return s.str();
}

std::string BacktestStats::print_str_short() const
{
    std::stringstream s;

    s << "--------------------\n";
    // Output all stats in order of importance
    s << "Profit factor: " << boost::format("%3.2f") % profit_factor << "\n";
    s << "Trades: " << num_trades << "\n";
    s << "Profit / Loss trades: " << num_profit_trades << " (" << 
        boost::format("%3.1f") % percent_profit_trades 
        << "%) / " << num_loss_trades << " (" << 
        boost::format("%3.1f") % (100.0 - percent_profit_trades) << "%)\n";
    s << "Gross profit: $" << boost::format("%3.2f") % gross_profit << "\n";
    s << "Gross loss: $" << boost::format("%3.2f") % gross_loss << "\n";

    s << "Net profit: $" << boost::format("%3.2f") % net_profit << "\n";

    s << "Long / Short positions: " << num_long_trades << " (" << 
        boost::format("%3.1f") % percent_long_trades 
        << "%) / " << num_short_trades << " (" << 
        boost::format("%3.1f") % (100.0 - percent_long_trades) << "%)\n";
    s << "--------------------\n";

    return s.str();
}

void BacktestStats::compute( std::vector< TradeInfo >& a_trades, double a_init_capital, std::string a_strategy_name )
{
    //double initial_capital;
    //double account_balance;
    initial_capital = a_init_capital;
    account_balance = a_init_capital;
    for (unsigned int i=0; i<a_trades.size(); i++)
        account_balance += a_trades[i].profit;

    //double net_profit;
    //double gross_profit;
    //double gross_loss;
    //double profit_factor;
    net_profit = 0;
    gross_profit = 0;
    gross_loss = 0;
    profit_factor = 0;
    for (unsigned int i=0; i<a_trades.size(); i++)
    {
        net_profit += a_trades[i].profit;
        if (a_trades[i].profit > 0)
            gross_profit += a_trades[i].profit;
        else
            if (a_trades[i].profit < 0)
                gross_loss += a_trades[i].profit;
    }
    if (gross_loss != 0)
        profit_factor = ABS(gross_profit / gross_loss);
    else
        profit_factor = gross_profit;

    //int num_trades;
    num_trades = (unsigned int)a_trades.size();

    //int num_long_trades;
    //int num_short_trades;
    //int num_profit_trades;
    //int num_loss_trades;
    //int num_even_trades;
    //double percent_profit_trades;
    //double percent_long_trades;
    //double percent_short_trades;
    //TradeInfo largest_profit_trade;
    //TradeInfo largest_loss_trade;
    num_long_trades = 0;
    num_short_trades = 0;
    num_profit_trades = 0;
    num_loss_trades = 0;
    num_even_trades = 0;
    percent_profit_trades = 0;
    percent_long_trades = 0;
    percent_short_trades = 0;
    double min_profit = 9999999.0;
    double max_profit = -9999999.0;
    for (unsigned int i=0; i<a_trades.size(); i++)
    {
        if (a_trades[i].position == LONG_POSITION)
            num_long_trades++;
        else
            if (a_trades[i].position == SHORT_POSITION)
                num_short_trades++;
            else
                throw std::runtime_error("invalid position in trade");

        if (a_trades[i].profit > 0)
            num_profit_trades++;
        if (a_trades[i].profit < 0)
            num_loss_trades++;
        if (a_trades[i].profit == 0)
            num_even_trades++;

        if (a_trades[i].profit > max_profit)
        {
            max_profit = a_trades[i].profit;
            largest_profit_trade = a_trades[i];
        }
        if (a_trades[i].profit < min_profit)
        {
            min_profit = a_trades[i].profit;
            largest_loss_trade = a_trades[i];
        }
    }
    if (num_trades > 0)
        percent_profit_trades = (double)num_profit_trades / (double)num_trades * 100.0;
    if (num_long_trades > 0)
        percent_long_trades = (double)num_long_trades / (double)num_trades * 100.0;
    if (num_short_trades > 0)
        percent_short_trades = (double)num_short_trades / (double)num_trades * 100.0;



    // TODO: add the calculations for these later
    /*
    int max_consecutive_profit_trades;
    int max_consecutive_loss_trades;
    double average_consecutive_profit_trades;
    double average_consecutive_loss_trades;*/

    //double average_trade_net_profit; // (net profit / num trades)
    //double average_profit_per_trade; // (gross profit / profit trades)
    //double average_loss_per_trade; // (gross loss / loss trades)
    //double ratio_of_avg_profit_avg_loss;
    average_trade_net_profit = 0;        
    average_profit_per_trade = 0;
    ratio_of_avg_profit_avg_loss = 0;
    if (num_trades > 0)
    {
        average_trade_net_profit = net_profit / (double)num_trades;        
        if (num_profit_trades > 0)
            average_profit_per_trade = gross_profit / (double)num_profit_trades;
        if (num_loss_trades > 0)
            average_loss_per_trade = gross_loss / (double)num_loss_trades;
        if (average_loss_per_trade != 0)
            ratio_of_avg_profit_avg_loss = ABS(average_profit_per_trade) / ABS(average_loss_per_trade);
    }


    //double largest_runup;
    //double largest_drawdown;
    //TradeInfo largest_runup_trade;
    //TradeInfo largest_drawdown_trade;
    //double average_runup_per_trade;
    //double average_drawdown_per_trade;
    //double largest_runup_as_percent_of_initial_balance;
    //double largest_drawdown_as_percent_of_initial_balance;
    //double account_size_required;
    largest_runup = 0;
    largest_drawdown = 0;
    average_runup_per_trade = 0;
    average_drawdown_per_trade = 0;
    largest_runup_as_percent_of_initial_balance = 0;
    largest_drawdown_as_percent_of_initial_balance = 0;
    account_size_required = 0;
    double max_runup = -9999999;
    double max_drawdown = 9999999;
    double total_runup = 0;
    double total_drawdown = 0;
    for (unsigned int i=0; i<a_trades.size(); i++)
    {
        total_runup += a_trades[i].runup;
        total_drawdown += a_trades[i].drawdown;

        if (a_trades[i].runup > max_runup)
        {
            max_runup = a_trades[i].runup;
            largest_runup_trade = a_trades[i];
        }
        if (a_trades[i].drawdown < max_drawdown)
        {
            max_drawdown = a_trades[i].drawdown;
            largest_drawdown_trade = a_trades[i];
        }
    }
    largest_runup = max_runup;
    largest_drawdown = max_drawdown;
    largest_runup_as_percent_of_initial_balance = largest_runup / initial_capital * 100.0;
    largest_drawdown_as_percent_of_initial_balance = largest_drawdown / initial_capital * 100.0;
    if (num_trades > 0)
    {
        average_runup_per_trade = total_runup / num_trades;
        average_drawdown_per_trade = total_drawdown / num_trades;
    }
    account_size_required = ABS(largest_drawdown);

    // TODO: add support for these later
    /*
    int total_time_in_trades; // in seconds
    int total_bars_in_trades;
    double average_time_in_trades;
    double average_bars_in_trades;
    int total_time_between_trades;
    int total_bars_between_trades;
    double average_time_between_trades;
    double average_bars_between_trades;
    double percent_of_time_in_market;
    int total_time; // in seconds
    */

    //double total_commissions_paid;
    //double total_slippage_paid;
    //int max_shares_or_contracts_held;
    //int total_shares_or_contracts_held;
    //double max_lots_traded;
    //double total_lots_traded; // for FOREX
    total_commissions_paid = 0;
    total_slippage_paid = 0;
    max_shares_or_contracts_held = 0;
    total_shares_or_contracts_held = 0;
    max_lots_traded = 0;
    total_lots_traded = 0;

    int max_shares = -1;
    double max_lots = -1;
    for (unsigned int i=0; i<a_trades.size(); i++)
    {
        total_commissions_paid += a_trades[i].commission_paid;
        total_slippage_paid += a_trades[i].slippage_paid;
        total_shares_or_contracts_held += a_trades[i].quantity.shares;
        total_lots_traded += a_trades[i].quantity.lots;

        if (a_trades[i].quantity.shares > max_shares)
            max_shares = a_trades[i].quantity.shares;
        if (a_trades[i].quantity.lots > max_lots)
            max_lots = a_trades[i].quantity.lots;
    }
    max_shares_or_contracts_held = max_shares;
    max_lots_traded = max_lots;

    strategy_name = a_strategy_name;
    list_of_trades = a_trades;
}

void BacktestStats::pycompute(py::list a_trades, double a_init_capital, std::string a_strategy_name)
{
    std::vector<TradeInfo> ts;
    int lts = py::len(a_trades);
    for(int i=0; i<lts; i++)
    {
        TradeInfo t = py::extract<TradeInfo>(a_trades[i]);
        ts.push_back(t);
    }

    compute(ts, a_init_capital, a_strategy_name);
}


BacktestStats::BacktestStats( MultiDataMatrix& data, BacktestResults& res, std::vector<BacktestOptions>& opt )
{
    //std::vector<std::string> symbols;
    for(unsigned int i=0; i<data.symbols.size(); i++)
    {
        symbols.push_back(data.symbols[i].name);
        if (start_datetime > data.symbols[i].dt[0])
            start_datetime = data.symbols[i].dt[0];
        if (end_datetime < data.symbols[i].dt[data.symbols[i].dt.size()-1])
            end_datetime = data.symbols[i].dt[data.symbols[i].dt.size()-1];
    }

    compute(res.trades, opt[0].initial_capital, res.strategy_name); // TODO: fix that, find a better solution
}

BacktestStats::BacktestStats( const DataMatrix& data, BacktestResults& res, const BacktestOptions& opt )
{
    //std::vector<std::string> symbols;
    symbols.push_back(data.name);
    start_datetime = data.dt[0];
    end_datetime = data.dt[data.dt.size()-1];

    compute(res.trades, opt.initial_capital, res.strategy_name);
}

BacktestStats::BacktestStats()
{
    initial_capital = 0;
    account_balance = 0;

    net_profit = 0;
    gross_profit = 0;
    gross_loss = 0;
    profit_factor = 0;

    num_trades = 0;

    num_long_trades = 0;
    num_short_trades = 0;
    num_profit_trades = 0;
    num_loss_trades = 0;
    num_even_trades = 0;
    percent_profit_trades = 0;
    percent_long_trades = 0;
    percent_short_trades = 0;

    percent_profit_trades = 0;
    percent_long_trades = 0;
    percent_short_trades = 0;

    average_trade_net_profit = 0;        
    average_profit_per_trade = 0;
    ratio_of_avg_profit_avg_loss = 0;
    largest_runup = 0;
    largest_drawdown = 0;
    average_runup_per_trade = 0;
    average_drawdown_per_trade = 0;
    largest_runup_as_percent_of_initial_balance = 0;
    largest_drawdown_as_percent_of_initial_balance = 0;
    account_size_required = 0;
    total_commissions_paid = 0;
    total_slippage_paid = 0;
    max_shares_or_contracts_held = 0;
    total_shares_or_contracts_held = 0;
    max_lots_traded = 0;
    total_lots_traded = 0;

    // todo: init the rest
    max_consecutive_profit_trades = 0;
    max_consecutive_loss_trades = 0;
    average_consecutive_profit_trades = 0;
    average_consecutive_loss_trades = 0;
    average_loss_per_trade = 0;
    total_time_in_trades = 0;
    total_bars_in_trades = 0;
    average_time_in_trades = 0;
    average_bars_in_trades = 0;
    total_time_between_trades = 0;
    total_bars_between_trades = 0;
    average_time_between_trades = 0;
    average_bars_between_trades = 0;
    percent_of_time_in_market = 0;
    total_time = 0;

    max_shares_or_contracts_held = 0;
    max_lots_traded = 0;

    strategy_name = "";
}

void BacktestStats::print_file( char* filename ) const
{
    std::ofstream ofile(filename);
    ofile << print_str();
    ofile.close();
}


BacktestOptions::BacktestOptions()
{
    backtest_mode = TRADESTATION;
    initial_capital = 100000;
    commission = 5;
    slippage = 0.01;
    commission_type = TRADE;
    slippage_type = SHARE;
    market_open_time = Time(9,30);
    market_close_time = Time(16,0);
}

Quantity::Quantity()
{
    shares = 0;
    lots = 0;
}

Quantity::Quantity( int sh )
{
    shares = sh;
    lots = 0;
}

Quantity::Quantity( double lt )
{
    shares = 0;
    lots = lt;
}

void Quantity::print()
{
	if (shares != 0)
	{
		std::cout << shares << " shares";
	}
	else
	{
		std::cout << format("%3.2f") % lots << " lots";
	}
}

std::string Quantity::print_str() const
{
	std::stringstream s;
	if (shares != 0)
	{
		s << shares << " shares";
	}
	else
	{
		s << format("%3.2f") % lots << " lots";
	}
	return s.str();
}
