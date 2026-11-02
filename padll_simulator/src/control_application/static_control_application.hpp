#pragma once

#include <filesystem>
#include <memory>
#include <vector>
#include <yaml-cpp/yaml.h>

#include "control_application.hpp"
#include "control_application/control_application_config_parser.hpp"

class StaticControlApplication : public ControlApplication {
public:
    StaticControlApplication(uint64_t node_count_, Bandwidths total_system_bandwidth_);
    StaticControlApplication(uint64_t node_count_, 
        Bandwidths total_system_bandwidth_, 
        std::vector<Bandwidths> predefined_allocation);

    static std::unique_ptr<ControlApplication> parse(
        const YAML::Node& root,
        const std::filesystem::path& config_path,
        uint64_t node_count,
        Bandwidths total_system_bandwidth);

    static std::vector<Bandwidths> parse_predefined_allocation(
        const YAML::Node& root,
        const std::filesystem::path& config_path,
        uint64_t expected_node_count);

    static std::vector<Bandwidths> parse_allocation_csv(const std::filesystem::path& path, uint64_t expected_node_count);

    static std::vector<Bandwidths> parse_allocation(
        const YAML::Node& root,
        const std::filesystem::path& config_path,
        uint64_t node_count);

    std::unordered_map<uint64_t, Bandwidths> compute(uint64_t timestamp, const std::vector<uint64_t>& active_nodes) override;

protected:
    std::vector<Bandwidths> static_allocation;
};