#include "feedback.h"


void EnteredToday::init_codegen_variables()
{
    // Define used variables for code generation
    m_variables["et"] = VAR_TYPE_BOOL;
    m_variables["prev_p"] = VAR_TYPE_INT;
    m_variables["cur_p"] = VAR_TYPE_INT;
}


void EnteredToday::update_data( const DataMatrix& data )
{
    dt = &(data.dt);
}

void EnteredToday::compute_init( const Env& env, int idx )
{
    et = false;
    prev_p = NO_POSITION;
    cur_p = env.current_position;
}

void EnteredToday::compute_tick( const Env& env, int idx )
{
	//std::cout << m_id << " " << (*dt)[idx].print_str() << " "
	//		  << (*dt)[prev_idx].print_str() << " "
	//		  << et << " " << env.current_position << " " << idx << " " << prev_idx << "\n";
    if ( (*dt)[idx].date() != (*dt)[prev_idx].date() )
    {
        et = false;
    }
    prev_idx = idx;

    prev_p = cur_p;
    cur_p = env.current_position;

    if ((prev_p == NO_POSITION) && (cur_p != NO_POSITION))
    {
        et = true;
    }
    if ((prev_p == SHORT_POSITION) && (cur_p == LONG_POSITION))
    {
        et = true;
    }
    if ((prev_p == LONG_POSITION) && (cur_p == SHORT_POSITION))
    {
        et = true;
    }

    m_outp = et;
}


std::string EnteredToday::get_el_init()
{
    std::stringstream s;
    s << var("et") << " = false; " 
      << var("prev_p") << " = 0; " 
      << var("cur_p") << " = Marketposition; ";
    return s.str();
}

std::string EnteredToday::get_el_tick()
{
    std::stringstream s;
    s << format(
        "if Date[0] <> Date[1] then "
        "   %s = false; "
        "%s = %s;"
        "%s = Marketposition;"
        "if (%s = 0) and (%s <> 0) then "
        "    %s = true;"
        "if (%s = -1) and (%s = 1) then "
        "    %s = true;"
        "if (%s = 1) and (%s = -1) then "
        "    %s = true;")

        % var("et") 
        % var("prev_p") % var("cur_p") 
        % var("cur_p") 
        % var("prev_p") % var("cur_p") 
        % var("et") 
        % var("prev_p") % var("cur_p") 
        % var("et") 
        % var("prev_p") % var("cur_p") 
        % var("et");

    s << var("outp") << " = " << var("et") << ";";

    return s.str();
}


std::string EnteredToday::get_mql_init()
{
    std::stringstream s;
    s << var("et") << " = false; " 
      << var("prev_p") << " = 0; " 
      << var("cur_p") << " = MarketPosition(); ";
    return s.str();
}

std::string EnteredToday::get_mql_tick()
{
    std::stringstream s;
    s << format(
        "if (TimeDay(Time[0]) != TimeDay(Time[1])) "
        "{ %s = false; }"
        "%s = %s;"
        "%s = MarketPosition();"
        "if ((%s == 0) && (%s != 0)) "
        "{ %s = true; }"
        "if ((%s == -1) && (%s == 1)) "
        "{ %s = true; }"
        "if ((%s == 1) && (%s == -1)) "
        "{ %s = true; }")

        % var("et") % var("prev_p") % var("cur_p") 
        % var("cur_p") 
        % var("prev_p") % var("cur_p") 
        % var("et") 
        % var("prev_p") % var("cur_p") 
        % var("et") 
        % var("prev_p") % var("cur_p") 
        % var("et");

    s << var("outp") << " = " << var("et") << ";";

    return s.str();
}


void ExitedToday::init_codegen_variables()
{
    // Define used variables for code generation
    m_variables["et"] = VAR_TYPE_BOOL;
    m_variables["prev_p"] = VAR_TYPE_INT;
    m_variables["cur_p"] = VAR_TYPE_INT;
}


void ExitedToday::update_data( const DataMatrix& data )
{
    dt = &(data.dt);
}

void ExitedToday::compute_init( const Env& env, int idx )
{
    et = false;
    prev_p = NO_POSITION;
    cur_p = env.current_position;
}

