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
  mkfifo(WKP, 0666);
  open(WKP, O_RDONLY);
  int from_client = 0;
  read(WKP,&from_client, sizeof(from_client));
  close(WKP);
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
  char pid[256];
  sprintf(pid, "%d", from_client);
  open(pid, O_WRONLY);
  srand(NULL);
  int num = rand() % 100000;
  write(pid,num, sizeof(int));
  open(WKP, O_RDONLY);
  int ack;
  read(WKP, &ack, sizeof(ack));
  if (ack == num + 1) {
    return from_client;
  }
  perror("Handshake failed");
  exit(1);
}


/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
  int pid = getpid();
  char PP[256];
  sprintf(PP, "%d", pid)
  mkfifo(PP, 0666); // make private pipe
  open(WKP, O_WRONLY);
  write(WKP, pid, sizeof(int));
  close(WKP);
  open(PP, O_RDONLY);
  int from_server;
  read(PP, &from_server, sizeof(int));
  unlink(PP);
  open(WKP, O_WRONLY);
  write(WKP, from_server + 1, sizeof(int));
  return from_server;
}


/*=========================
  server_connect
  args: int from_client

  handles the subserver portion of the 3 way handshake

  returns the file descriptor for the downstream pipe.
  =========================*/
int server_connect(int from_client) {
  int to_client  = 0;
  return to_client;
}


