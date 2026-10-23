#ifndef LONGPOLL_HEADER
#define LONGPOLL_HEADER

#include "types.h"
#include <stdint.h>

#include "core.h"

typedef enum {
	MESSAGE,
	INLINE_QUERY,
	CALLBACK_QUERY,
	CHOSEN_INLINE_RESULT,
	UNDEFINED
} noccio_event_type_e;

typedef struct noccio_event {
	noccio_event_type_e type;
	json_t *data;
	union {
		noccio_message_t message;
		noccio_inline_query_t inline_query;
		noccio_callback_query_t callback_query;
		noccio_chosen_inline_result_t chosen_inline_result;
	};
} noccio_event_t;

int64_t dispatch_updates(noccio_core_t *bot, char *curl_response);
void longpoll(noccio_core_t *handle);

#endif
