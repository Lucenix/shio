#pragma once

#include <filesystem>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <yaml-cpp/yaml.h>
#include <functional>

class WorkloadReplayer {
public:

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

};
