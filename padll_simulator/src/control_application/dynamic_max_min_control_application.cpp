#include "dynamic_max_min_control_application.hpp"

#include <algorithm>
#include <numeric>

DynamicMaxMinControlApplication::DynamicMaxMinControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_, std::vector<Bandwidths> static_allocation_)
    : StaticControlApplication(node_stats_, total_system_bandwidth_, static_allocation_) {
}

DynamicMaxMinControlApplication::DynamicMaxMinControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_)
    : StaticControlApplication(node_stats_, total_system_bandwidth_) {
}

std::unique_ptr<ControlApplication> DynamicMaxMinControlApplication::parse(
    const YAML::Node& root,
    const std::filesystem::path& config_path,
    std::vector<NodeStats*> node_stats,
    Bandwidths total_system_bandwidth) {

    if (root["predefined_allocation"].IsDefined()) {
        const auto& allocation_path = std::filesystem::path(ControlApplicationConfigParser::parse_entry<std::string>(root, "predefined_allocation"));

        if (allocation_path.is_absolute()) {
            return std::make_unique<StaticControlApplication>(node_stats, 
                total_system_bandwidth, 
                parse_allocation_csv(allocation_path));
        } else {
            return std::make_unique<DynamicMaxMinControlApplication>(node_stats, 
                total_system_bandwidth, 
                parse_allocation_csv(config_path.parent_path() / allocation_path));
        }
    } else {
        return std::make_unique<DynamicMaxMinControlApplication>(node_stats, total_system_bandwidth);
    }

}

std::vector<uint64_t> DynamicMaxMinControlApplication::order_by_bandwidth(const std::vector<uint64_t>& active_nodes) {
    std::vector<uint64_t> sorted_vector(active_nodes);
    std::stable_sort(
        sorted_vector.begin(), 
        sorted_vector.end(), 
        [this](uint64_t v1, uint64_t v2){return get_node_allocation(v1).metadata_bandwidth <= get_node_allocation(v2).metadata_bandwidth;}
    );
    return sorted_vector;
}

std::unordered_map<uint64_t, Bandwidths> DynamicMaxMinControlApplication::compute(uint64_t timestamp, const std::vector<uint64_t>& active_nodes) {
    (void)timestamp;

    //TODO same algorithm for data through lambda

    std::unordered_map<uint64_t, Bandwidths> result;
    auto sorted_nodes = order_by_bandwidth(active_nodes);

    uint64_t remaining_meta = total_system_bandwidth.metadata_bandwidth;

    for (uint64_t i = 0; i < sorted_nodes.size(); i++) {
        const uint64_t node_index       = sorted_nodes[i];
        const uint64_t fair_share_meta  = remaining_meta / (sorted_nodes.size() - i);
        const uint64_t allocated_meta   = std::min(get_node_allocation(node_index).metadata_bandwidth, fair_share_meta);

        result[node_index].metadata_bandwidth += allocated_meta;
        remaining_meta -= allocated_meta;

        if (remaining_meta == 0) {
            break;
        }
    }

    const uint64_t fair_share_remaining = remaining_meta / sorted_nodes.size();
    if (fair_share_remaining > 0) {
        for (uint64_t node_index : sorted_nodes) {
            result[node_index].metadata_bandwidth += fair_share_remaining;
        }
    }

    return result;
}
