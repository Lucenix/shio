#include "promotion_control_application.hpp"

#include <algorithm>
#include <numeric>

#include "csv.hpp"

PromotionControlApplication::PromotionControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_, std::vector<Bandwidths> static_allocation_, uint64_t threshold_factor_)
    : PSFAControlApplication(node_stats_, total_system_bandwidth_, static_allocation_, threshold_factor_)  {
}

PromotionControlApplication::PromotionControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_, uint64_t threshold_factor_)
    : PSFAControlApplication(node_stats_, total_system_bandwidth_, threshold_factor_) {
}

PromotionControlApplication::PromotionControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_, std::vector<Bandwidths> static_allocation_)
    : PSFAControlApplication(node_stats_, total_system_bandwidth_, static_allocation_) {
}

PromotionControlApplication::PromotionControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_)
    : PSFAControlApplication(node_stats_, total_system_bandwidth_) {
}

std::unique_ptr<ControlApplication> PromotionControlApplication::parse(
    const YAML::Node& root,
    const std::filesystem::path& config_path,
    std::vector<NodeStats*> node_stats,
    Bandwidths total_system_bandwidth) {

    std::optional<std::vector<Bandwidths>> static_allocation;
    std::optional<uint64_t> threshold_factor;
    std::unique_ptr<PromotionControlApplication> control_app; 

    if (root["threshold_factor"].IsDefined()) {
        threshold_factor = ControlApplicationConfigParser::parse_entry<uint64_t>(root, "threshold_factor");
    }

    if (root["predefined_allocation"].IsDefined()) {
        const auto& allocation_path = std::filesystem::path(ControlApplicationConfigParser::parse_entry<std::string>(root, "predefined_allocation"));

        if (allocation_path.is_absolute()) {
            static_allocation = parse_allocation_csv(allocation_path);
        } else {
            static_allocation = parse_allocation_csv(config_path.parent_path() / allocation_path);
        }
    }

    if (static_allocation.has_value() && threshold_factor.has_value()) {
        return std::make_unique<PromotionControlApplication>(node_stats, total_system_bandwidth, static_allocation.value(), threshold_factor.value());
    }

    if (static_allocation.has_value()) {
        return std::make_unique<PromotionControlApplication>(node_stats, total_system_bandwidth, static_allocation.value());
    }

    if (threshold_factor.has_value()) {
        return std::make_unique<PromotionControlApplication>(node_stats, total_system_bandwidth, threshold_factor.value());
    }

    return std::make_unique<PromotionControlApplication>(node_stats, total_system_bandwidth);
}

std::unordered_map<uint64_t, Bandwidths> PromotionControlApplication::compute(uint64_t timestamp, const std::vector<uint64_t>& active_nodes) {
    (void)timestamp;

    //TODO same algorithm for data through lambda

    std::unordered_map<uint64_t, Bandwidths> result;
    auto sorted_nodes = order_by_bandwidth(active_nodes);

    uint64_t remaining_meta = total_system_bandwidth.metadata_bandwidth;
    uint64_t total_usage    = 0;

    for (uint64_t i = 0; i < sorted_nodes.size(); i++) {
        const uint64_t node_index       = sorted_nodes[i];
        const uint64_t fair_share_meta  = remaining_meta / (sorted_nodes.size() - i);
        const uint64_t usage_meta       = last_enforced_usage[node_index].metadata_bandwidth;
                                          /* > 0? last_enforced_usage[node_index].metadata_bandwidth 
                                          : node_stats[node_index].requested_bandwidths.metadata_bandwidth; */
            total_usage += usage_meta;
        const uint64_t demand_meta      = get_node_allocation(node_index).metadata_bandwidth;
        const uint64_t threshold_meta   = (demand_meta - usage_meta) * threshold_factor;

        const uint64_t allocated_meta   = usage_meta <= demand_meta ? 
                                          std::min(usage_meta + threshold_meta, fair_share_meta) : 
                                          std::min(demand_meta, fair_share_meta);

        result[node_index].metadata_bandwidth += allocated_meta;
        remaining_meta -= allocated_meta;

        if (remaining_meta == 0) {
            break;
        }
    }

    if (remaining_meta > 0 && total_usage > 0) {
        for (uint64_t node_index : sorted_nodes) {
            const uint64_t usage_meta = last_enforced_usage[node_index].metadata_bandwidth;
                                        /* > 0 ? last_enforced_usage[node_index].metadata_bandwidth 
                                        : node_stats[node_index].requested_bandwidths.metadata_bandwidth; */
            const double usage_factor_meta = static_cast<double>(usage_meta) / total_usage;
            result[node_index].metadata_bandwidth += remaining_meta * usage_factor_meta;
        }
    }

    return result;
}
