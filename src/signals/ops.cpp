#include "ops.h"

void AndOp::init_op()
{
    // don't allow other input types than bool
    if ((m_children[0]->m_return_type != RT_BOOL) || (m_children[1]->m_return_type != RT_BOOL))
        throw std::runtime_error("Children of AND ops must return bool only.");

    // determine return type
    m_return_type = RT_BOOL;
    init_codegen_outp();
}

void AndOp::compute_tick( const Env& env, int idx )
{
    ASS(m_children.size() == 2);
    ASS(m_children[0]->m_return_type == RT_BOOL);
    ASS(m_children[1]->m_return_type == RT_BOOL);

    ReturnVariant v;
    bool a = get<bool>(m_children[0]->get_output());
    bool b = get<bool>(m_children[1]->get_output());
    v = a && b;
    m_outp = v;
}

std::string AndOp::print_str()
{
    std::stringstream s;
    s << format("(%s) and (%s)")
        % m_children[0]->print_str() 
        % m_children[1]->print_str();
    return s.str();
}

std::string AndOp::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") %
			("(" + m_children[0]->get_el_output() + ") and (" +
                 m_children[1]->get_el_output() + ")");
	return s.str();
}

std::string AndOp::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_mql_output() + ") && (" +
        m_children[1]->get_mql_output() + ")");
	return s.str();
}

void OrOp::init_op()
{
    // don't allow other input types than bool
    if ((m_children[0]->m_return_type != RT_BOOL) || (m_children[1]->m_return_type != RT_BOOL))
        throw std::runtime_error("Children of OR ops must return bool only.");

    // determine return type
    m_return_type = RT_BOOL;
    init_codegen_outp();
}

void OrOp::compute_tick( const Env& env, int idx )
{
    ASS(m_children.size() == 2);
    ASS(m_children[0]->m_return_type == RT_BOOL);
    ASS(m_children[1]->m_return_type == RT_BOOL);

    ReturnVariant v;
    bool a = get<bool>(m_children[0]->get_output());
    bool b = get<bool>(m_children[1]->get_output());
    v = a || b;
    m_outp = v;
}

std::string OrOp::print_str()
{
    std::stringstream s;
    s << format("(%s) or (%s)")
        % m_children[0]->print_str() 
        % m_children[1]->print_str();
    return s.str();
}

std::string OrOp::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_el_output() + ") or (" +
        m_children[1]->get_el_output() + ")");
	return s.str();
}

std::string OrOp::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_mql_output() + ") || (" +
        m_children[1]->get_mql_output() + ")");
	return s.str();
}

void NotOp::init_op()
{
    // don't allow other input types than bool
    if (m_children[0]->m_return_type != RT_BOOL)
        throw std::runtime_error("Children of NOT ops must return bool only.");

    // determine return type
    m_return_type = RT_BOOL;
    init_codegen_outp();
}

void NotOp::compute_tick( const Env& env, int idx )
{
    ASS(m_children.size() == 1);
    ASS(m_children[0]->m_return_type == RT_BOOL);

    bool a = get<bool>(m_children[0]->get_output());
    m_outp = !a;
}

std::string NotOp::print_str()
{
    std::stringstream s;
    s << format("not (%s)")
        % m_children[0]->print_str();
    return s.str();
}

std::string NotOp::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(not " + m_children[0]->get_el_output() + ")");
	return s.str();
}

std::string NotOp::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(! (" + m_children[0]->get_mql_output() + "))");
	return s.str();
}

void AddOp::init_op()
{
    // don't allow other input types than int/double
    if (((m_children[0]->m_return_type != RT_INT) && (m_children[0]->m_return_type != RT_FLOAT)) || 
        ((m_children[1]->m_return_type != RT_INT) && (m_children[1]->m_return_type != RT_FLOAT)))
        throw std::runtime_error("Children of ADD ops must return int/double only.");

    // determine return type
    if ((m_children[0]->m_return_type == RT_FLOAT) || (m_children[1]->m_return_type == RT_FLOAT))
        m_return_type = RT_FLOAT;
    else 
        m_return_type = RT_INT;
    init_codegen_outp();
}

