#include "node_replayer.hpp"

NodeReplayer::NodeReplayer(uint64_t start_timestamp_, std::string trace_path, uint64_t priority) : 
    start_timestamp(start_timestamp_), trace_reader(trace_path), node_stats(start_timestamp_, priority) {}

std::optional<Bandwidths> NodeReplayer::get_bandwidths(uint64_t original_timestamp) {
    csv::CSVRow row;
    if (original_timestamp < start_timestamp) return Bandwidths();
    if (trace_reader.read_row(row)) {
        return Bandwidths(row[1].get<uint64_t>(), row[2].get<uint64_t>());
    }
    return std::nullopt;
}