#pragma once

#include <memory>
#include <yaml-cpp/yaml.h>

#include "control_application.hpp"
#include "psfa_control_application.hpp"
#include "control_application/control_application_config_parser.hpp"

class PromotionControlApplication : public PSFAControlApplication {
public:
    PromotionControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_, std::vector<Bandwidths> static_allocation_, uint64_t threshold_fraction_);
    PromotionControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_, uint64_t threshold_fraction_);
    PromotionControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_, std::vector<Bandwidths> static_allocation_);
    PromotionControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_);

    static std::vector<uint64_t> parse_priorities_csv(const std::filesystem::path& path, uint64_t expected_node_count);

    static std::unique_ptr<ControlApplication> parse(
        const YAML::Node& root,
        const std::filesystem::path& config_path,
        std::vector<NodeStats*> node_stats,
        Bandwidths total_system_bandwidth);

    std::unordered_map<uint64_t, Bandwidths> compute(uint64_t timestamp, const std::vector<uint64_t>& active_nodes) override;
};
