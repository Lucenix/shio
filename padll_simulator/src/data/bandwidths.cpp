#include "bandwidths.hpp"

void Bandwidths::gather(const Bandwidths& gathered_bandwidth) {
    data_bandwidth += gathered_bandwidth.data_bandwidth;
    metadata_bandwidth += gathered_bandwidth.metadata_bandwidth;
}

bool Bandwidths::enforce(Bandwidths& computed_bandwidths) {
    const uint64_t granted_data = std::min(data_bandwidth, computed_bandwidths.data_bandwidth);
    const uint64_t granted_metadata = std::min(metadata_bandwidth, computed_bandwidths.metadata_bandwidth);

    const bool data_enforced = data_bandwidth <= computed_bandwidths.data_bandwidth;
    const bool metadata_enforced = metadata_bandwidth <= computed_bandwidths.metadata_bandwidth;

    computed_bandwidths.data_bandwidth = granted_data;
    computed_bandwidths.metadata_bandwidth = granted_metadata;

    data_bandwidth -= granted_data;
    metadata_bandwidth -= granted_metadata;

    return data_enforced && metadata_enforced;
}
