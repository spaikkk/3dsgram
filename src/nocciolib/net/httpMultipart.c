#include "net/httpMultipart.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HTTP_MULTIPART_DEFAULT_TYPE "application/octet-stream"

typedef struct {
  u8 *data;
  size_t size;
  size_t capacity;
} multipart_buffer;

static unsigned long boundary_counter;

static int valid_header_value(const char *value) {
  const unsigned char *p = (const unsigned char *)value;
  if (value == NULL || *value == '\0')
    return 0;
  for (; *p; ++p) {
    if (*p < 0x20 || *p == 0x7f)
      return 0;
  }
  return 1;
}

static int valid_disposition_value(const char *value) {
  const unsigned char *p = (const unsigned char *)value;
  if (value == NULL || *value == '\0')
    return 0;
  for (; *p; ++p) {
    if ((*p < 0x20 && *p != '\r' && *p != '\n') || *p == 0x7f)
      return 0;
  }
  return 1;
}

static char *duplicate_string(const char *value) {
  size_t length = strlen(value) + 1;
  char *copy = (char *)malloc(length);
  if (copy != NULL)
    memcpy(copy, value, length);
  return copy;
}

static int buffer_append(multipart_buffer *buffer, const void *data,
                         size_t size) {
  size_t needed;
  size_t capacity;
  u8 *grown;

  if (size > (size_t)-1 - buffer->size)
    return 0;
  needed = buffer->size + size;
  if (needed > buffer->capacity) {
    capacity = buffer->capacity ? buffer->capacity : 256;
    while (capacity < needed) {
      if (capacity > (size_t)-1 / 2) {
        capacity = needed;
        break;
      }
      capacity *= 2;
    }
    grown = (u8 *)realloc(buffer->data, capacity);
    if (grown == NULL)
      return 0;
    buffer->data = grown;
    buffer->capacity = capacity;
  }
  if (size != 0)
    memcpy(buffer->data + buffer->size, data, size);
  buffer->size = needed;
  return 1;
}

static int buffer_append_string(multipart_buffer *buffer, const char *text) {
  return buffer_append(buffer, text, strlen(text));
}

/* Percent-encode characters that need escaping in MIME disposition
 * parameters, following libcurl's compact MIME header handling. */
static int buffer_append_quoted(multipart_buffer *buffer, const char *value) {
  const unsigned char *p = (const unsigned char *)value;
  char escaped[4];
  for (; *p; ++p) {
    if (*p == '"' || *p == '\\' || *p == '\r' || *p == '\n') {
      snprintf(escaped, sizeof(escaped), "%%%02X", *p);
      if (!buffer_append(buffer, escaped, 3))
        return 0;
    } else if (!buffer_append(buffer, p, 1)) {
      return 0;
    }
  }
  return 1;
}

Result http_multipart_init(http_multipart *multipart) {
  if (multipart == NULL)
    return -1;
  memset(multipart, 0, sizeof(*multipart));
  /* A per-process counter plus rand() keeps boundaries distinct without
   * requiring a platform-specific random API. */
  ++boundary_counter;
  snprintf(multipart->boundary, sizeof(multipart->boundary),
           "--------------------------3dgram-%08lx-%08lx",
           (unsigned long)rand(), boundary_counter);
  return 0;
}

static Result add_part(http_multipart *multipart, const char *name,
                       const char *filename, const char *content_type,
                       const void *data, size_t size) {
  http_multipart_part *part;
  if (multipart == NULL || multipart->boundary[0] == '\0' ||
      !valid_disposition_value(name) ||
      (filename && !valid_disposition_value(filename)) ||
      (content_type && !valid_header_value(content_type)) ||
      (data == NULL && size != 0))
    return -1;

  part = (http_multipart_part *)calloc(1, sizeof(*part));
  if (part == NULL)
    return -1;
  part->name = duplicate_string(name);
  if (filename != NULL)
    part->filename = duplicate_string(filename);
  if (content_type != NULL)
    part->content_type = duplicate_string(content_type);
  if (size != 0) {
    part->data = (u8 *)malloc(size);
    if (part->data != NULL)
      memcpy(part->data, data, size);
  }
  if (part->name == NULL || (filename && part->filename == NULL) ||
      (content_type && part->content_type == NULL) ||
      (size != 0 && part->data == NULL)) {
    free(part->name);
    free(part->filename);
    free(part->content_type);
    free(part->data);
    free(part);
    return -1;
  }
  part->size = size;
  if (multipart->last != NULL)
    multipart->last->next = part;
  else
    multipart->first = part;
  multipart->last = part;
  return 0;
}

