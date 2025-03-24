#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int pid;
    int arrival;
    int burst;
} Process;

void swap(Process *a, Process *b) {
    Process temp = *a;
    *a = *b;
    *b = temp;
}

int compareArrival(const void *a, const void *b) {
    Process *p1 = (Process *)a;
    Process *p2 = (Process *)b;
    return p1->arrival - p2->arrival;
}

void fcfs(Process proc[], int n) {
    qsort(proc, n, sizeof(Process), compareArrival);
    int current_time = 0, total_wait = 0;
    for (int i = 0; i < n; i++) {
        if (current_time < proc[i].arrival)
            current_time = proc[i].arrival;
        int wait_time = current_time - proc[i].arrival;
        total_wait += wait_time;
        current_time += proc[i].burst;
    }
    printf("FCFS Average Waiting Time: %.2f\n", (float)total_wait / n);
}

void sjf(Process proc[], int n) {
    Process *processes = malloc(n * sizeof(Process));
    for (int i = 0; i < n; i++)
        processes[i] = proc[i];
    qsort(processes, n, sizeof(Process), compareArrival);
    int completed = 0, current_time = 0, total_wait = 0;
    int *done = calloc(n, sizeof(int));
    while (completed < n) {
        int idx = -1, minBurst = 1e9;
        for (int i = 0; i < n; i++) {
            if (!done[i] && processes[i].arrival <= current_time) {
                if (processes[i].burst < minBurst) {
                    minBurst = processes[i].burst;
                    idx = i;
                }
            }
        }
        if (idx == -1) {
            current_time++;
            continue;
        }
        total_wait += current_time - processes[idx].arrival;
        current_time += processes[idx].burst;
        done[idx] = 1;
        completed++;
    }
    printf("SJF Average Waiting Time: %.2f\n", (float)total_wait / n);
    free(processes);
    free(done);
}

void roundRobin(Process proc[], int n, int quantum) {
    int *remaining = malloc(n * sizeof(int));
    int *wait_time = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        remaining[i] = proc[i].burst;
        wait_time[i] = 0;
    }
    Process *processes = malloc(n * sizeof(Process));
    for (int i = 0; i < n; i++)
        processes[i] = proc[i];
    qsort(processes, n, sizeof(Process), compareArrival);
    int current_time = 0, done = 0;
    int *queue = malloc(n * 10 * sizeof(int)), front = 0, rear = 0;
    int *inQueue = calloc(n, sizeof(int));
    for (int i = 0; i < n; i++) {
        if (processes[i].arrival <= current_time && !inQueue[i]) {
            queue[rear++] = i;
            inQueue[i] = 1;
        }
    }
    if (rear == 0) {
        current_time = processes[0].arrival;
        queue[rear++] = 0;
        inQueue[0] = 1;
    }
    while (done < n) {
        if (front == rear) {
            for (int i = 0; i < n; i++) {
                if (remaining[i] > 0) {
                    current_time = processes[i].arrival;
                    queue[rear++] = i;
                    inQueue[i] = 1;
                    break;
                }
            }
        }
        int idx = queue[front++];
        inQueue[idx] = 0;
        if (current_time < processes[idx].arrival)
            current_time = processes[idx].arrival;
        if (remaining[idx] > quantum) {
            current_time += quantum;
            remaining[idx] -= quantum;
        } else {
            current_time += remaining[idx];
            wait_time[idx] = current_time - processes[idx].arrival - processes[idx].burst;
            remaining[idx] = 0;
            done++;
        }
        for (int i = 0; i < n; i++) {
            if (remaining[i] > 0 && !inQueue[i] && processes[i].arrival <= current_time) {
                queue[rear++] = i;
                inQueue[i] = 1;
            }
        }
        if (remaining[idx] > 0) {
            queue[rear++] = idx;
            inQueue[idx] = 1;
        }
    }
    int total_wait = 0;
    for (int i = 0; i < n; i++)
        total_wait += wait_time[i];
    printf("Round Robin Average Waiting Time (Quantum = %d): %.2f\n", quantum, (float)total_wait / n);
    free(remaining);
    free(wait_time);
    free(queue);
    free(inQueue);
    free(processes);
}

int main() {
    int n;
    printf("Enter the number of processes: ");
    scanf("%d", &n);
    Process *proc = malloc(n * sizeof(Process));
    for (int i = 0; i < n; i++) {
        proc[i].pid = i + 1;
        printf("Enter arrival time and burst time for process %d: ", i + 1);
        scanf("%d %d", &proc[i].arrival, &proc[i].burst);
    }
    int quantum;
    printf("Enter the time quantum for Round Robin: ");
    scanf("%d", &quantum);
    Process *proc_fcfs = malloc(n * sizeof(Process));
    Process *proc_sjf  = malloc(n * sizeof(Process));
    Process *proc_rr   = malloc(n * sizeof(Process));
    for (int i = 0; i < n; i++) {
        proc_fcfs[i] = proc[i];
        proc_sjf[i] = proc[i];
        proc_rr[i]  = proc[i];
    }
    printf("\n--- Scheduling Algorithms Simulation ---\n");
    fcfs(proc_fcfs, n);
    sjf(proc_sjf, n);
    roundRobin(proc_rr, n, quantum);
    free(proc);
    free(proc_fcfs);
    free(proc_sjf);
    free(proc_rr);
    return 0;
}
