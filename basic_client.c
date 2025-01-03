#include "pipe_networking.h"

int main() {

  int to_server;
  int from_server;
  from_server = client_handshake( &to_server );
  while(1){
    int num;
    if (read(from_server, &num, sizeof(num)) == 0) {
      exit(1);
    }
    printf("%d\n", num);
  }
  
  
}
