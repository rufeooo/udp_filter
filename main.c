#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

int main()
{
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(struct sockaddr_in));
    memset(&my_addr.sin_addr, 0, sizeof (struct in_addr));
    my_addr.sin_port = htons(5000);
    my_addr.sin_family = AF_INET;
    bind(s, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in));
    printf("Bind Errno: %d\n", errno);

    const int BUFLEN = 16;
    char buf[BUFLEN];
    memset(buf, 0, sizeof(buf));
    while (1)
    {
        ssize_t bytes = recvfrom(s, buf, BUFLEN, 0, 0, 0);

        printf("Bytes: %d Errno: %d\n", (int)bytes, errno);
        fwrite(buf, sizeof(char), bytes/sizeof(char), stdout);
    }

    return 0;
}
