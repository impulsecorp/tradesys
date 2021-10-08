# -*- coding: utf-8 -*-
"""
Created on Sat May 31 11:37:55 2014

@author: peter
"""
import os
import sys

from sqlalchemy import Table, Column, String, Enum, Integer, Float, UniqueConstraint, DateTime
from sqlalchemy import create_engine, MetaData, asc, desc, and_, or_, not_
from sqlalchemy.sql import select
import pandas as pd
import hashlib
import numpy as np
from .tstools import dmat2df, df2dmat, chunks, ProgressBar
from _tradesys import DataMatrix, MultiDataMatrix, DateTime as tsysDateTime
from datetime import datetime
import collections as coll

print(('TradeSys is connecting to the quote database ...',))
import socket
if socket.gethostname() == 'laptop':
    engine = create_engine('mysql+cymysql://root:peter123@localhost/tradesys_db', echo=False, pool_recycle=3600)
elif socket.gethostname() == 'desktop':
    engine = create_engine('mysql+cymysql://root:peter123@localhost/tradesys_db', echo=False, pool_recycle=3600)
elif socket.gethostname() == 'slave':
    engine = create_engine('mysql+cymysql://root:peter123@localhost/tradesys_db', echo=False, pool_recycle=3600)
else:
    engine = create_engine('mysql+cymysql://root:peter123@localhost/tradesys_db', echo=False, pool_recycle=3600)
metadata = MetaData(bind=engine, reflect=True)
print('ok')

def create_master_table():
    global metadata
    global engine

    quote_tables = Table('quote_tables', metadata,

                   Column('id', Integer, primary_key=True),

                   Column('symbol', String(32), primary_key=True, nullable=False),
                   Column('origin', String(100), primary_key=True, nullable=False),
                   Column('data_type', Enum('ticks', 'ohlc',
                                            name='data_type'), primary_key=True, nullable=False),
                   Column('period', Integer, primary_key=True, nullable=True),
                   Column('period_type', Enum('seconds', 'minutes', 'hours', 'days', 'weeks', 'months',
                                              name='period_type'), primary_key=True, nullable=True),
                   Column('table_name', String(300), nullable=False),

                   UniqueConstraint('symbol', 'origin', 'data_type', 'period', 'period_type', 'table_name', name='uix_1'),
                   )
    metadata.create_all(engine)
    return quote_tables

def get_table_name(symbol, origin, data_type, period, period_type):
    if (not symbol) or (not origin) or (not data_type):
        raise TypeError("symbol, origin or data_type can't be None or empty")
    for x in [symbol, origin, data_type, period_type]: # period can be int
        if not (isinstance(x, str) or (x is None)):
            raise TypeError('[symbol, origin, data_type, period_type] must be all strings or None')
    if not (isinstance(period, str) or isinstance(period, int) or (period is None)):
        raise TypeError('period must be a string, None or an int')
    if isinstance(period, str):
        period = int(period)
    if (not period_type) or (not period):
        period_type = None
        period = None
    if (data_type != 'ticks') and (data_type != 'ohlc') and (data_type != 'ohlcv'):
        raise ValueError("data_type must be either ticks or ohlc[v]")
    if period_type != None:
        if (period_type == 's') or (period_type == 'sec') or (period_type == 'seconds'):
            period_type = 'seconds'
        elif (period_type == 'm') or (period_type == 'min') or (period_type == 'mins') or (period_type == 'minutes'):
            period_type = 'minutes'
        elif (period_type == 'h') or (period_type == 'hour') or (period_type == 'hours'):
            period_type = 'hours'
        elif (period_type == 'd') or (period_type == 'day') or (period_type == 'days'):
            period_type = 'days'
        elif (period_type == 'w') or (period_type == 'week') or (period_type == 'weeks'):
            period_type = 'weeks'
        elif (period_type == 'mo') or (period_type == 'mon') or (period_type == 'months'):
            period_type = 'months'
        else:
            period_type = None
    # make the table name
    s = symbol.lower() + origin.lower() + data_type.lower() + str(period).lower() + str(period_type).lower()
    hs = hashlib.sha1(s.encode('utf-8')).hexdigest()
    table_name = 'quotes_' + hs

    return symbol, origin, data_type, period, period_type, table_name

