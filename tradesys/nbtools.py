from .tstools import *
from ._tradesys import *

import os
import sys
import socket
if 'laptop' not in socket.gethostname():
    os.environ['THEANO_FLAGS'] = 'mode=FAST_RUN,device=cpu,floatX=float32'
else:
    os.environ['THEANO_FLAGS'] = 'mode=FAST_RUN,device=cuda,floatX=float32,nvcc.flags=-arch=sm_35'
sys.path.append("/root")
sys.path.append("/home/peter")
sys.path.append("/home/peter/Desktop")
sys.path.append("/home/peter/Desktop/projects")
sys.path.append("/home/peter/Desktop/work")
sys.path.append("/home/peter/code/projects")
sys.path.append("/home/peter/code/work")
sys.path.append("/home/peter/code/common")

from ipywidgets import interact, interactive, fixed, IntSlider, FloatSlider
from IPython.display import clear_output, display, HTML

import cv2
import os
import pickle as pkl

from sklearn import linear_model
from sklearn.decomposition import PCA
from sklearn.preprocessing import scale, robust_scale, normalize
from sklearn.neural_network import MLPClassifier
from sklearn.neighbors import KNeighborsClassifier
from sklearn.svm import SVC
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier, AdaBoostClassifier, ExtraTreesClassifier, GradientBoostingClassifier
from sklearn.naive_bayes import GaussianNB
from sklearn.linear_model import LogisticRegression
from sklearn.ensemble import AdaBoostClassifier
import sklearn as sk

import random as rnd
import numpy as np
from numpy import *
import matplotlib.pyplot as plt
import itertools as itr

from keras.models import Sequential
from keras.layers.core import RepeatVector, Reshape, Permute, Dense, Dropout, Activation, Flatten
from keras.layers.convolutional import Convolution2D, MaxPooling2D, AveragePooling2D
from keras.layers.convolutional import Convolution1D, MaxPooling1D, AveragePooling1D
from keras.layers.local import LocallyConnected1D
from keras.layers.recurrent import LSTM, GRU, SimpleRNN
from keras.optimizers import SGD, Adadelta, Adagrad, Adam, RMSprop, Nadam

import cymysql as sql
import pandas as pd
import datetime
import time

ib = """
(

((((Price('high') - Price('close')) /
      (Price('high') - Price('low') + F(0.000000001)) * F(100.0)) >=
     F(['intrabar_percent_min', 0.0])) and
    (((Price('high') - Price('close')) /
      (Price('high') - Price('low') + F(0.000000001)) * F(100.0)) <=
     F(['intrabar_percent_max', 100.0])))

)
"""

exfob = """(B(['exit_at_first_opposite_bar', false]) == B(true) and (((VALUE_OF_LH(PRICE('open', bars_back=1) > PRICE('close', bars_back=1), INSIDE_MARKET) == B(true))

and (PRICE('open') < PRICE('close')))

or

((VALUE_OF_LH(PRICE('open', bars_back=1) < PRICE('close', bars_back=1), INSIDE_MARKET) == B(true))

and (PRICE('open') > PRICE('close')))))"""

otpob = """(((B(['only_trade_at_prev_opposite_bar', false]) == B(true))

and (

((PRICE('close', bars_back=1) > PRICE('open', bars_back=1)) and (PRICE('close') < PRICE('open')))

or

((PRICE('close', bars_back=1) < PRICE('open', bars_back=1)) and (PRICE('close') > PRICE('open')))

) or (B(['only_trade_at_prev_opposite_bar', false]) == B(false))))
"""

otpsb = """(((B(['only_trade_at_prev_same_bar', false]) == B(true))

and (

((PRICE('close', bars_back=1) > PRICE('open', bars_back=1)) and (PRICE('close') > PRICE('open')))

or

((PRICE('close', bars_back=1) < PRICE('open', bars_back=1)) and (PRICE('close') < PRICE('open')))

) or (B(['only_trade_at_prev_same_bar', false]) == B(false))))
"""

import xml.etree.ElementTree as et

