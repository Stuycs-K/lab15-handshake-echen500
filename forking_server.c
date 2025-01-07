#include "pipe_networking.h"
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>


void sighandler(int signo){
  unlink(WKP);
  exit(0);
}

void reap_zombie(int signo) {
  while (waitpid(-1, NULL, WNOHANG) > 0);
}
int main() {
  int to_client;
  int from_client;

  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, sighandler);
  signal(SIGCHLD, reap_zombie);
  srand(time(NULL));

  while (1) {
    from_client = server_setup();
    int pid = fork();
    if (pid == 0) {
        int to_client;
        server_handshake_half(&to_client, from_client);
        while (1) {
            int random_num = rand() % 100000000;
            if (write(to_client, &random_num, sizeof(random_num)) == -1) {
                close(to_client);
                close(from_client);
            
                exit(0);
            }
            sleep(1);
        }
    }
    else if (pid > 0) {
        close(from_client);
    }
    else {
        perror("fork subserver error");
    }
   
  }

  return 0;
}
 