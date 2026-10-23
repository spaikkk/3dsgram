#include "longpoll.h"
#include "net/httpClient.h"
#include "parser.h"
#include "types.h"
#include "utility/logger.h"
#include <jansson.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *get_event_str(json_t *update)
{
	void *it = json_object_iter(update);
	while (it) {
		const char *key = json_object_iter_key(it);
		if (strcmp(key, "update_id") != 0)
			return key;
		it = json_object_iter_next(update, it);
	}
	return NULL;
}

noccio_event_type_e get_type(const char *s)
{
	if (strcmp(s, "message") == 0)
		return MESSAGE;
	if (strcmp(s, "inline_query") == 0)
		return INLINE_QUERY;
	if (strcmp(s, "callback_query") == 0)
		return CALLBACK_QUERY;
	if (strcmp(s, "chosen_inline_result") == 0)
		return CHOSEN_INLINE_RESULT;

	return UNDEFINED;
}

static void dispatch_events(noccio_event_t *events, int events_size,
			    noccio_core_t *bot)
{
	for (int i = 0; i < events_size; ++i) {
		switch (events[i].type) {
		case MESSAGE:
			if (!bot->on_message)
				continue;
			events[i].message = (noccio_message_t){0};
			noccio_parse_message(events[i].data,
					     &events[i].message);
			bot->on_message(&events[i].message, bot->token,
					bot->on_message_userdata);
			noccio_message_clear(&events[i].message);
			break;
		case INLINE_QUERY:
			l_trace("Inline_query");
			if (!bot->on_inline_query)
				continue;
			events[i].inline_query = (noccio_inline_query_t){0};
			noccio_parse_inline_query(events[i].data,
						  &events[i].inline_query);
			bot->on_inline_query(&events[i].inline_query,
					     bot->token,
					     bot->on_inline_query_userdata);
			noccio_inline_query_clear(&events[i].inline_query);
			break;
		case CALLBACK_QUERY:
			l_trace("Callback query");
			break;
		case CHOSEN_INLINE_RESULT:
			l_trace("Chosen inline result");
			if (!bot->on_choosen_inline_result) {
				continue;
			}
			events[i].chosen_inline_result =
			    (noccio_chosen_inline_result_t){0};
			noccio_parse_chosen_inline_result(
			    events[i].data, &events[i].chosen_inline_result);
			bot->on_choosen_inline_result(
			    &events[i].chosen_inline_result, bot->token,
			    bot->on_choosen_inline_result_userdata);
			noccio_chosen_inline_result_clear(
			    &events[i].chosen_inline_result);
			break;

		case UNDEFINED:
			l_trace("Damiano soldi");
		}
	}
}

int64_t dispatch_updates(noccio_core_t *bot, char *curl_response)
{
	json_t *root;
	json_error_t error;
	root = json_loads(curl_response, 0, &error);

	if (!root) {
		l_error("json load error: %s", error.text);
		return -1;
	}
	if (!json_is_object(root)) {
		l_error("error: root is not an object") json_decref(root);
		return -1;
	}
	json_t *ok = json_object_get(root, "ok");
	if (!json_is_boolean(ok) || !json_boolean_value(ok)) {
		l_error("Telegram said not ok") json_decref(root);
		return -1;
	}
	json_t *result = json_object_get(root, "result");
	if (!json_is_array(result)) {
		l_error("Result is not an array") json_decref(root);
		return -1;
	}

	int64_t last_update = 0;
	int updates_count = json_array_size(result);
	noccio_event_t *updates = calloc(updates_count, sizeof(noccio_event_t));
	for (int i = 0; i < updates_count; ++i) {
		json_t *update = json_array_get(result, i);
		json_t *update_id = json_object_get(update, "update_id");
		last_update = (int64_t)json_integer_value(update_id);

		const char *event_str = get_event_str(update);
		if (!event_str) {
			l_error("Errore Gianluca Buonanno", last_update);
			continue;
		}
		updates[i].type = get_type(event_str);
		updates[i].data = json_object_get(update, event_str);
	}

	dispatch_events(updates, updates_count, bot);

	free(updates);

	json_decref(root);
	return last_update;
}

void longpoll(noccio_core_t *bot)
{
	int timeout = 30;
	char *response;
	int64_t last_update = 0;

	while (1) {
		l_info("Long poll started");
		response = NULL;
		char query[64];
		if (!last_update) {
			snprintf(query, sizeof(query), "timeout=%d", timeout);
		} else {
			snprintf(query, sizeof(query), "timeout=%d&offset=%ld",
				 timeout, last_update);
		}
		Url url = init_url("getUpdates", bot->token, query);
		if (make_request(&url, NULL, 0, &response) != 0) {
			if (response)
				free(response);
			continue;
		}
		l_info("%s", response);
		int dispatch_res = dispatch_updates(bot, response);
		if (dispatch_res > 0)
			last_update = dispatch_res + 1;
		free(response);
	}
}
