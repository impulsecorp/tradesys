try:
    from _tradesys import *
except:
    from tradesys import *
from datetime import datetime, timedelta
 
def DT2datetime(dt):
    return datetime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second)

def datetime2DT(dt):
    return DateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second)

def rising(num_bars=15, bar_height = 1.0, high_low_fraction = 0.25, gap = 0,
           start_price = 1.0, start_datetime=datetime(2000,1,1, 0,0),
           td = timedelta(minutes=1)):
    d = start_datetime
    op = start_price
    cl = op + bar_height
    hi = max(op, cl) + abs(op-cl)*high_low_fraction
    lo = min(op, cl) - abs(op-cl)*high_low_fraction
    data = DataMatrix()
    data.add_row(datetime2DT(d), op,hi,lo,cl, 1)
    for i in range(num_bars-1):
        d += td
        op = cl + gap
        cl = op + bar_height
        hi = max(op, cl) + abs(op-cl)*high_low_fraction
        lo = min(op, cl) - abs(op-cl)*high_low_fraction
        data.add_row(datetime2DT(d), op,hi,lo,cl,1)
    return data

def add_rising(data, num_bars=15, bar_height = 1.0, high_low_fraction = 0.25, gap = 0):
    x = data.get_dt_list()
    td = x[1]-x[0]
    d = x[-1]
    cl = data.close[-1]+gap
    for i in range(num_bars):
        d += td
        op = cl + gap
        cl = op + bar_height
        hi = max(op, cl) + abs(op-cl)*high_low_fraction
        lo = min(op, cl) - abs(op-cl)*high_low_fraction
        data.add_row(datetime2DT(d), op,hi,lo,cl,1)

def falling(num_bars=15, bar_height = 1.0, high_low_fraction = 0.25, gap = 0,
           start_price = 100.0, start_datetime=datetime(2000,1,1, 0,0),
           td = timedelta(minutes=1)):
    d = start_datetime
    op = start_price
    cl = op - bar_height
    hi = max(op, cl) + abs(op-cl)*high_low_fraction
    lo = min(op, cl) - abs(op-cl)*high_low_fraction
    data = DataMatrix()
    data.add_row(datetime2DT(d), op,hi,lo,cl,1)
    for i in range(num_bars-1):
        d += td
        op = cl - gap
        cl = op - bar_height
        hi = max(op, cl) + abs(op-cl)*high_low_fraction
        lo = min(op, cl) - abs(op-cl)*high_low_fraction
        data.add_row(datetime2DT(d), op,hi,lo,cl,1)
    return data

def add_falling(data, num_bars=15, bar_height = 1.0, high_low_fraction = 0.25, gap = 0):
    x = data.get_dt_list()
    td = x[1]-x[0]
    d = x[-1]
    cl = data.close[-1]-gap
    for i in range(num_bars):
        d += td
        op = cl - gap
        cl = op - bar_height
        hi = max(op, cl) + abs(op-cl)*high_low_fraction
        lo = min(op, cl) - abs(op-cl)*high_low_fraction
        data.add_row(datetime2DT(d), op,hi,lo,cl,1)

def stock_options():
    opts = BacktestOptions()
    opts.commission = 0
    opts.slippage = 0.0
    opts.backtest_mode = BacktestMode.TRADESTATION
    opts.market_open_time = Time(9,30)
    opts.market_close_time = Time(16,0)
    return opts

def fx_options():
    opts = BacktestOptions()
    opts.commission = 0
    opts.slippage = 0.0
    opts.backtest_mode = BacktestMode.METATRADER
    opts.market_open_time = Time(0,0)
    opts.market_close_time = Time(0,0)
    return opts

def opts2cpp(opts):
    " Converts backtest options to C++ "
    s = []
    s.append('BacktestOptions opts;')
    s.append('opts.commission = %3.6f;' % opts.commission)
    s.append('opts.slippage = %3.6f;' % opts.slippage)
    if opts.backtest_mode == BacktestMode.METATRADER:
        s.append('opts.backtest_mode = METATRADER;')
    else:
        s.append('opts.backtest_mode = TRADESTATION;')
    s.append('opts.market_open_time = Time(%d,%d);' % (opts.market_open_time.hour, opts.market_open_time.minute))
    s.append('opts.market_close_time = Time(%d,%d);' % (opts.market_close_time.hour, opts.market_close_time.minute))
    s = '\n'.join(s)
    return s

def data2cpp(data):
    " Converts test data to C++ "
    s = []
    s.append("DataMatrix data;")
    dt = data.get_dt_list()
    for i in range(len(data)):
        s.append("data.add_row( DateTime(%d, %d, %d, %d, %d, %d), %3.6f, %3.6f, %3.6f, %3.6f, %d);" 
                 % (dt[i].year, dt[i].month, dt[i].day, dt[i].hour, dt[i].minute, dt[i].second, 
                   data.open[i], data.high[i], data.low[i], data.close[i], data.volume[i]))
    s = '\n'.join(s)
    return s

