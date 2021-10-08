#!/usr/bin/python3

from setuptools import find_packages
#from distutils.core import setup, Extension
#try:
from setuptools import setup, Extension 

    # setuptools is stupid and rewrites "sources" to change '.pyx' to '.c'
    # if it can't find Pyrex (and in recent versions, Cython).
    #
    # This is a really stupid thing to do behind the users's back (since
    # it breaks development builds) especially with no way of disabling it
    # short of the hack below.
#    class Extension(_Extension):
#        def __init__(self, *args, **kwargs):
#            save_sources = kwargs.get('sources', None)
#            _Extension.__init__(self, *args, **kwargs)
#            self.sources = save_sources
#except ImportError:
#    from distutils.core import setup, Extension


import os.path
import sys
if sys.version_info[0] < 3:
    lb = 'boost_python'
else:
    lb = 'boost_python38'

gN = 1

if '--debug' in sys.argv:
    ddebug = True
else:
    ddebug = False

import socket
gN = 1
if socket.gethostname() == 'laptop':
    gN = 8
if socket.gethostname() == 'desktop':
    gN = 8
if socket.gethostname() == 'slave':
    gN = 1
if socket.gethostname() == 'b08s06uq':
    gN = 4

# monkey-patch for parallel compilation
def parallelCCompile(self, sources, output_dir=None, macros=None, include_dirs=None, debug=0, extra_preargs=None, extra_postargs=None, depends=None):
    global gN
    global ddebug
    # those lines are copied from distutils.ccompiler.CCompiler directly
    macros, objects, extra_postargs, pp_opts, build = self._setup_compile(output_dir, macros, include_dirs, sources, depends, extra_postargs)
    cc_args = self._get_cc_args(pp_opts, debug, extra_preargs)

    if not ddebug:
        if sys.platform == 'win32':
            cc_args += ['-Ofast','-flto','-march=native','-funroll-loops', '-std=gnu++11']
        else:
            cc_args += ['-fPIC','-m64','-Ofast','-flto','-march=native','-funroll-loops', '-std=c++11']
    else:
        try: del cc_args[cc_args.find('-O2')]
        except: pass
        if sys.platform == 'win32':
            cc_args += ['-Wall', '-O0', '-g', '-std=gnu++11']
        else:
            cc_args += ['-fPIC','-Wall', '-O0', '-g', '-std=c++11']

    if sys.platform == "win32": cc_args += ['-DNOTALIB']

    # parallel code
    N=gN # number of parallel compilations
    import multiprocessing.pool
    def _single_compile(obj):
        try: src, ext = build[obj]
        except KeyError: return
        self._compile(obj, src, ext, cc_args, extra_postargs, pp_opts)
    # convert to list, imap is evaluated on-demand
    list(multiprocessing.pool.ThreadPool(N).imap(_single_compile,objects))
    return objects
#import distutils.ccompiler
#distutils.ccompiler.CCompiler.compile=parallelCCompile


if sys.platform == "win32" :
    include_dirs = ["C:/boost/include","C:/ta-lib/include", "."]
    libraries=[]
    libraries += ["boost_system-vc141-mt-x64-1_71",
                    "boost_filesystem-vc141-mt-x64-1_71",
                    "boost_wserialization-vc141-mt-x64-1_71",
                    "boost_date_time-vc141-mt-x64-1_71",
                    "boost_random-vc141-mt-x64-1_71",
                    "boost_python36-vc141-mt-x64-1_71",
                    "boost_numpy36-vc141-mt-x64-1_71",
                    "python3", 
                    "ta_abstract_cmr",
                    "ta_common_cmr",
                    "ta_func_cmr",
                    "ta_libc_cmr",
                    ]
    library_dirs=['C:/boost/lib', 'C:/ta-lib/lib']
else :
    include_dirs = []
    libraries=[lb, "boost_serialization", "ta_lib"]
    library_dirs=[]

setup(name='tradesys',
      author="Peter Chervenski",
      author_email = "spookey@abv.bg",
      description = ("Software for creating and backtesting trading strategies."),

      packages=find_packages(),
      version='0.2',
      ext_modules=[Extension('tradesys._tradesys',

                 ['src/'+x for x in ['expression.cpp',
        		  'backtest_run.cpp',
        		  'data.cpp',
        		  'strategy.cpp',
                  'sformat.cpp',
        		  'dtime.cpp',
        		  'backtest_stats.cpp',
        		  'python_bindings.cpp',
                  'live.cpp',
        		  'signal_tree.cpp',
                  'signal_forest.cpp',
        		  'signals/ops.cpp',
                  'signals/constraints.cpp',
                  'signals/common_types.cpp',
        		  'signals/crosses.cpp',
        		  'signals/timesinarow.cpp',
        		  'signals/numeric.cpp',
        		  'signals/base_signal.cpp',
        		  'signals/times.cpp',
        		  'signals/gaps.cpp',
        		  'signals/raw.cpp',
        		  'signals/feedback.cpp',
        		  'signals/happened.cpp',
                  'signals/stops.cpp',
                  'signals/ta.cpp',
                  'signals/candles.cpp',
                  'signals/supertrend.cpp']],
                  include_dirs = include_dirs,
                  library_dirs = library_dirs,
                  libraries = libraries,
                  depends=[])]
      )
