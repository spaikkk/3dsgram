#ifndef METHODS_HEADER
#define METHODS_HEADER

#include "net/httpClient.h"
#include "types.h"
#include <stdint.h>

int noccio_get_full_chat(const char *token, int64_t id, char **res);
int noccio_send_message(const char *token, int64_t chat_id,
			char *text, noccio_inline_kb_markup_t *keyboard,
			char *parse_mode, noccio_message_t *cb);
int noccio_send_video(const char *token, int64_t chat_id,
		      noccio_input_media_video_t *media, noccio_message_t *cb);
int noccio_answer_inline_query(const char *token,
			       char *inline_query_id,
			       noccio_queryres_arr_t *results);
int noccio_edit_message_media(const char *token,
			      noccio_input_media_video_t *media,
			      int64_t chat_id, int64_t message_id,
			      char *inline_message_id);
int noccio_edit_message_text(const char *token, char *text,
			     int64_t chat_id, int64_t message_id,
			     char *inline_message_id);

int noccio_set_message_reaction(const char *token, int64_t chat_id,
				int64_t message_id, char *emoji);

#endif