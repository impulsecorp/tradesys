import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__)))
script_dir = os.path.dirname( os.path.abspath(__file__) )

import warnings
warnings.filterwarnings('ignore')

import numpy as np
import pickle as pkl
from keras.models import load_model

#from pympler import tracker

class BaseDataModel:
    """ Inherit from this class and call the inherited class DataModel """

    def input(self, timewin):

        # take the raw timewin and construct a DataMatrix from it
        self.datalen = timewin.shape[1]
        self.d_open = timewin[0, :]
        self.d_high = timewin[1, :]
        self.d_low = timewin[2, :]
        self.d_close = timewin[3, :]
        self.d_volume = timewin[4, :]
        self.d_year = timewin[5, :]
        self.d_mon = timewin[6, :]
        self.d_day = timewin[7, :]
        self.d_hour = timewin[8, :]
        self.d_min = timewin[9, :]
        self.d_sec = timewin[10, :]

        """
        self.datamatrix = DataMatrix()
        self.dts = []
        for i in range(self.datalen):
            d = DateTime(int(self.d_year[i]), int(self.d_mon[i]), int(self.d_day[i]), int(self.d_hour[i]), int(self.d_min[i]), int(self.d_sec[i]))
            self.dts.append(d)
            self.datamatrix.add_row(d, float(self.d_open[i]), float(self.d_high[i]), float(self.d_low[i]), float(self.d_close[i]), int(self.d_volume[i]))
        """

        self.prs = []
        self.frs = []

    def process(self):
        # The raw data (price range) + time of day (fixed range)
        self.prs.append((self.d_open, 'Open'))
        self.prs.append((self.d_high, 'High'))
        self.prs.append((self.d_low, 'Low'))
        self.prs.append((self.d_close, 'Close'))

        t = ((self.d_hour * 60 + self.d_min) - 240) / 960
        self.frs.append((t, 'Time'))

        # Stack and scale all data
        pr_data = np.vstack([x[0] for x in self.prs])
        fr_data = np.vstack([x[0] for x in self.frs])

        self.data = np.vstack([pr_data, fr_data])

    def cut_nans(self):
        # find the real start of the data (without NaN)
        nc = 0
        for cl in range(self.data.shape[1]):
            a = self.data[:, cl]
            if len(a[np.isnan(a)]) > 0:
                nc = cl
        # cut off the parts that contain NaN
        self.data = self.data[:, nc + 1:]
        return nc+1

    def cut_nans_fast(self, cutidx):
        self.data = self.data[:, cutidx:]

    def output(self, winlen):
        x = self.data[:, -winlen:]
        return x



class BaseMLModel:
    """ Inherit from this class and call the inherited class MLModel """

    def __init__(self):
        self.timewin = None
        self.winlen = None
        self.x = None
        self.model = None
        self.modeltype = None
        self.log = None
        self.cl = None
        self.datamodel = None
        self.do_log = False
        self.cutidx = None


    def init_module(self, max_bars_back, wl, modelname='model', datamodel = BaseDataModel, modeltype='', weights='', dolog=False):

        self.winlen = wl
        self.timewin = np.zeros((11, max_bars_back))
        self.modeltype = modeltype
        self.modelname = modelname

        cwd = os.getcwd()
        sys.path.insert(0, os.getcwd())
        self.datamodel = datamodel()


        if self.modeltype == 'sklearn':
            # append .pkl only for sklearn models
            mn = modelname
            if mn[-4:] != '.pkl': mn += '.pkl'
            self.model = pkl.load(open(mn, 'rb'))
        elif self.modeltype == 'keras':
            # keras model can be saved directly to HDF5
            self.model = load_model(modelname)

        self.do_log = bool(dolog)
        self.log = None

    def store_data_idx(self, idx, o, h, l, c, v, year, mon, day, hour, min, sec):
        self.timewin[0, idx] = o
        self.timewin[1, idx] = h
        self.timewin[2, idx] = l
        self.timewin[3, idx] = c
        self.timewin[4, idx] = v
        self.timewin[5, idx] = year
        self.timewin[6, idx] = mon
        self.timewin[7, idx] = day
        self.timewin[8, idx] = hour
        self.timewin[9, idx] = min
        self.timewin[10, idx] = sec

    def tick(self):
        self.datamodel.input(self.timewin[:,::-1])
        self.datamodel.process()
        if self.cutidx is None:
            self.cutidx = self.datamodel.cut_nans()
        else:
            self.datamodel.cut_nans_fast(self.cutidx)
        self.x = self.datamodel.output(self.winlen)

        if self.do_log:
            if self.log is None:
                self.log = self.x.reshape(1, -1)
            else:
                self.log = np.vstack([self.log, self.x.reshape(1, -1)])

        if self.modeltype == 'sklearn':
            self.x = self.x.reshape(1, -1)
        elif self.modeltype == 'keras':
            self.x = self.x.reshape(1, self.winlen, -1)
        else:
            self.x = self.x.reshape(1, -1)

    def write_log(self):
        if self.do_log:
            existing = [x for x in os.listdir(os.getcwd()) if 'x_log' in x]
            name = 'x_log.npy'
            if len(existing) > 0:
                name = 'x_log_' + str(len(existing)) + '.npy'
            np.save(name, self.log)
            print('Log written as',name)

    def get_response(self):
        an = self.model.predict(self.x)
        an = an[0]
        if an == 0:
            return 1
        elif an == 2:
            return -1
        else:
            return 0


MLModel = BaseMLModel
DataModel = BaseDataModel