void ExitedToday::compute_tick( const Env& env, int idx )
{
    if ( (*dt)[idx].date() != (*dt)[prev_idx].date() )
        et = false;
    prev_idx = idx;

    prev_p = cur_p;
    cur_p = env.current_position;

    if ((prev_p != NO_POSITION) && (cur_p == NO_POSITION))
        et = true;
    if ((prev_p == SHORT_POSITION) && (cur_p == LONG_POSITION))
        et = true;
    if ((prev_p == LONG_POSITION) && (cur_p == SHORT_POSITION))
        et = true;

	m_outp = et;
}

std::string ExitedToday::get_el_init()
{
    std::stringstream s;
    s << var("et") << " = false; " 
      << var("prev_p") << " = 0; " 
      << var("cur_p")  << " = Marketposition; ";
    return s.str();
}

std::string ExitedToday::get_el_tick()
{
    std::stringstream s;
    s << format(
        "if Date[0] <> Date[1] then "
        "   %s = false; "
        "%s = %s;"
        "%s = Marketposition;"
        "if (%s <> 0) and (%s = 0) then "
        "    %s = true;"
        "if (%s = -1) and (%s = 1) then "
        "    %s = true;"
        "if (%s = 1) and (%s = -1) then "
        "    %s = true;")

        % var("et") % var("prev_p") % var("cur_p") 
        % var("cur_p") 
        % var("prev_p") % var("cur_p") 
        % var("et") 
        % var("prev_p") % var("cur_p") 
        % var("et") 
        % var("prev_p") % var("cur_p") 
        % var("et");

    s << var("outp") << " = " << var("et") << ";";

    return s.str();
}

std::string ExitedToday::get_mql_init()
{
    std::stringstream s;
    s << var("et") << " = false; " 
      << var("prev_p") << " = 0; " 
      << var("cur_p")  << " = MarketPosition(); ";
    return s.str();
}

std::string ExitedToday::get_mql_tick()
{
    std::stringstream s;
    s << format(
        "if (TimeDay(Time[0]) != TimeDay(Time[1])) "
        "{ %s = false; }"
        "%s = %s;"
        "%s = MarketPosition();"
        "if ((%s != 0) && (%s == 0)) "
        "{ %s = true; }"
        "if ((%s == -1) && (%s == 1)) "
        "{ %s = true; }"
        "if ((%s == 1) && (%s == -1)) "
        "{ %s = true; }")

        % var("et") % var("prev_p") % var("cur_p") 
        % var("cur_p") 
        % var("prev_p") % var("cur_p") 
        % var("et") 
        % var("prev_p") % var("cur_p") 
        % var("et") 
        % var("prev_p") % var("cur_p") 
        % var("et");

    s << var("outp") << " = " << var("et") << ";";

    return s.str();
}


void BarsSinceFirstEvent::init_codegen_variables()
{
    // Define used variables for code generation
    m_variables["prev_p"] = VAR_TYPE_BOOL;
    m_variables["cur_p"] = VAR_TYPE_BOOL;
    m_variables["counter"] = VAR_TYPE_INT;
    m_variables["counting"] = VAR_TYPE_BOOL;
    m_variables["happened"] = VAR_TYPE_BOOL;
}

void BarsSinceFirstEvent::update_data( const DataMatrix& data )
{
    dt = &(data.dt);
}

void BarsSinceFirstEvent::compute_init( const Env& env, int idx )
{
    prev_p = false;
    if (m_children.size() > 0)
    {
    	cur_p = get<bool>(m_children[0]->get_output());
    }
    else
    {
    	cur_p = false;
    }
    counting = false;
    counter = -1;
}

void BarsSinceFirstEvent::compute_tick( const Env& env, int idx )
{
    prev_p = cur_p;
    if (m_children.size() > 0)
    {
    	cur_p = get<bool>(m_children[0]->get_output());
    }
    else
    {
    	cur_p = false;
    }

    if (counting)
    {
        counter++;
    }

    if ((prev_p == false) && (cur_p == true))
    {
        counting = true;
        counter = 0;
    }

	m_outp = counter;
}

std::string BarsSinceFirstEvent::get_el_init()
{
    std::stringstream s;
    if (m_children.size() > 0)
    {
		s << format(
			"%s = false;"
			"%s = %s;"
			"%s = false;"
			"%s = -1;"
			)
			% var("prev_p")
			% var("cur_p") % (m_children[0]->get_el_output())
			% var("counting")
			% var("counter");
    }
    else
    {
		s << format(
			"%s = false;"
			"%s = %s;"
			"%s = false;"
			"%s = -1;"
			)
			% var("prev_p")
			% var("cur_p") % (false)
			% var("counting")
			% var("counter");
    }

    return s.str();
}

