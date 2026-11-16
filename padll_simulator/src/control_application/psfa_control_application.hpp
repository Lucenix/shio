#pragma once

#include <memory>
#include <yaml-cpp/yaml.h>

#include "control_application.hpp"
#include "dynamic_max_min_control_application.hpp"
#include "control_application/control_application_config_parser.hpp"

class PSFAControlApplication : public DynamicMaxMinControlApplication {
public:
    PSFAControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_, std::vector<Bandwidths> static_allocation_, uint64_t threshold_fraction_);
    PSFAControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_, uint64_t threshold_fraction_);
    PSFAControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_, std::vector<Bandwidths> static_allocation_);
    PSFAControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_);

    static std::unique_ptr<ControlApplication> parse(
        const YAML::Node& root,
        const std::filesystem::path& config_path,
        std::vector<NodeStats*> node_stats,
        Bandwidths total_system_bandwidth);

    std::unordered_map<uint64_t, Bandwidths> compute(uint64_t timestamp, const std::vector<uint64_t>& active_nodes) override;
    bool enforce(uint64_t timestamp, std::unordered_map<uint64_t, Bandwidths>& computed_bandwidths) override;

    const double threshold_factor = 0.05;
    std::unordered_map<uint64_t, Bandwidths> last_enforced_usage;
};
