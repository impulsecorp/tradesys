/*
 * candles.cpp
 *
 *  Created on: Dec 8, 2014
 *      Author: peter
 */

#ifndef NOTALIB

#include "candles.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CDL_SIGNAL_IMP(X, TA_X) \
                            ParameterConstraintMap X::get_initial_constraints()          \
							{                                                            \
								ParameterConstraintMap t_constraints;                    \
								SP_constraints t_bars_back;                              \
								t_bars_back.usage_mode = CNT_MINMAX;                     \
								t_bars_back.min = 0;                                     \
								t_bars_back.max = 120;                                   \
								t_bars_back.mut_min = 0;                                 \
								t_bars_back.mut_max = 120;                               \
								t_bars_back.mut_power = 10;                              \
								t_bars_back.is_mutable = false;                          \
								t_bars_back.default_value = 0;                           \
								t_constraints["bars_back"] = t_bars_back;                \
								return t_constraints;                                    \
							}                                                            \
							void X::update_data(const DataMatrix& a_data)                \
							{                                                            \
								m_bars_back = convert_to_int(m_parameters["bars_back"]); \
								m_data.clear();                                          \
								m_data.resize(a_data.len());                             \
								int out_beg = 0;                                         \
								int out_nbelement = 0;                                   \
								TA_X(0, (int)m_data.size()-1,                            \
										&(a_data.open[0]),                               \
										&(a_data.high[0]),                               \
										&(a_data.low[0]),                                \
										&(a_data.close[0]),                              \
										&out_beg, &out_nbelement,                        \
										&(m_data[0])                                     \
										);                                               \
								shift_ta_data(m_data, out_beg);                          \
							}                                                            \
							void X::compute_tick(const Env& env, int idx)       \
							{                                                            \
								m_outp = m_data[idx - m_bars_back];                      \
							}                                                            \
							std::string X::get_el_tick()                                 \
							{                                                            \
								return "";                                               \
							}                                                            \
							std::string X::get_mql_tick()                                \
							{                                                            \
								return "";                                               \
							}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDL_SIGNAL_IMP(CDL_2CROWS, TA_CDL2CROWS);
