//
// Created by stefan on 8/10/23.
//

#include <arpa/inet.h
#include <cstring>
#include <netinet/in.h>>
#include <unistd.h>

int main() {
  sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(3000);
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  connect(fd, reinterpret_cast<sockaddr*>(&server), sizeof(server));
  char buffer[1024] = "a\nkdf\noakd\ndlafkfldsak\0\0lf";
  write(fd, buffer, 1024);
  read(fd, buffer, 1024);
  write(1, buffer, strlen(buffer));
}