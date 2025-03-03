Proof-of-Concept `C++` library for loading and performing analysis on CSV files
- [Soccer Performance Data](https://www.kaggle.com/datasets/michaelhegedusich/soccer-performance-data) included in `data/` for examples.

## Usage
The following is an example sequence of analysis steps:
- Load CSV from file with default constructor: `CSV soccer_stats = CSV(csv_path);`
- Optionally create a filter by first instantiating a `Comparison` object: `Equals_Str eq_p {"Practice"};` and then passing that object to the `CSV_Filter` constructor: `CSV_Filter st_eq_p = CSV_Filter("Session_Type", &eq_p);`
    - This example gives a filter that will skip over rows that do not have
        `Practice` in their `Session_Type` field.
- Prepare a CSV_View that will correspond to a specific type of statistical
    method: `CSV_Stat_View avg_max_speed_p = CSV_Stat_View(STAT_METHOD::MEAN);`
- Populate view from the CSV, optionally skipping over filtered rows using out prior specified filter: `soccer_stats.to_stat_view(avg_max_speed_p, "Name"s, "Max_Speed"s, st_eq_p);`
    - This example groups rows with the same field under the `Name` column and for each name, gets the mean of the data in the `Max_Speed` column. The passed filter skips non-practice data.
- Optionally Print as table: avg_max_speed_p.print();
- Optionally query the view: `avg_max_speed_p.bottom_n(3))`
    - This query returns the names of the 3 players with the slowest mean
        max speed in practices

## Extension
- Inherit `Comparison` class and overload `operator()` for new types of filters
- Specify new `STAT_METHODS` and handle new enumeration in `CSV::to_stat_view`
- Define new queries on `CSV_Stat_View`s as member methods
