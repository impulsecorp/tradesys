#include "data.h"
#include <algorithm>

#include <datetime.h> // compile with -I/path/to/python/include

// Utility function to reverse elements of an array
void reverse(int arr[], int n)
{

}

void DataMatrix::load_tsys_ifile(std::ifstream &ifile)
{
    if (ifile.is_open())
    {
        unsigned int num_records;
        unsigned int all_records;
        ifile >> name;
        ifile >> num_records;
        all_records = num_records;

        ProgressBar pb(num_records);
        std::cout << "Loading: " << name << ", " << num_records << " bars" << std::endl;
        while (num_records--)
        {
            if ((num_records % 10000) == 0)
                pb.update(all_records - num_records);

            double o, h, l, c, v;
            DateTime tm;

            ifile >> tm.year;
            ifile >> tm.month;
            ifile >> tm.day;
            ifile >> tm.hour;
            ifile >> tm.minute;
            ifile >> tm.second;

            ifile >> o;
            ifile >> h;
            ifile >> l;
            ifile >> c;
            ifile >> v;

            dt.push_back(tm);
            open.push_back(o);
            high.push_back(h);
            low.push_back(l);
            close.push_back(c);
            volume.push_back((int) v);
        }
        pb.finish();
    }
    else
    {
        throw std::runtime_error("TradeSys file I/O error");
    }

    // need to reverse the data
    std::reverse(dt.begin(), dt.end());
    std::reverse(open.begin(), open.end());
    std::reverse(high.begin(), high.end());
    std::reverse(low.begin(), low.end());
    std::reverse(close.begin(), close.end());
    std::reverse(volume.begin(), volume.end());
}

void DataMatrix::load_tsys(std::string a_filename)
{
    std::ifstream ifile(a_filename.c_str());
    load_tsys_ifile(ifile);
    ifile.close();
}

void DataMatrix::dump_tsys_ofile(std::ofstream &ofile)
{
    if (ofile.is_open())
    {
        unsigned int idx = 0;
        ofile << name << "\n";
        ofile << dt.size() << "\n";

        ProgressBar pb(dt.size());
        std::cout << "Saving: " << name << ", " << dt.size() << " bars" << std::endl;
        for (idx = 0; idx < dt.size(); idx++)
        {
            if ((idx % 10000) == 0)
                pb.update(idx);

            double o, h, l, c, v;
            DateTime tm;

            tm = dt[idx];
            o = open[idx];
            h = high[idx];
            l = low[idx];
            c = close[idx];
            v = volume[idx];

            ofile << tm.year << " ";
            ofile << tm.month << " ";
            ofile << tm.day << " ";
            ofile << tm.hour << " ";
            ofile << tm.minute << " ";
            ofile << tm.second << " ";

            ofile << o << " ";
            ofile << h << " ";
            ofile << l << " ";
            ofile << c << " ";
            ofile << v << "\n";
        }
        pb.finish();
    }
    else
    {
        throw std::runtime_error("TradeSys file I/O error");
    }
}

void DataMatrix::dump_tsys(std::string a_filename)
{
    std::ofstream ofile(a_filename.c_str());
    dump_tsys_ofile(ofile);
    ofile.close();
}

void DataMatrix::load_tradestation(std::string a_filename)
{
    // Set the name as the uppercase filename
    name = all_upper(a_filename);

    std::ifstream ifile(a_filename.c_str());
    load_tradestation_ifile(ifile);
    ifile.close();
}

