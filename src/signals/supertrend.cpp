#include "supertrend.h"

#ifndef NOTALIB

#include <ta-lib/ta_func.h>
#include <ta-lib/ta_common.h>
#include <ta-lib/ta_libc.h>
#include <ta-lib/ta_defs.h>

ParameterConstraintMap SuperTrendSignal::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;

	SP_constraints t_ATR_multiplier;
	t_ATR_multiplier.usage_mode = CNT_MINMAX;
	t_ATR_multiplier.min = 0.0;
	t_ATR_multiplier.max = 10.0;
	t_ATR_multiplier.mut_min = 0.0;
	t_ATR_multiplier.mut_max = 5.0;
	t_ATR_multiplier.mut_power = 0.5;
	t_ATR_multiplier.is_mutable = true;
	t_ATR_multiplier.default_value = 1.0;
	t_constraints["atr_mul"] = t_ATR_multiplier;

	SP_constraints t_ATR_period;
	t_ATR_period.usage_mode = CNT_MINMAX;
	t_ATR_period.min = 1;
	t_ATR_period.max = 240;
	t_ATR_period.mut_min = 1;
	t_ATR_period.mut_max = 60;
	t_ATR_period.mut_power = 10;
	t_ATR_period.is_mutable = true;
	t_ATR_period.default_value = 1;
	t_constraints["atr_period"] = t_ATR_period;

	SP_constraints t_median_period;	t_median_period.usage_mode = CNT_MINMAX;
	t_median_period.min = 1;
	t_median_period.max = 240;
	t_median_period.mut_min = 1;
	t_median_period.mut_max = 60;
	t_median_period.mut_power = 10;
	t_median_period.is_mutable = true;
	t_median_period.default_value = 1;
	t_constraints["median_period"] = t_median_period;

	return t_constraints;
}

void SuperTrendSignal::init_codegen_variables()
{
	m_variables["up"] = VAR_TYPE_FLOAT;
	m_variables["dn"] = VAR_TYPE_FLOAT;
	m_variables["trend"] = VAR_TYPE_INT;
	m_variables["prev_up"] = VAR_TYPE_FLOAT;
	m_variables["prev_dn"] = VAR_TYPE_FLOAT;
	m_variables["prev_trend"] = VAR_TYPE_INT;
	m_variables["flag"] = VAR_TYPE_INT;
	m_variables["flagh"] = VAR_TYPE_INT;
}

void SuperTrendSignal::update_data( const DataMatrix& data )
{
	// get the parameter values
	atr_multiplier = convert_to_float(m_parameters["atr_mul"]);
	atr_period = convert_to_int(m_parameters["atr_period"]);
	median_period = convert_to_int(m_parameters["median_period"]);

	close = data.close;

	// Compute the data
	atr.clear();    
	atr.resize(data.len());
	amedprice.clear();    
	amedprice.resize(data.len());

	// calculate
	int out_beg = 0;
	int out_nbelement = 0;

	if (atr_period > 1)
	{
		TA_ATR(0, (int)atr.size()-1, 
			&(data.high[0]),
			&(data.low[0]),
			&(data.close[0]),
			atr_period,  
			&out_beg, &out_nbelement, 
			&(atr[0])
			);
		shift_ta_data(atr, out_beg);
	}
	else
	{
		TA_TRANGE(0, (int)atr.size()-1, 
			&(data.high[0]),
			&(data.low[0]),
			&(data.close[0]),  
			&out_beg, &out_nbelement, 
			&(atr[0]));
	}


	// calculate medprice first, then average it
	TA_MEDPRICE(0, (int)amedprice.size()-1, 
		&(data.high[0]),
		&(data.low[0]), 
		&out_beg, 
		&out_nbelement, 
		&(amedprice[0]));
	if (median_period > 1)
	{
		TA_MA(0, (int)amedprice.size()-1, 
			&(amedprice[0]), 
			median_period, TA_MAType_SMA, 
			&out_beg, &out_nbelement, 
			&(amedprice[0])
			);
		shift_ta_data(amedprice, out_beg);
	}
}

void SuperTrendSignal::compute_init( const Env& env, int idx )
{
	up = dn = prev_up = prev_dn = prev_trend = 0;
	flag = flagh = 0;
	trend = 1;
}

