#include "static_control_application.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "csv.hpp"

StaticControlApplication::StaticControlApplication(uint64_t node_count_, Bandwidths total_system_bandwidth_)
    : ControlApplication(node_count_, total_system_bandwidth_) {
    static_allocation.resize(
        node_count,
        Bandwidths(
            total_system_bandwidth_.data_bandwidth / node_count,
            total_system_bandwidth_.metadata_bandwidth / node_count));
}

StaticControlApplication::StaticControlApplication(uint64_t node_count_, Bandwidths total_system_bandwidth_, std::vector<Bandwidths> predefined_allocation)
    : ControlApplication(node_count_, total_system_bandwidth_) {
    if (predefined_allocation.size() != node_count) {
        throw std::invalid_argument(std::format(
            "StaticControlApplication allocation size mismatch: expected {} entries for {} nodes, got {}",
            node_count,
            node_count_,
            predefined_allocation.size()));
    }
    static_allocation = std::move(predefined_allocation);
}



std::vector<Bandwidths> StaticControlApplication::parse_allocation_csv(const std::filesystem::path& path, uint64_t expected_node_count) {
    csv::CSVFormat format = csv::CSVFormat().no_header();
    csv::CSVReader reader(path.string(), format);

    std::vector<Bandwidths> allocations;
    for (const auto& row : reader) {
        if (row.size() < 2) {
            continue;
        }
        allocations.emplace_back(row[0].get<uint64_t>(), row[1].get<uint64_t>());
    }

    if (allocations.size() != expected_node_count) {
        throw std::invalid_argument(std::format(
            "StaticControlApplication allocation CSV has {} entries for {} nodes: {}",
            allocations.size(),
            expected_node_count,
            path.string()));
    }

    for (auto& allocation: allocations) {
        std::cout << allocation.metadata_bandwidth << ", ";
    }
    std::cout << std::endl;

    return allocations;
}

std::unique_ptr<ControlApplication> StaticControlApplication::parse(
    const YAML::Node& root,
    const std::filesystem::path& config_path,
    uint64_t node_count,
    Bandwidths total_system_bandwidth) {

    if (root["predefined_allocation"].IsDefined()) {
        const auto& allocation_path = std::filesystem::path(ControlApplicationConfigParser::parse_entry<std::string>(root, "predefined_allocation"));
        
        if (allocation_path.is_absolute()) {
            return std::make_unique<StaticControlApplication>(node_count, 
                total_system_bandwidth, 
                parse_allocation_csv(allocation_path, node_count));
        } else {
            return std::make_unique<StaticControlApplication>(node_count, 
                total_system_bandwidth, 
                parse_allocation_csv(config_path.parent_path() / allocation_path, node_count));
        }
    } else {
        return std::make_unique<StaticControlApplication>(node_count, total_system_bandwidth);
    }
}

std::unordered_map<uint64_t, Bandwidths> StaticControlApplication::compute(uint64_t timestamp, const std::vector<uint64_t>& active_nodes) {
    (void)timestamp;

    std::unordered_map<uint64_t, Bandwidths> result;
    for (auto node_index: active_nodes) {
        result[node_index] = static_allocation[node_index];
    }

    return result;
}
