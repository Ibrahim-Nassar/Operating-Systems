#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

// Function to list all contents of a directory
void list_directory(const char *dir_name) {
    DIR *dir = opendir(dir_name); // Open the directory
    if (!dir) {
        // Print an error message if the directory can't be opened
        fprintf(stderr, "ls: cannot open directory '%s': %s\n", dir_name, strerror(errno));
        return;
    }

    struct dirent *entry;

    // Read and print each entry in the directory
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir); // Close the directory
}

int main(int argc, char **argv) {
    // If no arguments are given, list the current directory
    if (argc == 1) {
        list_directory(".");
        return EXIT_SUCCESS;
    }

    // If one or more directories are passed, list each one
    for (int i = 1; i < argc; i++) {
        if (argc > 2) {
            printf("%s:\n", argv[i]); // Print directory name as a header if multiple args
        }

        list_directory(argv[i]);

        if (i < argc - 1) {
            printf("\n"); // Add a blank line between multiple listings
        }
    }

    return EXIT_SUCCESS;
}