void AddOp::compute_tick( const Env& env, int idx )
{
    ASS(m_children.size() == 2);
    ASS((m_children[0]->m_return_type == RT_INT) || (m_children[0]->m_return_type == RT_FLOAT));
    ASS((m_children[1]->m_return_type == RT_INT) || (m_children[1]->m_return_type == RT_FLOAT));

    // if one of the types held is a double, the result will be double
    ReturnVariant a = m_children[0]->get_output();
    ReturnVariant b = m_children[1]->get_output();

    if ((a.type() == typeid(double)) && (b.type() == typeid(double)))
    {
    	m_outp = (double)(get<double>(a) + get<double>(b));
    }
    else
    if ((a.type() == typeid(double)) && (b.type() == typeid(int)))
    {
    	m_outp = (double)(get<double>(a) + get<int>(b));
    }
    else
    if ((a.type() == typeid(int)) && (b.type() == typeid(double)))
    {
    	m_outp = (double)(get<int>(a) + get<double>(b));
    }
    else
    if ((a.type() == typeid(int)) && (b.type() == typeid(int)))
    {
    	m_outp = (int)(get<int>(a) + get<int>(b));
    }
    else
    {
    	m_outp = 0; // can never happen
    }
 }

std::string AddOp::print_str()
{
    std::stringstream s;
    s << format("(%s) + (%s)")
        % m_children[0]->print_str() 
        % m_children[1]->print_str();
    return s.str();
}

std::string AddOp::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_el_output() + ") + ("
               + m_children[1]->get_el_output() + ")");
	return s.str();
}

std::string AddOp::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_mql_output() + ") + ("
        + m_children[1]->get_mql_output() + ")");
	return s.str();
}

void SubOp::init_op()
{
    // don't allow other input types than int/double
    if (((m_children[0]->m_return_type != RT_INT) && (m_children[0]->m_return_type != RT_FLOAT)) || 
        ((m_children[1]->m_return_type != RT_INT) && (m_children[1]->m_return_type != RT_FLOAT)))
        throw std::runtime_error("Children of SUB ops must return int/double only.");

    // determine return type
    if ((m_children[0]->m_return_type == RT_FLOAT) || (m_children[1]->m_return_type == RT_FLOAT))
        m_return_type = RT_FLOAT;
    else 
        m_return_type = RT_INT;
    init_codegen_outp();
}

void SubOp::compute_tick( const Env& env, int idx )
{
    ASS(m_children.size() == 2);
    ASS((m_children[0]->m_return_type == RT_INT) || (m_children[0]->m_return_type == RT_FLOAT));
    ASS((m_children[1]->m_return_type == RT_INT) || (m_children[1]->m_return_type == RT_FLOAT));

    // if one of the types held is a double, the result will be double
    ReturnVariant a = m_children[0]->get_output();
    ReturnVariant b = m_children[1]->get_output();

    if ((a.type() == typeid(double)) && (b.type() == typeid(double)))
    {
    	m_outp = (double)(get<double>(a) - get<double>(b));
    }
    else
    if ((a.type() == typeid(double)) && (b.type() == typeid(int)))
    {
    	m_outp = (double)(get<double>(a) - get<int>(b));
    }
    else
    if ((a.type() == typeid(int)) && (b.type() == typeid(double)))
    {
    	m_outp = (double)(get<int>(a) - get<double>(b));
    }
    else
    if ((a.type() == typeid(int)) && (b.type() == typeid(int)))
    {
    	m_outp = (int)(get<int>(a) - get<int>(b));
    }
    else
    {
    	m_outp = 0; // can never happen
    }
}

std::string SubOp::print_str()
{
    std::stringstream s;
    s << format("(%s) - (%s)")
        % m_children[0]->print_str() 
        % m_children[1]->print_str();
    return s.str();
}

