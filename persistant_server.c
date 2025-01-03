#include "pipe_networking.h"
#include <time.h>
#include <fcntl.h>

int main() {
  int to_client;
  int from_client;  
  from_client = server_handshake( &to_client );
  while(1) {
    while(to_client) {
        srand(time(NULL));
        int random_num = rand() % 100000000;
         write(to_client, &random_num, sizeof(random_num));
        sleep(1);
    }
    from_client = server_handshake( &to_client );
  }
  
}