CDL_SIGNAL_IMP(CDL_3BLACKCROWS, TA_CDL3BLACKCROWS);
CDL_SIGNAL_IMP(CDL_3INSIDE, TA_CDL3INSIDE);
CDL_SIGNAL_IMP(CDL_3LINESTRIKE, TA_CDL3LINESTRIKE);
CDL_SIGNAL_IMP(CDL_3OUTSIDE, TA_CDL3OUTSIDE);
CDL_SIGNAL_IMP(CDL_3STARSINSOUTH, TA_CDL3STARSINSOUTH);
CDL_SIGNAL_IMP(CDL_3WHITESOLDIERS, TA_CDL3WHITESOLDIERS);
//CDL_SIGNAL_IMP(CDL_ABANDONEDBABY, TA_CDLABANDONEDBABY);
CDL_SIGNAL_IMP(CDL_ADVANCEBLOCK, TA_CDLADVANCEBLOCK);
CDL_SIGNAL_IMP(CDL_BELTHOLD, TA_CDLBELTHOLD);
CDL_SIGNAL_IMP(CDL_BREAKAWAY, TA_CDLBREAKAWAY);
CDL_SIGNAL_IMP(CDL_CLOSINGMARUBOZU, TA_CDLCLOSINGMARUBOZU);
CDL_SIGNAL_IMP(CDL_CONCEALBABYSWALL, TA_CDLCONCEALBABYSWALL);
CDL_SIGNAL_IMP(CDL_COUNTERATTACK, TA_CDLCOUNTERATTACK);
//CDL_SIGNAL_IMP(CDL_DARKCLOUDCOVER, TA_CDLDARKCLOUDCOVER);
CDL_SIGNAL_IMP(CDL_DOJI, TA_CDLDOJI);
CDL_SIGNAL_IMP(CDL_DOJISTAR, TA_CDLDOJISTAR);
CDL_SIGNAL_IMP(CDL_DRAGONFLYDOJI, TA_CDLDRAGONFLYDOJI);
CDL_SIGNAL_IMP(CDL_ENGULFING, TA_CDLENGULFING);
//CDL_SIGNAL_IMP(CDL_EVENINGDOJISTAR, TA_CDLEVENINGDOJISTAR);
//CDL_SIGNAL_IMP(CDL_EVENINGSTAR, TA_CDLEVENINGSTAR);
CDL_SIGNAL_IMP(CDL_GAPSIDESIDEWHITE, TA_CDLGAPSIDESIDEWHITE);
CDL_SIGNAL_IMP(CDL_GRAVESTONEDOJI, TA_CDLGRAVESTONEDOJI);
CDL_SIGNAL_IMP(CDL_HAMMER, TA_CDLHAMMER);
CDL_SIGNAL_IMP(CDL_HANGINGMAN, TA_CDLHANGINGMAN);
CDL_SIGNAL_IMP(CDL_HARAMI, TA_CDLHARAMI);
CDL_SIGNAL_IMP(CDL_HARAMICROSS, TA_CDLHARAMICROSS);
CDL_SIGNAL_IMP(CDL_HIGHWAVE, TA_CDLHIGHWAVE);
CDL_SIGNAL_IMP(CDL_HIKKAKE, TA_CDLHIKKAKE);
CDL_SIGNAL_IMP(CDL_HIKKAKEMOD, TA_CDLHIKKAKEMOD);
CDL_SIGNAL_IMP(CDL_HOMINGPIGEON, TA_CDLHOMINGPIGEON);
CDL_SIGNAL_IMP(CDL_IDENTICAL3CROWS, TA_CDLIDENTICAL3CROWS);
CDL_SIGNAL_IMP(CDL_INNECK, TA_CDLINNECK);
CDL_SIGNAL_IMP(CDL_INVERTEDHAMMER, TA_CDLINVERTEDHAMMER);
CDL_SIGNAL_IMP(CDL_KICKING, TA_CDLKICKING);
CDL_SIGNAL_IMP(CDL_KICKINGBYLENGTH, TA_CDLKICKINGBYLENGTH);
CDL_SIGNAL_IMP(CDL_LADDERBOTTOM, TA_CDLLADDERBOTTOM);
CDL_SIGNAL_IMP(CDL_LONGLEGGEDDOJI, TA_CDLLONGLEGGEDDOJI);
CDL_SIGNAL_IMP(CDL_LONGLINE, TA_CDLLONGLINE);
CDL_SIGNAL_IMP(CDL_MARUBOZU, TA_CDLMARUBOZU);
CDL_SIGNAL_IMP(CDL_MATCHINGLOW, TA_CDLMATCHINGLOW);
//CDL_SIGNAL_IMP(CDL_MATHOLD, TA_CDLMATHOLD);
//CDL_SIGNAL_IMP(CDL_MORNINGDOJISTAR, TA_CDLMORNINGDOJISTAR);
//CDL_SIGNAL_IMP(CDL_MORNINGSTAR, TA_CDLMORNINGSTAR);
CDL_SIGNAL_IMP(CDL_ONNECK, TA_CDLONNECK);
CDL_SIGNAL_IMP(CDL_PIERCING, TA_CDLPIERCING);
CDL_SIGNAL_IMP(CDL_RICKSHAWMAN, TA_CDLRICKSHAWMAN);
CDL_SIGNAL_IMP(CDL_RISEFALL3METHODS, TA_CDLRISEFALL3METHODS);
CDL_SIGNAL_IMP(CDL_SEPARATINGLINES, TA_CDLSEPARATINGLINES);
CDL_SIGNAL_IMP(CDL_SHOOTINGSTAR, TA_CDLSHOOTINGSTAR);
CDL_SIGNAL_IMP(CDL_SHORTLINE, TA_CDLSHORTLINE);
CDL_SIGNAL_IMP(CDL_SPINNINGTOP, TA_CDLSPINNINGTOP);
CDL_SIGNAL_IMP(CDL_STALLEDPATTERN, TA_CDLSTALLEDPATTERN);
CDL_SIGNAL_IMP(CDL_STICKSANDWICH, TA_CDLSTICKSANDWICH);
CDL_SIGNAL_IMP(CDL_TAKURI, TA_CDLTAKURI);
CDL_SIGNAL_IMP(CDL_TASUKIGAP, TA_CDLTASUKIGAP);
CDL_SIGNAL_IMP(CDL_THRUSTING, TA_CDLTHRUSTING);
CDL_SIGNAL_IMP(CDL_TRISTAR, TA_CDLTRISTAR);
CDL_SIGNAL_IMP(CDL_UNIQUE3RIVER, TA_CDLUNIQUE3RIVER);
CDL_SIGNAL_IMP(CDL_UPSIDEGAP2CROWS, TA_CDLUPSIDEGAP2CROWS);
CDL_SIGNAL_IMP(CDL_XSIDEGAP3METHODS, TA_CDLXSIDEGAP3METHODS);




#endif









