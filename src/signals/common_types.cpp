#include "common_types.h"

SignalParameter SPType2SP(SP_type a)
{
    SignalParameter x;

    if (a.type() == typeid(int))
    {
        x.contents = get<int>(a);
    }
    if (a.type() == typeid(double))
    {
    	x.contents = get<double>(a);
    }
    if (a.type() == typeid(bool))
    {
    	x.contents = get<bool>(a);
    }
    if (a.type() == typeid(Time))
    {
    	x.contents = get<Time>(a);
    }
    if (a.type() == typeid(std::string))
    {
    	x.contents = get<std::string>(a);
    }

    return x;
}

SP_type SP2SPType(SignalParameter a)
{
    if (a.contents.type() == typeid(int))
    {
        return get<int>(a.contents);
    }
    if (a.contents.type() == typeid(double))
    {
        return get<double>(a.contents);
    }
    if (a.contents.type() == typeid(bool))
    {
        return get<bool>(a.contents);
    }
    if (a.contents.type() == typeid(Time))
    {
        return get<Time>(a.contents);
    }
    if (a.contents.type() == typeid(std::string))
    {
        return get<std::string>(a.contents);
    }

    return 0;
}

void display_exp_parameter_map( ExplicitParameterMap& m )
{
    for(ExplicitParameterMap::iterator it = m.begin();
        it != m.end(); it++)
    {
        std::cout << it->first << " : ";
        if ((it->second).type() == typeid(int))
        {
            std::cout << "(int) " << get<int>(it->second) << "\n";
        }
        if ((it->second).type() == typeid(double))
        {
            std::cout << "(double) " << get<double>(it->second) << "\n";
        }
        if ((it->second).type() == typeid(bool))
        {
            std::cout << "(bool) " << get<bool>(it->second) << "\n";
        }
        if ((it->second).type() == typeid(std::string))
        {
            std::cout << "(std::string) " << get<std::string>(it->second) << "\n";
        }
        if ((it->second).type() == typeid(Time))
        {
            std::cout << "(Time) " << (get<Time>(it->second)).print_str() << "\n";
        }
    }
}