def mdata2cpp(mdata):
    " Converts test multi-data to C++ "
    s = []
    s.append("MultiDataMatrix mdata;")
    s.append("mdata.symbols.resize(%d);" % len(mdata.symbols))
    for idx,m in enumerate(mdata.symbols):
        if m.name:
            s.append('mdata.symbols[%d].name = "%s";' % (idx, m.name))
        else:
            s.append('mdata.symbols[%d].name = "DATA_%d";' % (idx, idx))
        dt = m.get_dt_list()
        for i in range(len(m)):
            s.append("mdata.symbols[%d].add_row( DateTime(%d, %d, %d, %d, %d, %d), %3.6f, %3.6f, %3.6f, %3.6f, %d);" 
                     % (idx, dt[i].year, dt[i].month, dt[i].day, dt[i].hour, dt[i].minute, dt[i].second, 
                       m.open[i], m.high[i], m.low[i], m.close[i], m.volume[i]))
    s = '\n'.join(s)
    return s

def signal_cpp_test(data, signal):
    " Outputs unit test boilerplate for testing with one signal tree logger "
    
    s = 'TEST(TestCategory, TestName)\n{\n'
    s += data2cpp(data)
    s += '\n\n'
    s += opts2cpp(fx_options())
    s += '\n\n'
    
    s += 'std::string s; s = "%s"; \n' % signal.replace('"',"'").replace('\n','"\n' + (' '*19) + '"')
    s += 'SignalTreeLoggerStrategy* st = new SignalTreeLoggerStrategy(s); \n\n'
    
    s += 'std::vector<int> i; std::vector<double> f; std::vector<bool> b; \n'
    s += 'backtest_single(data, *st, opts); \n'
    s += 'if (st->m_tree->m_return_type == RT_INT) { i = st->m_int_result; }; \n'
    s += 'if (st->m_tree->m_return_type == RT_FLOAT) { f = st->m_float_result; }; \n'
    s += 'if (st->m_tree->m_return_type == RT_BOOL) { b = st->m_bool_result; }; \n'

    s += '\n/* tests here with i, f, b */\n\n'
    
    s += 'delete st;\n}'
    
    s = s.split('\n')
    s = s[0:2] + ['    '+x for x in s[2:-1]] + [s[-1]]
    s = '\n'.join(s)
    
    return s

    
def strategy_cpp_test(data, options, strategy):
    " Outputs unit test boilerplate for testing with a single signal tree strategy "
    
    s = 'TEST(TestCategory, TestName)\n{\n'
    s += data2cpp(data)
    s += '\n\n'
    s += opts2cpp(options)
    s += '\n\n'
    
    s += 'std::string s; s = "%s"; \n' % strategy.replace('"',"'").replace('\n','"\n' + (' '*19) + '"')
    s += 'SignalTreeStrategy* st = new SignalTreeStrategy(s); \n\n'
    
    s += 'BacktestStats info; \n'
    s += 'info = backtest_single(data, *st, opts); \n'

    s += '\n/* tests here with info */\n\n'
    
    s += 'delete st;\n}'
    
    s = s.split('\n')
    s = s[0:2] + ['    '+x for x in s[2:-1]] + [s[-1]]
    s = '\n'.join(s)
    
    return s

def multi_signal_cpp_test(mdata, options, signal):
    " Outputs unit test boilerplate for testing with a single signal tree strategy "

    s = 'TEST(TestCategory, TestName)\n{\n'
    s += mdata2cpp(mdata)
    s += '\n\n'
    s += opts2cpp(options) + '\n'
    s += 'std::vector<BacktestOptions> mopts; mopts.resize(mdata.symbols.size()); \n'
    s += 'for(unsigned int i=0; i<mopts.size(); i++) mopts[i] = opts; \n'
    s += '\n\n'

    s += 'std::string s; s = "%s"; \n' % signal.replace('"',"'").replace('\n','"\n' + (' '*19) + '"')
    #s += 'Py_Initialize(); \n\n'

    s += 'std::vector< std::vector<double> > k; \n'
    s += 'k = get_float_multi_stree_log(s, mdata); \n'

    s += '\n/* tests here with k */\n\n'

    s += ';\n}'

    s = s.split('\n')
    s = s[0:2] + ['    '+x for x in s[2:-1]] + [s[-1]]
    s = '\n'.join(s)

    return s


def multi_strategy_cpp_test(mdata, options, strategy):
    " Outputs unit test boilerplate for testing with a single signal tree strategy "
    
    s = 'TEST(TestCategory, TestName)\n{\n'
    s += mdata2cpp(mdata)
    s += '\n\n'
    s += opts2cpp(options) + '\n'
    s += 'std::vector<BacktestOptions> mopts; mopts.resize(mdata.symbols.size()); \n'
    s += 'for(unsigned int i=0; i<mopts.size(); i++) mopts[i] = opts; \n'
    s += '\n\n'
    
    s += 'std::string s; s = "%s"; \n' % strategy.replace('"',"'").replace('\n','"\n' + (' '*19) + '"')
    s += 'SignalTreeMultiStrategy* st = new SignalTreeMultiStrategy(s, mdata); \n\n'
    
    s += 'BacktestStats info; \n'
    s += 'info = backtest_multi(mdata, *st, mopts); \n'

    s += '\n/* tests here with info */\n\n'
    
    s += 'delete st;\n}'
    
    s = s.split('\n')
    s = s[0:2] + ['    '+x for x in s[2:-1]] + [s[-1]]
    s = '\n'.join(s)
    
    return s

