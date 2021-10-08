#include "dtime.h"

void Date::verify()
{
    ASS(year >= 1970);
    ASS((month >= 1) && (month < 13));
    ASS((day >= 1) && (day < 32));
}

Date::Date()
{
    year = 1970; month = 1; day = 1; verify();
}

Date::Date( int y )
{
    year = y; month = 1; day = 1; verify();
}

Date::Date( int y, int mon )
{
    year = y; month = mon; day = 1; verify();
}

Date::Date( int y, int mon, int d )
{
    year = y; month = mon; day = d; verify();
}

long int Date::as_int() const
{
    return year * 400 + month * 32 + day;
}


/*void Date::from_int(long y)
{
    day = (y % 400) % 32;
    month = ((y % 400) - day) / 32;
    year = (-day - 32 * month + y) / 400;
}*/


bool Date::operator==( const Date& rhs ) const
{
    return ((this->year == rhs.year) &&
        (this->month == rhs.month) &&
        (this->day == rhs.day));
}

bool Date::operator!=( const Date& rhs ) const
{
    return ((this->year != rhs.year) ||
        (this->month != rhs.month) ||
        (this->day != rhs.day));
}

bool Date::operator<( const Date& rhs ) const
{
    return (as_int() < rhs.as_int());
}

bool Date::operator<=( const Date& rhs ) const
{
    return (as_int() <= rhs.as_int());
}

bool Date::operator>( const Date& rhs ) const
{
    return (as_int() > rhs.as_int());
}

bool Date::operator>=( const Date& rhs ) const
{
    return (as_int() >= rhs.as_int());
}

std::string Date::print_str() const
{
    char dt[128];
    sprintf(dt, "%04d-%02d-%02d", year, month, day);
    return std::string(dt);
}


void Time::verify()
{
    ASS((hour >= 0) && (hour < 24));
    ASS((minute >= 0) && (minute < 60));
    ASS((second >= 0) && (second < 60));
}

Time::Time()
{
    hour = 0; minute = 0; second = 0; millisecond = 0; verify();
}

Time::Time( int h )
{
    hour = h; minute = 0; second = 0; millisecond = 0; verify();
}

Time::Time( int h, int m )
{
    hour = h; minute = m; second = 0; millisecond = 0; verify();
}

Time::Time( int h, int m, int s )
{
    hour = h; minute = m; second = s; millisecond = 0; verify();
}

Time::Time(int h, int m, int s, int ms)
{
    hour = h; minute = m; second = s; millisecond = ms; verify();
}


long Time::as_int() const
{
    return hour * 3600*1000 + minute * 60*1000 + second*1000 + millisecond;
}


/*void Time::from_int(long a)
{
    //second = (a % 3600) % 60;
    //minute = ((a % 3600) - second) / 60;
    //hour = (-second - 32 * minute + a) / 3600;
}*/

bool Time::operator==( const Time& rhs ) const
{
    return ((this->hour == rhs.hour) &&
        (this->minute == rhs.minute) &&
        (this->second == rhs.second) &&
        (this->millisecond == rhs.millisecond)
    );
}

bool Time::operator<( const Time& rhs ) const
{
    return ( as_int() < rhs.as_int() );
}

bool Time::operator<=( const Time& rhs ) const
{
    return ( as_int() <= rhs.as_int() );
}

bool Time::operator>( const Time& rhs ) const
{
    return ( as_int() > rhs.as_int() );
}

bool Time::operator>=( const Time& rhs ) const
{
    return ( as_int() >= rhs.as_int() );
}

bool Time::operator!=( const Time& rhs ) const
{
    return ((this->hour != rhs.hour) ||
        (this->minute != rhs.minute) ||
        (this->second != rhs.second) ||
        (this->millisecond != rhs.millisecond));
}

std::string Time::print_str() const
{
    char dt[128];
    if (millisecond == 0)
    {
        sprintf(dt, "%02d:%02d:%02d", hour, minute, second);
    }
    else
    {
        sprintf(dt, "%02d:%02d:%02d.%03d", hour, minute, second, millisecond);
    }
    return std::string(dt);
}



void DateTime::verify()
{
    ASS(year >= 1970);
    ASS((month >= 1) && (month < 13));
    ASS((day >= 1) && (day < 32));
    ASS((hour >= 0) && (hour < 24));
    ASS((minute >= 0) && (minute < 60));
    ASS((second >= 0) && (second < 60));
    ASS((millisecond >= 0) && (millisecond < 1000));
}