void SuperTrendSignal::compute_tick( const Env& env, int idx )
{
	prev_up = up;
	prev_dn = dn;
	prev_trend = trend;

	up = amedprice[idx] + atr[idx] * atr_multiplier;
	dn = amedprice[idx] - atr[idx] * atr_multiplier;

	flag = 0; 
	flagh = 0;
	if (close[idx] > prev_up)
	{
		trend = 1;
	}
	else if (close[idx] < prev_dn)
	{
		trend = -1;
	}

	if ((trend < 0) && (prev_trend > 0))
	{
		flag = 1;
	}
	else
	{
		flag = 0;
	}

	if ((trend > 0) && (prev_trend < 0))
	{
		flagh = 1;
	}
	else
	{
		flagh = 0;
	}

	if ((trend > 0) && (dn < prev_dn))
	{
		dn = prev_dn;
	}
	if ((trend < 0) && (up > prev_up))
	{
		up = prev_up;
	}

	if (flag == 1)
	{
		up = amedprice[idx] + atr[idx] * atr_multiplier;
	}
	if (flagh == 1)
	{
		dn = amedprice[idx] - atr[idx] * atr_multiplier;
	}
}

std::string SuperTrendSignal::get_el_init()
{
	std::stringstream s;
	s << format(
		"%s = 0; "
		"%s = 0; "
		"%s = 1; "
		"%s = 0; "
		"%s = 0; "
		"%s = 0; "
		"%s = 0; "
		"%s = 0; "
		) 
		% var("up")
		% var("dn")
		% var("trend")
		% var("prev_up")
		% var("prev_dn")
		% var("prev_trend")
		% var("flag")
		% var("flagh");

	return s.str();
}

std::string SuperTrendSignal::get_el_tick()
{
	std::stringstream s;
	s << format(
		"%s = %s; "
		"%s = %s; "
		"%s = %s; "
		" "
		"%s = Average(MedianPrice, %s) + AvgTrueRange(%s)*%s; "
		"%s = Average(MedianPrice, %s) - AvgTrueRange(%s)*%s; "
		" "
		"%s = 0; "
		"%s = 0; "
		" "
		"if Close[0] > %s then %s = 1; "
		"if Close[0] < %s then %s = -1;"
		" "
		"if (%s < 0) and (%s > 0) then %s = 1 else %s = 0; "
		"if (%s > 0) and (%s < 0) then %s = 1 else %s = 0; "
		" "
		"if (%s > 0) and (%s < %s) then %s = %s; "
		"if (%s > 0) and (%s > %s) then %s = %s; "
		" "
		"if (%s = 1) then %s = Average(MedianPrice, %s) + AvgTrueRange(%s)*%s; "
		"if (%s = 1) then %s = Average(MedianPrice, %s) - AvgTrueRange(%s)*%s; "
		" ")

		% var("prev_up") % var("up")
		% var("prev_dn") % var("dn")
		% var("prev_trend") % var("trend")

		% var("up") % var("median_period") % var("atr_period") % var("atr_mul")
		% var("dn") % var("median_period") % var("atr_period") % var("atr_mul")

		% var("flag")
		% var("flagh")

		% var("prev_up")
		% var("trend")
		% var("prev_dn")
		% var("trend")

		% var("trend")
		% var("prev_trend")
		% var("flag")
		% var("flag")
		% var("trend")
		% var("prev_trend")
		% var("flagh")
		% var("flagh")

		% var("trend")
		% var("dn")
		% var("prev_dn")
		% var("dn")
		% var("prev_dn")

		% var("trend")
		% var("up")
		% var("prev_up")
		% var("up")
		% var("prev_up")

		% var("flag")
		% var("up") % var("median_period") % var("atr_period") % var("atr_mul")
		% var("flagh")
		% var("dn") % var("median_period") % var("atr_period") % var("atr_mul");

	return s.str();
}

std::string SuperTrendSignal::get_mql_init()
{
	std::stringstream s;
	s << format(
		"%s = 0; "
		"%s = 0; "
		"%s = 1; "
		"%s = 0; "
		"%s = 0; "
		"%s = 0; "
		"%s = 0; "
		"%s = 0; "
		) 
		% var("up")
		% var("dn")
		% var("trend")
		% var("prev_up")
		% var("prev_dn")
		% var("prev_trend")
		% var("flag")
		% var("flagh");

	return s.str();
}

