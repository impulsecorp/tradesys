#include "timesinarow.h"


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"


TimesInRow::TimesInRow(ParameterMap &ow, std::vector<Signal *> ch)
{
    INIT_SIGNAL(RT_INT, "TIMES_IN_ROW");
}

void TimesInRow::init_codegen_variables()
{
    m_variables["count"] = VAR_TYPE_INT;
}

void TimesInRow::compute_init( const Env& env, int idx )
{
    count = 0;
}

void TimesInRow::compute_tick( const Env& env, int idx )
{
    if (get<bool>(m_children[0]->get_output()))
    {
        count += 1;
    }
    else
    {
        count = 0;
    }

	m_outp = count;
}


std::string TimesInRow::get_el_init()
{
    return var("count") + " = 0;";
}

std::string TimesInRow::get_el_tick()
{
    std::stringstream s;
    s << fmt("if ({0}) then "
         "    begin  "
         "        {count} = {count} + 1;  "
         "    end  "
         "    else  "
         "    begin  "
         "        {count} = 0;  "
         "    end;  "
         "  "
         "{outp} = {count};") % m_efd;

    return s.str();
}

std::string TimesInRow::get_mql_init()
{
    return var("count") + " = 0;";
}

std::string TimesInRow::get_mql_tick()
{
    std::stringstream s;
    s << fmt("if ({0})  "
         "    {bg}  "
         "        {count} += 1;  "
         "    {en}  "
         "    else  "
         "    {bg}  "
         "        {count} = 0;  "
         "    {en}  "
         "  "
         "    {outp} = {count};") % m_mfd;

    return s.str();
}

ParameterConstraintMap StreakEnded::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;
    SP_constraints t_N;
    t_N.usage_mode = CNT_MINMAX;
    t_N.min = 0;
    t_N.max = 50;
    t_N.mut_min = 0;
    t_N.mut_max = 10;
    t_N.mut_power = 3;
    t_N.is_mutable = true;
    t_N.default_value = 1;
    t_constraints["n"] = t_N;
    return t_constraints;
}

void StreakEnded::init_codegen_variables()
{
    m_variables["count"] = VAR_TYPE_INT;
    m_variables["prev_count"] = VAR_TYPE_INT;
    m_variables["ended"] = VAR_TYPE_BOOL;
}

void StreakEnded::update_data( const DataMatrix& data )
{
    n = convert_to_int(m_parameters["n"]);
}

void StreakEnded::compute_init( const Env& env, int idx )
{
    count = 0;
    prev_count = 0;
    ended = false;
}

void StreakEnded::compute_tick( const Env& env, int idx )
{
    prev_count = count;
    if (get<bool>(m_children[0]->get_output()))
        count += 1;
    else
        count = 0;
    
    ended = (prev_count > n) && (count == 0);

	m_outp = ended;
}

std::string StreakEnded::get_el_init()
{
    std::stringstream s;
    s << format("%s = 0; %s = 0; %s = false;")
        % var("count") % var("prev_count") % var("ended");

    return s.str();
}

std::string StreakEnded::get_el_tick()
{
    std::stringstream s;
    s << fmt("{prev_count} = {count};  "
         "    if ({0}) then "
         "        {count} = {count} + 1  "
         "    else  "
         "        {count} = 0;  "
         "  "
         "    {ended} = ({prev_count} > {n}) and ({count} = 0);  "
         "  "
         "    {outp} = {ended};") % m_efd;

    return s.str();
}

std::string StreakEnded::get_mql_init()
{
    std::stringstream s;
    s << format("%s = 0; %s = 0; %s = false;")
        % var("count") % var("prev_count") % var("ended");

    return s.str();
}

std::string StreakEnded::get_mql_tick()
{
    std::stringstream s;
    s << fmt("{prev_count} = {count};  "
         "    if ({0})  "
         "        {count} = {count}+1;  "
         "    else  "
         "        {count} = 0;  "
         "      "
         "    {ended} = ({prev_count} > {n}) && ({count} == 0);  "
         "  "
         "    {outp} = {ended};") % m_mfd;

    return s.str();
}



ParameterConstraintMap StreakEndedPriceDiff::get_initial_constraints()
{
	ParameterConstraintMap t_constraints;
    SP_constraints t_N;
    t_N.usage_mode = CNT_MINMAX;
    t_N.min = 0;
    t_N.max = 50;
    t_N.mut_min = 0;
    t_N.mut_max = 10;
    t_N.mut_power = 3;
    t_N.is_mutable = true;
    t_N.default_value = 1;
    t_constraints["n"] = t_N;
    return t_constraints;
}

void StreakEndedPriceDiff::init_codegen_variables()
{
    m_variables["count"] = VAR_TYPE_INT;
    m_variables["prev_count"] = VAR_TYPE_INT;
    m_variables["ended"] = VAR_TYPE_BOOL;
    m_variables["price_bef"] = VAR_TYPE_FLOAT;
    m_variables["price_now"] = VAR_TYPE_FLOAT;
    m_variables["diff"] = VAR_TYPE_FLOAT;
    m_variables["prev_price"] = VAR_TYPE_FLOAT;
}

