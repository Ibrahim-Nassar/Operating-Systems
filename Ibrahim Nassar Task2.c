#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#define MAX_WORDS 10000
#define OVERLAP 100

char *globalWords[MAX_WORDS];
int globalCounts[MAX_WORDS];
int globalWordCount = 0;
pthread_mutex_t globalMutex = PTHREAD_MUTEX_INITIALIZER;
char *buffer;

void updateWord(const char *word) {
    pthread_mutex_lock(&globalMutex);
    for (int i = 0; i < globalWordCount; i++) {
        if (strcmp(globalWords[i], word) == 0) {
            globalCounts[i]++;
            pthread_mutex_unlock(&globalMutex);
            return;
        }
    }
    if (globalWordCount < MAX_WORDS) {
        globalWords[globalWordCount] = strdup(word);
        globalCounts[globalWordCount] = 1;
        globalWordCount++;
    }
    pthread_mutex_unlock(&globalMutex);
}

typedef struct {
    long start;
    long end;
} ThreadArg;

void *worker(void *arg) {
    ThreadArg *a = (ThreadArg *)arg;
    long pos = a->start;
    long originalEnd = a->end;
    long fileSize = strlen(buffer);
    long extendedEnd = (a->end + OVERLAP < fileSize) ? a->end + OVERLAP : fileSize;

    if (pos > 0) {
        while (pos < extendedEnd && !isspace(buffer[pos]))
            pos++;
    }
    while (pos < extendedEnd) {
        while (pos < extendedEnd && isspace(buffer[pos]))
            pos++;
        if (pos >= extendedEnd)
            break;
        long wordStart = pos;
        long wordEnd = pos;
        while (wordEnd < extendedEnd && !isspace(buffer[wordEnd]))
            wordEnd++;
        if (wordStart < originalEnd) {
            int len = wordEnd - wordStart;
            char *word = malloc(len + 1);
            strncpy(word, buffer + wordStart, len);
            word[len] = '\0';
            for (int k = 0; k < len; k++)
                word[k] = tolower(word[k]);
            updateWord(word);
            free(word);
        }
        pos = wordEnd;
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <filename> <num_threads>\n", argv[0]);
        return 1;
    }
    int numThreads = atoi(argv[2]);
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = malloc(size + 1);
    if (fread(buffer, 1, size, f) != size) {
        perror("fread");
        return 1;
    }
    buffer[size] = '\0';
    fclose(f);

    pthread_t *threads = malloc(numThreads * sizeof(pthread_t));
    ThreadArg *args = malloc(numThreads * sizeof(ThreadArg));
    long seg = size / numThreads;
    for (int i = 0; i < numThreads; i++) {
        args[i].start = i * seg;
        args[i].end = (i == numThreads - 1) ? size : (i + 1) * seg;
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }
    for (int i = 0; i < numThreads; i++)
        pthread_join(threads[i], NULL);

    int total = 0;
    for (int i = 0; i < globalWordCount; i++) {
        printf("%s: %d\n", globalWords[i], globalCounts[i]);
        total += globalCounts[i];
        free(globalWords[i]);
    }
    printf("Total Words: %d\n", total);

    free(buffer);
    free(threads);
    free(args);
    return 0;
}
