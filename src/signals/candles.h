/*
 * candles.h
 *
 *  Created on: Dec 8, 2014
 *      Author: peter
 */

#ifndef CANDLES_H_
#define CANDLES_H_

#ifndef NOTALIB

#include <ta-lib/ta_func.h>
#include <ta-lib/ta_common.h>
#include "base_signal.h"

/////////////////////////////////////////////////////////////////////////////////////////////
#define AS_STR(tok) #tok

#define CDL_SIGNAL(X)   class X : public Signal \
						{ \
						public: \
							std::vector<int> m_data; \
							unsigned int m_bars_back; \
							X(ParameterMap& ow, std::vector<Signal*> ch) \
							{ \
								INIT_SIGNAL(RT_INT, AS_STR(X)); \
							} \
							virtual ParameterConstraintMap get_initial_constraints(); \
							virtual void update_data(const DataMatrix& data); \
							virtual void compute_tick(const Env& env, int idx); \
							virtual std::string get_el_tick(); \
							virtual std::string get_mql_tick(); \
						};

//////////////////////////////////////////////////////////////////////////////////////////////

CDL_SIGNAL(CDL_2CROWS);
CDL_SIGNAL(CDL_3BLACKCROWS);
CDL_SIGNAL(CDL_3INSIDE);
CDL_SIGNAL(CDL_3LINESTRIKE);
CDL_SIGNAL(CDL_3OUTSIDE);
CDL_SIGNAL(CDL_3STARSINSOUTH);
CDL_SIGNAL(CDL_3WHITESOLDIERS);
//CDL_SIGNAL(CDL_ABANDONEDBABY);
CDL_SIGNAL(CDL_ADVANCEBLOCK);
CDL_SIGNAL(CDL_BELTHOLD);
CDL_SIGNAL(CDL_BREAKAWAY);
CDL_SIGNAL(CDL_CLOSINGMARUBOZU);
CDL_SIGNAL(CDL_CONCEALBABYSWALL);
CDL_SIGNAL(CDL_COUNTERATTACK);
//CDL_SIGNAL(CDL_DARKCLOUDCOVER);
CDL_SIGNAL(CDL_DOJI);
CDL_SIGNAL(CDL_DOJISTAR);
CDL_SIGNAL(CDL_DRAGONFLYDOJI);
CDL_SIGNAL(CDL_ENGULFING);
//CDL_SIGNAL(CDL_EVENINGDOJISTAR);
//CDL_SIGNAL(CDL_EVENINGSTAR);
CDL_SIGNAL(CDL_GAPSIDESIDEWHITE);
CDL_SIGNAL(CDL_GRAVESTONEDOJI);
CDL_SIGNAL(CDL_HAMMER);
CDL_SIGNAL(CDL_HANGINGMAN);
CDL_SIGNAL(CDL_HARAMI);
CDL_SIGNAL(CDL_HARAMICROSS);
CDL_SIGNAL(CDL_HIGHWAVE);
CDL_SIGNAL(CDL_HIKKAKE);
CDL_SIGNAL(CDL_HIKKAKEMOD);
CDL_SIGNAL(CDL_HOMINGPIGEON);
CDL_SIGNAL(CDL_IDENTICAL3CROWS);
CDL_SIGNAL(CDL_INNECK);
CDL_SIGNAL(CDL_INVERTEDHAMMER);
CDL_SIGNAL(CDL_KICKING);
CDL_SIGNAL(CDL_KICKINGBYLENGTH);
CDL_SIGNAL(CDL_LADDERBOTTOM);
CDL_SIGNAL(CDL_LONGLEGGEDDOJI);
CDL_SIGNAL(CDL_LONGLINE);
CDL_SIGNAL(CDL_MARUBOZU);
CDL_SIGNAL(CDL_MATCHINGLOW);
//CDL_SIGNAL(CDL_MATHOLD);
//CDL_SIGNAL(CDL_MORNINGDOJISTAR);
//CDL_SIGNAL(CDL_MORNINGSTAR);
CDL_SIGNAL(CDL_ONNECK);
CDL_SIGNAL(CDL_PIERCING);
CDL_SIGNAL(CDL_RICKSHAWMAN);
CDL_SIGNAL(CDL_RISEFALL3METHODS);
CDL_SIGNAL(CDL_SEPARATINGLINES);
CDL_SIGNAL(CDL_SHOOTINGSTAR);
CDL_SIGNAL(CDL_SHORTLINE);
CDL_SIGNAL(CDL_SPINNINGTOP);
CDL_SIGNAL(CDL_STALLEDPATTERN);
CDL_SIGNAL(CDL_STICKSANDWICH);
CDL_SIGNAL(CDL_TAKURI);
CDL_SIGNAL(CDL_TASUKIGAP);
CDL_SIGNAL(CDL_THRUSTING);
CDL_SIGNAL(CDL_TRISTAR);
CDL_SIGNAL(CDL_UNIQUE3RIVER);
CDL_SIGNAL(CDL_UPSIDEGAP2CROWS);
CDL_SIGNAL(CDL_XSIDEGAP3METHODS);

#endif


#endif /* CANDLES_H_ */
