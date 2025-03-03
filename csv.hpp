#include <limits>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <functional>

using namespace std;

// --------------------------------------------------------------------------
// Comparison Function Class Declaration(s) & Definition(s)
// --------------------------------------------------------------------------

// Abstract class for Comparison Function objects
// - Holds string value to be compared to in some way by overloading the
//   operator() method in subclasses
class Comparison {
public:
    Comparison(const string& v) : val{v} {};
    virtual bool operator()(const string& x) const =0;
    const string& get_val() const {return val;};
private:
    const string val;
};

// Inherits from Comparison class to perform comparison on internal `val` to
// check if it's equal to passed string. 
class Equals_Str: public Comparison {
public:
    Equals_Str(const string& v) : Comparison(v) {};
    bool operator()(const string& x) const override {return(this->get_val() == x);}
};


// --------------------------------------------------------------------------
// CSV Filter Class Declaration & Definition
// --------------------------------------------------------------------------

// Encapsulation of the more generic `Comparison` family to be specific to 
// filtering CSV rows.  
// - Default comparison is `nop`, i.e. no filtering
// - Holds header for CSV object to perform lookup on. Should be used to get
//   current row's string that is being filtered (see definition of
//   `CSV::to_stat_view` for example)
class CSV_Filter {
public:
    CSV_Filter() : comp{nullptr} {}
    CSV_Filter(const string& h, Comparison *f) : header{h}, comp{f} {}
    bool operator()(string x){if (!comp) return true; else return (*comp)(x);}
    const string& get_header() const {return header;}
private:
    Comparison* comp;
    const string header;
};


// --------------------------------------------------------------------------
// CSV Row Class Declaration and Definition
// --------------------------------------------------------------------------

// Container class for data in a CSV row
// - Uses string type since the data type for different CSV columns cannot be
//   known ahead of time 
// - This means that functions that perform analysis on this object should
//   always expect strings as input and perform conversions as needed
class CSV_Row {
public:
    CSV_Row(vector<string> input) : cells{move(input)} {}
    string& operator[](int i) {return cells[i];}
    const string& operator[](int i) const {return cells[i];}
private:
    vector<string> cells;
};


// --------------------------------------------------------------------------
// CSV Statistic View Declaration 
// --------------------------------------------------------------------------

// Options for statistical anaylsis on column data
enum class STAT_METHOD {MEAN, MEDIAN, MODE};

// Container class for CSV data that has been analyzed with `sm` STAT_METHOD
// - `view` maps a set of strings (e.g. names "Bill", "Anne", etc.) to their
//   calculated stat
// - Empty `CSV_Stat_View` should be passed to CSV::to_stat_view to populate
class CSV_Stat_View {
public:
    CSV_Stat_View (const STAT_METHOD& sm);
    double& operator[](const string& s);
    STAT_METHOD get_sm() const {return sm;}
    // print view map pairs, one per row
    void print() const;
    map<string, double>& get_view() {return view;}
    // query stat view(s)
    vector<string> gt (CSV_Stat_View& sv2);
    vector<string> bottom_n (int n);
private:
    map<string, double> view;
    double init_val;
    STAT_METHOD sm;
};


// --------------------------------------------------------------------------
// CSV Class Declaration 
// --------------------------------------------------------------------------

// Container class for CSV data 
// - 2D Vector matrix of strings (CSV_Rows are vectors<string> containers)
// - Headers map gives column index lookup in O(1) time 
// - Constructor loads CSV data from `file_path`
class CSV {
public:
    CSV(string file_path);
    // Copy data at column corresponding to header `h` and at row `i`
    string at_col_row(string h, int i);
    // Move and calculate relevant data to `CSV_Stat_view` after applying
    // filter `f`. Index view with `name_header`
    void to_stat_view(CSV_Stat_View &sv, string name_header, 
	string data_header, CSV_Filter f);
    // Overloaded `to_stat_view` without filter (creates nop filter that allows
    // everything through) 
    void to_stat_view(CSV_Stat_View &sv, string name_header, 
	    string data_header);
private:
    int num_cols;
    int num_rows;
    vector<CSV_Row> rows;
    map<string, int> headers;
};


// --------------------------------------------------------------------------
// Miscellaneous 
// --------------------------------------------------------------------------

double mean(const vector<double> &v);
// Note: mode and median edit vector in place for efficiency. Overwrite if
// preservation is needed 
double mode(vector<double>& v);
double median(vector<double> &v);
// parses next input stream line by comma into `fields` and returns success or
// failure to getline
int parse_next_line(istream& fs, vector<string>& fields);
