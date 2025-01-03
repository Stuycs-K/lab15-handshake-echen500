#include "pipe_networking.h"

int main() {

  int to_server;
  int from_server;
  from_server = client_handshake( &to_server );
  while(1){
    int num;
    read(from_server, &num, sizeof(num));
    printf("%d\n", num);
  }
  
  
}
