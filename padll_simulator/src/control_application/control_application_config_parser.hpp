#pragma once

#include <filesystem>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <yaml-cpp/yaml.h>
#include <functional>

#include "control_application.hpp"

class StaticControlApplication;
class DynamicMaxMinControlApplication;

class ControlApplicationConfigParser {
public:

    static std::unique_ptr<ControlApplication> create_from_yaml(
        const std::filesystem::path& config_path,
        uint64_t node_count);

    template <typename T>
    static T parse_entry(const YAML::Node& node, const std::string& entry) {
        const YAML::Node value = node[entry];
        if (!value || !value.IsDefined()) {
            throw std::invalid_argument(std::format("Missing required YAML entry '{}'", entry));
        }
        return value.as<T>();
    }

    template <typename T>
    static T parse_entry(const YAML::Node& node, const std::string& entry, const T& fallback) {
        const YAML::Node value = node[entry];
        if (!value || !value.IsDefined()) {
            return fallback;
        }
        return value.as<T>();
    }

    static Bandwidths read_total_system_bandwidth(const YAML::Node& root);

    static const std::unordered_map<
        std::string, 
        std::function<std::unique_ptr<ControlApplication>(
            const YAML::Node& root,
            const std::filesystem::path& config_path,
            uint64_t node_count,
            Bandwidths total_system_bandwidth)>> application_map;

};