std::string SubOp::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_el_output() + ") - ("
        + m_children[1]->get_el_output() + ")");
	return s.str();
}

std::string SubOp::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_mql_output() + ") - ("
        + m_children[1]->get_mql_output() + ")");
	return s.str();
}

void MulOp::init_op()
{
    // don't allow other input types than int/double
    if (((m_children[0]->m_return_type != RT_INT) && (m_children[0]->m_return_type != RT_FLOAT)) || 
        ((m_children[1]->m_return_type != RT_INT) && (m_children[1]->m_return_type != RT_FLOAT)))
        throw std::runtime_error("Children of MUL ops must return int/double only.");

    // determine return type
    if ((m_children[0]->m_return_type == RT_FLOAT) || (m_children[1]->m_return_type == RT_FLOAT))
        m_return_type = RT_FLOAT;
    else 
        m_return_type = RT_INT;
    init_codegen_outp();
}

void MulOp::compute_tick( const Env& env, int idx )
{
    ASS(m_children.size() == 2);
    ASS((m_children[0]->m_return_type == RT_INT) || (m_children[0]->m_return_type == RT_FLOAT));
    ASS((m_children[1]->m_return_type == RT_INT) || (m_children[1]->m_return_type == RT_FLOAT));

    // if one of the types held is a double, the result will be double
    ReturnVariant a = m_children[0]->get_output();
    ReturnVariant b = m_children[1]->get_output();

    if ((a.type() == typeid(double)) && (b.type() == typeid(double)))
    {
    	m_outp = (double)(get<double>(a) * get<double>(b));
    }
    else
    if ((a.type() == typeid(double)) && (b.type() == typeid(int)))
    {
    	m_outp = (double)(get<double>(a) * get<int>(b));
    }
    else
    if ((a.type() == typeid(int)) && (b.type() == typeid(double)))
    {
    	m_outp = (double)(get<int>(a) * get<double>(b));
    }
    else
    if ((a.type() == typeid(int)) && (b.type() == typeid(int)))
    {
    	m_outp = (int)(get<int>(a) * get<int>(b));
    }
    else
    {
    	m_outp = 0; // can never happen
    }
}

std::string MulOp::print_str()
{
    std::stringstream s;
    s << format("(%s) * (%s)")
        % m_children[0]->print_str() 
        % m_children[1]->print_str();
    return s.str();
}

std::string MulOp::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_el_output() + ") * ("
        + m_children[1]->get_el_output() + ")");
	return s.str();
}

std::string MulOp::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_mql_output() + ") * ("
        + m_children[1]->get_mql_output() + ")");
	return s.str();
}

void DivOp::init_op()
{
    // don't allow other input types than int/double
    if (((m_children[0]->m_return_type != RT_INT) && (m_children[0]->m_return_type != RT_FLOAT)) || 
        ((m_children[1]->m_return_type != RT_INT) && (m_children[1]->m_return_type != RT_FLOAT)))
        throw std::runtime_error("Children of DIV ops must return int/double only.");

    // determine return type
    if ((m_children[0]->m_return_type == RT_FLOAT) || (m_children[1]->m_return_type == RT_FLOAT))
        m_return_type = RT_FLOAT;
    else 
        m_return_type = RT_INT;
    init_codegen_outp();
}

void DivOp::compute_tick( const Env& env, int idx )
{
    ASS(m_children.size() == 2);
    ASS((m_children[0]->m_return_type == RT_INT) || (m_children[0]->m_return_type == RT_FLOAT));
    ASS((m_children[1]->m_return_type == RT_INT) || (m_children[1]->m_return_type == RT_FLOAT));

    // if one of the types held is a double, the result will be double
    ReturnVariant a = m_children[0]->get_output();
    ReturnVariant b = m_children[1]->get_output();

    if ((a.type() == typeid(double)) && (b.type() == typeid(double)))
    {
    	m_outp = (double)(get<double>(a) / get<double>(b));
    }
    else
    if ((a.type() == typeid(double)) && (b.type() == typeid(int)))
    {
    	m_outp = (double)(get<double>(a) / get<int>(b));
    }
    else
    if ((a.type() == typeid(int)) && (b.type() == typeid(double)))
    {
    	m_outp = (double)(get<int>(a) / get<double>(b));
    }
    else
    if ((a.type() == typeid(int)) && (b.type() == typeid(int)))
    {
    	m_outp = (int)(get<int>(a) / get<int>(b));
    }
    else
    {
    	m_outp = 0; // can never happen
    }
}

