

import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__)))
import re 
from datetime import timedelta
import time
import pickle as pkl
from _tradesys import DateTime, DataMatrix, MultiDataMatrix, eval_stree_expr, TradeInfo, PositionType

try:
    from IPython.core.display import clear_output
except:
    def clear_output():
        pass

try:
    import pandas as pd
    pandas_ok = True
except:
    pandas_ok = False

input_re = re.compile('(\[)(\'|\")([a-zA-Z0-9_]+)(\'|\")([ ,]+)([\-.:0-9a-zA-Z]+)(\])')
eq_input_re = re.compile('([a-zA-Z0-9_]+)=(\[)(\'|\")([a-zA-Z0-9_]+)(\'|\")([ ,]+)([\-.:0-9a-zA-Z]+)(\])')

def chunks(l, n):
    " Yield successive n-sized chunks from l. "
    for i in range(0, len(l), n):
        yield l[i:i+n]
        


def get_inputs_constraints(s):
    """
    Takes a signal tree strategy and returns a dict like { 'input_name' : { min/max constraints dict } } 
    """
    ex = eval_stree_expr(s)
    s = str(ex)
    exl = ex.flatten_tree()
    constraints = {}
    for si in exl:
        params = si.get_parameters(True)
        for k,v in list(params.items()):
            a = si.get_parameter_constraints(k)
            del a['usage_mode']
            constraints[k] = a
    nmap = {}
    for t in eq_input_re.findall(s):
        k = t[0]
        v = t[3]
        nmap[k] = v
    nc = {}
    for k, v in list(constraints.items()):
        try:
            nc[nmap[k]] = v
        except:
            pass
    return nc




def list_inputs(s):
    """
    Takes a signal tree strategy and returns a dict like { 'input_name' : value } 
    """
    #import pdb; pdb.set_trace()
    
    s = s.replace('"', "'")
    found = list(set(re.findall(input_re, s)))
    if not found:
        return {} # No inputs found in the strategy
    d = {}
    for f in found:
        if (':' in f[5]):
            d[f[2]] = str(f[5])
        elif (f[5].lower() == 'true') or (f[5].lower() == 'false'):
            if f[5].lower() == 'true':
                d[f[2]] = True
            if f[5].lower() == 'false':
                d[f[2]] = False
        elif '.' in f[5]:
            d[f[2]] = float(f[5])
        else:
            d[f[2]] = int(f[5])
    return d


def modify_inputs(s, sdict): 
    """
    returns an altered strategy where the inputs specified as sdict's keys are 
    replaced with the sdict's values. 
    
    sdict is like { 'input_name' : new_value } 
    """
    if (not sdict) or (not isinstance(sdict, dict)):
        return s 
    
    s = s.replace('"', "'")
    found = list(set(re.findall(input_re, s)))
    if not found:
        raise ValueError('No inputs found in the strategy.')

    # get all combinations and step through them. 
    keys = [x[0] for x in sorted(sdict.items())]
    vals = [x[1] for x in sorted(sdict.items())]
    
    modif = s
    for k, v in zip(keys, vals):
        # find where the k is
        idx = 0
        is_there = False
        for i, f in enumerate(found):
            if f[2] == k:
                idx = i
                is_there = True
                break
        if is_there:
            reptup = list(found[idx])
            orig = "".join(reptup)
            reptup[5] = str(v).lower()
            rep = "".join(reptup) 
            modif = modif.replace(orig, rep)
        else:
            pass # silent - do nothing
            #raise KeyError('The input '+k+' was not found in strategy')
    return modif

if pandas_ok:
    def dmat2df(dmat):
        " Takes a DataMatrix and returns a Pandas DataFrame "
        кур = DataMatrix()
        if not (кур.__class__ == dmat.__class__):
            print(кур.__class__, dmat.__class__)
            raise ValueError("Needs a DataMatrix as argument")
        af = pd.DataFrame(  {'open': dmat.open, 
                             'high': dmat.high, 
                             'low' : dmat.low, 
                             'close' : dmat.close, 
                             'volume' : dmat.volume, }, 
                             index = dmat.get_dt_list())
        af = af.reindex_axis(sorted(af.columns), axis=1) # sort column names # todo: check if this somehow reverses open/close
        return af

    def df2dmat(df):
        " Takes a Pandas DataFrame and returns a DataMatrix "
        if not isinstance(df, pd.DataFrame):
            raise ValueError("Needs a DataFrame as argument")
        dmat = DataMatrix()
        for i, k in df.iterrows():
            dt = DateTime(i.year, i.month, i.day, 
                          i.hour, i.minute, i.second)
            if len(k) == 5:
                o, h, l, c, v = k
            elif len(k) == 4:
                o, h, l, c = k 
                v = 1
            else:
                raise ValueError("The DataFrame doesn't appear to hold OHLC or OHLCV data")
            dmat.add_row(dt, float(o), float(h), float(l), float(c), int(v))
        return dmat
else:
    def dmat2df(dmat):
        raise NotImplementedError("Pandas not installed")
    def df2dmat(df):
        raise NotImplementedError("Pandas not installed")
    
