#include <netinet/udp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

int networkSocket;
struct sockaddr_in networkAddress;
int running = 1;

int
networkSend(void* buf, size_t len)
{
  return sendto(
      networkSocket,
      buf,
      len,
      0,
      (struct sockaddr*)&networkAddress,
      sizeof(struct sockaddr_in));
}

int udp_serve(int (filter_fn)(char*, int), in_addr_t addr, short port, const int buffer_bytes)
{
  int s = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in my_addr;
  memset(&my_addr, 0, sizeof(struct sockaddr_in));
  my_addr.sin_port = htons(port);
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = htonl(addr);
  bind(s, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in));
  printf(
      "Bind errno: %d Buffer size: %d port: %d\n",
      errno,
      buffer_bytes,
      port);

  char buf[buffer_bytes];
  memset(buf, 0, sizeof(buf));
  while (running)
  {
    ssize_t bytes = recvfrom(s, buf, buffer_bytes, 0, 0, 0);

    // printf("Bytes: %d Errno: %d\n", (int)bytes, errno);
    if (filter_fn((char*)&buf, bytes) > 0)
    {
      fwrite(buf, sizeof(char), bytes/sizeof(char), stdout);
      fflush(stdout);
    }
  }

  return 0;
}

void
initNetwork(short port)
{
  networkAddress.sin_port = htons(port);
  networkAddress.sin_family = AF_INET;
  networkAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
}

int main()
{
  networkSocket = socket(AF_INET, SOCK_DGRAM, 0);
  initNetwork(5000);

  int i = networkSend("Test\n", strlen("Test\n"));
  printf("send %d.", i);
}
