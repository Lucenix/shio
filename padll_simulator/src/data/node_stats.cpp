#include "node_stats.hpp"

void NodeStats::gather(uint64_t timestamp, const Bandwidths& gathered_bandwidth) {
    original_time = timestamp;
    requested_bandwidths.gather(gathered_bandwidth);
}

bool NodeStats::enforce(uint64_t timestamp, Bandwidths& computed_bandwidths) {
    if (requested_bandwidths.enforce(computed_bandwidths)) {
        total_time = timestamp;
        return true;
    }
    return false;
}

std::string NodeStats::final_result() const {
    return std::format(
        "final time: {}, overhead: {}%",
        total_time,
        static_cast<double>(total_time - original_time) / (original_time - start_time) * 100.0);
}

bool NodeStats::has_pending_requests() const {
    return requested_bandwidths.data_bandwidth > 0 || requested_bandwidths.metadata_bandwidth > 0;
}
