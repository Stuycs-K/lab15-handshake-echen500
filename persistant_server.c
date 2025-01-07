#include "pipe_networking.h"
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

void sighandler(int signo){
  unlink(WKP);
  exit(0);
}

int main() {
  int to_client;
  int from_client;

  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, sighandler);
  srand(time(NULL));

  while (1) {
    from_client = server_handshake(&to_client);
    while (1) {
      int random_num = rand() % 100000000;
      
      if (write(to_client, &random_num, sizeof(random_num)) == -1) {
        close(to_client);
        close(from_client);
       
        break;
      }

      sleep(1); 
    }
  }

  return 0;
}
 