#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <fstream>
#include <assert.h>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <boost/variant.hpp>
#include <boost/format.hpp>
#include <typeinfo> 
#include <math.h>
#include <limits>

#include <boost/python.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#define MULTICHARTS_COMPATIBLE
#define EL_ONCE_PER_BAR

//using namespace boost;
namespace py = boost::python;
namespace bs = boost;

#define ABS(x) ((x)>0?(x):(-(x)))
#define STR(x) std::string(x)

/*inline float round(float number)
{
    return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}*/

inline double round(double number)
{
    return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}

// align a vector of doubles so that [1, 2, 3, 0, 0] -> [0, 0, 1, 2, 3]
// where nb = 2
inline void shift_ta_data(std::vector<double>& data, int nb)
{
	for(int i = (int) (data.size() - 1); i > nb; i--)
	{
		data[i] = data[i - nb];
	}

	if (nb > 0)
	{
		for(int i=0; i<=nb; i++)
		{
			data[i] = 0;
		}
	}
}

inline void shift_ta_data(std::vector<int>& data, unsigned int nb)
{
	for(unsigned int i = (unsigned int)data.size()-1; i > nb; i--)
	{
		data[i] = data[i - nb];
	}
}



inline std::string all_upper(std::string s)
{
    std::string x = s;
    for(unsigned int i=0; i<x.size(); i++)
    {
        x[i] = (char) toupper(x[i]);
    }
    return x;
}

inline std::string all_lower(std::string s)
{
    std::string x = s;
    for(unsigned int i=0; i<x.size(); i++)
    {
        x[i] = (char) tolower(x[i]);
    }
    return x;
}

inline std::string capitalize(std::string s)
{
    std::string x = all_lower(s);
    if (x.size() > 0)
    {
        x[0] = (char) toupper(x[0]);
    }
    return x;
}


class ProgressBar
{
    double m_progress;
    double m_delta;
    double m_max_val;

public:

    ProgressBar(int a_mv) 
    { 
        m_max_val = (double)a_mv; 
        m_progress = 0; 
        m_delta = 1.0 / m_max_val;
    }

	void reset() 
    { 
        m_progress = 0; 
    }
	void update()
	{
		m_progress += m_delta;

		int t_barWidth = 70;

		std::cout << "[";
		int t_pos = (int)(t_barWidth * m_progress);
		for (int i = 0; i < t_barWidth; ++i)
		{
			if (i < t_pos) std::cout << "=";
			else if (i == t_pos) std::cout << ">";
			else std::cout << " ";
		}
		std::cout << "] " << int(m_progress * 100.0) << " %\r";
		std::cout.flush();
	}
	void update(int a_val)
	{
		m_progress = m_delta*a_val;

		int t_barWidth = 70;

		std::cout << "[";
		int t_pos = (int)(t_barWidth * m_progress);
		for (int i = 0; i < t_barWidth; ++i)
		{
			if (i < t_pos) std::cout << "=";
			else if (i == t_pos) std::cout << ">";
			else std::cout << " ";
		}
        int t_pp = int(m_progress * 100.0);
        if ((m_progress * 100.0) > 99.5 )
            t_pp = 100;
		std::cout << "] " << t_pp << " %\r";
		std::cout.flush();
	}
	void finish()
	{
		std::cout << std::endl;
	}
};


// my own assert

#ifdef _DEBUG

// kill any existing declarations
#ifdef ASS
#undef ASS
#endif

#define ASS(expr)\
        {\
        if( !(expr) )\
            {\
            std::cout << "\n*** ASSERT ***\n"; \
            assert(expr);\
            }\
        }

#else 

// ASSERT gets optimized out completely
#define ASS(expr)

#endif

#endif // COMMON_H