void DataMatrix::load_tradestation_ifile(std::ifstream &ifile)
{
    if (ifile.is_open())
    {
        std::string line;

        // skip the first line
        ifile >> line;
        ifile >> line;
        while (!ifile.eof())
        {
            //std::vector<std::string> stuff;
            double o = 0, h = 0, l = 0, c = 0;
            int v = 0;
            int u = 0, d = 0, oi = 0;
            DateTime tm;

            // process line

            // if there is one ':', it's TIME_HHMM, if there are two, it's TIME
            // count how many
            unsigned int nts = 0;
            unsigned int ncs = 0;
            for (unsigned int i = 0; i < line.size(); i++)
            {
                if (line[i] == ':')
                {
                    nts += 1;
                }
                if (line[i] == ',')
                {
                    ncs += 1;
                }
            }
            ASS((nts == 1) || (nts == 2));

            if (nts == 1)
            {
                if (ncs == 7)
                {
                    sscanf(line.c_str(), "%02d/%02d/%04d,%02d:%02d,%lf,%lf,%lf,%lf,%d,%d",
                           &(tm.month), &(tm.day), &(tm.year), &(tm.hour), &(tm.minute),
                           &o, &h, &l, &c, &u, &d
                    );
                }
                else
                {
                    sscanf(line.c_str(), "%02d/%02d/%04d,%02d:%02d,%lf,%lf,%lf,%lf,%d,%d",
                           &(tm.month), &(tm.day), &(tm.year), &(tm.hour), &(tm.minute),
                           &o, &h, &l, &c, &v, &oi
                    );
                }
            }
            else
            {
                if (nts == 2)
                {
                    if (ncs == 7)
                    {
                        sscanf(line.c_str(), "%02d/%02d/%04d,%02d:%02d:%02d,%lf,%lf,%lf,%lf,%d,%d",
                               &(tm.month), &(tm.day), &(tm.year), &(tm.hour), &(tm.minute), &(tm.second),
                               &o, &h, &l, &c, &u, &d
                        );
                    }
                    else
                    {
                        sscanf(line.c_str(), "%02d/%02d/%04d,%02d:%02d:%02d,%lf,%lf,%lf,%lf,%d,%d",
                               &(tm.month), &(tm.day), &(tm.year), &(tm.hour), &(tm.minute), &(tm.second),
                               &o, &h, &l, &c, &v, &oi
                        );
                    }
                }
                else
                {
                    throw std::runtime_error("Can't recognize time format in TradeStation file.");
                }
            }

            // volume = up + down
            if ((v == 0) && ((u != 0) || (d != 0)))
            {
                v = u + d;
            }

            dt.push_back(tm);
            open.push_back(o);
            high.push_back(h);
            low.push_back(l);
            close.push_back(c);
            volume.push_back((int) v);

            ifile >> line;
        }
    }
    else
    {
        throw std::runtime_error("TradeStation file I/O error");
    }
}


void DataMatrix::load_metatrader4(std::string a_filename)
{
    // Set the name as the uppercase filename
    name = all_upper(a_filename);

    std::ifstream ifile(a_filename.c_str());
    load_metatrader4_ifile(ifile);
    ifile.close();
}

void DataMatrix::load_metatrader4_ifile(std::ifstream &ifile)
{
    if (ifile.is_open())
    {
        std::string line;

        // skip the first line
        ifile >> line;
        ifile >> line;
        while (!ifile.eof())
        {
            //std::vector<std::string> stuff;
            double o = 0, h = 0, l = 0, c = 0;
            int v = 0;
            //int u=0, d=0;
            DateTime tm;

            // process line
            sscanf(line.c_str(), "%04d.%02d.%02d,%02d:%02d,%lf,%lf,%lf,%lf,%d",
                   &(tm.year), &(tm.month), &(tm.day), &(tm.hour), &(tm.minute),
                   &o, &h, &l, &c, &v
            );

            dt.push_back(tm);
            open.push_back(o);
            high.push_back(h);
            low.push_back(l);
            close.push_back(c);
            volume.push_back((int) v);

            ifile >> line;
        }
    }
    else
    {
        throw std::runtime_error("MetaTrader4 file I/O error");
    }
}


void DataMatrix::load_multicharts(std::string a_filename)
{
    // Set the name as the uppercase filename
    name = all_upper(a_filename);

    std::ifstream ifile(a_filename.c_str());
    load_multicharts_ifile(ifile);
    ifile.close();
}