def add_symbol(symbol, origin='tradestation', data_type='ohlc', period=1, period_type='minutes'):
    " Add a new empty symbol to the database "
    global metadata
    global engine

    try:
        quote_tables = metadata.tables['quote_tables']
    except:
        quote_tables = create_master_table()

    (symbol, origin,
     data_type, period, period_type,
     table_name) = get_table_name(symbol, origin, data_type, period, period_type)

    conn = engine.connect()

    # add the new table to the master table first
    ins = quote_tables.insert().values(symbol = symbol,
                                       origin = origin,
                                       data_type = data_type,
                                       period = period,
                                       period_type = period_type,
                                       table_name = table_name)
    try:
        conn.execute(ins)
    except Exception as ex:
        print(ex)

    # determine whether to make a tick table or ohlcv one
    if data_type == 'ticks':
        # the new table - ticks
        new_table = Table(table_name, metadata,

                          Column('dt', DateTime, primary_key=True, nullable=False),
                          Column('bid', Float, nullable=False),
                          Column('ask', Float, nullable=False)
                         )
    else:
        # the new table - OHLCV
        new_table = Table(table_name, metadata,

                          Column('dt', DateTime, primary_key=True, nullable=False),
                          Column('open', Float, nullable=False),
                          Column('high', Float, nullable=False),
                          Column('low', Float, nullable=False),
                          Column('close', Float, nullable=False),
                          Column('volume', Integer, nullable=True)
                         )
    try:
        metadata.create_all(engine)
        return new_table
    except Exception as ex:
        print(ex)
        return None




def update_symbol(data, symbol, origin='tradestation', data_type='ohlc', period=1, period_type='minutes'):
    " Update an existing symbol with data. If not exising, a new will be made. "
    (symbol, origin,
     data_type, period, period_type,
     table_name) = get_table_name(symbol, origin, data_type, period, period_type)

    global metadata
    global engine

    try:
        table = metadata.tables[table_name]
    except Exception as ex:
        # no such table? add it now
        #print ex
        table = add_symbol(symbol=symbol, origin=origin, data_type=data_type, period=period, period_type=period_type)

    if table is None:
        raise ValueError("Table is None")

    if isinstance(data, MultiDataMatrix):
        raise ValueError("MultiDataMatrix can't be sent to the database")
    if data_type == 'ohlc' and isinstance(data, pd.DataFrame):
        data = df2dmat(data)
    if data_type == 'ticks' and isinstance(data, DataMatrix):
        raise ValueError("Can't upload ticks with a DataMatrix, use pandas dataframes instead.")

    conn = engine.connect()

    #print 'Filtering already existing records..'

    if data_type == 'ohlc':
        # to check if the rows to be inserted are already present
        """
        sel = select([table.c.dt,
                      table.c.open,
                      table.c.high,
                      table.c.low,
                      table.c.close,
                      table.c.volume]).order_by(desc(table.c.dt))
        ar = []
        for x in conn.execute(sel):
            ar.append((x[0], x[1], x[2], x[3], x[4], x[5]))
        """
        dl = list(zip(data.get_dt_list(), data.open, data.high, data.low, data.close, data.volume))

    else:
        # to check if the rows to be inserted are already present
        """
        sel = select([table.c.dt,
                      table.c.bid,
                      table.c.ask]).order_by(desc(table.c.dt))
        ar = []
        for x in conn.execute(sel):
            ar.append((x[0], x[1], x[2]))
        """
        dl = []
#        import pdb; pdb.set_trace() # to check if i is a datetime or something else and needs conversion
        for i, x in data.iterrows():
            o, h, l, c, v = x
            dl.append((i, o, h, l, c, v))

    """
    if len(dl) > len(ar):
        r = list(set(dl) - set(ar))
    else:
        r = list(set(ar) - set(dl))


    # filter just the ones that will be new
    dl = r
    """

    if dl:
        chunk_size = 50000
        if len(dl) > chunk_size:
            print('Sending data chunks..')
            num_chunks = int(len(dl) / float(chunk_size))
            pr = ProgressBar(num_chunks)
            for i, ch in enumerate(chunks(dl, chunk_size)):
                ins = table.insert().prefix_with("IGNORE").values(ch)
                conn.execute(ins)
                pr.animate(i)
        else:
            print('Sending data..')
            ins = table.insert().prefix_with("IGNORE").values(dl)
            conn.execute(ins)
        print('done.')
    else:
        print('Everything up to date')


