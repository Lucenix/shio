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
#include "data/workload_replayer.hpp"
#include "csv.hpp"

// This simulator expects a CSV where the first column is a timestamp (seconds)
// and subsequent columns come in pairs: <data_bandwidth>,<metadata_bandwidth>
// for an arbitrary number of pairs. The simulator will print parsed values.

int main(int argc, char const *argv[]) {

    if (argc < 3) {
        std::cerr << std::format(
            "Usage: {} <workload_csv> <control_application_yaml> [output_csv_path]\n",
            argv[0]);
        return EXIT_FAILURE;
    } else {
        std::cerr << "Ran Command:";
        for (int i = 0; i < argc; i++) {
            std::cerr << " " << argv[i];
        }
        std::cerr << std::endl;
    }

    WorkloadReplayer workload_replayer(argv[1]);
    
    const uint64_t node_count = workload_replayer.get_node_count();
    
    auto control_app = ControlApplicationConfigParser::create_from_yaml(
        argv[2],
        workload_replayer.get_node_stats()
    );
    
    const std::filesystem::path output_path = argc > 3 ? std::filesystem::path(argv[3]) : "";
    std::function<void(uint64_t timestamp, const std::unordered_map<uint64_t, Bandwidths>)> write_enforced_row;
    std::ofstream enforced_csv(output_path);
    if (!enforced_csv) {
        write_enforced_row = [](uint64_t timestamp, const std::unordered_map<uint64_t, Bandwidths>& computed_bandwidths) noexcept {};
        std::cerr << std::format("Could not open output CSV for writing: {}\n", output_path.string());
    } else {
        enforced_csv << "timestamp,";
        for (size_t i = 0; i < node_count; ++i) {
            if (i > 0) {
                enforced_csv << ',';
            }
            enforced_csv << std::format("n{}_data,n{}_metadata", i, i);
        }
        enforced_csv << '\n';
        write_enforced_row = [&](uint64_t timestamp, const std::unordered_map<uint64_t, Bandwidths>& computed_bandwidths) {
            enforced_csv << timestamp;
            for (uint64_t p = 0; p < node_count; ++p) {
                const auto it = computed_bandwidths.find(p);
                const Bandwidths enforced = it != computed_bandwidths.end() ? it->second : Bandwidths{};
                enforced_csv << ',' << enforced.data_bandwidth << ',' << enforced.metadata_bandwidth;
            }
            enforced_csv << '\n';
        };
    }

    uint64_t original_timestamp = 0;
    bool all_enforced = false;
    auto control_step = [&](uint64_t timestamp, const std::unordered_map<uint64_t, Bandwidths>& original_bandwidths) {
        auto active_nodes = control_app->gather(timestamp, original_bandwidths);
        std::unordered_map<uint64_t, Bandwidths> computed_bandwidths;
        if (!active_nodes.empty()) {
            computed_bandwidths = control_app->compute(timestamp, active_nodes);
            all_enforced        = control_app->enforce(timestamp, computed_bandwidths);
        }
        write_enforced_row(timestamp, computed_bandwidths);
    };

    for (auto [timestamp, original_bandwidths]: workload_replayer) {
        control_step(timestamp, original_bandwidths);
    }

    //"\nEnforcing remaining bandwidths after timestamp {} until all requests are satisfied...\n", original_timestamp

    for (auto total_timestamp = workload_replayer.original_timestamp; !all_enforced; total_timestamp++) {
        control_step(total_timestamp, {});
    }

    if (enforced_csv) {
        std::cout << std::format("\nWrote enforced bandwidth CSV to {}\n", output_path.string());
    }

    std::cerr << "\nFinal Results";
    for (size_t i = 0; i < control_app->node_stats.size(); ++i) {
        std::cerr << std::format(",\nNode {}: {}", i, control_app->node_stats[i]->final_result());
    }
    std::cerr << '\n';

    return EXIT_SUCCESS;
}
