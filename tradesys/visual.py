import sys

#from ipykernel.tests.utils import wait_for_idle

sys.path.append("/root")
sys.path.append("/home/peter")
sys.path.append("/home/peter/Desktop")
sys.path.append("/home/peter/Desktop/projects")
sys.path.append("/home/peter/Desktop/work")
sys.path.append("/home/peter/code/projects")
sys.path.append("/home/peter/code/work")
sys.path.append("/home/peter/code/common")

import numpy as np
import matplotlib as mpl
import socket
if socket.gethostname() != 'desktop' and socket.gethostname() != 'laptop' and socket.gethostname() != 'oldlaptop':
    mpl.use('Agg')
import matplotlib.pyplot as plt
import pandas as pd
import collections

from _tradesys import DataMatrix, MultiDataMatrix, get_stree_log, get_multi_stree_log, get_stree_max_bars_back
from _tradesys import BacktestStats, DateTime, PositionType
from tstools import df2dmat

from datetime import datetime, timedelta
import matplotlib.dates as mdates
from mpl_finance import candlestick_ohlc
from time import strptime
from matplotlib.dates import AutoDateLocator, AutoDateFormatter
from matplotlib.collections import LineCollection, PolyCollection

def dt2datetime(dt):
    return datetime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second)

def datetime2dt(dt):
    return DateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second)

def dt2text(dt):
    return str(dt)

def text2dt(text):
    t = strptime(text, "%Y-%m-%d %H:%M:%S")
    return DateTime(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)

def text2datetime(text):
    t = strptime(text, "%Y-%m-%d %H:%M:%S")
    return datetime(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)

def datetime2text(dt):
    return str(datetime2dt(dt))

def idx2datetime(idx, mdl):
    return mdl[idx]

def datetime2idx(dt, mdl):
    return mdl.index(dt)

def idx2dt(idx, mdl):
    return datetime2dt(mdl[idx])

def dt2idx(dt, mdl):
    return mdl.index(dt2datetime(dt))


def plot_duration_info(data, info):
    """ Plot the data and trades as horizontal lines 
        X axis is time (bar idx), Y axis is price
    """
    
    if not isinstance(info, collections.Iterable):
        
        # one info
        plt.close()
        f, ax = plt.subplots(figsize=(18,3))
        # draw data
        plt.sca(ax)
        plt.plot(data.close, alpha=0.3);
        xs = []
        ys = []
        for t in info.list_of_trades:
            xs.append([t.bar_index_at_entry, t.bar_index_at_exit])
            ys.append([t.price_at_entry, t.price_at_exit])
        # draw series of lines
        for xl, yl in zip(xs, ys):
            plt.plot(xl, yl, 'k-', lw=2)

        m = min(data.close)
        for xl, yl in zip(xs, ys):
            plt.plot(xl, [m, m], 'r-', lw=3)
            
    else: 
        
        # many infos
        infos = info
        plt.close()
        f, axes = plt.subplots(len(infos), 1, figsize=(18,3*len(infos)))

        for ax, r in zip(axes, infos):
            
            # draw data
            plt.sca(ax)
            plt.plot(data.close, alpha=0.3);
            xs = []
            ys = []
            for t in r.list_of_trades:
                xs.append([t.bar_index_at_entry, t.bar_index_at_exit])
                ys.append([t.price_at_entry, t.price_at_exit])
                
            # draw series of lines
            for xl, yl in zip(xs, ys):
                plt.plot(xl, yl, 'k-', lw=2)

            m = min(data.close)
            for xl, yl in zip(xs, ys):
                plt.plot(xl, [m, m], 'r-', lw=2)
                
        
def plot_info(info):
    " Displays the backtest stats "
    """
    if len(info.symbols) > 1:
        # do multi-plot
        times = {}
        for l in info.list_of_trades:
            if dt2datetime(l.datetime_of_exit) not in times.keys():
                times[dt2datetime(l.datetime_of_exit)] = l.profit
            else:
                times[dt2datetime(l.datetime_of_exit)] += l.profit
    else:
        # standard plot
        times = {}
        for l in info.list_of_trades:
            if dt2datetime(l.datetime_of_exit) not in times.keys():
                times[dt2datetime(l.datetime_of_exit)] = l.profit
    eq = []
    ts = []
    td = 0
    for d in sorted(times.keys()):
        td += times[d]
        ts.append(d)
        eq.append(td)
    plt.plot(ts, eq);
    labels = plt.gca().get_xticklabels()
    plt.setp(labels, rotation=30, fontsize=10);
    """
    # equity chart
    cm = []
    c = 0
    for t in info.list_of_trades:
        c += t.profit
        cm.append(c)
    plt.plot(cm)