DateTime::DateTime()
{
    year = 1970; month = 1; day = 1;
    hour = 0; minute = 0; second = 0; millisecond = 0;
    verify();
}

DateTime::DateTime( int y )
{
    year = y; month = 1; day = 1;
    hour = 0; minute = 0; second = 0; millisecond = 0;
    verify();
}

DateTime::DateTime( int y, int mon )
{
    year = y; month = mon; day = 1;
    hour = 0; minute = 0; second = 0; millisecond = 0;
    verify();
}

DateTime::DateTime( int y, int mon, int d )
{
    year = y; month = mon; day = d;
    hour = 0; minute = 0; second = 0; millisecond = 0;
    verify();
}

DateTime::DateTime( int y, int mon, int d, int h )
{
    year = y; month = mon; day = d;
    hour = h; minute = 0; second = 0; millisecond = 0;
    verify();
}

DateTime::DateTime( int y, int mon, int d, int h, int m )
{
    year = y; month = mon; day = d;
    hour = h; minute = m; second = 0; millisecond = 0;
    verify();
}

DateTime::DateTime( int y, int mon, int d, int h, int m, int s )
{
    year = y; month = mon; day = d;
    hour = h; minute = m; second = s; millisecond = 0;
    verify();
}

DateTime::DateTime( int y, int mon, int d, int h, int m, int s, int ms )
{
    year = y; month = mon; day = d;
    hour = h; minute = m; second = s; millisecond = ms;
    verify();
}


DateTime::DateTime( int y, int mon, int d, Time& t )
{
    year = y; month = mon; day = d;
    hour = t.hour; minute = t.minute; second = t.second; millisecond = t.millisecond;
    verify();
}

DateTime::DateTime( Date a_date, int h, int m )
{
    year = a_date.year; month = a_date.month; day = a_date.day;
    hour = h; minute = m; second = 0; millisecond = 0;
    verify();
}

DateTime::DateTime( Date a_date, int h, int m, int s )
{
    year = a_date.year; month = a_date.month; day = a_date.day;
    hour = h; minute = m; second = s; millisecond = 0;
    verify();
}

DateTime::DateTime( Date a_date, int h, int m, int s, int ms)
{
    year = a_date.year; month = a_date.month; day = a_date.day;
    hour = h; minute = m; second = s; millisecond = ms;
    verify();
}

Date DateTime::date() const
{
    return Date(year, month, day);
}

Time DateTime::time() const
{
    return Time(hour, minute, second, millisecond);
}

long int DateTime::as_int() const
{
    long int dnum1=0, tnum1=0;

    dnum1 = date().as_int();
    tnum1 = time().as_int();

    long int r = dnum1 * (86400L*1000L) + tnum1;

    return ( r );
}


/*void DateTime::from_int(long a)
{
    Date d;
    Time t;

    t.from_int(a % 86400L);
    d.from_int((a - (a % 86400L)) / 86400L);

    year = d.year;
    month = d.month;
    day = d.day;

    hour = t.hour;
    minute = t.minute;
    second = t.second;
}*/


bool DateTime::operator==( const DateTime& rhs ) const
{
    return (this->time() == rhs.time()) && (this->date() == rhs.date());
}

bool DateTime::operator!=( const DateTime& rhs ) const
{
    return (this->time() != rhs.time()) || (this->date() != rhs.date());
}

bool DateTime::operator<( const DateTime& rhs ) const
{
    return (as_int() < rhs.as_int());
}

bool DateTime::operator<=( const DateTime& rhs ) const
{
    return (as_int() <= rhs.as_int());
}

bool DateTime::operator>( const DateTime& rhs ) const
{
    return (as_int() > rhs.as_int());
}

bool DateTime::operator>=( const DateTime& rhs ) const
{
	return (as_int() >= rhs.as_int());
}

std::string DateTime::print_str() const
{
    char dt[128];
    if (millisecond == 0)
    {
        sprintf(dt, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
    }
    else
    {
        sprintf(dt, "%04d-%02d-%02d %02d:%02d:%02d.%03d", year, month, day, hour, minute, second, millisecond);
    }
    return std::string(dt);
}

