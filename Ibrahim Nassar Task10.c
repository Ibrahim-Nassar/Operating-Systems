#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <crypt.h>
#include <time.h>

static const char salt_chars[] =
    "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

char *gen_salt(){
    char *s = malloc(3);
    if (!s) { perror("malloc"); exit(1); }
    s[0] = salt_chars[rand() % 64];
    s[1] = salt_chars[rand() % 64];
    s[2] = '\0';
    return s;
}

int main(int argc, char *argv[]){
    srand((unsigned)time(NULL));

    if (argc == 3 && strcmp(argv[1], "encrypt") == 0) {
        char *salt = gen_salt();
        char *e = crypt(argv[2], salt);
        if (!e) { perror("crypt"); return 1; }
        printf("%s\n", e);
        free(salt);
    }
    else if (argc == 4 && strcmp(argv[1], "check") == 0) {
        char *e = crypt(argv[2], argv[3]);
        if (!e) { perror("crypt"); return 1; }
        puts(strcmp(e, argv[3]) == 0 ? "Valid" : "Invalid");
    }
    else if (argc == 2 && strcmp(argv[1], "generate") == 0) {
        for (int i = 0; i < 10; i++) {
            char *salt = gen_salt();
            char *e = crypt("password", salt);
            if (!e) { perror("crypt"); return 1; }
            printf("%s\n", e);
            free(salt);
        }
    }
    else {
        fprintf(stderr,
            "Usage:\n"
            "  %s encrypt <password>\n"
            "  %s check <password> <encrypted>\n"
            "  %s generate\n",
            argv[0], argv[0], argv[0]
        );
        return 1;
    }

    return 0;
}

