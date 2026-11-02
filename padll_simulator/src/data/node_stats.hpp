#pragma once

#include <cstdint>
#include <format>
#include <string>

#include "bandwidths.hpp"

class NodeStats {
public:
    uint64_t start_time = INT64_MAX;
    uint64_t total_time = 0;
    uint64_t original_time = 0;
    Bandwidths requested_bandwidths;

    void gather(uint64_t timestamp, const Bandwidths& gathered_bandwidth);

    bool enforce(uint64_t timestamp, Bandwidths& computed_bandwidths);

    std::string final_result() const;

    bool has_pending_requests() const;
};