std::string BarsSinceFirstEvent::get_el_tick()
{
    std::stringstream s;
    s << format(
        "%s = %s;"
        "%s = %s;"
        
        "if (%s) then "
        "begin "
        "    %s = %s + 1;"
        "end; "
        
        "if ((not %s) and (%s)) then "
        "begin "
        "   %s = true;"
        "   %s = 0;"
        "end; "
        )
        % var("prev_p") % var("cur_p")
        % var("cur_p") % (m_children[0]->get_el_output())
        % var("counting")
        % var("counter") % var("counter")
        % var("prev_p") % var("cur_p")
        % var("counting")
        % var("counter");

    s << var("outp") << " = " << var("counter") << ";";

    return s.str();
}

std::string BarsSinceFirstEvent::get_mql_init()
{
    std::stringstream s;
    s << format(
        "%s = false;"
        "%s = %s;"
        "%s = false;"
        "%s = -1;"
        )
        % var("prev_p") 
        % var("cur_p") % (m_children[0]->get_mql_output())
        % var("counting") 
        % var("counter");

    return s.str();
}

std::string BarsSinceFirstEvent::get_mql_tick()
{
    std::stringstream s;
    s << format(
        "%s = %s;"
        "%s = %s;"

        "if (%s) "
        "{ "
        "    %s = %s + 1;"
        "} "

        "if ((!(%s) ) && (%s))  "
        "{ "
        "   %s = true;"
        "   %s = 0;"
        "} "
        )
        % var("prev_p") % var("cur_p")
        % var("cur_p") % (m_children[0]->get_mql_output())
        % var("counting")
        % var("counter") % var("counter")
        % var("prev_p") % var("cur_p")
        % var("counting")
        % var("counter");

    s << format("%s = %s;") % var("outp") % var("counter");

    return s.str();
}


void BarsSinceLastEvent::init_codegen_variables()
{
    // Define used variables for code generation
    m_variables["prev_p"] = VAR_TYPE_BOOL;
    m_variables["cur_p"] = VAR_TYPE_BOOL;
    m_variables["counter"] = VAR_TYPE_INT;
    m_variables["counting"] = VAR_TYPE_BOOL;
    m_variables["happened"] = VAR_TYPE_BOOL;
}

void BarsSinceLastEvent::update_data(const DataMatrix &data)
{
    dt = &(data.dt);
}

void BarsSinceLastEvent::compute_init(const Env &env, int idx)
{
    counter = -1;
    counting = false;
}

void BarsSinceLastEvent::compute_tick(const Env &env, int idx)
{
    if (counting)
    {
        counter++;
    }
    
    if (get<bool>(m_children[0]->get_output()))
    {
        counting = true;
        counter = 0;
    }
    
    m_outp = counter;
}

std::string BarsSinceLastEvent::get_el_init()
{
    return Signal::get_el_init();
}

std::string BarsSinceLastEvent::get_el_tick()
{
    return Signal::get_el_tick();
}

std::string BarsSinceLastEvent::get_mql_init()
{
    return Signal::get_mql_init();
}

std::string BarsSinceLastEvent::get_mql_tick()
{
    return Signal::get_mql_tick();
}


void BarsSinceEntry::init_codegen_variables()
{
    // Define used variables for code generation
    m_variables["prev_p"] = VAR_TYPE_INT;
    m_variables["cur_p"] = VAR_TYPE_INT;
    m_variables["counter"] = VAR_TYPE_INT;
    m_variables["counting"] = VAR_TYPE_BOOL;
}

void BarsSinceEntry::compute_init( const Env& env, int idx )
{
    counting = false;
    counter = -1;
    prev_p = NO_POSITION;
    cur_p = env.current_position;
}

