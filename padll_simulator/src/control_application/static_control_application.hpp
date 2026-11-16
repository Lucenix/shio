#pragma once

#include <filesystem>
#include <memory>
#include <vector>
#include <yaml-cpp/yaml.h>

#include "control_application.hpp"
#include "control_application/control_application_config_parser.hpp"

class StaticControlApplication : public ControlApplication {
public:
    StaticControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_);
    StaticControlApplication(std::vector<NodeStats*> node_stats_, 
        Bandwidths total_system_bandwidth_, 
        std::vector<Bandwidths> static_allocation_);

    static std::unique_ptr<ControlApplication> parse(
        const YAML::Node& root,
        const std::filesystem::path& config_path,
        std::vector<NodeStats*> node_stats_,
        Bandwidths total_system_bandwidth);

    static std::vector<Bandwidths> parse_allocation_csv(const std::filesystem::path& path);

    std::unordered_map<uint64_t, Bandwidths> compute(uint64_t timestamp, const std::vector<uint64_t>& active_nodes) override;

    Bandwidths get_node_allocation(uint64_t node_index);

protected:
    std::vector<Bandwidths> static_allocation;
};