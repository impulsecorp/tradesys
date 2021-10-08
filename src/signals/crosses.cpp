#include "crosses.h"

void CrossSignal::init_codegen_variables()
{
    m_variables["n1_bef"] = VAR_TYPE_FLOAT;
    m_variables["n1_now"] = VAR_TYPE_FLOAT;
    m_variables["n2_bef"] = VAR_TYPE_FLOAT;
    m_variables["n2_now"] = VAR_TYPE_FLOAT;
}

void CrossSignal::compute_init( const Env& env, int idx )
{
    n1_bef = 0;
    n1_now = 0;
    n2_bef = 0;
    n2_now = 0;
}

void CrossSignal::compute_tick( const Env& env, int idx )
{
    n1_bef = n1_now;
    n1_now = convert_to_float(m_children[0]->get_output());
    n2_bef = n2_now;
    n2_now = convert_to_float(m_children[1]->get_output());
}

std::string CrossSignal::get_el_init()
{
    std::stringstream s;
    s << Signal::get_el_init();

    Dict fd;
    fd("n1_bef", var("n1_bef"))
      ("n1_now", var("n1_now"))
	  ("n2_bef", var("n2_bef"))
	  ("n2_now", var("n2_now"))
	  ;

    s << fmt(
    		"{n1_bef} = 0;"
    		"{n1_now} = 0;"
    		"{n2_bef} = 0;"
    		"{n2_now} = 0;"
    		) % fd;

    return s.str();
}

std::string CrossSignal::get_el_tick()
{
    std::stringstream s;

    Dict fd;
    fd("n1_bef", var("n1_bef"))
      ("n1_now", var("n1_now"))
	  ("n2_bef", var("n2_bef"))
	  ("n2_now", var("n2_now"))
	  ("0", m_children[0]->get_el_output())
	  ("1", m_children[1]->get_el_output())
	  ;

    s << fmt(
       		"{n1_bef} = {n1_now};"
       		"{n1_now} = {0};"
       		"{n2_bef} = {n2_now};"
       		"{n2_now} = {1};"
       		) % fd;

    return s.str();
}

std::string CrossSignal::get_mql_init()
{
    std::stringstream s;
    s << Signal::get_mql_init();

    Dict fd;
    fd("n1_bef", var("n1_bef"))
      ("n1_now", var("n1_now"))
	  ("n2_bef", var("n2_bef"))
	  ("n2_now", var("n2_now"))
	  ;

    s << fmt(
    		"{n1_bef} = 0;"
    		"{n1_now} = 0;"
    		"{n2_bef} = 0;"
    		"{n2_now} = 0;"
    		) % fd;

    return s.str();
}

std::string CrossSignal::get_mql_tick()
{
    std::stringstream s;
    Dict fd;
    fd("n1_bef", var("n1_bef"))
      ("n1_now", var("n1_now"))
	  ("n2_bef", var("n2_bef"))
	  ("n2_now", var("n2_now"))
	  ("0", m_children[0]->get_mql_output())
	  ("1", m_children[1]->get_mql_output())
	  ;

    s << fmt(
       		"{n1_bef} = {n1_now};"
       		"{n1_now} = {0};"
       		"{n2_bef} = {n2_now};"
       		"{n2_now} = {1};"
       		) % fd;

    return s.str();
}

void Crosses::compute_tick( const Env& env, int idx )
{
	CrossSignal::compute_tick(env, idx);

    if  (((n1_bef != std::numeric_limits<double>::min()) &&
        (n1_bef != std::numeric_limits<double>::max()) &&

        (n2_bef != std::numeric_limits<double>::min()) &&
        (n2_bef != std::numeric_limits<double>::max()) &&

        (n1_now != std::numeric_limits<double>::min()) &&
        (n1_now != std::numeric_limits<double>::max()) &&

        (n2_now != std::numeric_limits<double>::min()) &&
        (n2_now != std::numeric_limits<double>::max()))

        &&
        
        (((n1_bef > n2_bef) && (n1_now < n2_now)) || ((n1_bef < n2_bef) && (n1_now > n2_now))))
    {
        m_outp = true;
    }
    else
    {
        m_outp = false;
    }
}


