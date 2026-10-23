#include "net/httpClient.h"
#include "net/httpMultipart.h"

#include <3ds/services/httpc.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HTTP_CHUNK_SIZE 0x1000
#define HTTP_UPLOAD_LIMIT UINT_MAX

Url init_url(const char *method, const char *token, const char *query) {
  Url result = {0};
  if (method != NULL)
    snprintf(result.method, sizeof(result.method), "%s", method);
  if (token != NULL)
    snprintf(result.token, sizeof(result.token), "%s", token);
  if (query != NULL)
    snprintf(result.query, sizeof(result.query), "%s", query);
  return result;
}

void get_url(Url *url_in, char *url_out, int is_post) {
  if (url_in == NULL || url_out == NULL)
    return;
  if (is_post) {
    snprintf(url_out, MAX_URL, "https://api.telegram.org/bot%s/%s",
             url_in->token, url_in->method);
  } else {
    snprintf(url_out, MAX_URL, "https://api.telegram.org/bot%s/%s?%s",
             url_in->token, url_in->method, url_in->query);
  }
}

static Result request_body(const char *url, HTTPC_RequestMethod method,
                           const char *content_type, const void *body,
                           size_t body_size, u8 **response) {
  httpcContext context;
  Result ret;
  u32 status_code = 0;
  size_t response_size = 0;
  size_t response_capacity = HTTP_CHUNK_SIZE + 1;
  u8 *response_data;

  if (response == NULL)
    return -1;
  *response = NULL;
  if (url == NULL || (body == NULL && body_size != 0) ||
      body_size > HTTP_UPLOAD_LIMIT)
    return -1;

  ret = httpcOpenContext(&context, method, url, method == HTTPC_METHOD_GET);
  if (ret != 0)
    return ret;

  /* Keep the SSL setting used by the existing 3DS client. */
  ret = httpcSetSSLOpt(&context, SSLCOPT_DisableVerify);
  if (ret != 0)
    goto close_context;
  ret = httpcSetKeepAlive(&context, HTTPC_KEEPALIVE_ENABLED);
  if (ret != 0)
    goto close_context;
  ret = httpcAddRequestHeaderField(&context, "User-Agent", "lib3dgram/1.0");
  if (ret != 0)
    goto close_context;
  if (content_type != NULL) {
    ret = httpcAddRequestHeaderField(&context, "Content-Type", content_type);
    if (ret != 0)
      goto close_context;
  }
  if (method == HTTPC_METHOD_POST && body_size != 0) {
    ret = httpcAddPostDataRaw(&context, (const u32 *)body, (u32)body_size);
    if (ret != 0)
      goto close_context;
  }

  ret = httpcBeginRequest(&context);
  if (ret != 0)
    goto close_context;
  ret = httpcGetResponseStatusCode(&context, &status_code);
  if (ret != 0)
    goto close_context;
  (void)status_code; /* Telegram reports API failures in its JSON response. */

  response_data = (u8 *)malloc(response_capacity);
  if (response_data == NULL) {
    ret = -1;
    goto close_context;
  }

  for (;;) {
    u32 received = 0;
    u8 *grown;
    if (response_capacity - response_size <= HTTP_CHUNK_SIZE) {
      if (response_capacity > (size_t)-1 - HTTP_CHUNK_SIZE - 1) {
        free(response_data);
        ret = -1;
        goto close_context;
      }
      response_capacity += HTTP_CHUNK_SIZE;
      grown = (u8 *)realloc(response_data, response_capacity);
      if (grown == NULL) {
        free(response_data);
        ret = -1;
        goto close_context;
      }
      response_data = grown;
    }
    ret = httpcDownloadData(&context, response_data + response_size,
                            HTTP_CHUNK_SIZE, &received);
    response_size += received;
    if (ret == (Result)HTTPC_RESULTCODE_DOWNLOADPENDING)
      continue;
    if (ret != 0) {
      free(response_data);
      goto close_context;
    }
    break;
  }

  response_data[response_size] = '\0';
  *response = response_data;
  ret = 0;

close_context:
  httpcCloseContext(&context);
  return ret;
}

