#pragma once

#include <memory>
#include <yaml-cpp/yaml.h>

#include "control_application.hpp"
#include "static_control_application.hpp"
#include "control_application/control_application_config_parser.hpp"

class DynamicMaxMinControlApplication : public StaticControlApplication {
public:
    DynamicMaxMinControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_, std::vector<Bandwidths> static_allocation_);
    DynamicMaxMinControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_);

    static std::unique_ptr<ControlApplication> parse(
        const YAML::Node& root,
        const std::filesystem::path& config_path,
        std::vector<NodeStats*> node_stats,
        Bandwidths total_system_bandwidth);

    std::unordered_map<uint64_t, Bandwidths> compute(uint64_t timestamp, const std::vector<uint64_t>& active_nodes) override;
    std::vector<uint64_t> order_by_bandwidth(const std::vector<uint64_t>& active_nodes);
};