void BarsSinceEntry::compute_tick( const Env& env, int idx )
{
    prev_p = cur_p;
    cur_p = env.current_position;

    if (counting)
    {
        counter++;
    }

    // if entered now, start counting
    if ((prev_p == NO_POSITION) && (cur_p != NO_POSITION))
    {
        counting = true;
        counter = 0;
    }
    // if exited now, stop counting
    if ((prev_p != NO_POSITION) && (cur_p == NO_POSITION))
    {
        counting = false;
        counter = -1;
    }

    // also reset/start the counter on position switching
    if (((prev_p == LONG_POSITION) && (cur_p == SHORT_POSITION)) || 
        ((prev_p == SHORT_POSITION) && (cur_p == LONG_POSITION)))
    {
        counting = true;
        counter = 0;
    }

	m_outp = counter;
}

std::string BarsSinceEntry::get_el_init()
{
    std::stringstream s;
    s << format(
        "%s = false;"
        "%s = -1;"
        "%s = 0;"
        "%s = MarketPosition;")

        % var("counting") 
        % var("counter") 
        % var("prev_p") 
        % var("cur_p");

    return s.str();
}

std::string BarsSinceEntry::get_el_tick()
{
    std::stringstream s;
    s << format(
        "%s = %s;"
        "%s = MarketPosition;"

        "if %s then %s = %s + 1;"
        "if (%s = 0) and (%s <> 0) then "
        "    begin "
        "    %s = true;"
        "    %s = 0;"
        "end;"
        "if (%s <> 0) and (%s = 0) then "
        "    begin "
        "    %s = false;"
        "    %s = -1;"
        "end;"

        "if (%s = -1) and (%s = 1) then "
        "    begin "
        "    %s = true;"
        "    %s = 0;"
        "end;"
        "if (%s = 1) and (%s = -1) then "
        "    begin "
        "    %s = true;"
        "    %s = 0;"
        "end;")

        % var("prev_p") % var("cur_p") 
        % var("cur_p") 
        % var("counting") % var("counter") % var("counter") 
        % var("prev_p") % var("cur_p") 
        % var("counting") 
        % var("counter") 
        % var("prev_p") % var("cur_p") 
        % var("counting") 
        % var("counter") 
        % var("prev_p") % var("cur_p") 
        % var("counting") 
        % var("counter") 
        % var("prev_p") % var("cur_p") 
        % var("counting") 
        % var("counter");

    s << format("%s = %s;") % var("outp") % var("counter");

    return s.str();
}


std::string BarsSinceEntry::get_mql_init()
{
    std::stringstream s;
    s << format(
        "%s = false;"
        "%s = -1;"
        "%s = 0;"
        "%s = MarketPosition();")

        % var("counting") 
        % var("counter") 
        % var("prev_p") 
        % var("cur_p");

    return s.str();
}

std::string BarsSinceEntry::get_mql_tick()
{
    std::stringstream s;
    s << format(
        "%s = %s;"
        "%s = MarketPosition();"

        "if (%s) { %s = %s + 1; }"
        "if ((%s == 0) && (%s != 0)) "
        "{ "
        "   %s = true;"
        "   %s = 0;"
        "} "
        "if ((%s != 0) && (%s == 0)) "
        "{ "
        "   %s = false;"
        "   %s = -1;"
        "} "

        "if ((%s == -1) && (%s == 1)) "
        "{ "
        "   %s = true;"
        "   %s = 0;"
        "} "
        "if ((%s == 1) && (%s == -1)) "
        "{ "
        "   %s = true;"
        "   %s = 0;"
        "} ")

        % var("prev_p") % var("cur_p") 
        % var("cur_p") 
        % var("counting") % var("counter") % var("counter") 
        % var("prev_p") % var("cur_p") 
        % var("counting") 
        % var("counter") 
        % var("prev_p") % var("cur_p") 
        % var("counting") 
        % var("counter") 
        % var("prev_p") % var("cur_p") 
        % var("counting") 
        % var("counter") 
        % var("prev_p") % var("cur_p") 
        % var("counting") 
        % var("counter");

    s << format("%s = %s;") % var("outp") % var("counter");

    return s.str();
}


void BarsSinceExit::init_codegen_variables()
{
    // Define used variables for code generation
    m_variables["prev_p"] = VAR_TYPE_INT;
    m_variables["cur_p"] = VAR_TYPE_INT;
    m_variables["counter"] = VAR_TYPE_INT;
    m_variables["counting"] = VAR_TYPE_BOOL;
}

void BarsSinceExit::compute_init( const Env& env, int idx )
{
    counting = false;
    counter = -1;
    prev_p = NO_POSITION;
    cur_p = env.current_position;
}

