///usr/bin/cc -o /tmp/${0%%.c} -pthread $0 && exec /tmp/${0%%.c}
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <pthread.h>

typedef struct {
	size_t number;
} ThreadArgs;

static void *work(void *_args)
{
	ThreadArgs *args = (ThreadArgs *)_args;

	printf("My number is: %d\n", args->number);

	return NULL;
}

int main(int argc, char **argv)
{
	size_t n_threads = sysconf(_SC_NPROCESSORS_ONLN) - 1;

	pthread_t *threads = malloc(n_threads * sizeof(pthread_t));
	ThreadArgs *thread_args = malloc(n_threads * sizeof(ThreadArgs));

	for (size_t i = 0; i < n_threads; ++i) {
		thread_args[i] = (ThreadArgs){
			.number = i,
		};

		pthread_create(&threads[i], NULL, work, &thread_args[i]);
	}

	for (size_t i = 0; i < n_threads; ++i) {
		pthread_join(threads[i], NULL);
	}

	free(thread_args);
	free(threads);
}