def process_date_range(date_range):
    # sanitaze the date/time range
    if date_range != None:
        if not isinstance(date_range, tuple):
            raise TypeError("date_range must be a 2-tuple of DateTime, datetime.datetime or tuple(int, ...).")
        if len(date_range) != 2:
            raise TypeError("date_range must be a 2-tuple of DateTime, datetime.datetime or tuple(int, ...).")
        if not (isinstance(date_range[0], tsysDateTime) or isinstance(date_range[0], datetime) or
            isinstance(date_range[0], tuple)):
            raise TypeError("date_range must be a 2-tuple of DateTime, datetime.datetime or tuple(int, ...).")
        if not (isinstance(date_range[1], tsysDateTime) or
            isinstance(date_range[1], datetime) or
            isinstance(date_range[1], tuple)):
            raise TypeError("date_range must be a 2-tuple of DateTime, datetime.datetime or tuple(int, int, ..., int).")
        if isinstance(date_range[0], tuple):
            for x in date_range[0]:
                if not isinstance(x, int):
                    raise TypeError("date_range must be a 2-tuple of DateTime, datetime.datetime or tuple(int, int, ..., int).")
            if len(date_range[0]) == 1:
                date_range = ((date_range[0][0], 1, 1), date_range[1])
            if len(date_range[0]) == 2:
                date_range = ((date_range[0][0], date_range[0][1], 1), date_range[1])

        if isinstance(date_range[1], tuple):
            for x in date_range[1]:
                if not isinstance(x, int):
                    raise TypeError("date_range must be a 2-tuple of DateTime, datetime.datetime or tuple(int, int, ..., int).")
            if len(date_range[1]) == 1:
                date_range = (date_range[0], (date_range[1][0], 1, 1))
            if len(date_range[1]) == 2:
                date_range = (date_range[0], (date_range[1][0], date_range[1][1], 1))

        # make the date_range made of datetime.datetime only
        if isinstance(date_range[0], tsysDateTime):
            date_range = (datetime(date_range[0].year, date_range[0].month,
                    date_range[0].day,
                    date_range[0].hour,
                    date_range[0].minute,
                    date_range[0].second),
                #date_range[0].millisecond,
                date_range[1])
        elif isinstance(date_range[0], tuple):
            date_range = datetime(*date_range[0]), date_range[1]
        if isinstance(date_range[1], tsysDateTime):
            date_range = date_range[0], datetime(date_range[1].year,
                date_range[1].month,
                date_range[1].day,
                date_range[1].hour,
                date_range[1].minute,
                date_range[1].second)
                    #date_range[1].millisecond,
        elif isinstance(date_range[1], tuple):
            date_range = date_range[0], datetime(*date_range[1])
    return date_range

