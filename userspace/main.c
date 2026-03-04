#include <stdio.h>
#include <pthread.h>

#define NUMOFTHREADS 3

void* thread_function(void* arg) {
    int id = *(int*)arg;
    printf("Thread %d running\n", id);
    return NULL;
}

int main() {

    pthread_t threads[NUMOFTHREADS];
    int ids[NUMOFTHREADS];

    for (int i = 0; i < NUMOFTHREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, thread_function, &ids[i]);
    }

    for (int i = 0; i < NUMOFTHREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}