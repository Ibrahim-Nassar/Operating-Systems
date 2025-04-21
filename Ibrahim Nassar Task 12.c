#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DOMAINS   100
#define MAX_OBJECTS   100
#define MAX_NAME_LEN   64

typedef struct {
    char name[MAX_NAME_LEN];
} Domain;

typedef struct {
    char name[MAX_NAME_LEN];
} Object;

Domain domains[MAX_DOMAINS];
Object objects[MAX_OBJECTS];
int rights[MAX_DOMAINS][MAX_OBJECTS]; /* bitmask: 1=read, 2=write, 4=execute */
int domain_count = 0, object_count = 0;

int find_domain(const char *name) {
    for (int i = 0; i < domain_count; i++)
        if (strcmp(domains[i].name, name) == 0)
            return i;
    return -1;
}

int find_object(const char *name) {
    for (int j = 0; j < object_count; j++)
        if (strcmp(objects[j].name, name) == 0)
            return j;
    return -1;
}

void add_domain(const char *name) {
    if (domain_count < MAX_DOMAINS && find_domain(name) < 0) {
        strcpy(domains[domain_count].name, name);
        for (int j = 0; j < object_count; j++)
            rights[domain_count][j] = 0;
        domain_count++;
    }
}

void remove_domain(const char *name) {
    int idx = find_domain(name);
    if (idx >= 0) {
        for (int i = idx; i < domain_count - 1; i++)
            domains[i] = domains[i + 1];
        for (int i = idx; i < domain_count - 1; i++)
            for (int j = 0; j < object_count; j++)
                rights[i][j] = rights[i + 1][j];
        domain_count--;
    }
}

void add_object(const char *name) {
    if (object_count < MAX_OBJECTS && find_object(name) < 0) {
        strcpy(objects[object_count].name, name);
        for (int i = 0; i < domain_count; i++)
            rights[i][object_count] = 0;
        object_count++;
    }
}

void remove_object(const char *name) {
    int idx = find_object(name);
    if (idx >= 0) {
        for (int j = idx; j < object_count - 1; j++)
            objects[j] = objects[j + 1];
        for (int i = 0; i < domain_count; i++)
            for (int j = idx; j < object_count - 1; j++)
                rights[i][j] = rights[i][j + 1];
        object_count--;
    }
}

void grant_rights(const char *d, const char *o, int mask) {
    int di = find_domain(d), oi = find_object(o);
    if (di >= 0 && oi >= 0)
        rights[di][oi] |= mask;
}

void revoke_rights(const char *d, const char *o, int mask) {
    int di = find_domain(d), oi = find_object(o);
    if (di >= 0 && oi >= 0)
        rights[di][oi] &= ~mask;
}

void show_acl() {
    printf("        ");
    for (int j = 0; j < object_count; j++)
        printf("%-10s", objects[j].name);
    printf("\n");
    for (int i = 0; i < domain_count; i++) {
        printf("%-8s", domains[i].name);
        for (int j = 0; j < object_count; j++) {
            int r = rights[i][j];
            char buf[4] = "---";
            if (r & 1) buf[0] = 'r';
            if (r & 2) buf[1] = 'w';
            if (r & 4) buf[2] = 'x';
            printf("%-10s", buf);
        }
        printf("\n");
    }
}

int parse_mask(const char *s) {
    int m = 0;
    for (; *s; s++) {
        if (*s == 'r') m |= 1;
        if (*s == 'w') m |= 2;
        if (*s == 'x') m |= 4;
    }
    return m;
}

int main() {
    char cmd[32], d[MAX_NAME_LEN], o[MAX_NAME_LEN], mstr[8];
    while (1) {
        printf("\ncmd> ");
        if (scanf("%31s", cmd) != 1) break;
        if (strcmp(cmd, "add_dom") == 0) {
            scanf("%63s", d);
            add_domain(d);
        } else if (strcmp(cmd, "rm_dom") == 0) {
            scanf("%63s", d);
            remove_domain(d);
        } else if (strcmp(cmd, "add_obj") == 0) {
            scanf("%63s", o);
            add_object(o);
        } else if (strcmp(cmd, "rm_obj") == 0) {
            scanf("%63s", o);
            remove_object(o);
        } else if (strcmp(cmd, "grant") == 0) {
            scanf("%63s %63s %7s", d, o, mstr);
            grant_rights(d, o, parse_mask(mstr));
        } else if (strcmp(cmd, "revoke") == 0) {
            scanf("%63s %63s %7s", d, o, mstr);
            revoke_rights(d, o, parse_mask(mstr));
        } else if (strcmp(cmd, "show") == 0) {
            show_acl();
        } else if (strcmp(cmd, "exit") == 0) {
            break;
        } else {
            printf("unknown command\n");
        }
        while (getchar() != '\n');
    }
    return 0;
}
