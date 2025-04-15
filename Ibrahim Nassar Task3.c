#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure to represent each memory frame
typedef struct {
    int page_number;       // Page number stored in the frame
    unsigned int age;      // Used to approximate "least recently used" using 8 bits
    int referenced;        // Flag to indicate if the page was referenced recently
} PageFrame;

// Counts how many unique page numbers are in the reference list
int count_unique_pages(int *references, int num_references) {
    int *unique = NULL;
    int num_unique = 0;

    for (int i = 0; i < num_references; i++) {
        int found = 0;
        // Check if this page is already counted
        for (int j = 0; j < num_unique; j++) {
            if (unique && unique[j] == references[i]) {
                found = 1;
                break;
            }
        }
        if (!found) {
            // Add the unique page to our list
            unique = realloc(unique, (num_unique + 1) * sizeof(int));
            if (!unique) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
            unique[num_unique++] = references[i];
        }
    }

    free(unique);
    return num_unique;
}

// Runs the aging page replacement algorithm for a given number of frames
int simulate_aging(int *references, int num_references, int num_frames) {
    PageFrame *frames = calloc(num_frames, sizeof(PageFrame)); // Allocate space for frames
    if (!frames) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    int page_faults = 0;

    // Initialize all frames to be empty
    for (int i = 0; i < num_frames; i++) {
        frames[i].page_number = -1;
        frames[i].age = 0;
        frames[i].referenced = 0;
    }

    // Loop through all page references
    for (int i = 0; i < num_references; i++) {
        int current_page = references[i];
        int found = 0;

        // Check if the page is already in memory
        for (int j = 0; j < num_frames; j++) {
            if (frames[j].page_number == current_page) {
                found = 1;
                frames[j].referenced = 1; // Mark as recently used
                break;
            }
        }

        // Page fault: page not found in memory
        if (!found) {
            page_faults++;
            int replace_index = -1;
            unsigned int min_age = 0xFFFFFFFF;

            // Find the frame with the smallest age (least recently used)
            for (int j = 0; j < num_frames; j++) {
                if (frames[j].page_number == -1) {
                    replace_index = j; // Empty slot
                    break;
                }
                if (frames[j].age < min_age) {
                    min_age = frames[j].age;
                    replace_index = j;
                }
            }

            // Replace the chosen frame with the new page
            frames[replace_index].page_number = current_page;
            frames[replace_index].referenced = 1;
            frames[replace_index].age = 0;
        }

        // Age update step for all frames (bit shifting)
        for (int j = 0; j < num_frames; j++) {
            if (frames[j].page_number != -1) {
                // Shift age to the right and set MSB if it was referenced
                frames[j].age = (frames[j].age >> 1) | (frames[j].referenced << 7);
                frames[j].referenced = 0; // Reset referenced flag for next cycle
            }
        }
    }

    free(frames);
    return page_faults;
}

int main() {
    const char *filename = "Task3_input.txt";
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    int *references = NULL;
    int num_references = 0;
    int max_references = 0;
    char line[256];

    // Read all the page numbers from the input file
    while (fgets(line, sizeof(line), file)) {
        int page;
        if (sscanf(line, "%d", &page) != 1) {
            fprintf(stderr, "Skipping invalid line: %s", line);
            continue;
        }
        // Expand memory if needed
        if (num_references >= max_references) {
            max_references = (max_references == 0) ? 1 : max_references * 2;
            int *temp = realloc(references, max_references * sizeof(int));
            if (!temp) {
                perror("realloc");
                fclose(file);
                free(references);
                return 1;
            }
            references = temp;
        }
        references[num_references++] = page;
    }
    fclose(file);

    if (num_references == 0) {
        fprintf(stderr, "No references read from file.\n");
        free(references);
        return 1;
    }

    int num_unique = count_unique_pages(references, num_references);

    // Run the algorithm for 1 up to the number of unique frames
    for (int num_frames = 1; num_frames <= num_unique; num_frames++) {
        int page_faults = simulate_aging(references, num_references, num_frames);
        double faults_per_1000 = (double)page_faults * 1000.0 / num_references;
        printf("%d %.2f\n", num_frames, faults_per_1000);
    }

    free(references);
    return 0;
}