def parse_multicharts_xml(f):
    xml = et.parse(f).getroot()

    prices = xml.getchildren()[1]
    trades = xml.getchildren()[2]

    data = DataMatrix()

    bars = prices.getchildren()
    for b in bars:
        d, o, h, l, c, v = b.getchildren()
        o, h, l, c = [float(x.text) for x in [o, h, l, c]]
        v = int(v.text)
        ye, mo, da, ho, mi, se = [int(x.text) for x in d.getchildren()]
        data.add_row(DateTime(ye, mo, da, ho, mi, se), o, h, l, c, v)

    # parse the trades
    def chunks(l, n):
        """Yield successive n-sized chunks from l."""
        for i in range(0, len(l), n):
            yield l[i:i+n]

    tlist = []
    k = list(chunks(trades, 2))
    if len(k[-1]) != 2:
        del k[-1]

    for entry, exit in k:
        t = TradeInfo()

        # get the entry data
        date, typ, qty, price, sig = entry.getchildren()
        ye, mo, da, ho, mi, se = [int(x.text) for x in date.getchildren()]

        t.datetime_of_entry = DateTime(ye, mo, da, ho, mi ,se)
        t.price_at_entry = float(price.text)
        if typ.text == 'Buy':
            t.position = PositionType.LONG_POSITION
        else:
            t.position = PositionType.SHORT_POSITION

        # get the exit data
        date, typ, profit, qty, price, sig = exit.getchildren()
        ye, mo, da, ho, mi, se = [int(x.text) for x in date.getchildren()]

        t.datetime_of_exit = DateTime(ye, mo, da, ho, mi ,se)
        t.price_at_exit = float(price.text)
        t.profit = float(profit.text)

        tlist.append(t)


    return data, tlist

# this condition determines the points when the strategy enters (agnostic of direction)
def is_entry(buy, sell):
    entry = """(
    (({buy}) and (not ({sell})) and (not HAPPENED_BB({buy}, 1)))
    or
    (({sell}) and (not ({buy})) and (not HAPPENED_BB({sell}, 1)))
    or
    (HAPPENED_BB({buy}, 1) and ((({sell}) and (not ({buy})) and (not HAPPENED_BB({sell}, 1)))))
    or
    (HAPPENED_BB({sell}, 1) and ((({buy}) and (not ({sell})) and (not HAPPENED_BB({buy}, 1)))))
    )
    """.format(buy=buy, sell=sell)
    return entry

def ignore_first_trades(s):
    buy, sell, close, sl, tp = split_strategy(s)

    return """
            ((I(['ignore_first_trades', 0]) > I(0)) and (({buy}) and
                (TIMES_HAPPENED_SINCE_FH({entry}, NEW_DAY) >
                                                   (I(['ignore_first_trades', 0])-I(2)))))

            or

            ((I(['ignore_first_trades', 0]) == I(0)) and ({buy}))

            ~

            ((I(['ignore_first_trades', 0]) > I(0)) and (({sell}) and
                (TIMES_HAPPENED_SINCE_FH({entry}, NEW_DAY) >
                                                   (I(['ignore_first_trades', 0])-I(2)))))

            or

            ((I(['ignore_first_trades', 0]) == I(0)) and ({sell}))

            ~
            {close}
            ~
            {sl}
            ~
            {tp}
            ~
            """.format(buy=buy, sell=sell, close=close, sl=sl, tp=tp, entry=entry)

def get_partitioned_counts(cl, rr, rt):
    " data, steps_forward, partition_num"
    cl = cl[rr:] - cl[0:-rr]
    cl = hstack([[0]*rr, cl])
    pdown = len(cl[cl < -rt])
    pup = len(cl[cl > rt])
    pmid = len(cl[(cl <= rt) & (cl >= -rt)])
    return pup, pmid, pdown

def partition_data_num(cl, predict_ahead, granularity=1000):
    a=[]
    b=[]
    c=[]
    ts = np.linspace(0.0, 0.1, granularity)
    for x in ts:
        au, am, ad = get_partitioned_counts(cl, predict_ahead, x)
        a.append(au)
        b.append(am)
        c.append(ad)
    a, b, c = array(a), array(b), array(c)
    rsl = ts[np.argmin(np.abs(a-b) + np.abs(a-c) + np.abs(b-c))]
    return max([0.01, rsl])

# remove pre/aftermarket data
def fix_weekends(d):
    indices = []
    for i in range(0, len(d), 390):
        # look for places where the price is all equal
        k = d.iloc[i:i+390]['close']
        all_eq = True
        a = k[0]
        for j in k:
            if j != a:
                all_eq = False
                break
        if all_eq:
            indices.append(d.iloc[i:i+390].index)
    if indices:
        mindices = indices[0]
        for m in indices[1:]:
            mindices = mindices | m
        di = d.index.difference(mindices)
        nd = d.ix[di]
        return nd
    else:
        return d

def fill_incomplete_datamatrix(data, freq='1 min', between_time=('09:31', '16:00'), fill_method='pad'):
    df = dmat2df(data)
    didx = pd.date_range(start=df.index[0].to_pydatetime(), end=df.index[-1].to_pydatetime(), freq='1 min')
    if between_time:
        didx = didx[didx.indexer_between_time(*between_time)]
    df = df.reindex(index=didx).fillna(method=fill_method)
    df = pd.DataFrame(index = df.index, columns = ['open', 'high', 'low', 'close', 'volume'], data = df)
    df = fix_weekends(df)
    k = df2dmat(df)
    k.name = data.name
    return k


