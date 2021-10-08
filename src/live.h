#ifndef live_h__
#define live_h__

#include "common.h"
#include "strategy.h"
#include "data.h"

// The live trading class encapsulates a live trading session. 
class LiveTrader
{
    // the DataMatrix holding the data stream. It's kept in constant size. 
    DataMatrix m_data; 

    // the strategy used
    BaseStrategy* m_strategy;
};

#endif // live_h__
