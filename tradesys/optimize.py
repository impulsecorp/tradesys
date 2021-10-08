#!/usr/bin/python
import collections
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__)))
import time
import random as rnd
import pickle
import itertools as it
import gzip
import multiprocessing as mpc

try:
    from ipyparallel import Client
    has_ipython = True
except:
    has_ipython = False

try:
    from _tradesys import backtest_single_stree, backtest_multi_stree
    from _tradesys import DataMatrix, MultiDataMatrix, BacktestOptions, Time
except:
    from tradesys._tradesys import backtest_single_stree, backtest_multi_stree
    from tradesys._tradesys import DataMatrix, MultiDataMatrix, BacktestOptions, Time

from .tstools import modify_inputs, list_inputs, ProgressBar


def convert_keys(keys):
    newkeys = []
    for key in keys:
        # replace 2 spaces with 1 until they no longer differ
        # this will shrink the whitespace 
        s = key
        ns = s.replace('  ', ' ')
        while s != ns:
            s = ns
            ns = s.replace('  ', ' ')
            
        # underscores also
        s = ns
        ns = s.replace('__', '_')
        while s != ns:
            s = ns
            ns = s.replace('__', '_')
        # replace all single spaces with a single underscore
        ns = ns.replace('_ ', '_').replace(' _','_')
        newkey = ns.replace(' ', '_')
        # trim any leading underscores
        # count how many
        n = 0
        for x in newkey:
            if x == '_':
                n += 1
            else: break
        newkey = newkey[n:]
        # add 1 leading underscore
        #newkey = '_' + newkey
        # done
        newkeys.append(newkey.lower())
    return newkeys
            
def init_kwargs(kwargs):
    # initialize all required parameters
    # sort by
    if 'sort_by' not in list(kwargs.keys()):
        sort_by = 'profit_factor'
    elif kwargs['sort_by'] is not None:
        sort_by = kwargs['sort_by']
    else:
        sort_by = 'profit_factor'
    # cutoff
    if 'cutoff' not in list(kwargs.keys()):
        cutoff = None
    elif kwargs['cutoff'] is not None:
        cutoff = kwargs['cutoff']
    else:
        cutoff = None
    # keep_empty
    if 'keep_empty' not in list(kwargs.keys()):
        keep_empty = None
    elif kwargs['keep_empty'] is not None:
        keep_empty = kwargs['keep_empty']
    else:
        keep_empty = None
    # keep_only
    if 'keep_only' not in list(kwargs.keys()):
        keep_only = None
    elif kwargs['keep_only'] is not None:
        keep_only = kwargs['keep_only']
    else:
        keep_only = None
    # parallel
    if 'parallel' not in list(kwargs.keys()):
        parallel = False
    elif kwargs['parallel'] is not None:
        parallel = kwargs['parallel']
    else:
        parallel = False
    # report_file
    if 'report_file' not in list(kwargs.keys()):
        report_file = None
    elif kwargs['report_file'] is not None:
        report_file = kwargs['report_file']
    else:
        report_file = None
    # report_save_freq
    if 'report_save_freq' not in list(kwargs.keys()):
        report_save_freq = 10000
    elif kwargs['report_save_freq'] is not None:
        report_save_freq = kwargs['report_save_freq']
    else:
        report_save_freq = 10000
    # client_json
    if 'client_json' not in list(kwargs.keys()):
        client_json = None
    elif kwargs['client_json'] is not None:
        client_json = kwargs['client_json']
    else:
        client_json = None
    # client_profile
    if 'client_profile' not in list(kwargs.keys()):
        client_profile = 'default'
    elif kwargs['client_profile'] is not None:
        client_profile = kwargs['client_profile']
    else:
        client_profile = 'default'
        
    return (parallel, sort_by, cutoff, keep_empty, keep_only, 
            report_file, report_save_freq, client_json, client_profile)