void BarsSinceExit::compute_tick( const Env& env, int idx )
{
    prev_p = cur_p;
    cur_p = env.current_position;

    if (counting)
        counter++;

    // if exited now, start counting
    if ((prev_p != NO_POSITION) && (cur_p == NO_POSITION))
    {
        counting = true;
        counter = 0;
    }
    // if entered now, stop counting
    if ((prev_p == NO_POSITION) && (cur_p != NO_POSITION))
    {
        counting = false;
        counter = -1;
    }

    // also reset/start the counter on position switching
    if (((prev_p == LONG_POSITION) && (cur_p == SHORT_POSITION)) || 
        ((prev_p == SHORT_POSITION) && (cur_p == LONG_POSITION)))
    {
        counting = true;
        counter = 0;
    }

	m_outp = counter;
}


std::string BarsSinceExit::get_el_init()
{
    std::stringstream s;
    s << format(
        "%s = false;"
        "%s = -1;"
        "%s = 0;"
        "%s = MarketPosition;")

        % var("counting") 
        % var("counter") 
        % var("prev_p") 
        % var("cur_p");

    return s.str();
}

std::string BarsSinceExit::get_el_tick()
{
    std::stringstream s;
    s << format(
    "%s = %s;"
    "%s = MarketPosition;"

    "if %s then %s = %s + 1;"
    "if (%s <> 0) and (%s = 0) then "
    "    begin "
    "    %s = true;"
    "%s = 0;"
    "end;"
    "if (%s = 0) and (%s <> 0) then "
    "    begin "
    "    %s = false;"
    "    %s = -1;"
    "end;"

    "if (%s = -1) and (%s = 1) then "
    "    begin "
    "    %s = true;"
    "    %s = 0;"
    "end;"
    "if (%s = 1) and (%s = -1) then "
    "    begin "
    "    %s = true;"
    "    %s = 0;"
    "end;")

    % var("prev_p") % var("cur_p") 
    % var("cur_p") 
    % var("counting") %  var("counter") %  var("counter") 
    % var("prev_p") %  var("cur_p") 
    % var("counting") 
    % var("counter") 
    % var("prev_p") %  var("cur_p") 
    % var("counting") 
    % var("counter") 
    % var("prev_p") %  var("cur_p") 
    % var("counting") 
    % var("counter") 
    % var("prev_p") %  var("cur_p") 
    % var("counting") 
    % var("counter");

    s << format("%s = %s;") % var("outp") % var("counter");

    return s.str();
}

std::string BarsSinceExit::get_mql_init()
{
    std::stringstream s;
    s << format(
        "%s = false;"
        "%s = -1;"
        "%s = 0;"
        "%s = MarketPosition();")

        % var("counting") 
        % var("counter") 
        % var("prev_p") 
        % var("cur_p");

    return s.str();
}

std::string BarsSinceExit::get_mql_tick()
{
    std::stringstream s;
    s << format(
        "%s = %s;"
        "%s = MarketPosition();"

        "if (%s) { %s = %s + 1; }"
        "if ((%s != 0) && (%s == 0)) "
        "{ "
        "   %s = true;"
        "   %s = 0;"
        "} "
        "if ((%s == 0) && (%s != 0)) "
        "{ "
        "   %s = false;"
        "   %s = -1;"
        "} "

        "if ((%s == -1) && (%s == 1)) "
        "{ "
        "   %s = true;"
        "   %s = 0;"
        "} "
        "if ((%s == 1) && (%s == -1)) "
        "{ "
        "   %s = true;"
        "   %s = 0;"
        "} ")

        % var("prev_p") % var("cur_p") 
        % var("cur_p") 
        % var("counting") %  var("counter") %  var("counter") 
        % var("prev_p") %  var("cur_p") 
        % var("counting") 
        % var("counter") 
        % var("prev_p") %  var("cur_p") 
        % var("counting") 
        % var("counter") 
        % var("prev_p") %  var("cur_p") 
        % var("counting") 
        % var("counter") 
        % var("prev_p") %  var("cur_p") 
        % var("counting") 
        % var("counter");

    s << format("%s = %s;") % var("outp") % var("counter");

    return s.str();
}


