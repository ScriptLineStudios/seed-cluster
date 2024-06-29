#ifndef _COMMON_H
#define _COMMON_H

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// These are constants that must be set in the settings for each program.
extern const int GLOBAL_NUMBER_OF_WORKERS;
extern const char *INPUT_FILEPATH, *OUTPUT_FILEPATH;
extern const bool TIME_PROGRAM;

// These are global variables that will by default be set to their constant equivalents, but can be modified as needed.
extern uint64_t localStartSeed, localSeedsToCheck;
extern int localNumberOfWorkers;

// This is defined in the filtering program.
// It must initialize all global variables prior to any workers being created.
void init_globals();
// This is defined in the filtering program.
// It must initialize a worker; it is also recommended it
// - call `get_next_seed(workerIndex, &seed)` once to fetch the first seed (or abort if the function returns false),
// - have a do-while loop check each seed against some conditions, calling `output_value()` if the conditions are met; and
// - have the condition for the do-while loop call `get_next_seed(NULL, &seed)` and abort if the function returns false,
// though custom implementations are still allowed if one wishes.
void *run_worker(void *workerIndex);

// This is defined in the main template.
// If workerIndex is not NULL, it returns the first seed for the specified worker.
// Otherwise if workerIndex is NULL, it returns the next seed in the sequence.
// Returns false when the end of the sequence is reached. 
bool get_next_seed(const void* workerIndex, uint64_t *seed);
// This is defined in the main template.
// It prints the information for a single result given a setup identical to `printf()`.
void output_value(const char *format, ...);

#endif