double myatof(std::string num)
{
    // unpack float to parts
    std::vector<std::string> parts;
    bs::split(parts, num, bs::is_any_of("."));

    double x = (double)(atoi(parts[0].c_str()));
    // now add the fractional part
    double f = (double)(atoi(parts[1].c_str()));
    f = f / pow(10, (parts[1].size()));
    return x + f;
}

void DataMatrix::load_multicharts_ifile(std::ifstream &ifile)
{
    if (ifile.is_open())
    {
        std::string line;

        while (!ifile.eof())
        {
            if ((line.find("<") == std::string::npos) && (line != ""))
            {
                double o = 0, h = 0, l = 0, c = 0;
                int v = 0;
                DateTime tm;

                // unpack line to parts
                std::vector<std::string> parts;
                bs::split(parts, line, bs::is_any_of(","));

                // process each part separately
                sscanf(parts[0].c_str(), "%d/%d/%d", &(tm.month), &(tm.day), &(tm.year));
                sscanf(parts[1].c_str(), "%02d:%02d:%02d", &(tm.hour), &(tm.minute), &(tm.second));

                o = myatof(parts[2]);
                h = myatof(parts[3]);
                l = myatof(parts[4]);
                c = myatof(parts[5]);
                v = atoi(parts[6].c_str());

                dt.push_back(tm);
                open.push_back(o);
                high.push_back(h);
                low.push_back(l);
                close.push_back(c);
                volume.push_back(v);
            }

            ifile >> line;
        }
    }
    else
    {
        throw std::runtime_error("MultiCharts file I/O error");
    }
}



void DataMatrix::shift_rows(int num)
{
    if (num < 1)
        throw std::runtime_error("Invalid num for shift_rows");

    if (dt.size() > num)
    {
        for(int i=0; i < (dt.size() - num); i++)
        {
            dt[i] = dt[i + num];
            open[i] = open[i + num];
            high[i] = high[i + num];
            low[i] = low[i + num];
            close[i] = close[i + num];
            volume[i] = volume[i + num];
        }

        for(int i=(dt.size()-1); i>= (dt.size()-num); i--)
        {
            dt[i] = dt[dt.size()-1];
            open[i] = open[open.size()-1];
            high[i] = high[high.size()-1];
            low[i] = low[low.size()-1];
            close[i] = close[close.size()-1];
            volume[i] = volume[volume.size()-1];
        }
    }
}

void DataMatrix::clip_dt(DateTime st, DateTime en)
{
    /*unsigned int s = dt.size();
    ASS((open.size() == s) &&
        (high.size() == s) &&
        (low.size() == s) &&
        (close.size() == s) &&
        (volume.size() == s));*/

    int st_idx = 0, en_idx = (int) len() - 1;

    // find the idx of the beginning
    for (int i = 0; i < (int) (dt.size()); i++)
    {
        if (dt[i] >= st)
        {
            st_idx = i;
            break;
        }
    }

    // find the idx of the end
    for (int i = 0; i < (int) (dt.size()); i++)
    {
        if (dt[i] > en)
        {
            en_idx = i;
            break;
        }
    }

    // Erase all data to st_idx and after end_idx
    if (st_idx > 0)
    {
        dt.erase(dt.begin(), dt.begin() + (st_idx));
        open.erase(open.begin(), open.begin() + (st_idx));
        high.erase(high.begin(), high.begin() + (st_idx));
        low.erase(low.begin(), low.begin() + (st_idx));
        close.erase(close.begin(), close.begin() + (st_idx));
        volume.erase(volume.begin(), volume.begin() + (st_idx));

        en_idx -= st_idx;
    }


    if (en_idx < ((int) len() - 1))
    {
        dt.erase(dt.begin() + en_idx, dt.end());
        open.erase(open.begin() + en_idx, open.end());
        high.erase(high.begin() + en_idx, high.end());
        low.erase(low.begin() + en_idx, low.end());
        close.erase(close.begin() + en_idx, close.end());
        volume.erase(volume.begin() + en_idx, volume.end());
    }
}

