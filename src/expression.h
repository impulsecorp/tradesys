#ifndef expression_h__
#define expression_h__

#include <string>
#include "signals/base_signal.h"
#include "signals/times.h"
#include "signals/numeric.h"
#include "signals/feedback.h"
#include "signals/raw.h"
#include "signals/gaps.h"
#include "signals/crosses.h"
#include "signals/timesinarow.h"
#include "signals/happened.h"
#include "signals/ops.h"
#include "signals/stops.h"
#include "signals/ta.h"
#include "signals/supertrend.h"
#include "signals/candles.h"

#include "common.h"

Signal *EvalExpr(std::string a_expr, std::map<std::string, Signal *> a_variables);

Signal *EvalExpr_NoVars(std::string a_expr);
std::string remove_comments(std::string a_expr);

#endif // expression_h__