std::string Crosses::get_el_tick()
{
    std::stringstream s;

    s << CrossSignal::get_el_tick();

    Dict fd;
	fd("n1_bef", var("n1_bef"))
	  ("n1_now", var("n1_now"))
	  ("n2_bef", var("n2_bef"))
	  ("n2_now", var("n2_now"))
	  ("lmin", limits["min"])
	  ("lmax", limits["max"])
	  ("outp", var("outp"))
	  ;

    s << fmt(
    		"if  ((({n1_bef} <> {lmin}) and "
    		"    ({n1_bef} <> {lmax}) and "
    		""
    		"    ({n2_bef} <> {lmin}) and "
    		"    ({n2_bef} <> {lmax}) and "
    		""
    		"    ({n1_now} <> {lmin}) and "
    		"    ({n1_now} <> {lmax}) and "
    		""
    		"    ({n2_now} <> {lmin}) and "
    		"    ({n2_now} <> {lmax}))"
    		""
    		"    and "
    		""
    		"    ((({n1_bef} > {n2_bef}) and  ({n1_now} < {n2_now})) or (({n1_bef} < {n2_bef}) and  ({n1_now} > {n2_now})))) then "
    		"begin "
    		"    {outp} = true;"
    		"end "
    		"else "
    		"begin "
    		"    {outp} = false;"
    		"end; "
       		) % fd;

    return s.str();
}

std::string Crosses::get_mql_tick()
{
    std::stringstream s;

    s << CrossSignal::get_mql_tick();

    Dict fd;
	fd("n1_bef", var("n1_bef"))
	  ("n1_now", var("n1_now"))
	  ("n2_bef", var("n2_bef"))
	  ("n2_now", var("n2_now"))
	  ("lmin", limits["min"])
	  ("lmax", limits["max"])
	  ("outp", var("outp"))

	  // language specific
	  ("b", "{")
	  ("e", "}")
	  ;

    s << fmt(
    		"if  ((({n1_bef} != {lmin}) && "
    		"        ({n1_bef} != {lmax}) && "
    		" "
    		"        ({n2_bef} != {lmin}) && "
    		"        ({n2_bef} != {lmax}) && "
    		" "
    		"        ({n1_now} != {lmin}) && "
    		"        ({n1_now} != {lmax}) && "
    		" "
    		"        ({n2_now} != {lmin}) && "
    		"        ({n2_now} != {lmax})) "
    		" "
    		"        && "
    		" "
    		"        ((({n1_bef} > {n2_bef}) && ({n1_now} < {n2_now})) || (({n1_bef} < {n2_bef}) && ({n1_now} > {n2_now})))) "
    		"    {b}"
    		"        {outp} = true; "
    		"    {e} "
    		"    else "
    		"    {b} "
    		"        {outp} = false; "
    		"    {e} "
    		) % fd;

    return s.str();
}

void CrossesAbove::compute_tick( const Env& env, int idx )
{
	CrossSignal::compute_tick(env, idx);

    if  (((n1_bef != std::numeric_limits<double>::min()) &&
        (n1_bef != std::numeric_limits<double>::max()) &&

        (n2_bef != std::numeric_limits<double>::min()) &&
        (n2_bef != std::numeric_limits<double>::max()) &&

        (n1_now != std::numeric_limits<double>::min()) &&
        (n1_now != std::numeric_limits<double>::max()) &&

        (n2_now != std::numeric_limits<double>::min()) &&
        (n2_now != std::numeric_limits<double>::max()))

        &&

        ((n1_bef > n2_bef) && (n1_now <= n2_now)))
        m_outp = true;
    else
        m_outp = false;
}

std::string CrossesAbove::get_el_tick()
{
    std::stringstream s;
    s << CrossSignal::get_el_tick();

    Dict fd;
	fd("n1_bef", var("n1_bef"))
	  ("n1_now", var("n1_now"))
	  ("n2_bef", var("n2_bef"))
	  ("n2_now", var("n2_now"))
	  ("lmin", limits["min"])
	  ("lmax", limits["max"])
	  ("outp", var("outp"))
	  ;

    s << fmt(
    		"if  ((({n1_bef} <> {lmin}) and "
    		"    ({n1_bef} <> {lmax}) and "
    		""
    		"    ({n2_bef} <> {lmin}) and "
    		"    ({n2_bef} <> {lmax}) and "
    		""
    		"    ({n1_now} <> {lmin}) and "
    		"    ({n1_now} <> {lmax}) and "
    		""
    		"    ({n2_now} <> {lmin}) and "
    		"    ({n2_now} <> {lmax}))"
    		""
    		"    and "
    		""
    		"    (({n1_bef} > {n2_bef}) and ({n1_now} <= {n2_now}))) then "
    		"begin "
    		"    {outp} = true;"
    		"end "
    		"else "
    		"begin "
    		"    {outp} = false;"
    		"end; "
       		) % fd;

    return s.str();
}