void DaysSinceEntry::init_codegen_variables()
{
    // Define used variables for code generation
    m_variables["prev_p"] = VAR_TYPE_INT;
    m_variables["cur_p"] = VAR_TYPE_INT;
    m_variables["counter"] = VAR_TYPE_INT;
    m_variables["counting"] = VAR_TYPE_BOOL;
}


void DaysSinceEntry::update_data( const DataMatrix& data )
{
    dt = &(data.dt);
}

void DaysSinceEntry::compute_init( const Env& env, int idx )
{
    counting = false;
    counter = -1;
    prev_p = NO_POSITION;
    cur_p = env.current_position;
    tbef = 0;
    tnow = (*dt)[idx].time().as_int();
}

void DaysSinceEntry::compute_tick( const Env& env, int idx )
{
    prev_p = cur_p;
    cur_p = env.current_position;

    if (counting && ( (*dt)[idx].date() != (*dt)[prev_idx].date() ))
    {
        counter++;
    }
    prev_idx = idx;

    // if entered now, start counting
    if ((prev_p == NO_POSITION) && (cur_p != NO_POSITION))
    {
        counting = true;
        counter = 0;
    }
    // if exited now, stop counting
    if ((prev_p != NO_POSITION) && (cur_p == NO_POSITION))
    {
        counting = false;
        counter = -1;
    }

    // also reset/start the counter on position switching
    if (((prev_p == LONG_POSITION) && (cur_p == SHORT_POSITION)) || 
        ((prev_p == SHORT_POSITION) && (cur_p == LONG_POSITION)))
    {
        counting = true;
        counter = 0;
    }


	m_outp = counter;
}

std::string DaysSinceEntry::get_el_init()
{
    std::stringstream s;
    s << format(
        "%s = false;"
        "%s = -1;"
        "%s = 0;"
        "%s = MarketPosition;")
        % var("counting") 
        % var("counter") 
        % var("prev_p") 
        % var("cur_p");
    return s.str();
}

std::string DaysSinceEntry::get_el_tick()
{
    std::stringstream s;
    s << format(
    "%s = %s;"
    "%s = MarketPosition;"

    "if (Date[0] <> Date[1]) and %s then %s = %s + 1;"

    "if (%s = 0) and (%s <> 0) then "
    "    begin "
    "    %s = true;"
    "    %s = 0;"
    "end;"
    "if (%s <> 0) and (%s = 0) then "
    "    begin "
    "    %s = false;"
    "    %s = -1;"
    "end;"

    "if (%s = -1) and (%s = 1) then "
    "    begin "
    "    %s = true;"
    "    %s = 0;"
    "end;"
    "if (%s = 1) and (%s = -1) then "
    "    begin "
    "    %s = true;"
    "    %s = 0;"
    "end;")
    % var("prev_p") %  var("cur_p") 
    % var("cur_p") 
    % var("counting") %  var("counter") %  var("counter") 
    % var("prev_p") %  var("cur_p") 
    % var("counting") 
    % var("counter") 
    % var("prev_p") %  var("cur_p") 
    % var("counting") 
    % var("counter") 
    % var("prev_p") %  var("cur_p") 
    % var("counting") 
    % var("counter") 
    % var("prev_p") %  var("cur_p") 
    % var("counting") 
    % var("counter");

    s << format("%s = %s;") % var("outp") % var("counter");

    return s.str();
}

std::string DaysSinceEntry::get_mql_init()
{
    std::stringstream s;
    s << format(
        "%s = false;"
        "%s = -1;"
        "%s = 0;"
        "%s = MarketPosition();")
        % var("counting") 
        % var("counter") 
        % var("prev_p") 
        % var("cur_p");
    return s.str();
}

std::string DaysSinceEntry::get_mql_tick()
{
    std::stringstream s;
    s << format(
        "%s = %s;"
        "%s = MarketPosition();"

        "if ((TimeDay(Time[0]) != TimeDay(Time[1])) && (%s)) { %s = %s + 1; }"

        "if ((%s == 0) && (%s != 0)) "
        "{ "
        "   %s = true;"
        "   %s = 0;"
        "} "
        "if ((%s != 0) && (%s == 0)) "
        "{ "
        "   %s = false;"
        "   %s = -1;"
        "} "

        "if ((%s == -1) && (%s == 1)) "
        "{ "
        "   %s = true;"
        "   %s = 0;"
        "} "
        "if ((%s == 1) && (%s == -1)) "
        "{ "
        "   %s = true;"
        "   %s = 0;"
        "} ")
        % var("prev_p") %  var("cur_p") 
        % var("cur_p") 
        % var("counting") %  var("counter") %  var("counter") 
        % var("prev_p") %  var("cur_p") 
        % var("counting") 
        % var("counter") 
        % var("prev_p") %  var("cur_p") 
        % var("counting") 
        % var("counter") 
        % var("prev_p") %  var("cur_p") 
        % var("counting") 
        % var("counter") 
        % var("prev_p") %  var("cur_p") 
        % var("counting") 
        % var("counter");

    s << format("%s = %s;") % var("outp") % var("counter");

    return s.str();
}


