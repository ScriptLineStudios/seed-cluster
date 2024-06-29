/*
    This main.c will be able to be compiled with any valid seed finder program
*/

#include <pthread.h>
#include <curl/curl.h>

#include "project/common.h"
#include "project/functions.c"

#define HTTP_IMPLEMENTATION
#include "libraries/http.h"

#include "cJSON/cJSON.h"

FILE *input_file = NULL, *output_file = NULL;
pthread_mutex_t outputMutex;

uint64_t GLOBAL_START_SEED = 0;
uint64_t GLOBAL_SEEDS_TO_CHECK = 100;
const int GLOBAL_NUMBER_OF_WORKERS = 8;
const char *OUTPUT_FILEPATH = "output.txt";
const char *INPUT_FILEPATH = NULL;
const bool TIME_PROGRAM = true;

bool get_next_seed(const void* worker_index, uint64_t *seed) {
	if (INPUT_FILEPATH) return fscanf(input_file, " %" PRId64 " \n", (int64_t *)seed) == 1;
	else {
		*seed = worker_index ? *(int *)worker_index + GLOBAL_START_SEED : *seed + GLOBAL_NUMBER_OF_WORKERS;
		return *seed - GLOBAL_START_SEED < GLOBAL_SEEDS_TO_CHECK;
	}
}

void output_value(const char *format, ...) {
	// vfprintf is thread-safe when printing to stdout, but not when printing to a file.
	// TODO: Is there any way to do this without mutexes?
	if (OUTPUT_FILEPATH) pthread_mutex_lock(&outputMutex);
	va_list args;
	va_start(args, format);
	// if (OUTPUT_FILEPATH) pthread_mutex_lock(&outputMutex);
	vfprintf(OUTPUT_FILEPATH ? output_file : stdout, format, args);
	if (OUTPUT_FILEPATH)  {
		fflush(OUTPUT_FILEPATH ? output_file : stdout);
		// pthread_mutex_unlock(&outputMutex);
	}
	va_end(args);
	if (OUTPUT_FILEPATH) pthread_mutex_unlock(&outputMutex);
}

cJSON *get_request_json(const char *url, void *args) {
	http_t* request = http_get(url, args);

	http_status_t status = HTTP_STATUS_PENDING;
	int prev_size = -1;
	while( status == HTTP_STATUS_PENDING ) {
		status = http_process( request );
		if(prev_size != (int)request->response_size ) {
			prev_size = (int) request->response_size;
		}
	}

	if(status == HTTP_STATUS_FAILED ) {
		printf( "HTTP request failed (%d): %s.\n", request->status_code, request->reason_phrase );
		http_release( request );
		return NULL;
	}

	cJSON *json = cJSON_Parse((char *)request->response_data);
	http_release(request);

	return json;
}

int init_client(void) {
	cJSON *json = get_request_json("http://127.0.0.1:5000/establish_connection", NULL);

	printf("%s\n", cJSON_Print(json));

	cJSON *cid = cJSON_GetObjectItemCaseSensitive(json, "cid");
	int client_id = cid->valueint;

 	cJSON_Delete(json);
	return client_id;
}

typedef struct {
	uint64_t start_seed;
	uint64_t end_seed;
} Work;

Work client_get_work(int cid) {
	char buffer[256];
	sprintf(buffer, "http://127.0.0.1:5000/get_work/%d", cid);
	cJSON *response_json = get_request_json(buffer, NULL);

	cJSON *work_json = cJSON_GetObjectItemCaseSensitive(response_json, "work_block");	
	uint64_t start_seed = (uint64_t)(cJSON_GetObjectItemCaseSensitive(work_json, "start_seed")->valueint);
	uint64_t end_seed = (uint64_t)(cJSON_GetObjectItemCaseSensitive(work_json, "end_seed")->valueint);		

	cJSON_Delete(response_json);

	return (Work){.start_seed=start_seed, .end_seed=end_seed};
}

int main() {
	if (INPUT_FILEPATH) {
		input_file = fopen(INPUT_FILEPATH, "r");
		if (!input_file) {
			fprintf(stderr, "Example main (pthreads).c: int main(): input_file = fopen(INPUT_FILEPATH, \"r\"): Failed to open %s.\n", INPUT_FILEPATH);
			exit(1);
		}
	}
	if (OUTPUT_FILEPATH) {
		output_file = fopen(OUTPUT_FILEPATH, "w");
		if (!output_file) {
			fprintf(stderr, "Example main (pthreads).c: int main(): output_file = fopen(OUTPUT_FILEPATH, \"w\"): Failed to open %s.\n", OUTPUT_FILEPATH);
			exit(1);
		}
	}
	init_globals();
	
	int client_id = init_client();
	printf("%d\n", client_id);

	Work work = client_get_work(client_id);

	struct timespec start_time, end_time;
	if (TIME_PROGRAM) clock_gettime(CLOCK_MONOTONIC, &start_time);
	pthread_t threads[GLOBAL_NUMBER_OF_WORKERS];
	int data[GLOBAL_NUMBER_OF_WORKERS];
	for (int i = 0; i < GLOBAL_NUMBER_OF_WORKERS; ++i) {
		data[i] = i;
		pthread_create(&threads[i], NULL, run_worker, &data[i]);
	}
	for (int i = 0; i < GLOBAL_NUMBER_OF_WORKERS; ++i) pthread_join(threads[i], NULL);
	if (INPUT_FILEPATH) fclose(input_file);
	if (OUTPUT_FILEPATH) {
		fflush(output_file);
		fclose(output_file);
	}
	if (TIME_PROGRAM) {
		clock_gettime(CLOCK_MONOTONIC, &end_time);
		fprintf(stdout, "(%.9g seconds)\n", end_time.tv_sec - start_time.tv_sec + (end_time.tv_nsec - start_time.tv_nsec)/1e9);
	}
	return 0;
}