static int read_file(const char *path, u8 **data, size_t *size) {
  FILE *file;
  long length;
  u8 *contents;
  size_t read_size;

  *data = NULL;
  *size = 0;
  file = fopen(path, "rb");
  if (file == NULL)
    return 0;
  if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) < 0 ||
      fseek(file, 0, SEEK_SET) != 0) {
    fclose(file);
    return 0;
  }
  contents = (u8 *)malloc(length ? (size_t)length : 1);
  if (contents == NULL) {
    fclose(file);
    return 0;
  }
  read_size = fread(contents, 1, (size_t)length, file);
  if (read_size != (size_t)length || ferror(file)) {
    free(contents);
    fclose(file);
    return 0;
  }
  fclose(file);
  *data = contents;
  *size = read_size;
  return 1;
}

static const char *file_basename(const char *path) {
  const char *base = path;
  const char *cursor;
  for (cursor = path; *cursor != '\0'; ++cursor) {
    if (*cursor == '/' || *cursor == '\\')
      base = cursor + 1;
  }
  return base;
}

static int telegram_reports_failure(const char *response) {
  const char *ok = strstr(response, "\"ok\"");
  if (ok == NULL)
    return 0;
  ok += 4;
  while (*ok == ' ' || *ok == '\t' || *ok == '\r' || *ok == '\n')
    ++ok;
  if (*ok++ != ':')
    return 0;
  while (*ok == ' ' || *ok == '\t' || *ok == '\r' || *ok == '\n')
    ++ok;
  return strncmp(ok, "false", 5) == 0;
}

int make_request(Url *url, HttpArgs *args, int args_len, char **response) {
  char request_url[MAX_URL];
  http_multipart multipart;
  u8 *body = NULL;
  size_t body_size = 0;
  u8 *result = NULL;
  const char *content_type = NULL;
  Result ret;
  int i;
  int multipart_initialized = 0;

  if (response == NULL)
    return -1;
  *response = NULL;
  if (url == NULL || args_len < 0 || (args_len > 0 && args == NULL))
    return -1;

  get_url(url, request_url, args_len > 0);
  if (request_url[0] == '\0' || strlen(request_url) >= MAX_URL - 1)
    return -1;

  if (args_len > 0) {
    ret = http_multipart_init(&multipart);
    if (ret != 0)
      return -1;
    multipart_initialized = 1;
    for (i = 0; i < args_len; ++i) {
      HttpArgs *arg = &args[i];
      const void *part_data;
      const char *filename = NULL;
      const char *part_type = arg->mime_type;
      size_t part_size;
      u8 *file_data = NULL;
      Result add_result;

      if (arg->name == NULL || arg->value == NULL)
        goto request_error;
      if (arg->is_disk_file) {
        if (!read_file(arg->value, &file_data, &part_size))
          goto request_error;
        part_data = file_data;
        filename = file_basename(arg->value);
        if (part_type == NULL)
          part_type = "application/octet-stream";
      } else if (arg->data_size > 0) {
        part_data = arg->value;
        part_size = arg->data_size;
        filename = arg->filename;
        if (filename != NULL && part_type == NULL)
          part_type = "application/octet-stream";
      } else {
        part_data = arg->value;
        part_size = strlen(arg->value);
      }
      add_result = http_multipart_add_data_part(
          &multipart, arg->name, filename, part_type, part_data, part_size);
      free(file_data);
      if (add_result != 0)
        goto request_error;
    }

    ret = http_multipart_serialize(&multipart, &body, &body_size);
    if (ret != 0)
      goto request_error;
    if (body_size > HTTP_UPLOAD_LIMIT)
      goto request_error;
    content_type = http_multipart_content_type(&multipart);
  }

  ret = request_body(request_url,
                     args_len > 0 ? HTTPC_METHOD_POST : HTTPC_METHOD_GET,
                     content_type, body, body_size, &result);
  free(body);
  if (multipart_initialized)
    http_multipart_free(&multipart);
  if (ret != 0 || result == NULL) {
    free(result);
    return -1;
  }
  if (telegram_reports_failure((const char *)result)) {
    free(result);
    return -1;
  }
  *response = (char *)result;
  return 0;

request_error:
  free(body);
  if (multipart_initialized)
    http_multipart_free(&multipart);
  return -1;
}

Result http_request(char *url, HTTPC_RequestMethod method, u8 **response) {
  const char *content_type =
      method == HTTPC_METHOD_POST ? "application/json" : NULL;
  return request_body(url, method, content_type, NULL, 0, response);
}
