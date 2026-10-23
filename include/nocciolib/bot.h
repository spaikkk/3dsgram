#ifndef BOT_HEADER
#define BOT_HEADER

#define MAX_TOKEN 128
#include "net/httpClient.h"
#include "types.h"

typedef void (*noccio_on_message_cb)(noccio_message_t *msg,
                                     const char *token,
                                     void *userdata);
typedef void (*noccio_on_inline_query_cb)(noccio_inline_query_t *inline_query,
                                          const char *token,
                                          void *userdata);
typedef void (*noccio_on_choosen_inline_result_cb)(
    noccio_chosen_inline_result_t *result, const char *token,
    void *userdata);

typedef struct noccio_bot_t noccio_bot;

noccio_bot *bot_init(char *token);
void bot_start(noccio_bot *bot);
void bot_free(noccio_bot *bot);
void bot_set_on_message(noccio_bot *bot, noccio_on_message_cb cb,
                        void *userdata);
void bot_set_on_inline_query(noccio_bot *bot, noccio_on_inline_query_cb cb,
                             void *userdata);
void bot_set_on_choosen_inline_result(noccio_bot *bot,
                                      noccio_on_choosen_inline_result_cb cb,
                                      void *userdata);

#endif
