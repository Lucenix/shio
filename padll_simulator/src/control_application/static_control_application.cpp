#include "static_control_application.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "csv.hpp"

StaticControlApplication::StaticControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_)
    : ControlApplication(node_stats_, total_system_bandwidth_), static_allocation(  node_stats_.size(), 
                                                                                    {Bandwidths(total_system_bandwidth_.data_bandwidth / node_stats_.size(),
                                                                                                total_system_bandwidth_.metadata_bandwidth / node_stats_.size())}) {
}

StaticControlApplication::StaticControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_, std::vector<Bandwidths> static_allocation_)
    : ControlApplication(node_stats_, total_system_bandwidth_) {
    static_allocation = std::move(static_allocation_);
}

std::vector<Bandwidths> StaticControlApplication::parse_allocation_csv(const std::filesystem::path& path) {
    csv::CSVFormat format = csv::CSVFormat();
    csv::CSVReader reader(path.string(), format);

    std::vector<Bandwidths> allocations;
    for (const auto& row : reader) {
        if (row.size() < 2) {
            continue;
        }
        allocations.emplace_back(row[0].get<uint64_t>(), row[1].get<uint64_t>());
    }

    std::cout << "Priorities read...";
    for (uint64_t i = 0; i < allocations.size(); i++) {
        std::cout << " " << i << ": " << allocations[i].data_bandwidth << ", " << allocations[i].metadata_bandwidth << ";";
    }
    std::cout << std::endl;

    return allocations;
}

std::unique_ptr<ControlApplication> StaticControlApplication::parse(
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
            return std::make_unique<StaticControlApplication>(node_stats, 
                total_system_bandwidth, 
                parse_allocation_csv(config_path.parent_path() / allocation_path));
        }
    } else {
        return std::make_unique<StaticControlApplication>(node_stats, total_system_bandwidth);
    }
}

std::unordered_map<uint64_t, Bandwidths> StaticControlApplication::compute(uint64_t timestamp, const std::vector<uint64_t>& active_nodes) {
    (void)timestamp;

    std::unordered_map<uint64_t, Bandwidths> result;
    for (auto node_index: active_nodes) {
        result[node_index] = static_allocation[node_stats[node_index]->priority];
    }

    return result;
}

Bandwidths StaticControlApplication::get_node_allocation(uint64_t node_index) {
    return static_allocation[node_stats[node_index]->priority];
}