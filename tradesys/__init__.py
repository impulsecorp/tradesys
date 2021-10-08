#!/usr/bin/python3
import os
import sys
import socket
sys.path.append(os.path.join(os.path.dirname(__file__)))
script_dir = os.path.dirname( os.path.abspath(__file__) )

import warnings
warnings.filterwarnings('ignore')

import matplotlib as mpl
if socket.gethostname() != 'desktop' and socket.gethostname() != 'laptop' and socket.gethostname() != 'slave':
    mpl.use('Agg')
    
from ._tradesys import *
from .optimize import optimize_inputs, sort_optimization_results_by
from .tstools import list_inputs, modify_inputs, get_inputs_constraints, df2dmat, dmat2df, time_range, NamedFileDict, ProgressBar
from .nbtools import *
from .visual import show_data, explore_data
from .visual import display_trade, explore_trades, display_event
from .visual import plot_duration_info, plot_info, dt2datetime, datetime2dt
#from . import db

import matplotlib.pyplot as plt
import pandas as pd
from datetime import datetime

# aliases
show_trade = display_trade
show_trades = explore_trades
examine_trades = explore_trades

display_data = show_data # todo: fix name inconsistency
examine_data = explore_data

true = True
false = False

class WebStrategy:
    def __init__(self, signals_dict):
        self.signals = {}
        self.ga_constraints = {}
        
        if isinstance(signals_dict, dict):
            for k,v in list(signals_dict.items()):
                self.signals[k] = v
                cs = get_inputs_constraints(v)
                self.ga_constraints = {kk:vv for kk, vv in list(self.ga_constraints.items()) + list(cs.items())}
            
        self.has_entry = any([x in self.signals 
                             for x in ['buy', 'sell']])
        self.has_exit = any([x in self.signals 
                             for x in ['close', 'sl', 'tp']])
        
        if all([x in self.signals for x in ['buy','sell']]):
            new_buy = "(((B(['reversed_entry', false]) == B(false)) and (%s)) or ((B(['reversed_entry', false]) == B(true)) and (%s)))" \
                                  % (self.signals['buy'], self.signals['sell'])
            new_sell = "(((B(['reversed_entry', false]) == B(false)) and (%s)) or ((B(['reversed_entry', false]) == B(true)) and (%s)))" \
                                  % (self.signals['sell'], self.signals['buy'])
            self.signals['buy'] = new_buy
            self.signals['sell'] = new_sell
        
        self.name = ""
        self.description = ""
        self.flags = ''
        self.ga_params = self.get_parameters()
    
    def get_parameters(self):
        inputs = {}
        for s,d in list(self.signals.items()):
            if d:
                k = list_inputs(d)
                if k:
                    inputs = dict(inputs, **k)
        return inputs
    
    def get_constraints(self):
        return self.ga_constraints

    def __repr__(self):
        buy = "B"
        sell = "B"
        close = "B"
        sl = "B"
        tp = "B"
        if hasattr(self, 'signals'):
            if self.signals:
                for k, v in list(self.signals.items()):
                    if k == 'buy':
                        buy = v
                    if k == 'sell':
                        sell = v
                    if k == 'close':
                        close = v
                    if k == 'sl':
                        sl = v
                    if k == 'tp':
                        tp = v

        return "\n;\n".join([_f for _f in [buy, sell, close, sl, tp] if _f])

class WebPairStrategy:
    def __init__(self, d0_signals_dict, d1_signals_dict):
        self.d0_signals = {}
        self.d1_signals = {}
        
        if isinstance(d0_signals_dict, dict):
            for k,v in list(d0_signals_dict.items()):
                self.d0_signals[k] = v
            
        if isinstance(d1_signals_dict, dict):
            for k,v in list(d1_signals_dict.items()):
                self.d1_signals[k] = v
                
        # the two signal dicts must have the same set of keys
        assert set(d0_signals_dict.keys()) == set(d1_signals_dict.keys())

        self.has_entry = any([x in self.d0_signals 
                             for x in ['buy', 'sell']])
        self.has_exit = any([x in self.d0_signals 
                             for x in ['close', 'sl', 'tp']])
        
        if all([x in self.d0_signals for x in ['buy','sell']]):
            new_buy = "(((B(['reversed_entry', false]) == B(false)) and (%s)) or ((B(['reversed_entry', false]) == B(true)) and (%s)))" \
                                  % (self.d0_signals['buy'], self.d0_signals['sell'])
            new_sell = "(((B(['reversed_entry', false]) == B(false)) and (%s)) or ((B(['reversed_entry', false]) == B(true)) and (%s)))" \
                                  % (self.d0_signals['sell'], self.d0_signals['buy'])
            self.d0_signals['buy'] = new_buy
            self.d0_signals['sell'] = new_sell
        
        if all([x in self.d1_signals for x in ['buy','sell']]):
            new_buy = "(((B(['reversed_entry', false]) == B(false)) and (%s)) or ((B(['reversed_entry', false]) == B(true)) and (%s)))" \
                                  % (self.d1_signals['buy'], self.d1_signals['sell'])
            new_sell = "(((B(['reversed_entry', false]) == B(false)) and (%s)) or ((B(['reversed_entry', false]) == B(true)) and (%s)))" \
                                  % (self.d1_signals['sell'], self.d1_signals['buy'])
            self.d1_signals['buy'] = new_buy
            self.d1_signals['sell'] = new_sell

        self.name = ""
        self.description = ""
        self.flags = ''
        
    def get_parameters(self):
        inputs = {}
        for s,d in list(self.d0_signals.items()):
            if d:
                k = list_inputs(d)
                if k:
                    inputs = dict(inputs, **k)
        for s,d in list(self.d1_signals.items()):
            if d:
                k = list_inputs(d)
                if k:
                    inputs = dict(inputs, **k)
        return inputs

    def __repr__(self):
        
        def st(signals):
            buy = "B"
            sell = "B"
            close = "B"
            sl = "B"
            tp = "B"
            if signals:
                for k, v in list(signals.items()):
                    if k == 'buy':
                        buy = v
                    if k == 'sell':
                        sell = v
                    if k == 'close':
                        close = v
                    if k == 'sl':
                        sl = v
                    if k == 'tp':
                        tp = v
            return "\n~\n".join([_f for _f in [buy, sell, close, sl, tp] if _f])
        
        s1 = st(self.d0_signals)
        s2 = st(self.d1_signals)
        
        return "%s\n | %s\n" % (s1, s2)