void DataMatrix::clip_idx(int st, int en)
{
    int st_idx = st, en_idx = en;
    if (st_idx > en_idx)
    {
        int tmp = st_idx;
        en_idx = st_idx;
        st_idx = tmp;
    }
    if (st_idx < 0)
    {
        st_idx = 0;
    }
    if (en_idx > ((int) len() - 1))
    {
        en_idx = len() - 1;
    }

    if ((st_idx <= 0) && (en_idx >= ((int) len() - 1)))
    {
        return;
    }

    if (st_idx > 0)
    {
        dt.erase(dt.begin(), dt.begin() + (st_idx));
        open.erase(open.begin(), open.begin() + (st_idx));
        high.erase(high.begin(), high.begin() + (st_idx));
        low.erase(low.begin(), low.begin() + (st_idx));
        close.erase(close.begin(), close.begin() + (st_idx));
        volume.erase(volume.begin(), volume.begin() + (st_idx));

        en_idx -= st_idx;
    }
    if ((en_idx > 0) && (en_idx <= (((int) len()) - 1)))
    {
        dt.erase(dt.begin() + en_idx, dt.end());
        open.erase(open.begin() + en_idx, open.end());
        high.erase(high.begin() + en_idx, high.end());
        low.erase(low.begin() + en_idx, low.end());
        close.erase(close.begin() + en_idx, close.end());
        volume.erase(volume.begin() + en_idx, volume.end());
    }
}

DataMatrix DataMatrix::slice_dt(DateTime st, DateTime en) const
{
    DataMatrix d = *this;
    d.clip_dt(st, en);
    return d;
}

DataMatrix DataMatrix::slice_idx(int st, int en) const
{
    DataMatrix d = *this;
    d.clip_idx(st, en);
    return d;
}

void DataMatrix::add_row(DateTime a_dt, double a_open, double a_high, double a_low,
                         double a_close, int a_volume)
{
    dt.push_back(a_dt);
    open.push_back(a_open);
    high.push_back(a_high);
    low.push_back(a_low);
    close.push_back(a_close);
    volume.push_back(a_volume);
}

void DataMatrix::clear()
{
    open.clear();
    high.clear();
    low.clear();
    close.clear();
    volume.clear();
    dt.clear();
}

unsigned int DataMatrix::len() const
{
    // All must be of equal length
    unsigned int s = (unsigned int) dt.size();
    ASS(open.size() == s);
    ASS(high.size() == s);
    ASS(low.size() == s);
    ASS(close.size() == s);
    ASS(volume.size() == s);

    return s;
}

py::list DataMatrix::get_dt_list()
{
    PyDateTime_IMPORT;

    py::list l;

    for (unsigned int i = 0; i < dt.size(); i++)
    {
        py::object o(py::handle<>(

                // the PyObject*
                PyDateTime_FromDateAndTime(
                        dt[i].year,
                        dt[i].month,
                        dt[i].day,
                        dt[i].hour,
                        dt[i].minute,
                        dt[i].second,
                        0)

        ));

        l.append(o);
    }

    return l;
}


bool DataMatrix::operator==(const DataMatrix &a) const
{
    // compare the datetime arrays
    if (dt.size() != a.dt.size())
    {
        return false;
    }
    for (unsigned int i = 0; i < dt.size(); i++)
    {
        if (dt[i] != a.dt[i])
        {
            return false;
        }
    }
    return ((open == a.open) &&
            (high == a.high) &&
            (low == a.low) &&
            (close == a.close) &&
            (volume == a.volume) &&
            (name == a.name));
}