def sort_optimization_results_by(saved_info, sort_by, keep_only):
    if keep_only:
        if sort_by in keep_only:
            # sort by this
            idx = keep_only.index(sort_by)
        else:
            idx = 0
        return sorted(saved_info, key = eval("lambda x: x[1][%d]" % idx))[::-1]
    else:
        try:
            return sorted(saved_info, key = eval("lambda x: x[1].%s" % sort_by))[::-1]
        except:
            return sorted(saved_info, key = lambda x: x[1].net_profit)[::-1]

def optimize_inputs(data, strategy, inputs, 
                    options=None, **kwargs):
    """
    Search all combinations of the inputs and sort them by specified BacktestStats member and method
    
    inputs is a dict like this:
    { input_name (str) : [v1, v2, ... vn] (iterable) }
    
    input_name must resemble an existing input/parameter member variable of the strategy.
    Spaces and generally improper names are corrected automatically.
    
    Accepted kwargs:
    
    sort_by (str): specifies the member in the BacktestStats class that will be the key for sorting, 
    highest first.
    
    cutoff (numeric, bool, callable, none): is an optional parameter that specifies which reports/infos to save - 
    they will be saved only if info.*sort_by* is larger than cutoff (if numeric) or different (if bool)
    If callable, then the info is passed to the function as parameter and the return value is tested for truth.
    
    keep_empty (anything): if evaluates to True, will keep reports that generate no trades
    keep_only (list of strings): if not empty, it will keep only the specified info members, 
                                 not the entire stats report
    
    parallel (bool) : controls whether we run the optimization in parallel or serial mode. 
    parallel_processes (int) : if parallel, how many CPUs to use
    use_ipython (bool) : If true (default is false), will use IPython for parallel computing
    
    report_file (str) : specifies if the user wants to save reports immediately after computing and where.
    report_save_freq (int) : if report_file is not none, save things each this many backtests 
                                  
    client_json (str) : path to the client JSON file used to connect to the computing cluster. 
    client_profile (str) : IPython profile, in case client_json is empty
    
    status : {'ETA' : str, 
              'text' : str, 
              'percents' : float} - the status shared ctypes variables to reflect the optimization progress 
              
    limit : (int) : maximum number of backtests to do
    
    use_gzip : (bool) : if True, use gzip to pack the results, else use pickle only. 

    """
    (parallel, sort_by, cutoff, keep_empty, keep_only, report_file, 
     report_save_freq, client_json, client_profile) = init_kwargs(kwargs)
     
    # todo: implement variant with multiprocessing for single PC
    try:
        use_ipython = kwargs['use_ipython']
    except KeyError:
        use_ipython = False

    try:
        quiet = kwargs['quiet']
    except KeyError:
        quiet = False
        
    try:
        status = kwargs['status']
    except KeyError:
        status = None
        
    if parallel and use_ipython and (not has_ipython):
        parallel = False
        
    try:
        parallel_processes = kwargs['parallel_processes']
    except:
        parallel_processes = 4
        
    try:
        use_gzip = kwargs['use_gzip']
    except KeyError:
        use_gzip = False

    # ensure parameters are not None
    if data is None or strategy is None or inputs is None:
        raise AttributeError
    
    if isinstance(data, DataMatrix):
        multi = False
    elif isinstance(data, MultiDataMatrix):
        multi = True
    else:
        raise ValueError("DataMatrix or MultiDataMatrix parameter required.")
    
    if not options:
        options = BacktestOptions() # use the defaults
        options.commission = 0
        options.slippage = 0
        options.market_open_time = Time(0)
        options.market_close_time = Time(0)
    
    # get all combinations and step through them. 
    keys = [x[0] for x in sorted(inputs.items())]
    vals = [x[1] for x in sorted(inputs.items())]
    # if any value is not iterable, make it so it is
    nvals = []
    for v in vals:
        if not hasattr(v, '__iter__'): # not iterable? 
            v = [v]
        nvals.append(v)
    vals = nvals
        
    val_combinations = list(it.product(*vals))
    
    # if the keys are not in the format _variable_name_with_blabla, 
    # convert them
    newkeys = convert_keys(keys)
    inps = list_inputs(strategy)
    # ensure each key in the converted (or not) keys exists as member in strategy
    for key in newkeys:
        if key not in list(inps.keys()):
            raise KeyError("The key "+key+" was not found in strategy "+repr(strategy))
    keys = newkeys
    
    # The optimizer will create a file containing what was saved so far. 
    # It can be used for future analysis, etc. 
    # It's a pickled list of ({ keys, vals }, info) tuples.
    try:
        if use_gzip:
            saved_info = pickle.load(gzip.open(report_file, 'rb'))
        else:
            saved_info = pickle.load(open(report_file, 'rb'))
    except:
        saved_info = []
    
    # remove all val combinations already present in saved_info
    saved_already = [tuple([x[0][k] for k in keys]) for x in saved_info]
    vals_for_computation = []
    for comb in val_combinations:
        if comb not in saved_already:
            vals_for_computation.append(comb)
    val_combinations = vals_for_computation 
        
    # now shuffle the combinations
    rnd.shuffle(val_combinations)
    
    # saves the backtest info to memory
    def keep(keys, comb, info, cutoff):
        if not keep_only:
            
            if cutoff is not None:
                # determine if it should be saved depending on cutoff
                if isinstance(cutoff, int) or isinstance(cutoff, float):
                    exec(("b = info."+sort_by+" > cutoff"), locals())
                    if b:
                        saved_info.append( ( dict(list(zip(keys, comb))), info) )
                elif isinstance(cutoff, collections.Callable):
                    if cutoff(info):
                        saved_info.append( ( dict(list(zip(keys, comb))), info) )
                else:
                    raise TypeError("Unknown cutoff type")
            else:
                # save by default
                saved_info.append( ( dict(list(zip(keys, comb))), info) )
            
        else:
            # keep only specific stuff
            kept = []
            for k in keep_only:
                try:
                    if k == 'list_of_trades':
                        kept.append(list(info.list_of_trades))
                    else:
                        # meta trick
                        exec(("kept.append(info."+k+")"), locals())
                except:
                    # no such member, don't do anything
                    pass
            # kept cannot be empty
            if not kept:
                kept = [info.net_profit]
            kept = tuple(kept)
            
            if cutoff is not None:
                # determine if it should be saved depending on cutoff
                if isinstance(cutoff, int) or isinstance(cutoff, float):
                    exec(("b = info."+sort_by+" > cutoff"), locals())
                    if b:
                        saved_info.append( ( dict(list(zip(keys, comb))), kept) )
                elif isinstance(cutoff, collections.Callable):
                    if cutoff(info):
                        saved_info.append( ( dict(list(zip(keys, comb))), kept) )
                else:
                    raise TypeError("Unknown cutoff type")
            else:
                saved_info.append( ( dict(list(zip(keys, comb))), kept) )

    # saves all current results to a file        
    def report():
        if report_file and ((i % report_save_freq) == 0):
            if use_gzip:
                pickle.dump(saved_info, gzip.open(report_file, 'wb'), protocol=3)
            else:
                pickle.dump(saved_info, open(report_file, 'wb'), protocol=3)

    curtime = time.time()
    
    if vals_for_computation:
        if not parallel:
            try:
                if not quiet: print('Backtesting..')
                progress = ProgressBar(len(val_combinations), status=status, quiet=quiet)
                # for each combination
                for i, comb in enumerate(val_combinations):
                    s = modify_inputs(strategy, dict(list(zip(keys, comb))))
                    if not multi:
                        info = backtest_single_stree(data, s, options)
                    else:
                        info = backtest_multi_stree(data, s, options)
                    if info.num_trades == 0:
                        if keep_empty:
                            keep(keys, comb, info, cutoff)
                    else:
                        keep(keys, comb, info, cutoff)
                        
                    progress.animate(i+1)
                    report()
                    
                    # cancel condition
                    if status: # also report status here
                        if status.haskey('command'):
                            if status.getval('command') == 'cancel':
                                break
                    
                progress.finish()

            except KeyboardInterrupt:
                pass
            
        else:
            
            if use_ipython:
                if client_json:
                    rc = Client(client_json)
                else:
                    if client_profile:
                        rc = Client(profile=client_profile)
                    else:
                        rc = Client()
                lbview = rc.load_balanced_view()
                dview = rc[:]

                dview.execute("import sys", block=True)
                dview.execute("sys.path.insert(0, '/home/peter/code/projects/tradesys/')", block=True)
                dview.execute("from tradesys import *", block=True)

                lbview.block = True
                dview.block = True
                
                if not quiet: print(('Engines:', len(dview)))

                if not quiet: print("Sending data..")

                # send the data to the engines
                dview['data'] = data
                dview['options'] = options
                dview.wait()
        
                # cleanup 
                # rc.purge_everything()
                
                # sync imports
                #with dview.sync_imports():
                #    import tradesys

                def backtester_ipy(tup):
                    m, c, s = tup
                    if not m:
                        info = backtest_single_stree(data, s, options)
                    else:
                        info = backtest_multi_stree(data, s, options)
                    return c, info


                try:

                    # make arg list
                    arg_list = [(multi, comb, modify_inputs(strategy, dict(list(zip(keys, comb)))))
                                for comb in val_combinations]

                    if not quiet: print('Backtesting..')
                    progress = ProgressBar(len(val_combinations), status=status, quiet=quiet)
                    # map the backtester to the arguments
                    amr = dview.imap(backtester_ipy, arg_list)  # , block=False)
                    #dview.wait_interactive()
                    for i, (comb, info) in enumerate(amr):
                        if info.num_trades == 0:
                            if keep_empty:
                                keep(keys, comb, info, cutoff)
                        else:
                            keep(keys, comb, info, cutoff)
                        progress.animate(i + 1)
                        report()
                        # cancel condition
                        if status:
                            if status.haskey('command'):
                                if status.getval('command') == 'cancel':
                                    break

                except KeyboardInterrupt:
                   pass

                progress.finish()


                # Cleanup the engines
                #try:
                dview['data'] = None
                dview['options'] = None
                #lbview.purge_results(jobs='all', targets='all')
                # except:
                #    pass

                #try:
                # rc.purge_everything()
                #rc.close()
                # except:
                #    pass

            # else use MPC
            else:

                def backtester_mpc(tup):
                    data, options, m, c, s = tup
                    if not m:
                        info = backtest_single_stree(data, s, options)
                    else:
                        info = backtest_multi_stree(data, s, options)
                    return c, info

                # TODO: Use futures when everything is moved to Python 3
                pool = mpc.Pool(processes = parallel_processes)
                try:
                    # make arg list
                    arg_list = [(data, options, multi, comb, modify_inputs(strategy, dict(list(zip(keys, comb))))) 
                                for comb in val_combinations]
                    if not quiet: print('Backtesting..')
                    progress = ProgressBar(len(val_combinations), status=status, quiet=quiet)
                    for i, (comb, info) in enumerate(pool.imap(backtester_mpc, arg_list)):
                        if info.num_trades == 0:
                            if keep_empty:
                                keep(keys, comb, info, cutoff)
                        else:
                            keep(keys, comb, info, cutoff)
                        progress.animate(i+1)
                        report()
                        # cancel condition
                        if status: 
                            if status.haskey('command'):
                                if status.getval('command') == 'cancel':
                                    break
                        
                    progress.finish()
                except KeyboardInterrupt:
                    pass
                pool.close()
    
    elapsed = time.time() - curtime
    if not quiet: print(('\nseconds elapsed: %3.2f' % elapsed))
    
    # to update the status if any
    progress = ProgressBar(len(val_combinations), status=status, quiet=quiet)
    progress.finish()
    
    # save and return sorted info
    s = sort_optimization_results_by(saved_info, sort_by, keep_only) 
    if val_combinations and report_file:
        if use_gzip:
            pickle.dump(s, gzip.open(report_file, 'wb'), protocol=3)
        else:
            pickle.dump(s, open(report_file, 'wb'), protocol=3)
    return s


if __name__ == '__main__':
    pass