std::string DivOp::print_str()
{
    std::stringstream s;
    s << format("(%s) / (%s)")
        % m_children[0]->print_str() 
        % m_children[1]->print_str();
    return s.str();
}

std::string DivOp::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_el_output() + ") / ("
        + m_children[1]->get_el_output() + ")");
	return s.str();
}

std::string DivOp::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_mql_output() + ") / ("
        + m_children[1]->get_mql_output() + ")");
	return s.str();
}


///////////////////
void MmodOp::init_op()
{
    // don't allow other input types than int/double
    if (((m_children[0]->m_return_type != RT_INT) /*&& (m_children[0]->m_return_type != RT_FLOAT)*/) ||
        ((m_children[1]->m_return_type != RT_INT) /*&& (m_children[1]->m_return_type != RT_FLOAT)*/))
        throw std::runtime_error("Children of DIV ops must return int only.");
    
    // determine return type
    m_return_type = RT_INT;
    
    init_codegen_outp();
}

void MmodOp::compute_tick( const Env& env, int idx )
{
    // if one of the types held is a double, the result will be double
    ReturnVariant a = m_children[0]->get_output();
    ReturnVariant b = m_children[1]->get_output();
    
    if ((a.type() == typeid(int)) && (b.type() == typeid(int)))
    {
        m_outp = (int)(get<int>(a) % get<int>(b));
    }
    else
    {
        m_outp = 0; // can never happen
    }
}

std::string MmodOp::print_str()
{
    std::stringstream s;
    s << format("(%s) % (%s)")
         % m_children[0]->print_str()
         % m_children[1]->print_str();
    return s.str();
}

std::string MmodOp::get_el_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % ("Mod(" + m_children[0]->get_el_output() + " , "
                                             + m_children[1]->get_el_output() + ")");
    return s.str();
}

std::string MmodOp::get_mql_tick()
{
    std::stringstream s;
    s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_mql_output() + ") % ("
                                             + m_children[1]->get_mql_output() + ")");
    return s.str();
}
//////////////////

void EqOp::init_op()
{
    // allow all types through

    // return type is always bool
    m_return_type = RT_BOOL;
    init_codegen_outp();
}

void EqOp::compute_tick( const Env& env, int idx )
{
    ASS(m_children.size() == 2);
    ASS((m_children[0]->m_return_type == RT_INT) || (m_children[0]->m_return_type == RT_FLOAT)
        || (m_children[0]->m_return_type == RT_BOOL));
    ASS((m_children[1]->m_return_type == RT_INT) || (m_children[1]->m_return_type == RT_FLOAT)
        || (m_children[1]->m_return_type == RT_BOOL));
    ASS(m_children[0]->m_return_type == m_children[1]->m_return_type);

    ReturnVariant a = m_children[0]->get_output();
    ReturnVariant b = m_children[1]->get_output();

    if ((a.type() == typeid(int)) && (b.type() == typeid(int)))
    {
    	m_outp = (bool)(get<int>(a) == get<int>(b));
    }
    else
    if ((a.type() == typeid(double)) && (b.type() == typeid(int)))
    {
    	m_outp = (bool)(get<double>(a) == get<int>(b));
    }
    else
    if ((a.type() == typeid(int)) && (b.type() == typeid(double)))
    {
    	m_outp = (bool)(get<int>(a) == get<double>(b));
    }
    else
    if ((a.type() == typeid(double)) && (b.type() == typeid(double)))
    {
    	m_outp = (bool)(get<double>(a) == get<double>(b));
    }
    else
    if ((a.type() == typeid(bool)) && (b.type() == typeid(bool)))
    {
    	m_outp = (bool)(get<bool>(a) == get<bool>(b));
    }
    else
    {
    	m_outp = false;
    }
}