void StreakEndedPriceDiff::update_data( const DataMatrix& data )
{
    cl = &(data.close);
    n = convert_to_int(m_parameters["n"]);
}

void StreakEndedPriceDiff::compute_init( const Env& env, int idx )
{
    count = 0;
    prev_count = 0;
    ended = false;

    price_bef = 0;
    prev_price = 0;
    diff = 0;
    price_now = (*cl)[idx];
}

void StreakEndedPriceDiff::compute_tick( const Env& env, int idx )
{
    prev_count = count;
    prev_price = price_bef;

    if (get<bool>(m_children[0]->get_output()))
    {
        count += 1;
        price_now = (*cl)[idx];
    }
    else
    {
        count = 0;
        price_bef = (*cl)[idx];
        price_now = (*cl)[idx];
    }

    if ((prev_count > n) && (count == 0))
    {
        ended = true;
        diff = ABS((*cl)[idx-1] - prev_price);
    }
    else
    {
        ended = false;
        diff = 0;
    }

	m_outp = diff;
}


std::string StreakEndedPriceDiff::get_el_init()
{
    std::stringstream s;
    s << format("%s = 0; %s = 0; %s = false;")
        % var("count") % var("prev_count") % var("ended") << 

        format("%s = 0; %s = 0; %s = 0; %s = close[0];")
        % var("prev_price") % var("diff")
        % var("price_bef") % var("price_now");

    return s.str();
}

std::string StreakEndedPriceDiff::get_el_tick()
{
    std::stringstream s;
    s << fmt("    {prev_count} = {count};  "
             "    {prev_price} = {price_bef};  "
             "  "
             "    if ({0}) then "
             "    begin  "
             "        {count} += 1;  "
             "        {price_now} = Close[0];  "
             "    end  "
             "    else  "
             "    begin  "
             "        {count} = 0;  "
             "        {price_bef} = Close[0];  "
             "        {price_now} = Close[0];  "
             "    end;  "
             "  "
             "    if (({prev_count} > {n}) and ({count} = 0)) then "
             "    begin  "
             "        {ended} = true;  "
             "        {diff} = Absvalue(Close[1] - {prev_price});  "
             "    end  "
             "    else  "
             "    begin  "
             "        {ended} = false;  "
             "        {diff} = 0;  "
             "    end;  "
             "  "
             "    {outp} = {diff};") % m_efd;

    return s.str();
}

std::string StreakEndedPriceDiff::get_mql_init()
{
    std::stringstream s;
    s << format("%s = 0; %s = 0; %s = false;")
        % var("count") % var("prev_count") % var("ended") << 

        format("%s = 0; %s = 0; %s = 0; %s = Close[0];")
        % var("prev_price") % var("diff")
        % var("price_bef") % var("price_now");

    return s.str();
}

std::string StreakEndedPriceDiff::get_mql_tick()
{
    std::stringstream s;
    s << fmt("    {prev_count} = {count};  "
             "    {prev_price} = {price_bef};  "
             "  "
             "    if ({0})  "
             "    {bg}  "
             "        {count} += 1;  "
             "        {price_now} = Close[0];  "
             "    {en}  "
             "    else  "
             "    {bg}  "
             "        {count} = 0;  "
             "        {price_bef} = Close[0];  "
             "        {price_now} = Close[0];  "
             "    {en}  "
             "  "
             "    if (({prev_count} > {n}) && ({count} == 0))  "
             "    {bg}  "
             "        {ended} = true;  "
             "        {diff} = MathAbs(Close[1] - {prev_price});  "
             "    {en}  "
             "    else  "
             "    {bg}  "
             "        {ended} = false;  "
             "        {diff} = 0;  "
             "    {en}  "
             "  "
             "    {outp} = {diff};") % m_mfd;

    return s.str();
}

void StreakLength::compute_tick( const Env& env, int idx )
{
	StreakEnded::compute_tick(env, idx);

	m_outp = prev_count;
}

std::string StreakLength::get_el_tick()
{
	std::stringstream s;
	s << StreakEnded::get_el_tick();
	s << format("%s = %s;") % var("outp") %  var("prev_count");
	return s.str();
}

std::string StreakLength::get_mql_tick()
{
	std::stringstream s;
	s << StreakEnded::get_mql_tick();
	s << format("%s = %s;") % var("outp") %  var("prev_count");
	return s.str();
}

void TimesHappened::init_codegen_variables()
{
    m_variables["count"] = VAR_TYPE_INT;
}

void TimesHappened::compute_init(const Env& env, int idx)
{
	count = 0;
}

void TimesHappened::compute_tick(const Env& env, int idx)
{
	if (get<bool>(m_children[0]->get_output()))
	{
		count += 1;
	}

	m_outp = count;
}

