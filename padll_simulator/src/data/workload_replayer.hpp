#pragma once

#include <filesystem>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <functional>

#include "node_replayer.hpp"

class WorkloadReplayer {
public:

    class iterator {
    public:
        uint64_t* original_timestamp;
        std::vector<NodeReplayer>* node_replayers;
        std::unordered_map<uint64_t, Bandwidths> current_timestamp_bandwidths;
        bool all_replayers_done = false;
    
        void populate_current_bandwidths();
        iterator();
        iterator(uint64_t* original_timestamp_, std::vector<NodeReplayer>* node_replayers_) ;
        const std::pair<const uint64_t, const std::unordered_map<uint64_t, Bandwidths>> operator*() const;
        iterator& operator++();
        bool operator!=(const iterator & other) const;
    };

    uint64_t original_timestamp = 0;
    std::vector<NodeReplayer> node_replayers;

    WorkloadReplayer(std::string config_path);

    std::vector<NodeStats*> get_node_stats();
    uint64_t get_node_count();

    iterator begin();
    iterator end();

};