def get_symbol(symbol=None, origin='tradestation', data_type='ohlc', period=1, period_type='minutes', **kwargs):
    """
    Retrieve a symbol from the database. Returns either a DataMatix for ohlc data or a pd.DataFrame for ticks.

    Accepted keyword arguments:
    ---------------------------

    resample : (None, 'ticks', 'base') - If not None, will attempt to produce the desired period by
               using higher resolution data. If it's ticks, it will attempt to use tick data for the same symbol/origin.
               If it's base, it will use the period of 1 for the given period_type, i.e. 1 s, 1 m or 1 d.
               Will raise exception if high resolution data is not available.

    date_range : Can be a tuple of DateTime, a tuple of datetime.datetime, or a tuple of tuples of ints representing a date.
                 It's the date range we are interested in.

    hour_range : (tuple of datetime.time, or a tuple of tuples of int) - the time_range to keep,
                 between 00:00:00 and 23:59:59

    as_pandas_df : if resolves to True, it will always return pandas DataFrames

    """

    if not symbol:
        return None

    global metadata
    global engine

    try:
        resample = kwargs['resample']
    except KeyError:
        resample = None

    try:
        date_range = kwargs['date_range']
    except KeyError:
        date_range = None

    try:
        hour_range = kwargs['hour_range']
    except KeyError:
        hour_range = None

    try:
        as_pandas_df = kwargs['as_pandas_df']
    except KeyError:
        as_pandas_df = None

    # resample == 'base' makes sense only if period > 1
    if resample == 'base' and period == 1:
        resample = None

    if resample == 'ticks':
        (symbol, origin_, data_type_,
        period_, period_type_, table_name) = get_table_name(symbol, origin, 'ticks', None, None)
    elif resample == 'base':
        (symbol_, origin_, data_type_,
        period_, period_type_, table_name) = get_table_name(symbol, origin, data_type, 1, period_type)
    else:
        (symbol, origin_, data_type_,
        period_, period_type_, table_name) = get_table_name(symbol, origin, data_type, period, period_type)
    date_range = process_date_range(date_range)
    #time_range = process_time_range(time_range)
    conn = engine.connect()

    try:
        table = metadata.tables[table_name]
    except KeyError:
        print("Symbol doesn't exist.")
        return None

    if data_type == 'ohlc':
        # to check if the rows to be inserted are already present
        if not date_range:
            sel = select([table.c.dt,
                          table.c.open,
                          table.c.high,
                          table.c.low,
                          table.c.close,
                          table.c.volume]).order_by(asc(table.c.dt))
        else:
            sel = select([table.c.dt,
                          table.c.open,
                          table.c.high,
                          table.c.low,
                          table.c.close,
                          table.c.volume]).where(and_( table.c.dt >= date_range[0],
                                                       table.c.dt <= date_range[1])).order_by(asc(table.c.dt))

        if (not resample) and (not as_pandas_df):
            r = DataMatrix()
            for x in conn.execute(sel):
                d = tsysDateTime(x[0].year,
                                 x[0].month,
                                 x[0].day,
                                 x[0].hour,
                                 x[0].minute,
                                 x[0].second,
                                 #x[0].millisecond,
                                 )
                r.add_row(d, x[1], x[2], x[3], x[4], x[5])
            r.name = symbol.upper()

        else:

            r = []
            ix = []
            for x in conn.execute(sel):
                ix.append(x[0])
                r.append(x[1:])
            r = pd.DataFrame(columns=['open', 'high', 'low', 'close', 'volume'], data = r, index = ix)

            if resample == 'base':
                #import pdb; pdb.set_trace()
                if (period_type.lower() == 's') or (period_type.lower() == 'sec') or (period_type.lower() == 'seconds'):
                    rs = 'S'
                if (period_type.lower() == 'm') or (period_type.lower() == 'min') or (period_type.lower() == 'minutes'):
                    rs = 'Min'
                if (period_type.lower() == 'd') or (period_type.lower() == 'day') or (period_type.lower() == 'days'):
                    rs = 'D'
                rs = str(period)+rs
                ohlcv = { 'open' : 'first', 'high' : 'max', 'low' : 'min', 'close' : 'last', 'volume' : 'sum' }
                af = r.shift(-1)
                af = af.resample(rs, how=ohlcv)
                af = af.reindex_axis(sorted(af.columns), axis=1) # sort column names
                # remove the NaNs
                af = af.shift(1)
                af = af.dropna()
                r = af

                if not as_pandas_df:
                    r = df2dmat(r)

    else:

        # to check if the rows to be inserted are already present
        if not date_range:
            sel = select([table.c.dt,
                          table.c.bid,
                          table.c.ask]).order_by(asc(table.c.dt))
        else:
            sel = select([table.c.dt,
                          table.c.bid,
                          table.c.ask]).where(and_(table.c.dt >= date_range[0],
                                                   table.c.dt <= date_range[1])).order_by(asc(table.c.dt))
        r, ix = [], []
        for x in conn.execute(sel):
            ix.append(x[0])
            r.append((x[1], x[2]))

        r = pd.DataFrame(columns=["bid", "ask"], data = r, index = ix)

        # if resample is set to ticks, and period/period_type is nonzero, turn that into ohlc data
        pass #TODO

        r.name = symbol.upper()

    return r

def list_symbols():
    global metadata
    global engine

    try:
        quote_tables = metadata.tables['quote_tables']
    except Exception as ex:
        print(ex)
        print("Master table is gone or other error, will attempt to create it..")
        quote_tables = create_master_table()
    sel = select([quote_tables.c.symbol,
                 quote_tables.c.origin,
                 quote_tables.c.data_type,
                 quote_tables.c.period,
                 quote_tables.c.period_type]).order_by(asc(quote_tables.c.symbol))
    conn = engine.connect()
    r = conn.execute(sel)
    rs = []
    for x in r:
        rs.append(x)
    return rs

def delete_symbol(symbol, origin, data_type='ohlc', period=1, period_type='minutes'):
    " Delete symbol "
    global metadata
    global engine

    (symbol, origin, data_type, period, period_type,
     table_name) = get_table_name(symbol, origin, data_type, period, period_type)

    try:
        metadata.tables[table_name].drop()
    except:
        print('No such table or other error')

    try:
        # delete the row from the master table
        quote_tables = metadata.tables['quote_tables']
    except:
        print("Master table is gone or other error.")
        return

    try:
        d = quote_tables.delete().where(quote_tables.c.table_name == table_name)
        conn = engine.connect()
        conn.execute(d)
    except:
        print("Couldn't delete row from master table")











