#include "control_application.hpp"
#include "dynamic_max_min_control_application.hpp"
#include "static_control_application.hpp"

#include <algorithm>
#include <cctype>

ControlApplication::ControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_)
    : node_stats(node_stats_), node_count(node_stats_.size()), total_system_bandwidth(total_system_bandwidth_) {}

std::vector<uint64_t> ControlApplication::gather(uint64_t timestamp, const std::unordered_map<uint64_t, Bandwidths>& gathered_bandwidths) {
    for (auto& [node_index, bandwidths] : gathered_bandwidths) {
        node_stats[node_index]->gather(timestamp, bandwidths);
    }

    std::vector<uint64_t> active_nodes;
    for (uint64_t node_index = 0; node_index < node_count; node_index++) {
        if (node_stats[node_index]->has_pending_requests()) {
            active_nodes.emplace_back(node_index);
        }
    }

    return active_nodes;
}

bool ControlApplication::enforce(uint64_t timestamp,
    std::unordered_map<uint64_t, Bandwidths>& computed_bandwidths) {
    bool all_enforced = true;
    for (auto& [node_index, bandwidths] : computed_bandwidths) {
        // do not try to enforce on completed nodes (even if bandwidth computed for them)
        // node_stats[node_index].has_pending_requests()  let it be a simulator responsability for now
        if (!node_stats[node_index]->enforce(timestamp, bandwidths)) {
            all_enforced = false;
        }
    }
    return all_enforced;
}