void DaysSinceExit::init_codegen_variables()
{
    // Define used variables for code generation
    m_variables["prev_p"] = VAR_TYPE_INT;
    m_variables["cur_p"] = VAR_TYPE_INT;
    m_variables["counter"] = VAR_TYPE_INT;
    m_variables["counting"] = VAR_TYPE_BOOL;
}


void DaysSinceExit::update_data( const DataMatrix& data )
{
    dt = &(data.dt);
}

void DaysSinceExit::compute_init( const Env& env, int idx )
{
    counting = false;
    counter = -1;
    prev_p = NO_POSITION;
    cur_p = env.current_position;
    tbef = 0;
    tnow = (*dt)[idx].time().as_int();
}

void DaysSinceExit::compute_tick( const Env& env, int idx )
{
    prev_p = cur_p;
    cur_p = env.current_position;

    if (counting && ( (*dt)[idx].date() != (*dt)[prev_idx].date() ))
        counter++;
    prev_idx = idx;

    // if exited now, start counting
    if ((prev_p != NO_POSITION) && (cur_p == NO_POSITION))
    {
        counting = true;
        counter = 0;
    }
    // if entered now, stop counting
    if ((prev_p == NO_POSITION) && (cur_p != NO_POSITION))
    {
        counting = false;
        counter = -1;
    }

    // also reset/start the counter on position switching
    if (((prev_p == LONG_POSITION) && (cur_p == SHORT_POSITION)) || 
        ((prev_p == SHORT_POSITION) && (cur_p == LONG_POSITION)))
    {
        counting = true;
        counter = 0;
    }

	m_outp = counter;
}

std::string DaysSinceExit::get_el_init()
{
    std::stringstream s;
    s << format(
        "%s = false;"
        "%s = -1;"
        "%s = 0;"
        "%s = MarketPosition;")

        % var("counting") 
        % var("counter") 
        % var("prev_p") 
        % var("cur_p");

    return s.str();
}

std::string DaysSinceExit::get_el_tick()
{
    std::stringstream s;
    s << format(
    "%s = %s;"
    "%s = MarketPosition;"

    "if (Date[0] <> Date[1]) and %s then %s = %s + 1;"

    "if (%s <> 0) and (%s = 0) then "
    "    begin "
    "    %s = true;"
    "    %s = 0;"
    "end;"
    "if (%s = 0) and (%s <> 0) then "
    "    begin "
    "    %s = false;"
    "    %s = -1;"
    "end;"

    "if (%s = -1) and (%s = 1) then "
    "    begin "
    "    %s = true;"
    "    %s = 0;"
    "end;"
    "if (%s = 1) and (%s = -1) then "
    "    begin "
    "    %s = true;"
    "    %s = 0;"
    "end;")

    % var("prev_p") % var("cur_p") 
    % var("cur_p") 
    % var("counting") % var("counter") % var("counter") 
    % var("prev_p") % var("cur_p") 
    % var("counting") 
    % var("counter") 
    % var("prev_p") % var("cur_p") 
    % var("counting") 
    % var("counter") 
    % var("prev_p") % var("cur_p") 
    % var("counting") 
    % var("counter") 
    % var("prev_p") % var("cur_p") 
    % var("counting") 
    % var("counter");

    s << format("%s = %s;") % var("outp") % var("counter");

    return s.str();
}

std::string DaysSinceExit::get_mql_init()
{
    std::stringstream s;
    s << format(
        "%s = false;"
        "%s = -1;"
        "%s = 0;"
        "%s = MarketPosition();")

        % var("counting") 
        % var("counter") 
        % var("prev_p") 
        % var("cur_p");

    return s.str();
}

