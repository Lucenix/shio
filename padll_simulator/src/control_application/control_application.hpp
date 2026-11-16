#pragma once

#include <cstdint>
#include <format>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "data/bandwidths.hpp"
#include "data/node_stats.hpp"

class ControlApplication {
public:
    ControlApplication(std::vector<NodeStats*> node_stats_, Bandwidths total_system_bandwidth_);

    virtual ~ControlApplication() = default;

    std::vector<uint64_t> gather(uint64_t timestamp, const std::unordered_map<uint64_t, Bandwidths>& gathered_bandwidths);

    virtual std::unordered_map<uint64_t, Bandwidths> compute(uint64_t timestamp, const std::vector<uint64_t>& active_nodes) = 0;

    virtual bool enforce(uint64_t timestamp, std::unordered_map<uint64_t, Bandwidths>& computed_bandwidths);

    std::vector<NodeStats*> node_stats;
    uint64_t node_count;
    Bandwidths total_system_bandwidth;
};

