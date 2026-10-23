#include "bot.h"
#include "core.h"
#include "longpoll.h"
#include "utility/logger.h"

#include <ini.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct noccio_bot_t {
  noccio_core_t core;
};

noccio_bot *bot_init(char *token) {
  noccio_bot *bot = calloc(1, sizeof(noccio_bot));
  if (!bot) {
    exit(1);
  }
  bot->core.token = strdup(token);
  l_info("Bot initialized");
  return bot;
}

void bot_free(noccio_bot *bot) {
  free(bot->core.token);
  free(bot);
}

void bot_start(noccio_bot *bot) { longpoll(&bot->core); }

void bot_set_on_message(noccio_bot *bot, noccio_on_message_cb cb,
                        void *userdata) {
  bot->core.on_message = cb;
  bot->core.on_message_userdata = userdata;
}

void bot_set_on_inline_query(noccio_bot *bot, noccio_on_inline_query_cb cb,
                             void *userdata) {
  bot->core.on_inline_query = cb;
  bot->core.on_inline_query_userdata = userdata;
}

void bot_set_on_choosen_inline_result(noccio_bot *bot,
                                      noccio_on_choosen_inline_result_cb cb,
                                      void *userdata) {
  bot->core.on_choosen_inline_result = cb;
  bot->core.on_choosen_inline_result_userdata = userdata;
}