def candlestick2_ohlc(ax, opens, highs, lows, closes, width=4,
                      colorup='k', colordown='r',
                      alpha=0.75,
                      ):
    """Represent the open, close as a bar line and high low range as a
    vertical line.
    NOTE: this code assumes if any value open, low, high, close is
    missing they all are missing
    Parameters
    ----------
    ax : `Axes`
        an Axes instance to plot to
    opens : sequence
        sequence of opening values
    highs : sequence
        sequence of high values
    lows : sequence
        sequence of low values
    closes : sequence
        sequence of closing values
    ticksize : int
        size of open and close ticks in points
    colorup : color
        the color of the lines where close >= open
    colordown : color
        the color of the lines where close <  open
    alpha : float
        bar transparency
    Returns
    -------
    ret : tuple
        (lineCollection, barCollection)
    """

    delta = width / 2.
    barVerts = [((i - delta, open),
                 (i - delta, close),
                 (i + delta, close),
                 (i + delta, open))
                for i, open, close in zip(range(len(opens)), opens, closes)
                if open != -1 and close != -1]

    rangeSegments = [((i, low), (i, high))
                     for i, low, high in zip(range(len(lows)), lows, highs)
                     if low != -1]

    colorup = (1.0,0.6,0,alpha)
    colordown = (0.0,0.6,1.0,alpha)
    colord = {True: colorup, False: colordown}
    colors = [colord[open < close]
              for open, close in zip(opens, closes)
              if open != -1 and close != -1]

    useAA = 0,  # use tuple here
    lw = 0.5,   # and here
    rangeCollection = LineCollection(rangeSegments,
                                     colors=colors,
                                     linewidths=lw,
                                     antialiaseds=useAA,
                                     )

    barCollection = PolyCollection(barVerts,
                                   facecolors=colors,
                                   edgecolors=colors,
                                   antialiaseds=useAA,
                                   linewidths=lw,
                                   )

    minx, maxx = 0, len(rangeSegments)
    miny = min([low for low in lows if low != -1])
    maxy = max([high for high in highs if high != -1])

    corners = (minx, miny), (maxx, maxy)
    ax.update_datalim(corners)
    ax.autoscale_view()

    # add these last
    ax.add_collection(barCollection)
    ax.add_collection(rangeCollection)
    return rangeCollection, barCollection


def plot_group(ax, group, dt, dl, candles=False, alpha=1.0, abstime=False):
    """ Plots a group of signals. If candles is True, the first 4 will be displayed as OHLC candles """
    plt.sca(ax)

    if not abstime:
        dt = list(range(len(dt)))

    def dataplot(ax, dt, d, s):
        s = [x for x in s.split() if x]
        s = ' '.join(s)
        if len(s) > 32:
            s = s[0:32] # limit too long labels to 128 characters
        if isinstance(d[0], bool):
            da = np.array(d)
            dd = np.array(dt)
            mtx = np.where(da == True)[0]
            if len(mtx)>0:
                line, = ax.plot(dd[mtx], da[mtx], 'ro', markersize=10, label=s, alpha=alpha)
            mtx = np.where(da == False)[0]
            if len(mtx)>0:
                line, = ax.plot(dd[mtx], da[mtx], 'bo', markersize=10, label=s, alpha=alpha)
        else:
            line, = ax.plot(dt, d, label=s, alpha=alpha)
        return (line,)

    if candles and len(group) >= 4:
        if abstime:
            # to calculate the correct line width, need to infer the shortest time period in the series
            # and convert that to a fraction from a day
            dldiffs = []
            for i in range(1, len(dl)):
                dldiffs.append( dl[i] - dl[i-1] )
            md = min(dldiffs)
            divisor = timedelta(days=1) / md
            candlestick_ohlc(ax=ax, quotes=zip(dt, group[0][1],
                                                   group[1][1],
                                                   group[2][1],
                                                   group[3][1]), width=1/divisor, alpha=alpha);
        else:
            candlestick2_ohlc(ax=ax, opens=group[0][1],
                                     highs=group[1][1],
                                     lows=group[2][1],
                                     closes=group[3][1],
                              width=1, alpha=alpha)
        # the rest of the data if any
        if len(group) > 4:
            lines = []
            for s, d in group[4:]:
                line, = dataplot(ax, dt, d, s)
                line.set_alpha(1.0)
                lines.append(line)
            plt.legend(handles=lines)
    else:
        lines = []
        for s, d in group:
            line, = dataplot(ax, dt, d, s)
            line.set_alpha(1.0)
            lines.append(line)
        plt.legend(handles=lines)

    if abstime:
        adl = AutoDateLocator(minticks=1)
        adf = AutoDateFormatter(adl)

        ax.xaxis.set_major_locator(adl)
        ax.xaxis.set_major_formatter(adf)


