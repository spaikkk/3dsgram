#ifndef CORE_HEADER
#define CORE_HEADER
#include "bot.h"

typedef struct noccio_core {
  char *token;
  noccio_on_message_cb on_message;
  void *on_message_userdata;
  noccio_on_inline_query_cb on_inline_query;
  void *on_inline_query_userdata;
  noccio_on_choosen_inline_result_cb on_choosen_inline_result;
  void *on_choosen_inline_result_userdata;
} noccio_core_t;

#endif
