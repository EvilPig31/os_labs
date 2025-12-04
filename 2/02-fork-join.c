///usr/bin/cc -o /tmp/${0%%.c} -pthread $0 && exec /tmp/${0%%.c}
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <pthread.h>

typedef struct {
	size_t thread_id;
	size_t n_threads;
} ThreadArgs;

static void *task_a(void *_args)
{
	ThreadArgs *args = (ThreadArgs*)_args;

	printf("Thread #%d is doing task A\n", args->thread_id);
	usleep(350000);

	return NULL;
}

static void *task_b(void *_args)
{
	ThreadArgs *args = (ThreadArgs*)_args;

	printf("Thread #%d is doing task B\n", args->thread_id);
	usleep(500000);

	return NULL;
}

static void *task_c(void *_args)
{
	ThreadArgs *args = (ThreadArgs*)_args;

	printf("Thread #%d is doing task C\n", args->thread_id);
	usleep(100000);

	return NULL;
}

int main(int argc, char **argv)
{
	size_t n_threads = sysconf(_SC_NPROCESSORS_ONLN) - 1;

	pthread_t *threads = malloc(n_threads * sizeof(pthread_t));
	ThreadArgs *thread_args = malloc(n_threads * sizeof(ThreadArgs));

	for (size_t i = 0; i < n_threads; ++i) {
		thread_args[i] = (ThreadArgs){
			.thread_id = i,
			.n_threads = n_threads,
		};

		pthread_create(&threads[i], NULL, task_a, &thread_args[i]);
	}

	for (size_t i = 0; i < n_threads; ++i) {
		pthread_join(threads[i], NULL);
	}

	printf("*** Some serial work in-between A and B\n");

	for (size_t i = 0; i < n_threads; ++i) {
		thread_args[i] = (ThreadArgs){
			.thread_id = i,
			.n_threads = n_threads,
		};

		pthread_create(&threads[i], NULL, task_b, &thread_args[i]);
	}

	for (size_t i = 0; i < n_threads; ++i) {
		pthread_join(threads[i], NULL);
	}

	printf("*** Some serial work in-between B and C\n");

	for (size_t i = 0; i < n_threads; ++i) {
		thread_args[i] = (ThreadArgs){
			.thread_id = i,
			.n_threads = n_threads,
		};

		pthread_create(&threads[i], NULL, task_c, &thread_args[i]);
	}

	for (size_t i = 0; i < n_threads; ++i) {
		pthread_join(threads[i], NULL);
	}

	printf("*** Some serial work after C\n");

	free(thread_args);
	free(threads);
}
