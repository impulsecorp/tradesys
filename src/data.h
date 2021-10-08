#ifndef DATA_H
#define DATA_H

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include "common.h"
#include "dtime.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>


enum { DATA_TIME_INDEX_OPEN = 0,
       DATA_TIME_INDEX_CLOSE = 1};

class TickDataMatrix; // fwd

class DataMatrix
{
public:
    std::vector<DateTime> dt;
    std::vector<double> open;
    std::vector<double> high;
    std::vector<double> low;
    std::vector<double> close;
    std::vector<int> volume;
    std::string name;

    unsigned int len() const;

    void load_tsys(std::string a_filename);
    void load_tsys_ifile(std::ifstream& ifile);  // Read from an already opened file stream

    void dump_tsys(std::string a_filename);
    void dump_tsys_ofile(std::ofstream& ofile);  // Write to an already opened file stream

    void load_tradestation(std::string a_filename);
    void load_tradestation_ifile(std::ifstream& ifile);  // Read from an already opened file stream

    void load_metatrader4(std::string a_filename);
    void load_metatrader4_ifile(std::ifstream& ifile);  // Read from an already opened file stream

    void load_multicharts(std::string a_filename);
    void load_multicharts_ifile(std::ifstream& ifile);  // Read from an already opened file stream

    // trim the excess data that is not in the datetime interval of [st .. en]
    void clip_dt(DateTime st, DateTime en);
    // trim the excess data that is not in the index interval of [st .. en]
    void clip_idx(int st, int en);

    // returns a trimmed copy by DateTime
    DataMatrix slice_dt(DateTime st, DateTime en) const;
    // returns a trimmed copy by index
    DataMatrix slice_idx(int st, int en) const;

    void clear();
    void add_row(DateTime a_dt, double open, double high, double low, double close, int volume);

    py::list get_dt_list();

    //TickDataMatrix make_ticks(int ticks_per_bar, bool precise, int time_index, int seconds_per_bar);

	// todo: data diff/comparisons, clone, add noise, resampling,
	// sample from ticks

    bool operator==(const DataMatrix &a) const;

    bool operator!=(const DataMatrix &a) const;

    // Serialization
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & dt;
        ar & open;
        ar & high;
        ar & low;
        ar & close;
        ar & volume;
        ar & name;
    }

    void shift_rows(int num);
};

class TickDataMatrix
{
public:
    std::vector<DateTime> dt;
    std::vector<double> price;
    std::string name;
    int data_period_seconds;
    int data_time_index;

    int len() const;

    // trim the excess data that is not in the datetime interval of [st .. en]
    void clip_dt(DateTime st, DateTime en);

    // trim the excess data that is not in the index interval of [st .. en]
    void clip_idx(int st, int en);

    // returns a trimmed copy by DateTime
    TickDataMatrix slice_dt(DateTime st, DateTime en) const;

    // returns a trimmed copy by index
    TickDataMatrix slice_idx(int st, int en) const;

    void clear();

    void add_tick(DateTime a_dt, double a_price);

    py::list get_dt_list();

    // todo: data diff/comparisons, clone, add noise, resampling,
    // sample from ticks

    bool operator==(const TickDataMatrix &a) const;

    bool operator!=(const TickDataMatrix &a) const;

    // Serialization
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & dt;
        ar & price;
        ar & name;
    }
};


class MultiDataMatrix
{
public:
    std::vector<DataMatrix> symbols;

    // Load from a text file
    void load_tsys(std::string fname);

    // Get length of largest DataMatrix
    unsigned int len() const;

    // trim the excess data that is not in the datetime interval of [st .. en]
    void clip_dt(DateTime st, DateTime en);
    // trim the excess data that is not in the index interval of [st .. en]
    void clip_idx(int st, int en);

    // returns a trimmed copy by DateTime
    MultiDataMatrix slice_dt(DateTime st, DateTime en) const;
    // returns a trimmed copy by index
    MultiDataMatrix slice_idx(int st, int en) const;

    // Serialization
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & symbols;
    }
};


struct DataMatrix_pickle_suite : py::pickle_suite
{
    static py::object getstate(const DataMatrix& a)
    {
        std::ostringstream os;
        boost::archive::text_oarchive oa(os);
        oa << a;
        return py::str (os.str());
    }

    static void setstate(DataMatrix& a, py::object entries)
    {
        py::str s = py::extract<py::str> (entries)();
        std::string st = py::extract<std::string>(s)();
        std::istringstream is (st);

        boost::archive::text_iarchive ia(is);
        ia >> a;
    }
};


struct MultiDataMatrix_pickle_suite : py::pickle_suite
{
    static py::object getstate(const MultiDataMatrix& a)
    {
        std::ostringstream os;
        boost::archive::text_oarchive oa(os);
        oa << a;
        return py::str (os.str());
    }

    static void setstate(MultiDataMatrix& a, py::object entries)
    {
        py::str s = py::extract<py::str> (entries)();
        std::string st = py::extract<std::string>(s)();
        std::istringstream is (st);

        boost::archive::text_iarchive ia(is);
        ia >> a;
    }
};

struct TickDataMatrix_pickle_suite : py::pickle_suite
{
    static py::object getstate(const TickDataMatrix& a)
    {
        std::ostringstream os;
        boost::archive::text_oarchive oa(os);
        oa << a;
        return py::str (os.str());
    }

    static void setstate(TickDataMatrix& a, py::object entries)
    {
        py::str s = py::extract<py::str> (entries)();
        std::string st = py::extract<std::string>(s)();
        std::istringstream is (st);

        boost::archive::text_iarchive ia(is);
        ia >> a;
    }
};




enum PositionType { SHORT_POSITION = -1, NO_POSITION = 0, LONG_POSITION = 1 };


#endif
