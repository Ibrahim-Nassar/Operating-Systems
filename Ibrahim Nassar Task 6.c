#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

void list_directory(const char *dir_name) {
    DIR *dir = opendir(dir_name);
    if (!dir) {
        fprintf(stderr, "ls: cannot open directory '%s': %s\n", dir_name, strerror(errno));
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
}

int main(int argc, char **argv) {
    if (argc == 1) {
        list_directory(".");
        return EXIT_SUCCESS;
    }

    for (int i = 1; i < argc; i++) {
        if (argc > 2) {
            printf("%s:\n", argv[i]);
        }

        list_directory(argv[i]);

        if (i < argc - 1) {
            printf("\n");
        }
    }

    return EXIT_SUCCESS;
}