std::string EqOp::print_str()
{
    std::stringstream s;
    s << format("(%s) == (%s)")
        % m_children[0]->print_str() 
        % m_children[1]->print_str();
    return s.str();
}

std::string EqOp::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_el_output() + ") = ("
        + m_children[1]->get_el_output() + ")");
	return s.str();
}

std::string EqOp::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_mql_output() + ") == ("
        + m_children[1]->get_mql_output() + ")");
	return s.str();
}

void NeqOp::init_op()
{
    // allow all types through

    // return type is always bool
    m_return_type = RT_BOOL;
    init_codegen_outp();
}

void NeqOp::compute_tick( const Env& env, int idx )
{
    ASS(m_children.size() == 2);
    ASS((m_children[0]->m_return_type == RT_INT) || (m_children[0]->m_return_type == RT_FLOAT)
        || (m_children[0]->m_return_type == RT_BOOL));
    ASS((m_children[1]->m_return_type == RT_INT) || (m_children[1]->m_return_type == RT_FLOAT)
        || (m_children[1]->m_return_type == RT_BOOL));
    ASS(m_children[0]->m_return_type == m_children[1]->m_return_type);

    ReturnVariant a = m_children[0]->get_output();
    ReturnVariant b = m_children[1]->get_output();

    if ((a.type() == typeid(int)) && (b.type() == typeid(int)))
    {
    	m_outp = (bool)(get<int>(a) != get<int>(b));
    }
    else
    if ((a.type() == typeid(double)) && (b.type() == typeid(int)))
    {
    	m_outp = (bool)(get<double>(a) != get<int>(b));
    }
    else
    if ((a.type() == typeid(int)) && (b.type() == typeid(double)))
    {
    	m_outp = (bool)(get<int>(a) != get<double>(b));
    }
    else
    if ((a.type() == typeid(double)) && (b.type() == typeid(double)))
    {
    	m_outp = (bool)(get<double>(a) != get<double>(b));
    }
    else
    if ((a.type() == typeid(bool)) && (b.type() == typeid(bool)))
    {
    	m_outp = (bool)(get<bool>(a) != get<bool>(b));
    }
    else
    {
    	m_outp = false;
    }
}

std::string NeqOp::print_str()
{
    std::stringstream s;
    s << format("(%s) != (%s)")
        % m_children[0]->print_str() 
        % m_children[1]->print_str();
    return s.str();
}

std::string NeqOp::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_el_output() + ") <> ("
        + m_children[1]->get_el_output() + ")");
	return s.str();
}

std::string NeqOp::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_mql_output() + ") != ("
        + m_children[1]->get_mql_output() + ")");
	return s.str();
}

void GrOp::init_op()
{
    // don't allow other input types than int/double
    if (((m_children[0]->m_return_type != RT_INT) && (m_children[0]->m_return_type != RT_FLOAT)) || 
        ((m_children[1]->m_return_type != RT_INT) && (m_children[1]->m_return_type != RT_FLOAT)))
        throw std::runtime_error("Children of GR ops must return int/double only.");

    // return type is always bool
    m_return_type = RT_BOOL;
    init_codegen_outp();
}

