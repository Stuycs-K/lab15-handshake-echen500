#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include "pipe_networking.h"
#define READ 0
#define WRITE 1

//UPSTREAM = to the server / from the client
//DOWNSTREAM = to the client / from the server
/*=========================
  server_setup

  creates the WKP and opens it, waiting for a  connection.
  removes the WKP once a connection has been made

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_setup() {
  if (mkfifo(WKP, 0666) == -1) {
    perror("error creating WKP");
    remove(WKP);
    exit(1);
  }
  int from_client = open(WKP, O_RDONLY);
  if (from_client == -1) {
    perror("error opening WKP");
    exit(1);
  }
  unlink(WKP);
  return from_client;
}

/*=========================
  server_handshake 
  args: int * to_client

  Performs the server side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe (Client's private pipe).

  returns the file descriptor for the upstream pipe (see server setup).
  =========================*/
int server_handshake(int *to_client) {
  int from_client = server_setup();

  char client_pipe[256];
  if (read(from_client, client_pipe, sizeof(client_pipe)) <= 0) {
    perror("error reading client pipe name");
    unlink(WKP);
    exit(1);
  }
 

  *to_client = open(client_pipe, O_WRONLY);
  if (*to_client == -1) {
    perror("error opening client pipe");
    exit(1);
  }
  srand(time(NULL));
  int random_num = rand() % 100000;
  //printf("SYN_ACK: %d\n", random_num);
  if (write(*to_client, &random_num, sizeof(random_num)) == -1) {
    perror("error writing to client");
    exit(1);
  }

  int ack;
  if (read(from_client, &ack, sizeof(ack)) <= 0 || ack != random_num + 1) {
    perror("Handshake failed");
    close(*to_client);
    exit(1);
  }
 
  
  
  //printf("Server: Handshake complete.\n");

  // char test_byte;
  // if (read(from_client, &test_byte, 1) <= 0) {
  //     perror("error receiving test byte from client");
  //     close(*to_client);
  //     exit(1);
  // }
  // printf("To server, from client(should be C): %c\n", test_byte);

  // test_byte = 'S'; 
  // if (write(*to_client, &test_byte, 1) == -1) {
  //     perror("error sending test byte to client");
  //     close(*to_client);
  //     exit(1);
  // }
  return from_client;
}

int server_handshake_half(int *to_client, int from_client) {
  char client_pipe[256];
  if (read(from_client, client_pipe, sizeof(client_pipe)) <= 0) {
    perror("error reading client pipe name");
    unlink(WKP);
    exit(1);
  }
 

  *to_client = open(client_pipe, O_WRONLY);
  if (*to_client == -1) {
    perror("error opening client pipe");
    exit(1);
  }
  srand(time(NULL));
  int random_num = rand() % 100000;
  //printf("SYN_ACK: %d\n", random_num);
  if (write(*to_client, &random_num, sizeof(random_num)) == -1) {
    perror("error writing to client");
    exit(1);
  }

  int ack;
  if (read(from_client, &ack, sizeof(ack)) <= 0 || ack != random_num + 1) {
    perror("Handshake failed");
    close(*to_client);
    exit(1);
  }
 
  
  
  //printf("Server: Handshake complete.\n");

  // char test_byte;
  // if (read(from_client, &test_byte, 1) <= 0) {
  //     perror("error receiving test byte from client");
  //     close(*to_client);
  //     exit(1);
  // }
  // printf("To server, from client(should be C): %c\n", test_byte);

  // test_byte = 'S'; 
  // if (write(*to_client, &test_byte, 1) == -1) {
  //     perror("error sending test byte to client");
  //     close(*to_client);
  //     exit(1);
  // }
  return from_client;
}

/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
  char private_pipe[256];
  sprintf(private_pipe, "%d", getpid());

  if (mkfifo(private_pipe, 0666) == -1) {
    perror("error creating private pipe");
    exit(1);
  }

  *to_server = open(WKP, O_WRONLY);
  if (*to_server == -1) {
    perror("error connecting to server");
    unlink(private_pipe);
    exit(1);
  }

  if (write(*to_server, private_pipe, sizeof(private_pipe)) == -1) {
    perror("error sending PP name");
    unlink(private_pipe);
    exit(1);
  }

  int private_fd = open(private_pipe, O_RDONLY);
  if (private_fd == -1) {
    perror("error opening PP");
    unlink(private_pipe);
    exit(1);
  }

  int server_num;
  if (read(private_fd, &server_num, sizeof(server_num)) <= 0) {
    perror("error reading from server");
    unlink(private_pipe);
    close(private_fd);
    exit(1);
  }

  int ack = server_num + 1;
  //printf("ACK: %d\n", ack);
  if (write(*to_server, &ack, sizeof(ack)) == -1) {
    perror("error sending ack to server");
    unlink(private_pipe);
    close(private_fd);
    exit(1);
  }

  // printf("Client: Handshake complete.\n");
  // char test_byte = 'C'; 
  // if (write(*to_server, &test_byte, 1) == -1) {
  //     perror("error sending byte to server");
  //     unlink(private_pipe);
  //     close(private_fd);
  //     exit(1);
  // }

  // if (read(private_fd, &test_byte, 1) <= 0) {
  //     perror("error receiving byte from server");
  //     unlink(private_pipe);
  //     close(private_fd);
  //     exit(1);
  // }
  // printf("To client, from server (Should be S): %c\n", test_byte);
  unlink(private_pipe);
  return private_fd;
}


/*=========================
  server_connect
  args: int from_client

  handles the subserver portion of the 3 way handshake

  returns the file descriptor for the downstream pipe.
  =========================*/
int server_connect(int from_client) {
  char client_pipe[256];
  
  if (read(from_client, client_pipe, sizeof(client_pipe)) <= 0) {
    perror("error reading client pipe name");
    exit(1);
  }

  int to_client = open(client_pipe, O_WRONLY);
  if (to_client == -1) {
    perror("error opening client pipe");
    exit(1);
  }
  srand(time(NULL));
  int random_num = rand() % 100000;
  if (write(to_client, &random_num, sizeof(random_num)) == -1) {
    perror("error writing to client");
    close(to_client);
    exit(1);
  }

  int ack;
  if (read(from_client, &ack, sizeof(ack)) <= 0 || ack != random_num + 1) {
    perror("error: Handshake failed");
    close(to_client);
    exit(1);
  }

  printf("Subserver: Handshake with client complete.\n");
  return to_client;
}