def info2df(info):
    " Converts a BacktestStats object to a Pandas DataFrame containing the list of trades "
    def conv_dt(d):
        " make datetime.datetime out of DateTime "
        return datetime(d.year, d.month, d.day, d.hour, d.minute, d.second)
    def conv_q(q):
        " convert quantity to float "
        if q.lots != 0:
            return float(q.lots)
        else:
            return float(q.shares)
    d = [(x.symbol, 'buy' if x.position == 1 else 'sell', 
          conv_dt(x.datetime_of_entry), conv_dt(x.datetime_of_exit), 
          conv_q(x.quantity), x.price_at_entry, x.price_at_exit, x.profit, 
          x.commission_paid, x.slippage_paid, x.runup, x.drawdown, x.comment)
         for x in info.list_of_trades]
    cols = ['symbol', 'position', 
          'entry_time', 'exit_time', 
          'quantity', 'entry_price', 'exit_price', 'profit', 
          'comm_paid', 'slip_paid', 'runup', 'drawdown', 'comment']
    return pd.DataFrame(columns=cols, data=d)

def inst(t, x):
    return x in str(type(t))

def backtest(data, strategy, **kwargs):
    """ unified backtest() with full support for IPython, Pandas and Matplotlib 
    Backtests the given strategy and returns the info. Printing/plotting the results is also possible.
    
    Supported keywords:
    0
    options : the BacktestOptions used for the backtest
    prt : if True, will print the info
    plot : if True, will plot the info
    modify_inputs : dict of { 'name' : value } pairs 00
    df_trades : also return a Pandas dataframe of the list of trades
    
    """

    # sanitize stuff
    if data is None or strategy is None:
        raise TypeError("data and strategy can't be None")
    #print(type(data), type(MultiDataMatrix()), 'MultiDataMatrix' in str(type(data)))
    if not (inst(data, 'DataMatrix') or inst(data, 'MultiDataMatrix') or isinstance(data, pd.DataFrame)
            or isinstance(data, tuple) or isinstance(data, dict)):
        raise TypeError("""data can be one of the following types: 
        DataMatrix, MultiDataMatrix, pandas DataFrame, tuple/dict for DB lookup, or list of DataMatrix, DataFrame or tuple/dict 
        (for multi-data)""")
    if not strategy:
        raise TypeError("Empty strategy")
        
    try:
        options = kwargs['options']
    except KeyError:
        options = BacktestOptions()
    if not options:
        options = BacktestOptions()
        
    try:
        print_info = kwargs['prt']
    except KeyError:
        print_info = False
        
    try:
        df_trades = kwargs['df_trades']
    except KeyError:
        df_trades = False

    try:
        plot_info = kwargs['plot']
    except KeyError:
        plot_info = False

    try:
        m_inputs = kwargs['modify_inputs']
    except KeyError:
        m_inputs = None

    # make it a DataMatrix from now on
    if isinstance(data, pd.DataFrame):
        data = df2dmat(data)
    elif isinstance(data, tuple): # get from DB
        from . import db
        data = db.get_symbol(*data)
        if data is None:
            raise ValueError("Symbol doesn't exist")
    elif isinstance(data, dict):  # get from DB
        from . import db
        data = db.get_symbol(**data)
        if data is None:
            raise ValueError("Symbol doesn't exist")
    
    # remove comments first
    strategy = remove_comments(strategy)
    
    # modify inputs
    if m_inputs:
        strategy = modify_inputs(strategy, m_inputs)
    
    # determine type of strategy (single/forest/multi)
    stype = ''
    if '|' in strategy:
        stype = 'multi'
    elif '$' in strategy:
        stype = 'forest'
    else:
        stype = 'single'
        
    if ('stype' == 'multi') and inst(data, 'DataMatrix'):
        raise TypeError("Can't use multi-strategy with a single DataMatrix/DataFrame")
        
    # backtest
    if stype == 'single':
        info = backtest_single_stree(data, strategy, options)
    elif stype == 'forest':
        info = backtest_single_sforest(data, strategy, options)
    elif stype == 'multi':
        info = backtest_multi_stree(data, strategy, options)
    else:
        raise ValueError("Unknown strategy type (single, forest or multi?)")
        
    if print_info:
        print( info )
    if plot_info:
        visual.plot_info( info )
        
    if df_trades:
        return info, info2df(info)
    else:
        return info