Result http_multipart_add_data_part(http_multipart *multipart,
                                    const char *name, const char *filename,
                                    const char *content_type,
                                    const void *data, size_t size) {
  return add_part(multipart, name, filename, content_type, data, size);
}

Result http_multipart_add_part(http_multipart *multipart, const char *name,
                               const char *value) {
  if (value == NULL)
    return -1;
  return http_multipart_add_data_part(multipart, name, NULL, NULL, value,
                                      strlen(value));
}

Result http_multipart_add_file_part(http_multipart *multipart,
                                    const char *name, const char *filename,
                                    const char *content_type,
                                    const void *data, size_t size) {
  if (filename == NULL)
    return -1;
  return http_multipart_add_data_part(
      multipart, name, filename,
      content_type ? content_type : HTTP_MULTIPART_DEFAULT_TYPE, data, size);
}

const char *http_multipart_content_type(const http_multipart *multipart) {
  static char content_type[96];
  if (multipart == NULL || multipart->boundary[0] == '\0')
    return NULL;
  snprintf(content_type, sizeof(content_type), "multipart/form-data; boundary=%s",
           multipart->boundary);
  return content_type;
}

Result http_multipart_serialize(const http_multipart *multipart, u8 **body,
                                size_t *size) {
  const http_multipart_part *part;
  multipart_buffer buffer = {0};
  char line[128];

  if (multipart == NULL || body == NULL || size == NULL ||
      multipart->boundary[0] == '\0')
    return -1;
  *body = NULL;
  *size = 0;
  for (part = multipart->first; part != NULL; part = part->next) {
    if (!buffer_append_string(&buffer, "--") ||
        !buffer_append_string(&buffer, multipart->boundary) ||
        !buffer_append_string(&buffer,
                              "\r\nContent-Disposition: form-data; name=\"") ||
        !buffer_append_quoted(&buffer, part->name) ||
        !buffer_append_string(&buffer, "\""))
      goto allocation_error;
    if (part->filename != NULL) {
      if (!buffer_append_string(&buffer, "; filename=\"") ||
          !buffer_append_quoted(&buffer, part->filename) ||
          !buffer_append_string(&buffer, "\""))
        goto allocation_error;
    }
    if (!buffer_append_string(&buffer, "\r\n"))
      goto allocation_error;
    if (part->content_type != NULL) {
      int written = snprintf(line, sizeof(line), "Content-Type: %s\r\n",
                             part->content_type);
      if (written < 0 || (size_t)written >= sizeof(line) ||
          !buffer_append(&buffer, line, (size_t)written))
        goto allocation_error;
    }
    if (!buffer_append_string(&buffer, "\r\n") ||
        !buffer_append(&buffer, part->data, part->size) ||
        !buffer_append_string(&buffer, "\r\n"))
      goto allocation_error;
  }
  if (!buffer_append_string(&buffer, "--") ||
      !buffer_append_string(&buffer, multipart->boundary) ||
      !buffer_append_string(&buffer, "--\r\n"))
    goto allocation_error;
  *body = buffer.data;
  *size = buffer.size;
  return 0;

allocation_error:
  free(buffer.data);
  return -1;
}

void http_multipart_free(http_multipart *multipart) {
  http_multipart_part *part;
  if (multipart == NULL)
    return;
  part = multipart->first;
  while (part != NULL) {
    http_multipart_part *next = part->next;
    free(part->name);
    free(part->filename);
    free(part->content_type);
    free(part->data);
    free(part);
    part = next;
  }
  memset(multipart, 0, sizeof(*multipart));
}
