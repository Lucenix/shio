#include "control_application_config_parser.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string>

#include "dynamic_max_min_control_application.hpp"
#include "static_control_application.hpp"
#include "psfa_control_application.hpp"
#include "promotion_control_application.hpp"

const std::unordered_map<
    std::string, 
    std::function<std::unique_ptr<ControlApplication>(
        const YAML::Node& root,
        const std::filesystem::path& config_path,
        std::vector<NodeStats*> node_stats,
        Bandwidths total_system_bandwidth)>> ControlApplicationConfigParser::application_map{
    {"static",          StaticControlApplication::parse},
    {"dynamic_max_min", DynamicMaxMinControlApplication::parse},
    {"psfa",            PSFAControlApplication::parse},
    {"promotion",       PromotionControlApplication::parse}
};

std::unique_ptr<ControlApplication> ControlApplicationConfigParser::create_from_yaml(
    const std::filesystem::path& config_path,
    std::vector<NodeStats*> node_stats
) {

    auto config = YAML::LoadFile(config_path);
    const Bandwidths total_system_bandwidth = read_total_system_bandwidth(config["total_system_bandwidth"]);
    std::string application_name = parse_entry<std::string>(config, "control_application");

    return application_map.at(application_name)(config, config_path, node_stats, total_system_bandwidth);
    
}

Bandwidths ControlApplicationConfigParser::read_total_system_bandwidth(const YAML::Node& value) {

    if (value && value.IsDefined() && value.IsMap()) {
        return Bandwidths{
            parse_entry<uint64_t>(value, "data", 0),
            parse_entry<uint64_t>(value, "metadata", 110000)
        };
    }

    return Bandwidths{0, 110000};
}

