#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Recursively scans a directory and updates a histogram based on file sizes
void traverse_dir(const char *dir_path, int bin_width, unsigned long **hist, int *hist_size) {
    DIR *dir = opendir(dir_path); // Open the directory
    if (dir == NULL) {
        perror(dir_path); // Print error if it can't be opened
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip the current (.) and parent (..) directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Construct full file/directory path
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        struct stat statbuf;
        if (lstat(path, &statbuf) < 0) {
            perror(path); // Skip if error getting info
            continue;
        }

        // If it's a subdirectory, go deeper recursively
        if (S_ISDIR(statbuf.st_mode)) {
            traverse_dir(path, bin_width, hist, hist_size);
        }
        // If it's a regular file, process its size
        else if (S_ISREG(statbuf.st_mode)) {
            off_t file_size = statbuf.st_size;
            int bin_index = file_size / bin_width; // Determine which bin this file belongs to

            // Resize histogram array if this bin doesn't exist yet
            if (bin_index >= *hist_size) {
                int new_size = bin_index + 1;
                unsigned long *new_hist = realloc(*hist, new_size * sizeof(unsigned long));
                if (new_hist == NULL) {
                    perror("realloc");
                    exit(EXIT_FAILURE);
                }

                // Initialize new bins to zero
                for (int i = *hist_size; i < new_size; i++) {
                    new_hist[i] = 0;
                }

                *hist = new_hist;
                *hist_size = new_size;
            }

            (*hist)[bin_index]++; // Increment the count for the appropriate bin
        }
    }

    closedir(dir); // Done reading the directory
}

int main(int argc, char *argv[]) {
    // Expect exactly two arguments: starting directory and bin width
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <start_directory> <bin_width>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *start_dir = argv[1];
    int bin_width = atoi(argv[2]); // Convert input to integer

    if (bin_width <= 0) {
        fprintf(stderr, "Error: bin_width must be a positive integer.\n");
        return EXIT_FAILURE;
    }

    // Allocate space for the histogram (start small, grow dynamically)
    int hist_size = 16;
    unsigned long *hist = calloc(hist_size, sizeof(unsigned long));
    if (hist == NULL) {
        perror("calloc");
        return EXIT_FAILURE;
    }

    // Start traversing the directory
    traverse_dir(start_dir, bin_width, &hist, &hist_size);

    // Print the histogram (file size ranges and counts)
    printf("Histogram of file sizes (bin width: %d bytes):\n", bin_width);
    for (int i = 0; i < hist_size; i++) {
        if (hist[i] > 0) {
            unsigned long lower = i * bin_width;
            unsigned long upper = (i + 1) * bin_width - 1;
            printf("%10lu - %10lu : %lu\n", lower, upper, hist[i]);
        }
    }

    free(hist); // Free allocated memory
    return EXIT_SUCCESS;
}
