/* Chat server using TCP for reliable, ordered broadcast */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 12345
#define BUF 1024

static int clients[100];
static int n_clients = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void broadcast(int sender, char *msg, int len){
    pthread_mutex_lock(&lock);
    for(int i = 0; i < n_clients; i++){
        int fd = clients[i];
        if(fd != sender) send(fd, msg, len, 0);
    }
    pthread_mutex_unlock(&lock);
}

void *handler(void *arg){
    int c = *(int*)arg;
    free(arg);

    char buf[BUF];
    while(1){
        int n = recv(c, buf, BUF, 0);
        if(n <= 0) break;
        broadcast(c, buf, n);
    }

    close(c);
    pthread_mutex_lock(&lock);
    for(int i = 0; i < n_clients; i++){
        if(clients[i] == c){
            clients[i] = clients[--n_clients];
            break;
        }
    }
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main(){
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if(s < 0){ perror("socket"); return 1; }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if(bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        perror("bind"); return 1;
    }
    if(listen(s, 5) < 0){
        perror("listen"); return 1;
    }

    while(1){
        int *c = malloc(sizeof(int));
        if(!c) continue;

        *c = accept(s, NULL, NULL);
        if(*c < 0){
            perror("accept");
            free(c);
            continue;
        }

        pthread_mutex_lock(&lock);
        clients[n_clients++] = *c;
        pthread_mutex_unlock(&lock);

        pthread_t tid;
        if(pthread_create(&tid, NULL, handler, c) != 0){
            perror("pthread_create");
            close(*c);
            free(c);
        }
        pthread_detach(tid);
    }

    close(s);
    return 0;
}
