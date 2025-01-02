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
    exit(1);
  }
  int from_client = open(WKP, O_RDONLY);
  if (from_client == -1) {
    perror("error opening WKP");
    exit(1);
  }
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
    perror("Error reading client pipe name");
    unlink(WKP);
    exit(1);
  }
  unlink(WKP);

  *to_client = open(client_pipe, O_WRONLY);
  if (*to_client == -1) {
    perror("Error opening client pipe");
    exit(1);
  }

  int random_num = rand() % 100000;
  if (write(*to_client, &random_num, sizeof(random_num)) == -1) {
    perror("Error writing to client");
    exit(1);
  }

  int ack;
  if (read(from_client, &ack, sizeof(ack)) <= 0 || ack != random_num + 1) {
    perror("Handshake failed");
    close(*to_client);
    exit(1);
  }

  printf("Server: Handshake successful.\n");
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
    perror("Error creating private pipe");
    exit(1);
  }

  *to_server = open(WKP, O_WRONLY);
  if (*to_server == -1) {
    perror("Error connecting to server");
    unlink(private_pipe);
    exit(1);
  }

  if (write(*to_server, private_pipe, sizeof(private_pipe)) == -1) {
    perror("Error sending PP name");
    unlink(private_pipe);
    exit(1);
  }

  int private_fd = open(private_pipe, O_RDONLY);
  if (private_fd == -1) {
    perror("Error opening PP");
    unlink(private_pipe);
    exit(1);
  }

  int server_num;
  if (read(private_fd, &server_num, sizeof(server_num)) <= 0) {
    perror("Error reading from server");
    unlink(private_pipe);
    close(private_fd);
    exit(1);
  }

  int ack = server_num + 1;
  if (write(*to_server, &ack, sizeof(ack)) == -1) {
    perror("Error sending ack to server");
    unlink(private_pipe);
    close(private_fd);
    exit(1);
  }

  unlink(private_pipe);
  printf("Client: Handshake successful.\n");
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
    perror("Error reading client pipe name");
    exit(1);
  }

  int to_client = open(client_pipe, O_WRONLY);
  if (to_client == -1) {
    perror("Error opening client pipe");
    exit(1);
  }

  int random_num = rand() % 100000;
  if (write(to_client, &random_num, sizeof(random_num)) == -1) {
    perror("Error writing to client");
    close(to_client);
    exit(1);
  }

  int ack;
  if (read(from_client, &ack, sizeof(ack)) <= 0 || ack != random_num + 1) {
    perror("ERROR: Handshake failed");
    close(to_client);
    exit(1);
  }

  printf("Subserver: Handshake with client successful.\n");
  return to_client;
}


