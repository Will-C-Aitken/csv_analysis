#include "csv.hpp"

int main() {

    string csv_path = "data/soccer_performance_data.csv";
    // Load CSV into data structure
    CSV soccer_stats = CSV(csv_path);

    // Perform Analyses
    Equals_Str eq_p {"Practice"};
    CSV_Filter st_eq_p = CSV_Filter("Session_Type", &eq_p);
    CSV_Stat_View avg_max_speed_p = CSV_Stat_View(STAT_METHOD::MEAN);
    soccer_stats.to_stat_view(avg_max_speed_p, "Name"s, "Max_Speed"s, st_eq_p);
    cout << "Player Average Session Max Speed (Practice)" << '\n';
    avg_max_speed_p.print();

    Equals_Str eq_g {"Game"};
    CSV_Filter st_eq_g = CSV_Filter("Session_Type", &eq_g);
    CSV_Stat_View avg_max_speed_g = CSV_Stat_View(STAT_METHOD::MEAN);
    soccer_stats.to_stat_view(avg_max_speed_g, "Name"s, "Max_Speed"s, st_eq_g);
    cout << '\n' << "Player Average Session Max Speed (Game)" << '\n';
    avg_max_speed_g.print();

    CSV_Stat_View mode_sleep_quality = CSV_Stat_View(STAT_METHOD::MODE);
    soccer_stats.to_stat_view(mode_sleep_quality, "Name"s, "Sleep_Quality"s);
    cout << '\n' << "Player Mode Sleep Quality" << '\n';
    mode_sleep_quality.print();

    CSV_Stat_View median_sleep_quality = CSV_Stat_View(STAT_METHOD::MEDIAN);
    soccer_stats.to_stat_view(median_sleep_quality, "Name"s, "Sleep_Quality"s);
    cout << '\n' << "Player Median Sleep Quality" << '\n';
    median_sleep_quality.print();

    CSV_Stat_View avg_sleep_quality = CSV_Stat_View(STAT_METHOD::MEAN);
    soccer_stats.to_stat_view(avg_sleep_quality, "Name"s, "Sleep_Quality"s);
    cout << '\n' << "Player Mean Sleep Quality" << '\n';
    avg_sleep_quality.print();

    // Query stat views
    cout << '\n' << "Players Faster in Game than Practice:" << '\n';
    for (const auto& s : avg_max_speed_g.gt(avg_max_speed_p)) {
	cout << s << '\n';
    }

    int n = 3;
    cout << '\n' << "N = " << n <<
	" Players with worst average Sleep Quality:" << '\n';
    for (const auto& s : avg_sleep_quality.bottom_n(n)) {
	cout << s << '\n';
    }

    return 0;
}


// --------------------------------------------------------------------------
// CSV Stat View Definitions
// --------------------------------------------------------------------------

CSV_Stat_View::CSV_Stat_View (const STAT_METHOD& sm) : sm{sm} {};

// return keys where this sv value is greater than sv2
vector<string> CSV_Stat_View::gt (CSV_Stat_View& sv2) {
    vector<string> results;
    for (auto& it : sv2.get_view()) {
	if (view[it.first] > it.second)
	    results.push_back(it.first);
    }
    return results;
}

// return keys of lowest values in view
vector<string> CSV_Stat_View::bottom_n (int n) {
    vector<string> results;

    // first create data structure that inverts map
    vector<pair<double, string>> inv_view_map;
    for (auto& it : view) {
	inv_view_map.push_back(pair(it.second, it.first));
    }
    
    // sort inverted map and get bottom n
    sort(inv_view_map.begin(), inv_view_map.end());
    for (int i = 0; i < n; i++) {
	results.push_back(inv_view_map[i].second);
    }
    return results;
}

double& CSV_Stat_View::operator[](const string& s) {
    return view[s];
}

void CSV_Stat_View::print() const {
    for (auto const& item : view) 
	cout << item.first << ": " << item.second << '\n';
}

// --------------------------------------------------------------------------
// CSV Class Definitions
// --------------------------------------------------------------------------

// Constructor that loads CSV from file
CSV::CSV(string file_path) {
    ifstream f;
    f.open(file_path);

    vector<string> fields;
    parse_next_line(f, fields);

    int i = 0;
    for (const string& header : fields) {
	// copy headers and give index
	headers[header] = i;
	i++;
    }
    num_cols = i;

    i = 0;
 
    while (parse_next_line(f, fields)) {
	i++;
	// emplace does construction of CSV_Row in place
	rows.emplace_back(fields);
    }
    num_rows = i;
}

// read-only accessor
string CSV::at_col_row(string h, int i) {
    return rows[i][headers[h]];
}

// Member function for converting to a `CSV_Stat_view`
void CSV::to_stat_view(CSV_Stat_View &sv, string name_header, 
	string data_header, CSV_Filter f) {

    // `collection` holds intermediate representation of numeric data. The
    // vector<double> will be passed to mean, mode, etc. function to reduce to
    // the single value in `view` member
    map<string, vector<double>> collection;
    for (int i = 0; i < num_rows; i++) {

	// apply filter
	if (!f(at_col_row(f.get_header(), i)))
	    continue;

	// Get the view key from the name column specified
	string key_idx = rows[i][headers[name_header]];
	// Convert string data from the data column to numeric
	double cur_val = stod(at_col_row(data_header, i));
	collection[key_idx].push_back(cur_val); 
    }

    STAT_METHOD sm = sv.get_sm();
    // Loop over each key in collection and calculate stat
    for (auto& it : collection) {
	switch(sm) {
	case STAT_METHOD::MEAN:
	    sv[it.first] = mean(collection[it.first]);
	    break;
	case STAT_METHOD::MODE:
	    sv[it.first] = mode(collection[it.first]);
	    break;
	case STAT_METHOD::MEDIAN:
	    sv[it.first] = median(collection[it.first]);
	    break;
	}
    }
}

// just calls to_stat_view with nop filter
void CSV::to_stat_view(CSV_Stat_View &sv, string name_header, 
	string data_header) {
    to_stat_view(sv, name_header, data_header, CSV_Filter());
}

// --------------------------------------------------------------------------
// Miscellaneous 
// --------------------------------------------------------------------------

double mean(const vector<double> &v) {
    double res = 0.0;
    int count = 0;
    for (const double& d : v) {
	res += d;
	count++;
    }
    return res/count;
}

double median(vector<double> &v) {
    size_t n = v.size() / 2;
    // only need to sort halfway (up to n)
    nth_element(v.begin(), v.begin()+n, v.end());
    return v[n];
}

double mode(vector<double>& v) {
    sort(v.begin(), v.end());
    double mode; 
    double cur_d = v[0];
    int cur_count = 1;
    int max_count = 0;
    
    // pop first element since it's already counted
    v.erase(v.begin()); 

    for (const double& d : v) {
	if (d != cur_d) {
	    if (cur_count > max_count) {
		mode = cur_d;
		max_count = cur_count;
	    }
	    cur_d = d;
	    cur_count = 1;
	}
	else
	    cur_count++;
    }

    // in case the last count is mode
    if (cur_count > max_count)
	mode = cur_d;

    return mode;
}

int parse_next_line(istream& fs, vector<string>& container) {
    string line;
    vector<string> fields;
    if (!getline(fs, line))
	return 0;
    // Handle differences in windows and linux csvs
    if (line.length() > 0 && line.back() == '\r')
	line.pop_back();

    stringstream ss(line);
    string field;
    while(getline(ss, field, ','))
	fields.push_back(field);
    // copy fields to container
    container = fields;
    return 1;
}