std::string CrossesAbove::get_mql_tick()
{
    std::stringstream s;

    s << CrossSignal::get_mql_tick();

    Dict fd;
    	fd("n1_bef", var("n1_bef"))
    	  ("n1_now", var("n1_now"))
    	  ("n2_bef", var("n2_bef"))
    	  ("n2_now", var("n2_now"))
    	  ("lmin", limits["min"])
    	  ("lmax", limits["max"])
    	  ("outp", var("outp"))

		  // language specific
		  ("b", "{")
		  ("e", "}")
    	  ;

	s << fmt(
			"if  ((({n1_bef} != {lmin}) && "
			"        ({n1_bef} != {lmax}) && "
			" "
			"        ({n2_bef} != {lmin}) && "
			"        ({n2_bef} != {lmax}) && "
			" "
			"        ({n1_now} != {lmin}) && "
			"        ({n1_now} != {lmax}) && "
			" "
			"        ({n2_now} != {lmin}) && "
			"        ({n2_now} != {lmax})) "
			" "
			"        && "
			" "
			"        (({n1_bef} > {n2_bef}) && ({n1_now} <= {n2_now}))) "
			"    {b} "
			"        {outp} = true; "
			"    {e} "
			"    else "
			"    {b} "
			"        {outp} = false; "
			"    {e} "
			) % fd;

    return s.str();
}

void CrossesBelow::compute_tick( const Env& env, int idx )
{
	CrossSignal::compute_tick(env, idx);

    if  (((n1_bef != std::numeric_limits<double>::min()) &&
        (n1_bef != std::numeric_limits<double>::max()) &&

        (n2_bef != std::numeric_limits<double>::min()) &&
        (n2_bef != std::numeric_limits<double>::max()) &&

        (n1_now != std::numeric_limits<double>::min()) &&
        (n1_now != std::numeric_limits<double>::max()) &&

        (n2_now != std::numeric_limits<double>::min()) &&
        (n2_now != std::numeric_limits<double>::max()))

        &&

        ((n1_bef < n2_bef) && (n1_now >= n2_now)))
    	m_outp = true;
    else
    	m_outp = false;
}

std::string CrossesBelow::get_el_tick()
{
    std::stringstream s;

    s << CrossSignal::get_el_tick();

    Dict fd;
	fd("n1_bef", var("n1_bef"))
	  ("n1_now", var("n1_now"))
	  ("n2_bef", var("n2_bef"))
	  ("n2_now", var("n2_now"))
	  ("lmin", limits["min"])
	  ("lmax", limits["max"])
	  ("outp", var("outp"))
	  ;

    s << fmt(
    		"if  ((({n1_bef} <> {lmin}) and "
    		"    ({n1_bef} <> {lmax}) and "
    		""
    		"    ({n2_bef} <> {lmin}) and "
    		"    ({n2_bef} <> {lmax}) and "
    		""
    		"    ({n1_now} <> {lmin}) and "
    		"    ({n1_now} <> {lmax}) and "
    		""
    		"    ({n2_now} <> {lmin}) and "
    		"    ({n2_now} <> {lmax}))"
    		""
    		"    and "
    		""
    		"    (({n1_bef} < {n2_bef}) and ({n1_now} >= {n2_now}))) then "
    		"begin "
    		"    {outp} = true;"
    		"end "
    		"else "
    		"begin "
    		"    {outp} = false;"
    		"end; "
       		) % fd;

    return s.str();
}

std::string CrossesBelow::get_mql_tick()
{
    std::stringstream s;

    s << CrossSignal::get_mql_tick();

    Dict fd;
    	fd("n1_bef", var("n1_bef"))
    	  ("n1_now", var("n1_now"))
    	  ("n2_bef", var("n2_bef"))
    	  ("n2_now", var("n2_now"))
    	  ("lmin", limits["min"])
    	  ("lmax", limits["max"])
    	  ("outp", var("outp"))

		  // language specific
		  ("b", "{")
		  ("e", "}")
    	  ;

	s << fmt(
			"if  ((({n1_bef} != {lmin}) && "
			"        ({n1_bef} != {lmax}) && "
			" "
			"        ({n2_bef} != {lmin}) && "
			"        ({n2_bef} != {lmax}) && "
			" "
			"        ({n1_now} != {lmin}) && "
			"        ({n1_now} != {lmax}) && "
			" "
			"        ({n2_now} != {lmin}) && "
			"        ({n2_now} != {lmax})) "
			" "
			"        && "
			" "
			"        (({n1_bef} < {n2_bef}) && ({n1_now} >= {n2_now}))) "
			"    {b} "
			"        {outp} = true; "
			"    {e} "
			"    else "
			"    {b} "
			"        {outp} = false; "
			"    {e} "
			) % fd;

    return s.str();
}