bool DataMatrix::operator!=(const DataMatrix &a) const
{
    // compare the datetime arrays
    if (dt.size() != a.dt.size())
    {
        return true;
    }
    for (unsigned int i = 0; i < dt.size(); i++)
    {
        if (dt[i] != a.dt[i])
        {
            return true;
        }
    }
    return (!(((open == a.open) &&
               (high == a.high) &&
               (low == a.low) &&
               (close == a.close) &&
               (volume == a.volume) &&
               (name == a.name))));
}

int TickDataMatrix::len() const
{
    return price.size();
}

void TickDataMatrix::clip_dt(DateTime st, DateTime en)
{
    int st_idx = 0, en_idx = (int) len() - 1;

    // find the idx of the beginning
    for (int i = 0; i < (int) (dt.size()); i++)
    {
        if (dt[i] >= st)
        {
            st_idx = i;
            break;
        }
    }

    // find the idx of the end
    for (int i = 0; i < (int) (dt.size()); i++)
    {
        if (dt[i] > en)
        {
            en_idx = i;
            break;
        }
    }

    // Erase all data to st_idx and after end_idx
    if (st_idx > 0)
    {
        dt.erase(dt.begin(), dt.begin() + (st_idx));
        price.erase(price.begin(), price.begin() + (st_idx));

        en_idx -= st_idx;
    }


    if (en_idx < ((int) len() - 1))
    {
        dt.erase(dt.begin() + en_idx, dt.end());
        price.erase(price.begin() + en_idx, price.end());
    }
}

void TickDataMatrix::clip_idx(int st, int en)
{
    int st_idx = st, en_idx = en;
    if (st_idx > en_idx)
    {
        int tmp = st_idx;
        en_idx = st_idx;
        st_idx = tmp;
    }
    if (st_idx < 0)
    {
        st_idx = 0;
    }
    if (en_idx > ((int) len() - 1))
    {
        en_idx = len() - 1;
    }

    if ((st_idx <= 0) && (en_idx >= ((int) len() - 1)))
    {
        return;
    }

    if (st_idx > 0)
    {
        dt.erase(dt.begin(), dt.begin() + (st_idx));
        price.erase(price.begin(), price.begin() + (st_idx));
        en_idx -= st_idx;
    }
    if ((en_idx > 0) && (en_idx <= (((int) len()) - 1)))
    {
        dt.erase(dt.begin() + en_idx, dt.end());
        price.erase(price.begin() + en_idx, price.end());
    }
}

TickDataMatrix TickDataMatrix::slice_dt(DateTime st, DateTime en) const
{
    TickDataMatrix d = *this;
    d.clip_dt(st, en);
    return d;
}

TickDataMatrix TickDataMatrix::slice_idx(int st, int en) const
{
    TickDataMatrix d = *this;
    d.clip_idx(st, en);
    return d;
}

void TickDataMatrix::clear()
{
    dt.clear();
    price.clear();
    data_period_seconds=0;
    data_time_index=0;
}

void TickDataMatrix::add_tick(DateTime a_dt, double a_price)
{
    dt.push_back(a_dt);
    price.push_back(a_price);
}

py::list TickDataMatrix::get_dt_list()
{
    PyDateTime_IMPORT;

    py::list l;

    for (unsigned int i = 0; i < dt.size(); i++)
    {
        py::object o(py::handle<>(

                // the PyObject*
                PyDateTime_FromDateAndTime(
                        dt[i].year,
                        dt[i].month,
                        dt[i].day,
                        dt[i].hour,
                        dt[i].minute,
                        dt[i].second,
                        0)

        ));

        l.append(o);
    }

    return l;
}

bool TickDataMatrix::operator==(const TickDataMatrix &a) const
{
    // compare the datetime arrays
    if (dt.size() != a.dt.size())
    {
        return false;
    }
    for (unsigned int i = 0; i < dt.size(); i++)
    {
        if (dt[i] != a.dt[i])
        {
            return false;
        }
    }
    return ((price == a.price) && (name == a.name));
}

