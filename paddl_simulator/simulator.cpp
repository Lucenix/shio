#include <stdio.h>
#include <stdlib.h>

void bandwidth_parser() {
    
}

int main(int argc, char const *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <bandwidth_timeseries_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t read_bytes = 0;
    char*  timeseries_line = NULL;
    FILE*  bandwidths_file = fopen(argv[1], "r");

    while (getline (&timeseries_line, &read_bytes, bandwidths_file) != -1) {
        char timestamp[50];
        char bandwidth[50];

        if (sscanf (timeseries_line, "%[^,],%s", timestamp, bandwidth) < 2) {
            continue; // Skip malformed lines
        }

        printf("Timestamp: %s, Bandwidth: %s\n", timestamp, bandwidth);
    }

    return 0;
}
