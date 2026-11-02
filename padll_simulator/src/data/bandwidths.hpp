#pragma once

#include <cstdint>
#include <format>
#include <iostream>

class Bandwidths {
public:
    uint64_t data_bandwidth = 0;
    uint64_t metadata_bandwidth = 0;

    Bandwidths() = default;

    Bandwidths(uint64_t data_bandwidth_, uint64_t metadata_bandwidth_) noexcept
        : data_bandwidth(data_bandwidth_), metadata_bandwidth(metadata_bandwidth_) {}

    void gather(const Bandwidths& gathered_bandwidth);

    bool enforce(Bandwidths& computed_bandwidths);
};