# data exploration functions
def show_data(dmatrix, focus=0, samey=False, signals=[], defsignals=[], candles=[], axes_arr=None):
    matrices = MultiDataMatrix()
    if isinstance(dmatrix, DataMatrix):
        matrices.symbols.append(dmatrix)
    elif isinstance(dmatrix, MultiDataMatrix):
        for e in dmatrix.symbols:
            matrices.symbols.append(e)
    elif isinstance(dmatrix, pd.DataFrame):
        matrices.symbols.append(df2dmat(dmatrix))

    # display the default group
    leng = len(signals)+1
    if candles:
        leng += 1
    if not axes_arr:
        f, axarr = plt.subplots(leng, figsize=(14, 4 * leng), sharex=True)
    else:
        if len(axes_arr) != leng:
            raise ValueError("Need as many axes as there are charts to display (%d)" % leng)
        axarr = axes_arr

    # process the times
    mdl = []
    for dmtx in matrices.symbols:
        mdl += dmtx.get_dt_list()
    mdl = sorted(list(set(mdl)))
    mdt = [mdates.date2num(x) for x in mdl]

    for idx, dmtx in enumerate(matrices.symbols):

        alpha = 1.0
        if isinstance(dmatrix, MultiDataMatrix):
            alpha = 0.01
        if focus == idx:
            alpha = 1.0


        # the default signal group that always contains Close
        defgroup = [('Open', get_multi_stree_log("PRICE('open', data=%d)" % idx, matrices)[idx]),
                    ('High', get_multi_stree_log("PRICE('high', data=%d)" % idx, matrices)[idx]),
                    ('Low',  get_multi_stree_log("PRICE('low', data=%d)" % idx, matrices)[idx]),
                    ('Close', get_multi_stree_log("PRICE('close', data=%d)" % idx, matrices)[idx])]
        if defsignals:
            for s in defsignals:
                k = get_multi_stree_log(s, matrices)[idx]
                defgroup.append((s, k))

        candlegroup = []
        if candles and len(candles) >= 4:
            for s in candles:
                k = get_multi_stree_log(s, matrices)[idx]
                candlegroup.append((s, k))

        # the other signals
        groups = []
        if signals:
            for s in signals:
                if isinstance(s, str):
                    k = get_multi_stree_log(s, matrices)[idx]
                    groups.append([(s, k)])
                elif isinstance(s, list) or isinstance(s, tuple):
                    g = []
                    for el in s:
                        k = get_multi_stree_log(el, matrices)[idx]
                        g.append((el, k))
                    groups.append(g)
                else:
                    raise ValueError('Signal must be a string or an iterable of strings')
        if idx == 0 or samey:
            if leng == 1:
                plot_group(axarr, defgroup, mdt, mdl, candles=True, alpha=alpha)
            else:
                plot_group(axarr[0], defgroup, mdt, mdl, candles=True, alpha=alpha)
                if candlegroup:
                    plot_group(axarr[1], candlegroup, mdt, mdl, candles=True, alpha=alpha)
        else:
            if leng == 1:
                plot_group(axarr.twinx(), defgroup, mdt, mdl, candles=True, alpha=alpha)
            else:
                plot_group(axarr[0].twinx(), defgroup, mdt, mdl, candles=True, alpha=alpha)
                if candlegroup:
                    plot_group(axarr[1].twinx(), candlegroup, mdt, mdl, candles=True, alpha=alpha)


        # display each group in its own subplot
        for i,g in enumerate(groups):
            if candlegroup: i+=1
            if idx == 0 or samey:
                plot_group(axarr[i+1], g, mdt, mdl, candles=False, alpha=alpha)
            else:
                plot_group(axarr[i+1].twinx(), g, mdt, mdl, candles=False, alpha=alpha)

            if i < len(groups)-1:
                plt.tick_params(
                                axis='x',          # changes apply to the x-axis
                                which='both',      # both major and minor ticks are affected
                                bottom='off',      # ticks along the bottom edge are off
                                top='off',         # ticks along the top edge are off
                                labelbottom='off') # labels along the bottom edge are off
            else:
                plt.tick_params(
                                axis='x',          # changes apply to the x-axis
                                which='both',      # both major and minor ticks are affected
                                bottom='off',      # ticks along the bottom edge are off
                                top='on',         # ticks along the top edge are off
                                labelbottom='on') # labels along the bottom edge are off

                # if not axarr:
                #    plt.show()

def get_trades_in_idx_range(list_of_trades, idx, window_width):
    r = []
    for t in list_of_trades:
        if (idx <= t.bar_index_at_entry <= (idx+window_width)) or (idx <= t.bar_index_at_exit <= (idx+window_width)):
            r.append(t)
    r = list(set(r))
    return r


def get_limit_orders_in_idx_range(limit_orders, idx, window_width):
    r = []
    for t in limit_orders:
        if (idx <= t.bar_idx_at_creation <= (idx+window_width)) or (idx <= t.bar_idx_at_expiry <= (idx+window_width)):
            r.append(t)
    r = list(set(r))
    return r


