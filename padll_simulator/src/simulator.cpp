/**
 *   Copyright (c) 2026 INESC TEC.
 **/

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "control_application/control_application.hpp"
#include "control_application/control_application_config_parser.hpp"
#include "csv.hpp"

// This simulator expects a CSV where the first column is a timestamp (seconds)
// and subsequent columns come in pairs: <data_bandwidth>,<metadata_bandwidth>
// for an arbitrary number of pairs. The simulator will print parsed values.

int main(int argc, char const *argv[]) {

    if (argc < 3 || argc > 5) {
        std::cerr << std::format(
            "Usage: {} <bandwidth_timeseries_file> <control_application_yaml> [output_csv_path]\n",
            argv[0]);
        return EXIT_FAILURE;
    }

    const std::filesystem::path input_path = argv[1];
    const std::filesystem::path control_app_config_path = argv[2];
    
    csv::CSVReader reader(argv[1]);
    const auto col_names = reader.get_col_names();
    const uint64_t col_count = static_cast<uint64_t>(col_names.size());

    if (col_count < 3) {
        std::cerr << "CSV must have a header row with at least 3 columns\n";
        return EXIT_FAILURE;
    }

    if ((col_count - 1) % 2 != 0) {
        std::cerr << std::format("CSV must have an even number of data/meta columns (after timestamp), has {} \n", col_count - 1);
        return EXIT_FAILURE;
    }

    const uint64_t pair_count = (col_count - 1) / 2;

    const std::filesystem::path output_path = argc > 3 ? std::filesystem::path(argv[3]) : "";

    auto control_app = ControlApplicationConfigParser::create_from_yaml(
        control_app_config_path,
        pair_count);
    
    std::ofstream enforced_csv(output_path);
    if (!enforced_csv) {
        std::cerr << std::format("Could not open output CSV for writing: {}\n", output_path.string());
    } else {
        for (size_t i = 0; i < col_names.size(); ++i) {
            if (i > 0) {
                enforced_csv << ',';
            }
            enforced_csv << col_names[i];
        }
        enforced_csv << '\n';
    }

    const auto write_enforced_row = [&](uint64_t timestamp, const std::unordered_map<uint64_t, Bandwidths>& computed_bandwidths) {
        if (enforced_csv) {
            enforced_csv << timestamp;
            for (uint64_t p = 0; p < pair_count; ++p) {
                const auto it = computed_bandwidths.find(p);
                const Bandwidths enforced = it != computed_bandwidths.end() ? it->second : Bandwidths{};
                enforced_csv << ',' << enforced.data_bandwidth << ',' << enforced.metadata_bandwidth;
            }
            enforced_csv << '\n';
        }
    };

    uint64_t original_timestamp = 0;
    bool all_enforced = true;

    for (csv::CSVRow &row : reader) {
        original_timestamp = row[0].get<uint64_t>();

        csv::CSVRow::iterator it = row.begin() + 1;
        std::unordered_map<uint64_t, Bandwidths> original_bandwidths;
        for (uint64_t p = 0; p < pair_count; ++p, it += 2) {
            auto data_bw = it->get<uint64_t>(), meta_bw = (it + 1)->get<uint64_t>();
            if (data_bw != 0 || meta_bw != 0) {
                original_bandwidths[p] = Bandwidths(data_bw, meta_bw);
            }
        }

        auto active_nodes = control_app->gather(original_timestamp, original_bandwidths);
        std::unordered_map<uint64_t, Bandwidths> computed_bandwidths;
        if (!active_nodes.empty()) {
            computed_bandwidths = control_app->compute(original_timestamp, active_nodes);
            all_enforced        = control_app->enforce(original_timestamp, computed_bandwidths);
        }
        write_enforced_row(original_timestamp, computed_bandwidths);
    }

    std::cout << std::format("\nEnforcing remaining bandwidths after timestamp {} until all requests are satisfied...\n", original_timestamp);

    for (auto total_timestamp = original_timestamp; !all_enforced; total_timestamp++) {
        auto active_nodes = control_app->gather(original_timestamp, {});
        std::unordered_map<uint64_t, Bandwidths> computed_bandwidths;
        if (!active_nodes.empty()) {
            computed_bandwidths = control_app->compute(total_timestamp, active_nodes);
            Bandwidths total_bandwidths;
            for (auto& [node_index, bandwidths] : computed_bandwidths) {
                total_bandwidths.gather(bandwidths);
            }
            if (total_bandwidths.metadata_bandwidth > control_app->total_system_bandwidth.metadata_bandwidth) {
                std::cerr << std::format("Computed metadata bandwidth {} surpassed total system bandwidth {}\n", 
                                            total_bandwidths.metadata_bandwidth,
                                            control_app->total_system_bandwidth.metadata_bandwidth);
                return EXIT_FAILURE;
            }
            all_enforced        = control_app->enforce(total_timestamp, computed_bandwidths);
        }
        write_enforced_row(total_timestamp, computed_bandwidths);
    }

    if (enforced_csv) {
        std::cout << std::format("\nWrote enforced bandwidth CSV to {}\n", output_path.string());
    }

    std::cerr << "\nFinal Results";
    for (size_t i = 0; i < control_app->node_stats.size(); ++i) {
        std::cerr << std::format(",\nNode {}: {}", i, control_app->node_stats[i].final_result());
    }
    std::cerr << '\n';

    return EXIT_SUCCESS;
}