std::string TimesHappened::get_el_init()
{
    std::stringstream s;
    
    s << fmt("{count} = 0;"
             ) % m_efd;

	return s.str();
}

std::string TimesHappened::get_el_tick()
{
    std::stringstream s;

    s << fmt("if ({0}) then   "
             "begin   "
             "  {count} = {count} + 1;  "
             "end;   "
             "  "
             "{outp} = {count}; "
            ) % m_efd;

    return s.str();
}

std::string TimesHappened::get_mql_init()
{
    std::stringstream s;

    s << fmt("{count} = 0;"
            ) % m_mfd;

    return s.str();
}

std::string TimesHappened::get_mql_tick()
{
    std::stringstream s;

    s << fmt("if ({0})    "
             "{bg}   "
             "  {count} = {count} + 1;  "
             "{en}   "
             "  "
             "{outp} = {count}; "
             ) % m_mfd;

    return s.str();
}

void TimesHappenedSinceLH::init_codegen_variables()
{
	m_variables["count"] = VAR_TYPE_INT;
}

void TimesHappenedSinceLH::compute_init(const Env& env, int idx)
{
	count = 0;
}

void TimesHappenedSinceLH::compute_tick(const Env& env, int idx)
{
	if (get<bool>(m_children[0]->get_output()))
	{
		count += 1;
	}

	if (get<bool>(m_children[1]->get_output()) && !(get<bool>(m_children[0]->get_output())))
	{
		count = 0;
	}

	m_outp = count;
}

std::string TimesHappenedSinceLH::get_el_init()
{
    std::stringstream s;
    
    s << fmt("{count} = 0; "
            ) % m_efd;
    
    return s.str();
}

std::string TimesHappenedSinceLH::get_el_tick()
{
    std::stringstream s;
    
    s << fmt("if ({0}) then   "
             "begin   "
             "  {count} = {count} + 1;  "
             "end;   "
             "  "
             "if ({1}) and (not ({0})) then   "
             "begin   "
             "  {count} = 0;  "
             "end;   "
             "  "
             "{outp} = {count};"
            ) % m_efd;
    
    return s.str();
}

std::string TimesHappenedSinceLH::get_mql_init()
{
    std::stringstream s;
    
    s << fmt("{count} = 0; "
    ) % m_mfd;
    
    return s.str();
}

std::string TimesHappenedSinceLH::get_mql_tick()
{
    std::stringstream s;
    
    s << fmt(" if ({0}) "
             " {bg}  "
             "  {count} += 1;  "
             " {en}  "
             "  "
             " if (({1}) && (!({0})))  "
             " {bg}  "
             "  {count} = 0;  "
             " {en}  "
             "  "
             " {outp} = {count};"
    ) % m_mfd;
    
    return s.str();
}

void TimesHappenedSinceFH::init_codegen_variables()
{
	m_variables["count"] = VAR_TYPE_INT;
	m_variables["prev"] = VAR_TYPE_BOOL;
}

void TimesHappenedSinceFH::compute_init(const Env& env, int idx)
{
	count = 0;
	prev = false;
}

void TimesHappenedSinceFH::compute_tick(const Env& env, int idx)
{
	if (get<bool>(m_children[0]->get_output()))
	{
		count += 1;
	}

	if (get<bool>(m_children[1]->get_output()) && (!prev))
	{
		count = 0;
	}
	prev = get<bool>(m_children[1]->get_output());

	m_outp = count;
}

std::string TimesHappenedSinceFH::get_el_init()
{
    std::stringstream s;
    
    s << fmt("{count} = 0; "
             "{prev} = false; "
    ) % m_efd;
    
    return s.str();
}

std::string TimesHappenedSinceFH::get_el_tick()
{
    std::stringstream s;
    
    s << fmt("    if ({0}) then "
             "    begin  "
             "        {count} = {count} + 1;  "
             "    end;  "
             "  "
             "    if ({1} and (not {prev})) then  "
             "    begin  "
             "        {count} = 0;  "
             "    end;  "
             "    {prev} = {1};  "
             "  "
             "    {outp} = {count};"
    ) % m_efd;
    
    return s.str();
}

std::string TimesHappenedSinceFH::get_mql_init()
{
    std::stringstream s;
    
    s << fmt("{count} = 0; "
             "{prev} = false; "
    ) % m_mfd;
    
    return s.str();

}

std::string TimesHappenedSinceFH::get_mql_tick()
{
    std::stringstream s;
    
    s << fmt("    if ({0})  "
             "    {bg}  "
             "        {count} += 1;  "
             "    {en}  "
             "  "
             "    if ({1} && (!{prev}))  "
             "    {bg}  "
             "        {count} = 0;  "
             "    {en}  "
             "    {prev} = {1};  "
             "  "
             "    {outp} = {count};"
    ) % m_mfd;
    
    return s.str();
}


#pragma clang diagnostic pop