#ifndef DTIME_H
#define DTIME_H

#include <stdio.h>
#include "common.h"

#include <boost/python.hpp>
#include <boost/python/str.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_woarchive.hpp>
#include <boost/archive/text_wiarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace py = boost::python;

//#include <boost/date_time/posix_time/posix_time.hpp> // include all types plus i/o
//#include <boost/date_time/gregorian/gregorian.hpp>
//using namespace boost::gregorian;
//using namespace boost::posix_time;

// For sophisticated time math, I should use boost.


class Date 
{
public:
    int year, month, day; // 1970..+inf, 1..12, 1..31

    void verify();

    Date();
    Date(int y);
    Date(int y, int mon);
    Date(int y, int mon, int d);

    long int as_int() const;

    //void from_int(long a);

    // Comparison operators
    bool operator==(const Date& rhs) const;
    bool operator!=(const Date& rhs) const;
    bool operator < (const Date& rhs) const;
    bool operator <= (const Date& rhs) const;
    bool operator > (const Date& rhs) const;
    bool operator >= (const Date& rhs) const;

    std::string print_str() const;

    // Serialization
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & year;
		ar & month;
		ar & day;
	}
};


struct Date_pickle_suite : py::pickle_suite
{
    static py::object getstate(const Date& a)
    {
        std::ostringstream os;
        boost::archive::text_oarchive oa(os);
        oa << a;
        return py::str (os.str());
    }

    static void setstate(Date& a, py::object entries)
    {
        py::str s = py::extract<py::str> (entries)();
        std::string st = py::extract<std::string>(s)();
        std::istringstream is (st);

        boost::archive::text_iarchive ia(is);
        ia >> a;
    }
};



class Time
{
public:
    int hour, minute, second, millisecond; // 0..23, 0..59, 0..59, 0..999

    void verify();

    // Constructors
    Time();
    Time(int h);
    Time(int h, int m);
    Time(int h, int m, int s);
    Time(int h, int m, int s, int ms);

    long int as_int() const;

    //void from_int(long a);

    // Comparison operators
    bool operator==(const Time& rhs) const;
    bool operator!=(const Time& rhs) const;
    bool operator < (const Time& rhs) const;
    bool operator <= (const Time& rhs) const;
    bool operator > (const Time& rhs) const;
    bool operator >= (const Time& rhs) const;

    std::string print_str() const;

    // Serialization
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & hour;
        ar & minute;
        ar & second;
        ar & millisecond;
    }
};

struct Time_pickle_suite : py::pickle_suite
{
    static py::object getstate(const Time& a)
    {
        std::ostringstream os;
        boost::archive::text_oarchive oa(os);
        oa << a;
        return py::str(os.str());
    }

    static void setstate(Time& a, py::object entries)
    {
        py::str s = py::extract<py::str> (entries)();
        std::string st = py::extract<std::string>(s)();
        std::istringstream is (st);

        boost::archive::text_iarchive ia(is);
        ia >> a;
    }
};



class DateTime
{
public:
    int year, month, day; // 0..+inf, 1..12, 1..31
    int hour, minute, second, millisecond; // 0..23, 0..59, 0..59, 0..999

    void verify();

    // Constructors
    DateTime();
    DateTime(int y);
    DateTime(int y, int mon);
    DateTime(int y, int mon, int d);
    DateTime(int y, int mon, int d, int h);
    DateTime(int y, int mon, int d, int h, int m);
    DateTime(int y, int mon, int d, int h, int m, int s);
    DateTime(int y, int mon, int d, int h, int m, int s, int ms);
    DateTime(int y, int mon, int d, Time& t);
    DateTime(Date a_date, int h, int m);
    DateTime(Date a_date, int h, int m, int s);
    DateTime(Date a_date, int h, int m, int s, int ms);

    Date date() const;
    Time time() const;
    long int as_int() const;

    //void from_int(long a);

    // Comparison operators
    bool operator==(const DateTime& rhs) const;
    bool operator!=(const DateTime& rhs) const;
    bool operator < (const DateTime& rhs) const;
    bool operator <= (const DateTime& rhs) const;
    bool operator > (const DateTime& rhs) const;
    bool operator >= (const DateTime& rhs) const;

    std::string print_str() const;

    // Serialization
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & year;
        ar & month;
        ar & day;
        ar & hour;
        ar & minute;
        ar & second;
        ar & millisecond;
    }
};

struct DateTime_pickle_suite : py::pickle_suite
{
    static py::object getstate(const DateTime& a)
    {
        std::ostringstream os;
        boost::archive::text_oarchive oa(os);
        oa << a;
        return py::str (os.str());
    }

    static void setstate(DateTime& a, py::object entries)
    {
        py::str s = py::extract<py::str> (entries)();
        std::string st = py::extract<std::string>(s)();
        std::istringstream is (st);

        boost::archive::text_iarchive ia(is);
        ia >> a;
    }
};


#endif // DTIME_H
