#include "pipe_networking.h"
#include <time.h>
#include <fcntl.h>

int main() {
  int to_client;
  int from_client;  
  //from_client = server_handshake( &to_client );
  while(1) {
    from_client = server_handshake( &to_client );
    while(1) {
        srand(time(NULL));
        int random_num = rand() % 100000000;
        if (write(to_client, &random_num, sizeof(random_num)) == 0) {
            close(to_client);
            close(from_client);
            unlink(to_client);
            break;
        }
        sleep(1);
    
    }
  }
  
}
