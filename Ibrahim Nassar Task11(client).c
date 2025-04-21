#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 12345
#define BUF 1024

void *recv_thread(void *arg){
    int s = *(int*)arg;
    char buf[BUF];
    while(1){
        int n = recv(s, buf, BUF, 0);
        if(n <= 0) break;
        write(STDOUT_FILENO, buf, n);
    }
    exit(0);
}

int main(int argc, char *argv[]){
    const char *server_ip;
    if(argc >= 2){
        server_ip = argv[1];
    } else {
        server_ip = "127.0.0.1";
    }

    int s = socket(AF_INET, SOCK_STREAM, 0);
    if(s < 0){ perror("socket"); return 1; }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(PORT);
    if(inet_pton(AF_INET, server_ip, &addr.sin_addr) <= 0){
        perror("inet_pton");
        return 1;
    }

    if(connect(s, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        perror("connect");
        return 1;
    }

    pthread_t tid;
    if(pthread_create(&tid, NULL, recv_thread, &s) != 0){
        perror("pthread_create");
        return 1;
    }

    char buf[BUF];
    while(fgets(buf, BUF, stdin)){
        send(s, buf, strlen(buf), 0);
    }

    close(s);
    return 0;
}