bool TickDataMatrix::operator!=(const TickDataMatrix &a) const
{
    // compare the datetime arrays
    if (dt.size() != a.dt.size())
    {
        return true;
    }
    for (unsigned int i = 0; i < dt.size(); i++)
    {
        if (dt[i] != a.dt[i])
        {
            return true;
        }
    }
    return (!(((price == a.price) &&
               (name == a.name))));
}


void MultiDataMatrix::load_tsys(std::string fname)
{
    std::ifstream ifile(fname.c_str());
    int num_symbols = 0;
    ifile >> num_symbols;
    while (num_symbols--)
    {
        DataMatrix dm;
        dm.load_tsys_ifile(ifile);
        symbols.push_back(dm);
    }
    ifile.close();
}

unsigned int MultiDataMatrix::len() const
{
    unsigned int t_max_l = 0;
    for (unsigned int i = 0; i < symbols.size(); i++)
    {
        if (t_max_l < symbols[i].len())
        {
            t_max_l = symbols[i].len();
        }
    }

    if (t_max_l <= 0)
    {
        throw std::runtime_error("Data matrices are of unequal size");
    }

    return t_max_l;
}

void MultiDataMatrix::clip_dt(DateTime st, DateTime en)
{
    for (unsigned int i = 0; i < symbols.size(); i++)
    {
        symbols[i].clip_dt(st, en);
    }
}

void MultiDataMatrix::clip_idx(int st, int en)
{
    for (unsigned int i = 0; i < symbols.size(); i++)
    {
        symbols[i].clip_idx(st, en);
    }
}

MultiDataMatrix MultiDataMatrix::slice_dt(DateTime st, DateTime en) const
{
    MultiDataMatrix m = *this;
    m.clip_dt(st, en);
    return m;
}

MultiDataMatrix MultiDataMatrix::slice_idx(int st, int en) const
{
    MultiDataMatrix m = *this;
    m.clip_idx(st, en);
    return m;
}

/*TickDataMatrix DataMatrix::make_ticks(int ticks_per_bar, bool precise, int time_index, int seconds_per_bar)
{
    TickDataMatrix t;
    t.name = name;
    t.data_period_seconds = seconds_per_bar;
    t.data_time_index = time_index;

    if (ticks_per_bar > 60)
        std::runtime_error("Tick rate higher than 1 per second is not supported.");
    if (ticks_per_bar < 1)
        std::runtime_error("Invalid tick rate.");
    if ((seconds_per_bar < 1) || (seconds_per_bar < ticks_per_bar))
        std::runtime_error("Invalid data timeframe.");

    // for all bars
    for (int i = 0; i < dt.size(); i++)
    {
        // the time interpolation
        double start_time=0;
        double end_time=0;
        double delta_time=1;
        double start_price=0;
        double end_price=0;
        double delta_price=1;

        if (time_index == DATA_TIME_INDEX_OPEN)
        {
            start_time = dt[i].as_int();
            end_time = start_time + seconds_per_bar;

            start_price = open[i];
            if (i == (dt.size()-1)) // last bar?
            {
                end_price = close[i];
            }
            else
            {
                end_price = open[i + 1];
            }
        }
        else if (time_index == DATA_TIME_INDEX_CLOSE)
        {
            end_time = dt[i].as_int();
            start_time = end_time - seconds_per_bar;

            if (i==0)
            {
                start_price = open[i];
                end_price = close[i];
            }
            else
            {
                start_price = close[i-1];
                end_price = close[i];
            }
        }

        delta_time = (end_time - start_time) / ticks_per_bar;
        delta_price = (end_price - start_price) / ticks_per_bar;

        double st = start_time, sp = start_price;
        if (time_index == DATA_TIME_INDEX_CLOSE)
        {
            st += (seconds_per_bar/ticks_per_bar);
            sp += delta_price;
        }
        for (int j = 0; j < ticks_per_bar; j++)
        {
            DateTime a;
            a.from_int((long)st);

            t.dt.push_back(a);
            t.price.push_back(sp);

            sp += delta_price;
            st += delta_time;
        }
    }

    return t;
}
*/
