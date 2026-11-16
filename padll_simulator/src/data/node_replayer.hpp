#include <filesystem>
#include "node_stats.hpp"

#include "csv.hpp"

class NodeReplayer {
public:

    uint64_t start_timestamp;
    csv::CSVReader trace_reader;
    NodeStats node_stats;

    NodeReplayer(uint64_t start_timestamp, std::string trace_path, uint64_t priority);

    std::optional<Bandwidths> get_bandwidths(uint64_t original_timestamp);

};