def explore_data(dmatrix, signals=[], defsignals=[], candles=[], trades=[], logs={}, limit_orders=None, legend_loc='upper right'):
    from ipywidgets import interactive
    from ipywidgets.widgets import Button, Text, VBox, HBox, IntSlider, FloatSlider, ToggleButton
    from IPython.display import display
    float_slider = FloatSlider
    int_slider = IntSlider

    matrices = MultiDataMatrix()
    if isinstance(dmatrix, DataMatrix):
        matrices.symbols.append(dmatrix)
    elif isinstance(dmatrix, MultiDataMatrix):
        for e in dmatrix.symbols:
            matrices.symbols.append(e)
    elif isinstance(dmatrix, pd.DataFrame):
        matrices.symbols.append(df2dmat(dmatrix))

    b_defgroups = []
    b_candles = []
    b_groups = []

    # process the times
    mdl = []
    for dmtx in matrices.symbols:
        mdl += dmtx.get_dt_list()
    mdl = sorted(list(set(mdl)))
    mdt = [mdates.date2num(x) for x in mdl]

    for idx, dmtx in enumerate(matrices.symbols):

        # the default signal group that always contains Close
        defgroup = [('Open', get_multi_stree_log("PRICE('open', data=%d)" % idx, matrices)[idx]),
                    ('High', get_multi_stree_log("PRICE('high', data=%d)" % idx, matrices)[idx]),
                    ('Low',  get_multi_stree_log("PRICE('low', data=%d)"  % idx, matrices)[idx]),
                    ('Close', get_multi_stree_log("PRICE('close', data=%d)" % idx, matrices)[idx])]
        if defsignals:
            for s in defsignals:
                k = get_multi_stree_log(s, matrices)[idx]
                defgroup.append((s, k))

        candlegroup = []
        if candles and len(candles) >= 4:
            for s in candles:
                k = get_multi_stree_log(s, matrices)[idx]
                candlegroup.append((s, k))

        # the other signals
        groups = []
        # add the logs to the regular group/signals
        if logs:
            for k,v in logs.items():
                groups.append([(k,v)])
        if signals:
            for s in signals:
                if isinstance(s, str):
                    k = get_multi_stree_log(s, matrices)[idx]
                    groups.append([(s, k)])
                elif isinstance(s, list) or isinstance(s, tuple):
                    g = []
                    for el in s:
                        k = get_multi_stree_log(el, matrices)[idx]
                        g.append((el, k))
                    groups.append(g)
                else:
                    raise ValueError('Signal must be a string or an iterable of strings')


        b_defgroups.append(defgroup)
        b_candles.append(candlegroup)
        b_groups.append(groups)

    # for the plots
    def dis(**kw):
        # display the default group
        leng = len(signals)+1
        if candles:
            leng += 1
        if logs:
            leng += len(list(logs.keys()))
        f, axarr = plt.subplots(leng, figsize=(14, 4 * leng), sharex=True)

        for iidx, dmtx in enumerate(matrices.symbols):

            alpha = 1.0
            if isinstance(dmatrix, MultiDataMatrix):
                alpha = 0.1
            if kw['focus'] == iidx:
                alpha = 1.0

            defgroup = b_defgroups[iidx]
            candlegroup = b_candles[iidx]
            groups = b_groups[iidx]

            st, en = kw['idx'], kw['idx'] + kw['window_width']

            dfglen = len(defgroup[0][1])

            if st < 0: st = 0
            if st > (dfglen)-1: st = (dfglen)-1
            if en > (dfglen)-1: en = (dfglen)-1

            if kw['timetg']:
                abstime=True
            else:
                abstime=False

            if ((en-st) > 3):
                ppp = [(x[0], x[1][kw['idx'] : kw['idx'] + kw['window_width']]) for x in defgroup]
                if idx != 0 and not kw['tg']:
                    if leng == 1:
                        axx = axarr.twinx()
                    else:
                        axx = axarr[0].twinx()
                else:
                    if leng == 1:
                        axx = axarr
                    else:
                        axx = axarr[0]
                plot_group(axx, ppp, mdt[kw['idx'] : kw['idx'] + kw['window_width']],
                                           mdl[kw['idx'] : kw['idx'] + kw['window_width']],
                           candles=True, alpha=alpha, abstime=abstime)
                # annotate trades if any
                if trades:
                    ts = get_trades_in_idx_range(trades, kw['idx'], kw['window_width'])
                    for t in ts:
                        if t.symbol == dmtx.name:
                            annotate_entry(t, axx, kw['idx'], kw['window_width'], alpha, abstime)
                        #if kw['idx'] <= t.bar_index_at_exit <= (kw['idx']+kw['window_width']):
                        #    annotate_exit(t, axx, kw['idx'], kw['window_width'], False)

                # annotate limit orders if any
                if limit_orders:
                    ts = get_limit_orders_in_idx_range(limit_orders, kw['idx'], kw['window_width'])
                    for t in ts:
                        if t.symbol == dmtx.name:
                            annotate_limit_entry(t, axx, kw['idx'], kw['window_width'], alpha, abstime)

                if candlegroup:
                    if idx != 0 and not kw['tg']:
                        axx = axarr[1].twinx()
                    else:
                        axx = axarr[1]

                    ppp = [(x[0], x[1][kw['idx'] : kw['idx'] + kw['window_width']]) for x in candlegroup]
                    plot_group(axx, ppp, mdt[kw['idx'] : kw['idx'] + kw['window_width']],
                                              mdl[kw['idx'] : kw['idx'] + kw['window_width']],
                               candles=True, alpha=alpha, abstime=abstime)

                # display each group in its own subplot
                for i,d in enumerate(groups):
                    if candlegroup: i+=1

                    ppp = ppp = [(x[0], x[1][kw['idx'] : kw['idx'] + kw['window_width']]) for x in d]

                    if idx != 0 and not kw['tg']:
                        axx = axarr[i+1].twinx()
                    else:
                        axx = axarr[i+1]

                    plot_group(axx, ppp, mdt[kw['idx'] : kw['idx'] + kw['window_width']],
                                                mdl[kw['idx'] : kw['idx'] + kw['window_width']],
                               candles=True, alpha=alpha, abstime=abstime)

                    if (candlegroup and ((i-1) < len(groups)-1)) or ((not candlegroup) and (i < len(groups)-1)):
                        plt.tick_params(
                                        axis='x',          # changes apply to the x-axis
                                        which='both',      # both major and minor ticks are affected
                                        bottom='off',      # ticks along the bottom edge are off
                                        top='off',         # ticks along the top edge are off
                                        labelbottom='off') # labels along the bottom edge are off
                    else:
                        plt.tick_params(
                                        axis='x',          # changes apply to the x-axis
                                        which='both',      # both major and minor ticks are affected
                                        bottom='off',      # ticks along the bottom edge are off
                                        top='on',         # ticks along the top edge are off
                                        labelbottom='on') # labels along the bottom edge are off
        #display(f)
        plt.show()

    ww = IntSlider(min=0, max=12000, step=5, value=300, sync=False)
    idx = IntSlider(min=0, max=len(dmatrix), step=1, value=0, sync=False)
    if trades:
        trade = IntSlider(min=0, max=len(trades)-1, step=1, value=0, sync=False)
    focus = IntSlider(min=0, max=len(matrices.symbols)-1, step=1, value=0, sync=False)
    dt = Text(value=dt2text(mdl[0]), description="Date/time")
    tg = ToggleButton(description='abs')
    timetg = ToggleButton(description='abstime')

    b_nextday = Button(description="+1 day", value=0)
    def clb_n(x):
        # get current date
        d = text2datetime(dt.value)
        d += timedelta(days=1)
        dt.value = datetime2text(d)
    b_nextday.on_click(clb_n)

    b_prevday = Button(description="-1 day", value=0)
    def clb_p(x):
        # get current date
        d = text2datetime(dt.value)
        d -= timedelta(days=1)
        dt.value = datetime2text(d)
    b_prevday.on_click(clb_p)

    b_nextday3 = Button(description="+3 days", value=0)
    def clb_n3(x):
        # get current date
        d = text2datetime(dt.value)
        d += timedelta(days=3)
        dt.value = datetime2text(d)
    b_nextday3.on_click(clb_n3)

    b_prevday3 = Button(description="-3 days", value=0)
    def clb_p3(x):
        # get current date
        d = text2datetime(dt.value)
        d -= timedelta(days=3)
        dt.value = datetime2text(d)
    b_prevday3.on_click(clb_p3)

    def dtlb(x):
        try:
            d = text2datetime(dt.value)
        except:
            return
        dists = [abs(x-d) for x in mdl]
        mdidx = dists.index(min(dists))
        idx.value = mdidx
        dt.value = datetime2text(mdl[mdidx])
    dt.observe(dtlb)

    def idxlb(x):
        try:
            dt.value = dt2text(idx2dt(idx.value, mdl))
        except:
            pass
    idx.observe(idxlb)

    if trades:
        def trlb(x):
            try:
                ts = [x for x in trades if x.symbol == matrices.symbols[focus.value].name]
                trade.max = len(ts)
                idx.value = ts[trade.value].bar_index_at_entry - ww.value//3
            except:
                pass
        trade.observe(trlb)

        i = interactive( dis,

                     window_width = ww,
                     idx = idx,
                     trade = trade,
                     focus = focus,

                     #dt = dt,

                     #b_nextday = b_nextday,
                     #b_prevday = b_prevday,

                     #b_nextday3 = b_nextday3,
                     #b_prevday3 = b_prevday3,

                     tg = tg,
                     timetg = timetg,

                     )

        #bb1 = VBox([idx, trade, focus, ww])
    else:
        i = interactive( dis,

                         window_width = ww,
                         idx = idx,
                         focus = focus,

                         #dt = dt,

                         #b_nextday = b_nextday,
                         #b_prevday = b_prevday,

                         #b_nextday3 = b_nextday3,
                         #b_prevday3 = b_prevday3,

                         tg = tg,
                         timetg = timetg,

                         )

        #bb1 = VBox([idx, focus, ww])

    #bb2_1 = HBox([b_prevday, b_nextday])
    #bb2_2 = HBox([b_prevday3, b_nextday3])
    #bb2 = VBox([bb2_1, bb2_2])

    #hbox = HBox([bb1, bb2])
    #box = VBox([hbox, dt, tg, timetg])

    #display(box)
    idx.set_trait("value", len(dmatrix)//2)

    return i


def annotate_entry_fancy(trade, ax, idx, window_width, alpha=1.0):
    plt.sca(ax)
    
    entry_anno_str = ""
    anno_fc = (0,0,0)
    profit_fc = (0,0,0)

    if trade.position == 1: # Long
        entry_anno_str = ("Buy " + str(trade.quantity) + '\n' + #" shares " + "\n" +
                          "on " + str(trade.datetime_of_entry).replace(' ', '\n     ') + "\n" +  
                          "at $" + 
                    str(trade.price_at_entry) )
        anno_fc = (0.9, 0.65, 0.65, )
    if trade.position == -1: # Short
        entry_anno_str = ("Sell " + str(trade.quantity) + '\n' + #" shares " + "\n" +
                          "on " + str(trade.datetime_of_entry).replace(' ', '\n     ') + "\n" +  
                          "at $" + 
                    str(trade.price_at_entry) )
        anno_fc = (0, 0.65, 0.7, )

    if trade.profit > 0:
        profit_fc = (1, 0, 1)
    elif trade.profit < 0:
        profit_fc = (0.2, 0, 0.8)
    elif trade.profit == 0:
        profit_fc = (0.3, 0.3, 0.3)
    else:
        raise ValueError

    ax.annotate(entry_anno_str,
                xy=(trade.bar_index_at_entry-idx, trade.price_at_entry),
                xycoords='data',
                xytext=(0.2, 0.95),
                textcoords='axes fraction',
                arrowprops=dict(arrowstyle="fancy, head_length=1.5, head_width=0.75",
                                connectionstyle="arc3,rad=0.15", 
                                fc=anno_fc, ec="k", # colors
                                alpha=alpha
                                ),
                bbox=dict(boxstyle="square", fc=anno_fc, alpha=alpha),
                alpha=alpha,
                )
    
    # if the exit is also present, annotate it as well, but make it very transparent
    if (trade.bar_index_at_exit < (idx + window_width)):
        ax.annotate("Exit",
                    xy= (trade.bar_index_at_exit-idx, trade.price_at_exit),
                    xycoords='data',
                    xytext=(0.55,0.06),
                    textcoords='axes fraction',
                    arrowprops=dict(arrowstyle="fancy, head_length=1.5, head_width=0.75",
                                    connectionstyle="arc3,rad=0.15", 
                                    fc=anno_fc, ec="k", alpha=alpha*0.20, # colors
                                    ),
                    bbox=dict(boxstyle="square", fc=anno_fc, alpha=alpha*0.20),
                    alpha=alpha*0.5
                    )

def annotate_exit_fancy(trade, ax, idx, window_width, alpha=1.0):
    exit_anno_str = ""
    anno_fc = (0,0,0)
    profit_fc = (0,0,0)

    if trade.position == 1: # Long
        exit_anno_str = ("Close long\n" + 
                         "on " + str(trade.datetime_of_exit).replace(' ', '\n     ') + "\n" + 
                         "at $" + 
                    str(trade.price_at_exit) )
        anno_fc = (0.9, 0.65, 0.65)
    if trade.position == -1: # Short
        exit_anno_str = ("Close short\n" + 
                         "on " + str(trade.datetime_of_exit).replace(' ', '\n     ') + "\n" + 
                         "at $" + 
                    str(trade.price_at_exit) )
        anno_fc = (0, 0.65, 0.7)

    ax.annotate(exit_anno_str,
                xy=(trade.bar_index_at_exit - idx, trade.price_at_exit),
                xycoords='data',
                xytext=(0.8, 0.05), 
                textcoords='axes fraction',
                arrowprops=dict(arrowstyle="fancy, head_length=1.5, head_width=0.75",
                                connectionstyle="arc3,rad=0.15", 
                                fc=anno_fc, ec="k", # colors
                                ),
                bbox=dict(boxstyle="square", fc=anno_fc),
                alpha=alpha,
                )

    # if the entry is also present, annotate it as well, but make it very transparent
    if (trade.bar_index_at_entry > (trade.bar_index_at_exit - window_width)):
        ax.annotate("Entry",
                    xy=(trade.bar_index_at_entry - idx, trade.price_at_entry),
                    xycoords='data',
                    xytext=(0.25,0.91),
                    textcoords='axes fraction',
                    arrowprops=dict(arrowstyle="fancy, head_length=1.5, head_width=0.75",
                                    connectionstyle="arc3,rad=0.15", 
                                    fc=anno_fc, ec="k", alpha=alpha*0.20, # colors
                                    ),
                    bbox=dict(boxstyle="square", fc=anno_fc, alpha=alpha*0.20),
                    alpha=alpha*0.5
                    )

def annotate_profit_fancy(ax, trade):
    if trade.profit > 0:
        profit_fc = (1, 0, 1)
    elif trade.profit < 0:
        profit_fc = (0.2, 0, 0.8)
    elif trade.profit == 0:
        profit_fc = (0.3, 0.3, 0.3)
    else:
        raise ValueError
    bbox_props = dict(boxstyle="round", fc=profit_fc, ec="0.5", alpha=0.3)
    if trade.profit > 0:
        ax.text(-0.15, 0.0,
                "$%3.2f" % trade.profit,
                size=12, color='k',
                bbox=bbox_props, transform=ax.transAxes)
    elif trade.profit == 0:
        ax.text(-0.15, 0.0,
                "$%3.2f" % trade.profit,
                size=12, color='k',
                bbox=bbox_props, transform=ax.transAxes)
    elif trade.profit < 0:
        ax.text(-0.15, 0.0,
                "-$%3.2f" % -trade.profit,
                size=12, color='k',
                bbox=bbox_props, transform=ax.transAxes)



def annotate_entry(trade, ax, idx, window_width, alpha, abstime):
    plt.sca(ax)
    if trade.profit < 0:
        cl = 'b--'
    elif trade.profit > 0:
        cl = 'r--'
    else:
        cl = 'g--'

    startx = trade.bar_index_at_entry-idx
    endx = trade.bar_index_at_exit-idx
    starty = trade.price_at_entry
    endy = trade.price_at_exit

    if startx < 0:
        startx = 0
    if endx > (window_width):
        endx = window_width

    if abstime:
        startx = dt2datetime(trade.datetime_of_entry)
        endx = dt2datetime(trade.datetime_of_exit)

    ax.plot([startx, endx],
            [starty, endy], cl, lw=3, alpha=alpha)

    if not ((trade.bar_index_at_entry-idx) < 0):# or ((trade.bar_index_at_exit-idx) > (window_width)):
        if trade.position == PositionType.LONG_POSITION:
            cl = (1.0, 0.7, 0.0)
            ax.text(startx, starty, "B", fontsize=9, verticalalignment='center', horizontalalignment='center', style='oblique', alpha=alpha)
        else:
            cl = (0.7, 1.0, 0.0)
            ax.text(startx, starty, "S", fontsize=9, verticalalignment='center', horizontalalignment='center', style='oblique', alpha=alpha)
        ax.plot(startx, starty, marker='o', color=cl, markersize=12, alpha=alpha)


def annotate_exit(trade, ax, idx, window_width, alpha):
    pass

def annotate_limit_entry(limit_order, ax, idx, window_width, alpha, abstime):
    plt.sca(ax)

    startx = limit_order.bar_idx_at_creation - idx
    endx = limit_order.bar_idx_at_expiry - idx
    starty = limit_order.price_for_entry
    endy = limit_order.price_for_entry

    if startx < 0:
        startx = 0
    if endx > (window_width):
        endx = window_width

    if abstime:
        startx = dt2datetime(limit_order.datetime_of_creation)
        endx = dt2datetime(limit_order.datetime_of_expiry)

    if limit_order.position == PositionType.LONG_POSITION:
        cl = (0.0, 0.9, 0.7)#'b-'
    else:
        cl = (0.0, 0.7, 0.9)#'r-'

    alpha *= 0.75
    if not limit_order.did_enter:
        alpha *= 0.2

    ax.plot([startx, endx],
            [starty, endy], color=cl, lw=1, alpha=alpha)

    if not ((limit_order.bar_idx_at_creation-idx) < 0):# or ((trade.bar_index_at_exit-idx) > (window_width)):
        if limit_order.position == PositionType.LONG_POSITION:
            cl = (0.0, 0.7, 0.7)
            ax.text(startx, starty, "LB", fontsize=8, verticalalignment='center', horizontalalignment='center', style='oblique', alpha=alpha)
        else:
            cl = (0.0, 0.7, 0.7)
            ax.text(startx, starty, "LS", fontsize=8, verticalalignment='center', horizontalalignment='center', style='oblique', alpha=alpha)
        ax.plot(startx, starty, marker='o', color=cl, markersize=15, alpha=alpha)



def get_mbb(signals, defsignals, candles):
    mbb = 0
    if defsignals:
        mbb = max([mbb] + [get_stree_max_bars_back(x) for x in defsignals])
    if candles:
        mbb = max([mbb] + [get_stree_max_bars_back(x) for x in candles])
    if signals:
        for s in signals:
            if isinstance(s, list) or isinstance(s, tuple):
                mbb = max([mbb] + [get_stree_max_bars_back(x) for x in s])
            else:
                mbb = max([mbb] + [get_stree_max_bars_back(s)])
    return mbb

def display_event(dmtx, trade, signals=None, defsignals=None, candles=None, annotate_candles=False,
                  event_type=None, axes=[], window_width=16):

    # to adjust for max_bars_back
    ombb = get_mbb(signals, defsignals, candles)
    if ombb > window_width:
        window_width += ombb - window_width

    if event_type=='entry':
        event_idx = trade.bar_index_at_entry
    elif event_type=='exit':
        event_idx = trade.bar_index_at_exit
    event_st, event_en = event_idx - window_width, event_idx + window_width
        

    # clip the values 
    if event_st < 0: event_st = 0
    if event_st > len(dmtx)-1: event_st = len(dmtx)-1
    if event_en > len(dmtx)-1: event_en = len(dmtx)-1

    # for all left axes, display the entry
    ##################################
    # DRAW EVENT
    ##################################
    if (event_en - event_st) >= 4:
        mbb = ombb
        if isinstance(dmtx, DataMatrix):
            sliced_dmtx = dmtx.slice_idx(event_st-mbb, event_en)
        elif isinstance(dmtx, pd.DataFrame):
            sliced_dmtx = dmtx.iloc[event_st-mbb:event_en]
        else:
            raise TypeError("Unknown data type - must be a DataMatrix or a pandas DataFrame")
            
        if isinstance(sliced_dmtx, pd.DataFrame):
            dm = df2dmat(sliced_dmtx)
        else:
            dm = sliced_dmtx
            
        if event_st-mbb < 0:
            mbb = 0
            
        # process the times
        dl = dm.get_dt_list()
        dt = [mdates.date2num(x) for x in dl]
        dl = dl[mbb:]
        dt = dt[mbb:]

        # the default signal group that always contains Close
        defgroup = [('Open',  dm.open),
                    ('High',  dm.high),
                    ('Low',   dm.low),
                    ('Close', dm.close)]
        if defsignals:
            for s in defsignals:
                k = get_stree_log(s, dm)
                defgroup.append((s, k))

        candlegroup = []
        if candles and len(candles) >= 4:
            for s in candles:
                k = get_stree_log(s, dm)
                candlegroup.append((s, k))
            
        # the default groups 
        event_sl_defgroup = [(x[0], x[1][mbb:]) for x in defgroup]
        event_sl_candlegroup = [(x[0], x[1][mbb:]) for x in candlegroup]
        
        # the other signal groups
        groups = []
        if signals:
            for s in signals:
                if isinstance(s, str):
                    k = get_stree_log(s, dm)
                    groups.append([(s, k)])
                elif isinstance(s, list) or isinstance(s, tuple):
                    g = []
                    for el in s:
                        k = get_stree_log(el, dm)
                        g.append((el, k))
                    groups.append(g)
                else:
                    raise ValueError('Signal must be a string or an iterable of strings')
        event_sl_groups = [[(x[0], x[1][mbb:]) for x in y] for y in groups]
        
        if len(axes) > 1:
            
            # Mix defgroup on the first ax
            ax = axes[0]
            plt.sca(ax)
            plot_group(ax, event_sl_defgroup, dt, dl, candles=True)
            if event_type=='entry':
                annotate_entry_fancy(trade, ax, 0, len(dt))
            elif event_type=='exit':
                annotate_exit_fancy(trade, ax, 0, len(dt))
            
            if candlegroup:
                ax = axes[1]
                plt.sca(ax)
                plot_group(ax, event_sl_candlegroup, dt, dl, candles=True)
                if annotate_candles:
                    if event_type=='entry':
                        annotate_entry_fancy(trade, ax, 0, len(dt))
                    elif event_type=='exit':
                        annotate_exit_fancy(trade, ax, 0, len(dt))
            
            # display each group in its own subplot
            for i,g in enumerate(event_sl_groups):
                if candlegroup: i+=1
                ax = axes[i+1]
                plt.sca(ax)
                plot_group(ax, g, dt, dl, candles=False)

                if (candlegroup and (i-1) < len(event_sl_groups)-1) or ((not candlegroup) and i < len(event_sl_groups)-1):
                    plt.tick_params(
                                    axis='x',          # changes apply to the x-axis
                                    which='both',      # both major and minor ticks are affected
                                    bottom='off',      # ticks along the bottom edge are off
                                    top='off',         # ticks along the top edge are off
                                    labelbottom='off') # labels along the bottom edge are off
                else:
                    plt.tick_params(
                                    axis='x',          # changes apply to the x-axis
                                    which='both',      # both major and minor ticks are affected
                                    bottom='off',      # ticks along the bottom edge are off
                                    top='on',         # ticks along the top edge are off
                                    labelbottom='on') # labels along the bottom edge are off
        else:
            # Mix defgroup on the first ax
            ax = axes[0]
            plt.sca(ax)
            plot_group(ax, event_sl_defgroup, dt, dl, candles=True)
            if event_type=='entry':
                annotate_entry_fancy(trade, ax, 0, len(dt))
            elif event_type=='exit':
                annotate_exit_fancy(trade, ax, 0, len(dt))
            plt.tick_params(
                            axis='x',          # changes apply to the x-axis
                            which='both',      # both major and minor ticks are affected
                            bottom='off',      # ticks along the bottom edge are off
                            top='on',         # ticks along the top edge are off
                            labelbottom='on') # labels along the bottom edge are off



def display_trade(dmtx, trade, signals=None, defsignals=None, candles=None, annotate_candles=False,
                  **kwargs):
    """ Displays information about a TradeInfo with Matplotlib and pandas, 
    optionally plots the behavior of signals.
    """
    # get the window_width parameter straight
    try:
        window_width = int(kwargs['window_width'])
    except KeyError:
        try:
            window_width = int(kwargs['window_size'])
        except KeyError:
            window_width = 16

    # num_rows
    num_rows = 1
    if candles:
        num_rows += 1
    if signals:
        num_rows += len(signals)
        
    if num_rows > 1:
        fs = (16, 3.5 * num_rows)
    else:
        fs = figsize=(16, 4.5)

    # create subplots and axes
    if num_rows > 1:
        f, all_axes = plt.subplots(num_rows, 2, sharex=True, figsize=fs)
        axes_left, axes_right = all_axes[:, 0], all_axes[:, 1] # the diff
        [x.yaxis.tick_left() for x in axes_left]
        [x.yaxis.tick_right() for x in axes_right]
    else:
        f, all_axes = plt.subplots(num_rows, 2, sharex=True, figsize=fs)
        axes_left, axes_right = [all_axes[0]], [all_axes[1]] # the diff
        [x.yaxis.tick_left() for x in axes_left]
        [x.yaxis.tick_right() for x in axes_right]
        
    f.suptitle(trade.symbol, fontsize=16)
    
    # display the trade
    #------------------
    display_event(dmtx, trade, signals=signals, defsignals=defsignals, candles=candles, annotate_candles=annotate_candles,
                  event_type='entry', axes=axes_left, window_width=window_width)
    display_event(dmtx, trade, signals=signals, defsignals=defsignals, candles=candles, annotate_candles=annotate_candles,
                event_type='exit', axes=axes_right, window_width=window_width)

    
    # shift them a bit left (half window width)
    #entry_st = int(entry_st + window_width/2)
    #entry_en = int(entry_en + window_width/2)
    #exit_st = int(exit_st - window_width/2)
    #exit_en = int(exit_en - window_width/2)

    #plt.tight_layout()
    #plt.show()
    

def explore_trades(dmtx, trades, defsignals=None, signals=None, candles=None,
                   annotate_candles=False):
    from ipywidgets import interactive, IntSlider, FloatSlider
    from ipywidgets import Box
    from IPython.display import display
    float_slider = FloatSlider
    int_slider = IntSlider

    """ Interactively explore a list of TradeInfos (or the one contained in the BacktestStats
    object given)
    """
    if isinstance(trades, BacktestStats):
        trades = trades.list_of_trades
    def dis(**kwargs):
        #plt.clf()
        trade = trades[kwargs['idx']]
        display_trade(dmtx, trade, window_width=kwargs['window_width'], 
                      defsignals = defsignals, signals = signals, candles = candles,
                      annotate_candles = annotate_candles)
        plt.show()
    
    i = interactive( dis,
                 
                 window_width = IntSlider(min=0, max=500, step=5, value=15, sync=False), 
                 idx = IntSlider(min=0, max=len(trades)-1, step=1, value=0, sync=False), 

                 )

    #box = Box(i.children)
    #display(box)
    return i
    
    
    
    
    
    
if __name__ == '__main__':
    pass



































