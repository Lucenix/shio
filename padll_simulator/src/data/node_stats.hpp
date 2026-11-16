#pragma once

#include <cstdint>
#include <format>
#include <string>

#include "bandwidths.hpp"

class NodeStats {
public:
    uint64_t start_time = 0;
    uint64_t original_time = 0;
    uint64_t total_time = 0;
    uint64_t priority = 0;

    Bandwidths requested_bandwidths;

    NodeStats() = default;
    NodeStats(uint64_t start_time_, uint64_t priority_) : start_time(start_time_), priority(priority_) {}

    void gather(uint64_t timestamp, const Bandwidths& gathered_bandwidth);

    bool enforce(uint64_t timestamp, Bandwidths& computed_bandwidths);

    std::string final_result() const;

    bool has_pending_requests() const;
};
