#ifndef _FUNCTIONS_C
#define _FUNCTIONS_C

#include <inttypes.h>
#include "cubiomes/generator.h"
#include "common.h"

void init_globals() {}

void *run_worker(void *worker_index) {
	uint64_t seed;
    Generator g;
    setupGenerator(&g, MC_1_20, 0);

	if (!get_next_seed(worker_index, &seed)) return NULL;
    do {
        applySeed(&g, DIM_OVERWORLD, seed);

        int scale = 1; // scale=1: block coordinates, scale=4: biome coordinates
        int x = 0, y = 63, z = 0;
        int biomeID = getBiomeAt(&g, scale, x, y, z);

        if (biomeID == mushroom_fields) {
            output_value("%" PRIu64 "\n", seed);
        }           
	} while (get_next_seed(NULL, &seed));
	return NULL;
}

#endif