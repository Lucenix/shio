#include "workload_replayer.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string>
#include <format>

#include "csv.hpp"

WorkloadReplayer::WorkloadReplayer(std::string config_path) {

    csv::CSVReader reader(config_path);
    if (reader.get_col_names().size() != 3) {
        throw std::invalid_argument(std::format("The file replayer {} does not contain 3 columns", config_path));
    }

    for (csv::CSVRow &row : reader) {
        const auto& start_timestamp = row[0].get<uint64_t>();
        const auto& trace_file      = row[1].get<std::string>(); 
        const auto& priority        = row[2].get<uint64_t>();
        
        std::cout << std::format("Node added to workload... start timestamp: {},\ttrace file: {},\tpriority: {}\n", 
            start_timestamp, std::filesystem::path(trace_file).filename().string(), priority);

        node_replayers.emplace_back(start_timestamp, trace_file, priority);
    }
}

std::vector<NodeStats*> WorkloadReplayer::get_node_stats() {
    std::vector<NodeStats*> node_stats;
    node_stats.reserve(node_replayers.size());
    for (auto& node_replayer: node_replayers) {
        node_stats.emplace_back(&node_replayer.node_stats);
    }
    return node_stats;
}

uint64_t WorkloadReplayer::get_node_count() {
    return node_replayers.size();
}

WorkloadReplayer::iterator WorkloadReplayer::begin() {
    return WorkloadReplayer::iterator(&original_timestamp, &node_replayers);
}

WorkloadReplayer::iterator WorkloadReplayer::end() {
    return WorkloadReplayer::iterator();
}

void WorkloadReplayer::iterator::populate_current_bandwidths() {
    uint64_t nodes_done = 0;
    current_timestamp_bandwidths = {};
    for (uint64_t i = 0; i < node_replayers->size(); i++) {
        std::optional<Bandwidths> node_bandwidths = (*node_replayers)[i].get_bandwidths(*original_timestamp);
        if (!node_bandwidths.has_value()) {
            nodes_done++;
        } else if (node_bandwidths.value().data_bandwidth != 0 || node_bandwidths.value().metadata_bandwidth != 0) {
            current_timestamp_bandwidths[i] = node_bandwidths.value();
        }
    }
    all_replayers_done = nodes_done == node_replayers->size();
}

WorkloadReplayer::iterator::iterator(uint64_t* original_timestamp_, std::vector<NodeReplayer>* node_replayers_) : 
    original_timestamp(original_timestamp_), node_replayers(node_replayers_) {
    populate_current_bandwidths();
}

WorkloadReplayer::iterator::iterator() : all_replayers_done(true) {}

WorkloadReplayer::iterator& WorkloadReplayer::iterator::operator++() {
    (*original_timestamp)++;
    populate_current_bandwidths();
    return *this;
}

const std::pair<const uint64_t, const std::unordered_map<uint64_t, Bandwidths>> WorkloadReplayer::iterator::operator*() const {
    return std::pair(*original_timestamp, current_timestamp_bandwidths);
}

bool WorkloadReplayer::iterator::operator!=(const WorkloadReplayer::iterator & other) const {
    return all_replayers_done != other.all_replayers_done;
}

