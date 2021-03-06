#include <errno.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

int running;
FILE *output_file;
int udp_serve(int (filter_fn)(char*, int), short port, const int buffer_bytes)
{
  int s = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in my_addr;
  memset(&my_addr, 0, sizeof(struct sockaddr_in));
  my_addr.sin_port = htons(port);
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = htonl(0);
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
      fwrite(buf, sizeof(char), bytes/sizeof(char), output_file);
      fflush(output_file);
    }
  }

  return 0;
}

int no_filter()
{
  return 1;
}

pcre2_code* re;
pcre2_match_data* pcre2data;
// Returns number of captured pairs + 1.
// Negative on failure to match.
int perform_match(char* subject, int len)
{
  int rc = pcre2_match(
      re,
      (PCRE2_SPTR8)subject,
      len,
      0,
      0,
      pcre2data,
      NULL);

  return rc;
}

int pcre_filter_serve(char *pattern, short port, int buffer_bytes)
{
  int pcre_err = 0;
  PCRE2_SIZE errOffset = 0;
  re = pcre2_compile(
      (PCRE2_SPTR8)pattern,
      strlen(pattern),
      0,
      &pcre_err, 
      &errOffset, 0);

  if (re == NULL)
  {
    PCRE2_UCHAR buffer[256];
    pcre2_get_error_message(pcre_err, buffer, 256);
    printf("Pattern compile failed with error %d. %s.", pcre_err, buffer);
    return 2;
  }

  pcre2data = pcre2_match_data_create_from_pattern(re, 0);

  if (pcre2data == NULL)
    return 3;

  printf("Matching pattern: \"%s\"\n", pattern);
  running = 1;
  udp_serve(perform_match, port, buffer_bytes);
  pcre2_match_data_free(pcre2data);
  pcre2_code_free(re);

  return 0;
}

void exit_with_usage(char* app)
{
  fprintf(
      stderr,
      "Usage: %s [-p port] [-e \"regex\"] -f file -b buffer_bytes\n",
      app);
  exit(1);
}

int main(int argc, char** argv)
{
  char* filter = ".*";
  short port = 5000;
  int opt, buffer_bytes = 4096;

  output_file = stdout;
  while ((opt = getopt(argc, argv, "hp:e:b:f:")) != -1) {
    switch (opt) {
      case 'p':
        port = atoi(optarg);
        break;
      case 'e':
        filter = optarg;
        break;
      case 'b':
        buffer_bytes = atoi(optarg);
        break;
      case 'f':
        output_file = fopen(optarg, "w");
        if (output_file == NULL)
        {
          printf("Failed to open file %s", optarg);
          exit_with_usage(argv[0]);
        }

        break;
      default: /* '?' */
        exit_with_usage(argv[0]);
    }
  }

  return pcre_filter_serve(filter, port, buffer_bytes);
}