std::string DaysSinceExit::get_mql_tick()
{
    std::stringstream s;
    s << format(
        "%s = %s;"
        "%s = MarketPosition();"

        "if ((TimeDay(Time[0]) != TimeDay(Time[1])) && (%s)) { %s = %s + 1; }"

        "if ((%s != 0) && (%s == 0)) "
        "{ "
        "   %s = true;"
        "   %s = 0;"
        "} "
        "if ((%s == 0) && (%s != 0)) "
        "{ "
        "   %s = false;"
        "   %s = -1;"
        "} "

        "if ((%s == -1) && (%s == 1)) "
        "{ "
        "   %s = true;"
        "   %s = 0;"
        "} "
        "if ((%s == 1) && (%s == -1)) "
        "{ "
        "   %s = true;"
        "   %s = 0;"
        "}")

        % var("prev_p") % var("cur_p") 
        % var("cur_p") 
        % var("counting") % var("counter") % var("counter") 
        % var("prev_p") % var("cur_p") 
        % var("counting") 
        % var("counter") 
        % var("prev_p") % var("cur_p") 
        % var("counting") 
        % var("counter") 
        % var("prev_p") % var("cur_p") 
        % var("counting") 
        % var("counter") 
        % var("prev_p") % var("cur_p") 
        % var("counting") 
        % var("counter");

    s << format("%s = %s;") % var("outp") % var("counter");

    return s.str();
}


void Buying::compute_tick( const Env& env, int idx )
{
	m_outp = (env.current_position == LONG_POSITION);
}

std::string Buying::get_el_tick()
{
	std::stringstream s;
    s << format("%s = %s;") % var("outp") % "(Marketposition=1)";
    return s.str();
}

std::string Buying::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % "(MarketPosition() == 1)";
	return s.str();
}


void Selling::compute_tick( const Env& env, int idx )
{
	m_outp = (env.current_position == SHORT_POSITION);
}

std::string Selling::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % "(Marketposition=-1)";
	return s.str();
}

std::string Selling::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % "(MarketPosition() == -1)";
	return s.str();
}


void InsideMarket::compute_tick( const Env& env, int idx )
{
	m_outp = (env.current_position != NO_POSITION);
}

std::string InsideMarket::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % "(Marketposition<>0)";
	return s.str();
}

std::string InsideMarket::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % "(MarketPosition() != 0)";
	return s.str();
}


void OutsideMarket::compute_tick( const Env& env, int idx )
{
	m_outp = (env.current_position == NO_POSITION);
}

std::string OutsideMarket::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % "(Marketposition=0)";
	return s.str();
}

std::string OutsideMarket::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % "(MarketPosition() == 0)";
	return s.str();
}

void StopLossHit::compute_tick( const Env& env, int idx )
{
	m_outp = m_parent_strategy->m_stop_loss_tree->get_output();
}

std::string StopLossHit::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % (m_parent_strategy->m_stop_loss_tree->get_el_output());
	return s.str();
}

std::string StopLossHit::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % (m_parent_strategy->m_stop_loss_tree->get_mql_output());
	return s.str();
}


void TakeProfitHit::compute_tick( const Env& env, int idx )
{
	m_outp = m_parent_strategy->m_take_profit_tree->get_output();
}

std::string TakeProfitHit::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % (m_parent_strategy->m_take_profit_tree->get_el_output());
	return s.str();
}

std::string TakeProfitHit::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % (m_parent_strategy->m_take_profit_tree->get_mql_output());
	return s.str();
}


void LastExitReason::compute_tick( const Env& env, int idx )
{
	m_outp = m_parent_strategy->m_last_exit_reason;
}

std::string LastExitReason::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % "(last_exit_reason)";
	return s.str();
}

std::string LastExitReason::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % "(last_exit_reason)";
	return s.str();
}


void CurrentProfit::compute_tick( const Env& env, int idx )
{
	m_outp = env.potential_profit;
}

std::string CurrentProfit::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % "(PositionProfit)";
	return s.str();
}

std::string CurrentProfit::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % "(PositionProfit())";
	return s.str();
}


void LastProfit::compute_tick( const Env& env, int idx )
{
	m_outp = m_parent_strategy->m_last_profit;
}

std::string LastProfit::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % "(last_profit)";
	return s.str();
}

std::string LastProfit::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % "(last_profit)";
	return s.str();
}
