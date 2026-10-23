#ifndef HTTPCLIENT_HEADER
#define HTTPCLIENT_HEADER

#include <3ds.h>
#include <stddef.h>

#define MAX_URL 512
#define INL_QUERY_MAX_QUERY 128
#define MAX_METHOD 128
#define MAX_BOT_TOKEN 128

typedef struct {
  char method[MAX_METHOD];
  char token[MAX_BOT_TOKEN];
  char query[INL_QUERY_MAX_QUERY];
} Url;

typedef struct {
  char *name;
  char *value;
  char *mime_type;
  int is_disk_file;
  size_t data_size;
  char *filename;
} HttpArgs;

typedef struct {
  char *response;
  size_t size;
} r_memory;

Url init_url(const char *method, const char *token, const char *query);
void get_url(Url *url_in, char *url_out, int is_post);

int make_request(Url *url, HttpArgs *args, int args_len, char **response);

Result http_request(char *url, HTTPC_RequestMethod method, u8 **response);

#endif
