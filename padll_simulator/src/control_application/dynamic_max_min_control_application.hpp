#pragma once

#include <memory>
#include <yaml-cpp/yaml.h>

#include "control_application.hpp"
#include "static_control_application.hpp"
#include "control_application/control_application_config_parser.hpp"

class DynamicMaxMinControlApplication final : public StaticControlApplication {
public:
    DynamicMaxMinControlApplication(uint64_t node_count_, Bandwidths total_system_bandwidth_, std::vector<Bandwidths> static_allocation_);
    DynamicMaxMinControlApplication(uint64_t node_count_, Bandwidths total_system_bandwidth_);

    static std::unique_ptr<ControlApplication> parse(
        const YAML::Node& root,
        const std::filesystem::path& config_path,
        uint64_t node_count,
        Bandwidths total_system_bandwidth);

    std::unordered_map<uint64_t, Bandwidths> compute(uint64_t timestamp, const std::vector<uint64_t>& active_nodes) override;
};