void GrOp::compute_tick( const Env& env, int idx )
{
    ASS(m_children.size() == 2);
    ASS((m_children[0]->m_return_type == RT_INT) || (m_children[0]->m_return_type == RT_FLOAT));
    ASS((m_children[1]->m_return_type == RT_INT) || (m_children[1]->m_return_type == RT_FLOAT));
    ASS(m_children[0]->m_return_type == m_children[1]->m_return_type);

    ReturnVariant a = m_children[0]->get_output();
    ReturnVariant b = m_children[1]->get_output();

    if ((a.type() == typeid(int)) && (b.type() == typeid(int)))
    {
    	m_outp = (bool)(get<int>(a) > get<int>(b));
    }
    else
    if ((a.type() == typeid(double)) && (b.type() == typeid(int)))
    {
    	m_outp = (bool)(get<double>(a) > get<int>(b));
    }
    else
    if ((a.type() == typeid(int)) && (b.type() == typeid(double)))
    {
    	m_outp = (bool)(get<int>(a) > get<double>(b));
    }
    else
    if ((a.type() == typeid(double)) && (b.type() == typeid(double)))
    {
    	m_outp = (bool)(get<double>(a) > get<double>(b));
    }
    else
    {
    	m_outp = false;
    }
}

std::string GrOp::print_str()
{
    std::stringstream s;
    s << format("(%s) > (%s)")
        % m_children[0]->print_str() 
        % m_children[1]->print_str();
    return s.str();
}

std::string GrOp::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_el_output() + ") > ("
        + m_children[1]->get_el_output() + ")");
	return s.str();
}

std::string GrOp::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_mql_output() + ") > ("
        + m_children[1]->get_mql_output() + ")");
	return s.str();
}

void GreOp::init_op()
{
    // don't allow other input types than int/double
    if (((m_children[0]->m_return_type != RT_INT) && (m_children[0]->m_return_type != RT_FLOAT)) || 
        ((m_children[1]->m_return_type != RT_INT) && (m_children[1]->m_return_type != RT_FLOAT)))
        throw std::runtime_error("Children of GRE ops must return int/double only.");

    // return type is always bool
    m_return_type = RT_BOOL;
    init_codegen_outp();
}

void GreOp::compute_tick( const Env& env, int idx )
{
    ASS(m_children.size() == 2);
    ASS((m_children[0]->m_return_type == RT_INT) || (m_children[0]->m_return_type == RT_FLOAT));
    ASS((m_children[1]->m_return_type == RT_INT) || (m_children[1]->m_return_type == RT_FLOAT));
    ASS(m_children[0]->m_return_type == m_children[1]->m_return_type);

    ReturnVariant a = m_children[0]->get_output();
    ReturnVariant b = m_children[1]->get_output();

    if ((a.type() == typeid(int)) && (b.type() == typeid(int)))
    {
    	m_outp = (bool)(get<int>(a) >= get<int>(b));
    }
    else
    if ((a.type() == typeid(double)) && (b.type() == typeid(int)))
    {
    	m_outp = (bool)(get<double>(a) >= get<int>(b));
    }
    else
    if ((a.type() == typeid(int)) && (b.type() == typeid(double)))
    {
    	m_outp = (bool)(get<int>(a) >= get<double>(b));
    }
    else
    if ((a.type() == typeid(double)) && (b.type() == typeid(double)))
    {
    	m_outp = (bool)(get<double>(a) >= get<double>(b));
    }
    else
    {
    	m_outp = false;
    }
}

std::string GreOp::print_str()
{
    std::stringstream s;
    s << format("%s >= %s")
        % m_children[0]->print_str() 
        % m_children[1]->print_str();
    return s.str();
}

std::string GreOp::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_el_output() + ") >= ("
        + m_children[1]->get_el_output() + ")");
	return s.str();
}

std::string GreOp::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_mql_output() + ") >= ("
        + m_children[1]->get_mql_output() + ")");
	return s.str();
}

void LeOp::init_op()
{
    // don't allow other input types than int/double
    if (((m_children[0]->m_return_type != RT_INT) && (m_children[0]->m_return_type != RT_FLOAT)) || 
        ((m_children[1]->m_return_type != RT_INT) && (m_children[1]->m_return_type != RT_FLOAT)))
        throw std::runtime_error("Children of LE ops must return int/double only.");

    // return type is always bool
    m_return_type = RT_BOOL;
    init_codegen_outp();
}

