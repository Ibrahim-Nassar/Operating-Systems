#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    const char *input_filename = argv[1];
    FILE *f = fopen(input_filename, "r");
    int n, m;
    fscanf(f, "%d %d", &n, &m);
    
    int *E = malloc(m * sizeof(int));
    for (int i = 0; i < m; i++) 
    {
        fscanf(f, "%d", &E[i]);
    }
    
    int **C = malloc(n * sizeof(int *));
    for (int p = 0; p < n; p++) 
    {
        C[p] = malloc(m * sizeof(int));
        for (int i = 0; i < m; i++) {
            fscanf(f, "%d", &C[p][i]);
        }
    }
    
    int **R = malloc(n * sizeof(int *));
    for (int p = 0; p < n; p++) 
    {
        R[p] = malloc(m * sizeof(int));
        for (int i = 0; i < m; i++) {
            fscanf(f, "%d", &R[p][i]);
        }
    }
    fclose(f);
    
    int *available = malloc(m * sizeof(int));
    for (int i = 0; i < m; i++) 
    {
        int sum = 0;
        for (int p = 0; p < n; p++) {
            sum += C[p][i];
        }
        available[i] = E[i] - sum;
    }
    
    int *work = malloc(m * sizeof(int));
    memcpy(work, available, m * sizeof(int));
    int *marked = calloc(n, sizeof(int));
    
    while (1) {
        int prev_marked = 0;
        for (int p = 0; p < n; p++) 
        {
            if (marked[p]) {
                prev_marked++;
            }
        }
        
        for (int p = 0; p < n; p++) 
        {
            if (!marked[p]) {
                int can_allocate = 1;
                for (int i = 0; i < m; i++) 
                {
                    if (R[p][i] > work[i]) 
                    {
                        can_allocate = 0;
                        break;
                    }
                }
                if (can_allocate) 
                {
                    marked[p] = 1;
                    for (int i = 0; i < m; i++) {
                        work[i] += C[p][i];
                    }
                }
            }
        }
        
        int current_marked = 0;
        for (int p = 0; p < n; p++) 
        {
            if (marked[p]) {
                current_marked++;
            }
        }
        
        if (current_marked == prev_marked) 
        {
            break;
        }
    }
    
    int deadlocked_count = 0;
    for (int p = 0; p < n; p++) 
    {
        if (!marked[p]) {
            deadlocked_count++;
        }
    }
    
    if (deadlocked_count > 0) {
        printf("Deadlock detected. Processes involved are:");
        for (int p = 0; p < n; p++) 
        {
            if (!marked[p]) {
                printf(" %d", p);
            }
        }
        printf("\n");
    } else {
        printf("No deadlock detected.\n");
    }
    
    free(available);
    free(work);
    free(marked);
    free(E);
    for (int p = 0; p < n; p++) 
    {
        free(C[p]);
        free(R[p]);
    }
    free(C);
    free(R);
    
    return 0;
}
