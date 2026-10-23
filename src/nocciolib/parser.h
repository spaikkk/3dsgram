#ifndef PARSER_HEADER
#define PARSER_HEADER
#include "types.h"
#include <jansson.h>

int noccio_parse_message(json_t *obj, noccio_message_t *msg);
int noccio_parse_chat(json_t *obj, noccio_chat_t *chat);
int noccio_parse_user(json_t *obj, noccio_user_t *user);
int noccio_parse_message_entity(json_t *obj, noccio_message_entity_t *entity);
int noccio_parse_photo_size(json_t *obj, noccio_photo_size_t *photosize);
int noccio_parse_animation(json_t *obj, noccio_animation_t *anim);
int noccio_parse_audio(json_t *obj, noccio_audio_t *audio);
int noccio_parse_document(json_t *obj, noccio_document_t *document);
int noccio_parse_video(json_t *obj, noccio_video_t *video);
int noccio_parse_voice(json_t *obj, noccio_voice_t *voice);
int noccio_parse_inline_query(json_t *obj, noccio_inline_query_t *query);
int noccio_parse_callback_query(json_t *obj, noccio_callback_query_t *cb_query);
int noccio_parse_chosen_inline_result(json_t *obj,
				      noccio_chosen_inline_result_t *res);

#endif