#ifndef HTTP_MULTIPART_HEADER
#define HTTP_MULTIPART_HEADER

#include <stddef.h>
#include <3ds.h>

/* Small multipart/form-data builder for raw libctru HTTP POST bodies.
 * Part data is copied when added, so callers may release their buffers.
 */
typedef struct http_multipart_part {
  char *name;
  char *filename;
  char *content_type;
  u8 *data;
  size_t size;
  struct http_multipart_part *next;
} http_multipart_part;

typedef struct {
  char boundary[64];
  http_multipart_part *first;
  http_multipart_part *last;
} http_multipart;

Result http_multipart_init(http_multipart *multipart);
Result http_multipart_add_part(http_multipart *multipart, const char *name,
                               const char *value);
Result http_multipart_add_data_part(http_multipart *multipart,
                                    const char *name, const char *filename,
                                    const char *content_type,
                                    const void *data, size_t size);
Result http_multipart_add_file_part(http_multipart *multipart,
                                    const char *name, const char *filename,
                                    const char *content_type,
                                    const void *data, size_t size);
const char *http_multipart_content_type(const http_multipart *multipart);
Result http_multipart_serialize(const http_multipart *multipart, u8 **body,
                                size_t *size);
void http_multipart_free(http_multipart *multipart);

#endif