std::string SuperTrendSignal::get_mql_tick()
{
	std::stringstream s;
	s << format(
		"%s = %s; "
		"%s = %s; "
		"%s = %s; "
		" "
		"%s = iMA(NULL,0, %s, 0, MODE_SMA, PRICE_MEDIAN, 0) + iATR(NULL, 0, %s, 0)*%s; "
		"%s = iMA(NULL,0, %s, 0, MODE_SMA, PRICE_MEDIAN, 0) - iATR(NULL, 0, %s, 0)*%s; "
		" "
		"%s = 0; "
		"%s = 0; "
		" "
		"if (Close[0] > %s) { %s = 1; } "
		"if (Close[0] < %s) { %s = -1; } "
		" "
		"if ((%s < 0) && (%s > 0)) { %s = 1; } else { %s = 0; } "
		"if ((%s > 0) && (%s < 0)) { %s = 1; } else { %s = 0; } "
		" "
		"if ((%s > 0) && (%s < %s)) { %s = %s; } "
		"if ((%s > 0) && (%s > %s)) { %s = %s; } "
		" "
		"if (%s == 1) { %s = iMA(NULL,0, %s, 0, MODE_SMA, PRICE_MEDIAN, 0) + iATR(NULL, 0, %s, 0)*%s; } "
		"if (%s == 1) { %s = iMA(NULL,0, %s, 0, MODE_SMA, PRICE_MEDIAN, 0) - iATR(NULL, 0, %s, 0)*%s; } "
		" ")

		% var("prev_up") % var("up")
		% var("prev_dn") % var("dn")
		% var("prev_trend") % var("trend")

		% var("up") % var("median_period") % var("atr_period") % var("atr_mul")
		% var("dn") % var("median_period") % var("atr_period") % var("atr_mul")

		% var("flag")
		% var("flagh")

		% var("prev_up")
		% var("trend")
		% var("prev_dn")
		% var("trend")

		% var("trend")
		% var("prev_trend")
		% var("flag")
		% var("flag")
		% var("trend")
		% var("prev_trend")
		% var("flagh")
		% var("flagh")

		% var("trend")
		% var("dn")
		% var("prev_dn")
		% var("dn")
		% var("prev_dn")

		% var("trend")
		% var("up")
		% var("prev_up")
		% var("up")
		% var("prev_up")

		% var("flag")
		% var("up") % var("median_period") % var("atr_period") % var("atr_mul")
		% var("flagh")
		% var("dn") % var("median_period") % var("atr_period") % var("atr_mul");

	return s.str();
}

void SuperTrendUp::compute_tick( const Env& env, int idx )
{
	SuperTrendSignal::compute_tick(env, idx);

	if (/*(prev_trend == -1) && */(trend == 1))
	{
		m_outp = true;
	}
	else
	{
		m_outp = false;
	}
}

std::string SuperTrendUp::get_el_tick()
{
	std::stringstream s;
	std::stringstream s1;
	s << SuperTrendSignal::get_el_tick();
	s1 << (format("({(%s = -1) and }(%s = 1))") % var("prev_trend") % var("trend"));
	s << format("%s = %s;") % var("outp") % s1.str();
    return s.str();
}

std::string SuperTrendUp::get_mql_tick()
{
	std::stringstream s;
	std::stringstream s1;
	s << SuperTrendSignal::get_mql_tick();
	s1 << (format("(/*(%s == -1) && */(%s == 1))") % var("prev_trend") % var("trend"));
	s << format("%s = %s;") % var("outp") % s1.str();
	return s.str();
}

void SuperTrendDown::compute_tick( const Env& env, int idx )
{
	SuperTrendSignal::compute_tick(env, idx);

	if (/*(prev_trend == 1) && */(trend == -1))
	{
		m_outp = true;
	}
	else
	{
		m_outp = false;
	}
}

std::string SuperTrendDown::get_el_tick()
{
	std::stringstream s;
	std::stringstream s1;
	s << SuperTrendSignal::get_el_tick();
	s1 << format("({(%s = 1) and }(%s = -1))") % var("prev_trend") % var("trend");
	s << format("%s = %s;") % var("outp") % (s1.str());
	return s.str();
}

std::string SuperTrendDown::get_mql_tick()
{
	std::stringstream s;
	std::stringstream s1;
	s << SuperTrendSignal::get_mql_tick();
	s1 << format("(/*(%s == 1) && */(%s == -1))") % var("prev_trend") % var("trend");
	s << format("%s = %s;") % var("outp") % (s1.str());
	return s.str();
}


#endif





