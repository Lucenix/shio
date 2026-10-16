#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "csv.hpp"

// This simulator expects a CSV where the first column is a timestamp (seconds)
// and subsequent columns come in pairs: <data_bandwidth>,<metadata_bandwidth>
// for an arbitrary number of pairs. The simulator will print parsed values.

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        std::cerr << std::format("Usage: {} <bandwidth_timeseries_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    try {
        csv::CSVReader reader(argv[1]);
        size_t col_count = reader.get_col_names().size();
        size_t pair_count;

        if (col_count == 0) {
            std::cerr << "CSV must have a header row with at least one column\n";
            return EXIT_FAILURE;
        }

        if ((col_count - 1) % 2 != 0 || (pair_count = (col_count - 1) / 2) == 0) {
            std::cerr << "CSV must have an even number greater than 0 of data/meta columns (after timestamp)\n";
            return EXIT_FAILURE;
        }

        for (csv::CSVRow &row : reader) {
            // First column: timestamp
            auto timestamp = row[0].get<uint64_t>();
            std::cout << "Timestamp: " << timestamp;

            // Iterate over pairs
            csv::CSVRow::iterator it = row.begin() + 1; // Skip timestamp
            for (size_t p = 0; p < pair_count; ++p, it += 2) {
                
                auto data_bw = it->get<float>();
                auto meta_bw = (it + 1)->get<float>();

                std::cout << ", pair" << p << "(data=" << data_bw << ",meta=" << meta_bw << ")";
            }

            std::cout << '\n';
        }
    } catch (const std::exception &e) {
        std::cerr << "Failed to parse CSV: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