class NamedFileDict(object):
    def __init__(self, name, **kwargs):
        # if the object already exists, do nothing
        self.name = name
        if os.path.isfile('/tmp/'+name):
            pass
        else:
            # create the new file
            f = open('/tmp/'+name, 'wb')
            pkl.dump(kwargs, f, protocol=2)
            f.close()
            
    def getval(self, key):
        try:
            # read the dict from the file, and return the value
            d = pkl.load(open('/tmp/'+self.name, 'rb'))
            return d[key]
        except:
            pass

    def setval(self, key, val):
        try:
            f = open('/tmp/'+self.name, 'rb')
            d = pkl.load(f)
            f.close()
            d[key] = val
            f = open('/tmp/'+self.name, 'wb')
            pkl.dump(d, f)
            f.close()
        except: 
            pass
        
    def haskey(self, key):
        try:
            f = open('/tmp/'+self.name, 'rb')
            d = pkl.load(f)
            f.close()
            return key in d
        except:
            pass
        
class ProgressBar:
    def __init__(self, all_iterations, status = None, quiet=False):
        self.all_iterations = all_iterations
        self.prog_bar = '[]'
        self.fill_char = '.'
        self.width = 40
        self.__update_amount(0)
        self.start_time = time.time()
        self.status = status
        self.quiet = quiet

    def animate(self, iter):
        self.update_iteration(iter, time.time() - self.start_time)
        
        if not self.quiet:
            if (iter % 100) == 0:
                try:
                    clear_output(wait=True)
                except:
                    pass
            print('\r'+str(self), end='')
            sys.stdout.flush()

        if self.status:
            self.status.setval('ETA', self.ETA)
            self.status.setval('text', '%d of %d complete' % (iter, self.all_iterations))
            self.status.setval('percents', (iter / float(self.all_iterations)) * 100.0)
            
    def finish(self):
        if self.status:
            self.status.setval('ETA', str(timedelta(seconds=0)))
            self.status.setval('text', 'done')
            self.status.setval('percents', 100.0)

    def update_iteration(self, elapsed_iter, elapsed_seconds):
        self.__update_amount((elapsed_iter / float(self.all_iterations)) * 100.0)
        # calculate ETA
        if elapsed_seconds > 0 and elapsed_iter > 0:
            seconds_per_iter = elapsed_seconds / float(elapsed_iter)
        else:
            seconds_per_iter = 0
        time_left = (self.all_iterations - elapsed_iter)*seconds_per_iter
        self.prog_bar += '  %d of %d complete, ETA: %s' % (elapsed_iter, self.all_iterations, 
                                                           timedelta(seconds=int(time_left)))
        self.ETA = str(timedelta(seconds=int(time_left)))

    def __update_amount(self, new_amount):
        percent_done = int(round((new_amount / 100.0) * 100.0))
        all_full = self.width - 2
        num_hashes = int(round((percent_done / 100.0) * all_full))
        self.prog_bar = '[' + self.fill_char * num_hashes + ' ' * (all_full - num_hashes) + ']'
        pct_place = (len(self.prog_bar) // 2) - len(str(percent_done))
        pct_string = '%d%%' % percent_done
        self.prog_bar = self.prog_bar[0:pct_place] + \
            (pct_string + self.prog_bar[pct_place + len(pct_string):])

    def __str__(self):
        return str(self.prog_bar)
    



def time_range(start, end, step_or_num, use_seconds=False):
    """ 
    Takes a start and end (h, m) or (h, m, s) tuple 
    and outputs a range in between, with times formatted as strings. 
    
    the step_or_num can be datetime.timedelta or int,
    if it's an int, it's treated as number of steps to make from start to end, 
    i.e. number of elements in the array
    """
    from datetime import datetime, timedelta
    if not start or not end or step_or_num is None:
        raise TypeError("No start or end or step given")
        
    if not isinstance(start, tuple) or not isinstance(end, tuple):
        raise TypeError("Start and end must be tuples of ints")
    
    if len(start) == 2:
        st = datetime(2000,1,1, start[0], start[1])
    elif len(start) == 3:
        st = datetime(2000,1,1, start[0], start[1], start[2])
    else:
        raise TypeError("Start needs to be a tuple of 2 or 3 ints")
    
    if len(end) == 2:
        en = datetime(2000,1,1, end[0], end[1])
    elif len(end) == 3:
        en = datetime(2000,1,1, end[0], end[1], end[2])
    else:
        raise TypeError("End needs to be a tuple of 2 or 3 ints")
        
    if isinstance(step_or_num, int):
        if step_or_num > 1:
            ns = (en-st).seconds
            stp = timedelta(seconds= int(ns / float(step_or_num-1)))
        else:
            raise TypeError("Step must be > 1")
    elif isinstance(step_or_num, timedelta):
        stp = step_or_num
    else:
        raise TypeError("Step is the wrong type (int/timedelta allowed only)")
            
    res = []
    cur = st
    while cur < en:
        if not use_seconds:
            acur = datetime(cur.year, cur.month, cur.day, 
                           cur.hour, cur.minute, 0)
        else:
            acur = cur
        res.append(str(acur.time()))
        cur += stp
    if res[::-1][0] != str(en.time()):
        res.append(str(en.time()))
        
    if isinstance(step_or_num, int):
        if len(res) > step_or_num:
            res = res[0:step_or_num]
        
    return res


def split_strategy(s):
    buy, sell, close, *k = s.split('~')
    sl = ""
    try:
        sl = k[0]
    except:
        pass
    tp = ""
    try:
        tp = k[1]
    except:
        pass
    return buy, sell, close, sl, tp

def load_fx_ticks(filename):
    " loads ticks generated by the MT4 tick logger, returns DataFrame"
    pass