void LeOp::compute_tick( const Env& env, int idx )
{
    ASS(m_children.size() == 2);
    ASS((m_children[0]->m_return_type == RT_INT) || (m_children[0]->m_return_type == RT_FLOAT));
    ASS((m_children[1]->m_return_type == RT_INT) || (m_children[1]->m_return_type == RT_FLOAT));
    ASS(m_children[0]->m_return_type == m_children[1]->m_return_type);

    ReturnVariant a = m_children[0]->get_output();
    ReturnVariant b = m_children[1]->get_output();

    if ((a.type() == typeid(int)) && (b.type() == typeid(int)))
    {
    	m_outp = (bool)(get<int>(a) < get<int>(b));
    }
    else
    if ((a.type() == typeid(double)) && (b.type() == typeid(int)))
    {
    	m_outp = (bool)(get<double>(a) < get<int>(b));
    }
    else
    if ((a.type() == typeid(int)) && (b.type() == typeid(double)))
    {
    	m_outp = (bool)(get<int>(a) < get<double>(b));
    }
    else
    if ((a.type() == typeid(double)) && (b.type() == typeid(double)))
    {
    	m_outp = (bool)(get<double>(a) < get<double>(b));
    }
    else
    {
    	m_outp = false;
    }
}

std::string LeOp::print_str()
{
    std::stringstream s;
    s << format("(%s) < (%s)")
        % m_children[0]->print_str() 
        % m_children[1]->print_str();
    return s.str();
}

std::string LeOp::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_el_output() + ") < ("
        + m_children[1]->get_el_output() + ")");
	return s.str();
}

std::string LeOp::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_mql_output() + ") < ("
        + m_children[1]->get_mql_output() + ")");
	return s.str();
}

void LeeOp::init_op()
{
    // don't allow other input types than int/double
    if (((m_children[0]->m_return_type != RT_INT) && (m_children[0]->m_return_type != RT_FLOAT)) || 
        ((m_children[1]->m_return_type != RT_INT) && (m_children[1]->m_return_type != RT_FLOAT)))
        throw std::runtime_error("Children of LEE ops must return int/double only.");

    // return type is always bool
    m_return_type = RT_BOOL;
    init_codegen_outp();
}

void LeeOp::compute_tick( const Env& env, int idx )
{
    ASS(m_children.size() == 2);
    ASS((m_children[0]->m_return_type == RT_INT) || (m_children[0]->m_return_type == RT_FLOAT));
    ASS((m_children[1]->m_return_type == RT_INT) || (m_children[1]->m_return_type == RT_FLOAT));
    ASS(m_children[0]->m_return_type == m_children[1]->m_return_type);

    ReturnVariant a = m_children[0]->get_output();
    ReturnVariant b = m_children[1]->get_output();

    if ((a.type() == typeid(int)) && (b.type() == typeid(int)))
    {
    	m_outp = (bool)(get<int>(a) <= get<int>(b));
    }
    else
    if ((a.type() == typeid(double)) && (b.type() == typeid(int)))
    {
    	m_outp = (bool)(get<double>(a) <= get<int>(b));
    }
    else
    if ((a.type() == typeid(int)) && (b.type() == typeid(double)))
    {
    	m_outp = (bool)(get<int>(a) <= get<double>(b));
    }
    else
    if ((a.type() == typeid(double)) && (b.type() == typeid(double)))
    {
    	m_outp = (bool)(get<double>(a) <= get<double>(b));
    }
    else
    {
    	m_outp = false;
    }
}

std::string LeeOp::print_str()
{
    std::stringstream s;
    s << format("(%s) <= (%s)")
        % m_children[0]->print_str() 
        % m_children[1]->print_str();
    return s.str();
}

std::string LeeOp::get_el_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_el_output() + ") <= ("
        + m_children[1]->get_el_output() + ")");
	return s.str();
}

std::string LeeOp::get_mql_tick()
{
	std::stringstream s;
	s << format("%s = %s;") % var("outp") % ("(" + m_children[0]->get_mql_output() + ") <= ("
        + m_children[1]->get_mql_output() + ")");
	return s.str();
}
