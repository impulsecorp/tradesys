#include <stdio.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#ifndef WIN32

#include "gtest/gtest.h"

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#else

int main(int argc, char **argv)
{
    //test_multi_data_simple_strategy();
	//test_interactive_expressions();
    return 0;
}

#endif




/*
// Minimal strategy
class MinimalStrategy: public BaseStrategy
{
public:
    MinimalStrategy()
    {
        m_max_bars_back = 1;
    }
    virtual ~MinimalStrategy(){}

    virtual Order* tick(const DataMatrix& data, const Env& env, const unsigned int idx)
    {
        if (data.dt[idx].time() == Time(9, 45))
            return new Order(BUY, 1000);

        if (data.dt[idx].time() == Time(9, 55))
            return new Order(SELL, 1000);

        if (data.dt[idx].time() > Time(12, 0))
            return new Order(CLOSE);

        return NULL;
    }
};

// Minimal multi-strategy
class MinimalMultiStrategy: public BaseStrategy
{
public:
    MinimalMultiStrategy()
    {
        m_max_bars_back = 30;
    }
    virtual ~MinimalMultiStrategy(){}


    // Override this method to create the strategy behavior
    virtual std::vector<OrderPair>
        mtick(MultiDataMatrix& data,
              std::vector<Env>& env,
              std::vector<unsigned int>& idxd)
    {
    	std::vector<OrderPair> orders;

    	for(unsigned int i=0; i<data.symbols.size(); i++)
    	{
    		int qty = (int)(100000.0 / data.symbols[i].close[idxd[i]]);

    		bool bu = data.symbols[i].close[idxd[i] - 2] < data.symbols[i].close[idxd[i]];
    		bool se = data.symbols[i].close[idxd[i] - 1] < data.symbols[i].close[idxd[i]];
    		bool cl = data.symbols[i].close[idxd[i] - 3] < data.symbols[i].close[idxd[i]];

    		if (cl)
    		{
    			orders.push_back( OrderPair(i, new Order(CLOSE)) );
    		}
    		else
    		{
    			if (!(bu && se))
    			{
    				if (bu)
    					orders.push_back( OrderPair(i, new Order(BUY, qty)) );
    				else
    				if (se)
    					orders.push_back( OrderPair(i, new Order(SELL, qty)) );
    			}
    		}
    	}

    	return orders;
    }
};


void test_single_data_simple_strategy()
{
	DataMatrix data;
	MinimalStrategy st;
	BacktestOptions opt;

#ifdef WIN32
	data.load_tsys("/Users/Peter/Desktop/tradesys/spy-1m.tsys");
#else
	data.load_tsys("spy-1m.tsys");
#endif

	printf("len: %d\n", data.len());

    // BacktestStats must be deleted after use
	BacktestStats res;
    res = backtest_single(data, st, opt);
#ifdef WIN32
    res.print_file("/Users/Peter/Desktop/report.txt");
#else
    res.print_file("/home/peter/Desktop/report.txt");
#endif


	ProgressBar pb(100);
	for(int i=0; i<100; i++)
	{
		res = backtest_single(data, st, opt);
		//printf("Trades: %d Profit: %3.2f idx: %d\n", t_res.m_trades.size(), t_profit, i);
		pb.update(i);
	}
	pb.finish();
}


void test_multi_data_simple_strategy()
{
	MultiDataMatrix data;
	MinimalMultiStrategy st;
	BacktestStats res;

#ifdef WIN32
    data.symbols.resize(2);
    data.symbols[0].load_tradestation("/Users/Peter/Desktop/tradesys/aapl-1m.txt");
    data.symbols[1].load_tradestation("/Users/Peter/Desktop/tradesys/goog-1m.txt");
#else
    data.symbols.resize(2);
    data.symbols[0].load_tradestation("aapl-1m.txt");
    data.symbols[1].load_tradestation("goog-1m.txt");
#endif

	std::vector<BacktestOptions> opts;
	opts.resize(data.symbols.size());

	printf("len: %d\n", data.len());

	for(int i=0; i<100; i++)
	{
		res = backtest_multi(data, st, opts);
#ifdef WIN32
        res.print_file("/Users/Peter/Desktop/report.txt");
#else
        res.print_file("/home/peter/Desktop/report.txt");
#endif
		printf("Trades: %d Profit: %3.2f idx: %d\n", res.num_trades, res.net_profit, i);
	}
}

#if 0
void test_expressions()
{
    DataMatrix data;
    BacktestOptions opt;
    opt.commission = 0;
    opt.slippage = 0.0001;

#ifdef WIN32
    data.load_tradestation("eurusd-1s.txt");
#else
    data.load_tradestation("eurusd-1s.txt");
#endif

    printf("Data length before clip: %d bars\n", data.len());

    data.clip_dt(DateTime(2013,8,10), DateTime(2014));

    printf("Data length after clip: %d bars\n\n", data.len());

    std::string s = "(not ENTERED_TODAY) and OUTSIDE_MARKET and TIME_IS_BEFORE(['end_time', 12:00]) and CROSSES( PRICE , VALUE_OF_WHEN_FIRST_HAPPENED( MIN_VALUE_SINCE( PRICE('low'), ['start_time', 09:31] ), "
        "PRICE_PERCENT_UP(min=['min_price_movement', 0.005], mode='since', since=['start_time', 09:31]) ) ) ~ "
        "(not ENTERED_TODAY) and OUTSIDE_MARKET and TIME_IS_BEFORE(['end_time', 12:00]) and CROSSES( PRICE , VALUE_OF_WHEN_FIRST_HAPPENED( MAX_VALUE_SINCE( PRICE('high'), ['start_time', 09:31] ), "
        "PRICE_PERCENT_DOWN(min=['min_price_movement', 0.005], mode='since', since=['start_time', 09:31]) ) ) ~ TIME_IS(['exit_time', 15:50])~ ";
        //"# timeframe_sec = 300 # exec_timeframe_sec = 1";
        //"# timeframe_sec = 300 # exec_timeframe_sec = 60"
    
    // Backtest strategy
    BacktestStats res;
    res = backtest_single_stree(data, s, opt);
    printf("%s", (res.print_str_short()).c_str());
    res.print_file("detailed_report.txt");

    //std::cout << st->print_EasyLanguage();

    // print the strategy EasyLanguage
    std::ofstream o1("el.txt");
    o1 << make_EL(s);
    o1.close();

    std::ofstream o2("mql4.txt");
    o2 << make_MQL4(s);
    o2.close();
}
#endif


void test_interactive_expressions()
{
    DataMatrix data;
    BacktestOptions opt;
    opt.commission = 0;
    opt.slippage = 0.0001;

#ifdef WIN32
    data.load_tradestation("eurusd-5m.txt");
#else
    data.load_tradestation("eurusd-5m.txt");
#endif

    printf("Data length before clip: %d bars\n", data.len());

    data.clip_dt(DateTime(2008), DateTime(2014));

    printf("Data length after clip: %d bars\n\n", data.len());

    std::vector<std::string> tokens;
    std::string line;
    std::string inp;
    unsigned int num_ms = 0;
    while(true)
    {
        // read the signal tree strategy from std::cin 
        inp = "";
        line = "";
        tokens.clear();
        num_ms = 0;

        printf("\n\nPaste strategy here: ");

        // read characters until 5 ~s are in

        while(num_ms < 5)
        {
            std::getline(std::cin, line);
            // count ~s in line
            for(unsigned int i=0; i<line.size(); i++)
            {
                if (line[i] == '~')
                {
                    num_ms++;
                }
            }
            
            inp += line;
        }
        
	    // Backtest strategy
	    BacktestStats res;
	    res = backtest_single_stree(data, inp, opt);
	    printf("%s", (res.print_str_short()).c_str());
	    //res.print_file("detailed_report.txt");
	    
        //std::cout << st->print_EasyLanguage();

        // print the strategy EasyLanguage
        std::ofstream o1("el.txt");
        o1 << make_EL(inp);
        o1.close();

        std::ofstream o2("mql4.txt");
        o2 << make_MQL4(inp);
        o2.close();
    }
}


#if 0
void test_interactive_expressions_multi()
{
    DataMatrix data;
    BacktestOptions opt;
    opt.commission = 0;
    opt.slippage = 0.0001;

#ifdef WIN32

    data.load_tsys("spy-1m.tsys");

#else

    data.load_tsys("spy-1m.tsys");

#endif
    printf("Data length: %d bars\n\n", data.len());

    std::vector<std::string> tokens;
    std::string line;
    std::string inp;
    while(true)
    {
        // read the signal tree strategy from std::cin 
        inp = "";
        line = "";
        tokens.clear();

        printf("\n\nPaste strategy here: ");

        // read characters until 5 ~s are in

        while(1)
        {
            std::getline(std::cin, line);
            // count ~s in line
            if (line == "end")
            {
                break;
            }

            inp += line;
        }

        // Backtest strategy
        BacktestStats res;
        res = backtest_single_sforest(data, inp, opt);
        printf("%s", (res.print_str_short()).c_str());
        res.print_file("detailed_report.txt");

        //std::cout << st->print_EasyLanguage();

        // print the strategy EasyLanguage
        std::ofstream o1("el.txt");
        o1 << make_forest_EL(inp);
        o1.close();

        std::ofstream o2("mql4.txt");
        o2 << make_forest_MQL4(inp);
        o2.close();
    }
}
#endif
*/


