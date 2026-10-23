#include "types.h"
#include "jansson.h"
#include "net/httpClient.h"
#include "utility/logger.h"
#include <stdlib.h>

#define nocciofree(ptr) \
	do { \
		if (ptr) { \
			free(ptr); \
			ptr = NULL; \
		} \
	} while (0);

noccio_message_t *noccio_message_init()
{
	noccio_message_t *msg = calloc(1, sizeof(noccio_message_t));
	return msg;
}

noccio_chat_t *noccio_chat_init()
{
	noccio_chat_t *chat = calloc(1, sizeof(noccio_chat_t));
	return chat;
}

noccio_user_t *noccio_user_init()
{
	noccio_user_t *user = calloc(1, sizeof(noccio_user_t));
	return user;
}

noccio_message_entity_t *noccio_message_entity_init()
{
	noccio_message_entity_t *entity =
	    calloc(1, sizeof(noccio_message_entity_t));
	return entity;
}

noccio_photo_size_t *noccio_photo_size_init()
{
	noccio_photo_size_t *photosize = calloc(1, sizeof(noccio_photo_size_t));
	return photosize;
}

noccio_animation_t *noccio_animation_init()
{
	noccio_animation_t *anim = calloc(1, sizeof(noccio_animation_t));
	return anim;
}

noccio_audio_t *noccio_audio_init()
{
	noccio_audio_t *audio = calloc(1, sizeof(noccio_audio_t));
	return audio;
}

noccio_document_t *noccio_document_init()
{
	noccio_document_t *document = calloc(1, sizeof(noccio_document_t));
	return document;
}

noccio_video_t *noccio_video_init()
{
	noccio_video_t *video = calloc(1, sizeof(noccio_video_t));
	return video;
}

noccio_voice_t *noccio_voice_init()
{
	noccio_voice_t *voice = calloc(1, sizeof(noccio_voice_t));
	return voice;
}

noccio_inline_query_t *noccio_inline_query_init()
{
	noccio_inline_query_t *inl_query =
	    calloc(1, sizeof(noccio_inline_query_t));
	return inl_query;
}

noccio_callback_query_t *noccio_callback_query_init()
{
	noccio_callback_query_t *cb_query =
	    calloc(1, sizeof(noccio_callback_query_t));
	return cb_query;
}

noccio_chosen_inline_result_t *noccio_chosen_inline_result_init()
{
	noccio_chosen_inline_result_t *res =
	    calloc(1, sizeof(noccio_chosen_inline_result_t));
	return res;
}

noccio_input_media_video_t *noccio_input_media_video_init()
{
	noccio_input_media_video_t *media =
	    calloc(1, sizeof(noccio_input_media_video_t));
	return media;
}

int noccio_message_clear(noccio_message_t *msg)
{
	if (!msg)
		return -1;

	noccio_chat_clear(msg->chat);
	nocciofree(msg->chat);

	noccio_user_clear(msg->from);
	nocciofree(msg->from);

	noccio_message_clear(msg->reply_to_message);
	nocciofree(msg->reply_to_message);

	nocciofree(msg->text);

	if (msg->entities.count > 0) {
		for (int i = 0; i < msg->entities.count; ++i)
			noccio_message_entity_clear(&msg->entities.arr[i]);

		nocciofree(msg->entities.arr);
		msg->entities.count = 0;
	}

	noccio_animation_clear(msg->animation);
	nocciofree(msg->animation);

	noccio_audio_clear(msg->audio);
	nocciofree(msg->audio);

	noccio_document_clear(msg->document);
	nocciofree(msg->document);

	// noccio_photo_size_free(msg->photo);
	noccio_video_clear(msg->video);
	nocciofree(msg->video);

	noccio_voice_clear(msg->voice);
	nocciofree(msg->voice);

	if (msg->photo.count > 0) {
		for (int i = 0; i < msg->photo.count; ++i)
			noccio_photo_size_clear(&msg->photo.arr[i]);

		nocciofree(msg->photo.arr);
	}

	nocciofree(msg->caption);

	return 0;
}

int noccio_chat_clear(noccio_chat_t *chat)
{
	if (!chat)
		return -1;
	nocciofree(chat->title);
	nocciofree(chat->username);
	nocciofree(chat->description);
	return 0;
}

int noccio_user_clear(noccio_user_t *user)
{
	if (!user)
		return -1;
	nocciofree(user->first_name);
	nocciofree(user->last_name);
	nocciofree(user->username);
	return 0;
}

int noccio_message_entity_clear(noccio_message_entity_t *entity)
{
	if (!entity)
		return -1;
	nocciofree(entity->type);
	nocciofree(entity->url);
	nocciofree(entity->language);
	nocciofree(entity->custom_emoji_id);
	nocciofree(entity->date_time_format);
	noccio_user_clear(entity->user);
	nocciofree(entity->user);
	// qua forse le palle del free(entity) boh
	return 0;
}

int noccio_photo_size_clear(noccio_photo_size_t *photosize)
{
	if (!photosize)
		return -1;
	nocciofree(photosize->file_id);
	nocciofree(photosize->file_unique_id);
	return 0;
}

int noccio_animation_clear(noccio_animation_t *anim)
{
	if (!anim)
		return -1;

	nocciofree(anim->file_id);
	nocciofree(anim->file_unique_id);
	nocciofree(anim->file_name);
	nocciofree(anim->mime_type);
	noccio_photo_size_clear(anim->thumbnail);
	nocciofree(anim->thumbnail);
	return 0;
}

int noccio_audio_clear(noccio_audio_t *audio)
{
	if (!audio)
		return -1;

	nocciofree(audio->file_id);
	nocciofree(audio->file_unique_id);
	nocciofree(audio->performer);
	nocciofree(audio->title);
	nocciofree(audio->file_name);
	noccio_photo_size_clear(audio->thumbnail);
	nocciofree(audio->thumbnail);
	return 0;
}

int noccio_document_clear(noccio_document_t *document)
{
	if (!document)
		return -1;

	nocciofree(document->file_id);
	nocciofree(document->file_unique_id);
	nocciofree(document->file_name);
	nocciofree(document->mime_type);
	noccio_photo_size_clear(document->thumbnail);
	nocciofree(document->thumbnail);
	return 0;
}

int noccio_video_clear(noccio_video_t *video)
{
	if (!video)
		return -1;

	nocciofree(video->file_id);
	nocciofree(video->file_unique_id);
	noccio_photo_size_clear(video->thumbnail);
	nocciofree(video->thumbnail);
	nocciofree(video->file_name);
	nocciofree(video->mime_type);
	return 0;
}

int noccio_voice_clear(noccio_voice_t *voice)
{
	if (!voice)
		return -1;

	nocciofree(voice->file_id);
	nocciofree(voice->file_unique_id);
	nocciofree(voice->mime_type);
	return 0;
}

int noccio_inline_query_clear(noccio_inline_query_t *query)
{
	if (!query)
		return -1;

	nocciofree(query->id);
	noccio_user_clear(query->from);
	nocciofree(query->from);
	nocciofree(query->query);
	nocciofree(query->offset);
	nocciofree(query->chat_type);
	return 0;
}

int noccio_callback_query_clear(noccio_callback_query_t *cb_query)
{
	if (!cb_query)
		return -1;

	nocciofree(cb_query->id);
	noccio_user_clear(cb_query->from);
	nocciofree(cb_query->from);
	nocciofree(cb_query->inline_message_id);
	nocciofree(cb_query->chat_instance);
	nocciofree(cb_query->data);
	return 0;
}

int noccio_chosen_inline_result_clear(noccio_chosen_inline_result_t *res)
{
	if (!res)
		return -1;

	nocciofree(res->result_id);
	noccio_user_clear(res->from);
	nocciofree(res->from);
	nocciofree(res->inline_message_id);
	nocciofree(res->query);
	return 0;
}

int noccio_input_media_video_clear(noccio_input_media_video_t *media)
{
	if (!media)
		return -1;
	nocciofree(media->type);
	nocciofree(media->media);

	// optional field
	nocciofree(media->caption);
	return 0;
}

char *noccio_inline_keyboard_to_string(noccio_inline_kb_markup_t *keyboard)
{
	if (keyboard == NULL || keyboard->inline_keyboard == NULL) {
		return NULL;
	}

	json_t *root = json_object();
	if (!root)
		return NULL;

	json_t *keyboard_array = json_array();
	if (!keyboard_array) {
		json_decref(root);
		return NULL;
	}

	for (int i = 0; i < keyboard->rows_count; i++) {
		json_t *row_array = json_array();
		for (int j = 0; j < keyboard->cols_on_row[i]; j++) {
			noccio_inline_kb_button_t *btn =
			    &keyboard->inline_keyboard[i][j];
			json_t *btn_obj = json_object();
			if (btn->text != NULL) {
				json_object_set_new(btn_obj, "text",
						    json_string(btn->text));
			} else {
				json_object_set_new(btn_obj, "text",
						    json_string(""));
			}
			if (btn->callback_data != NULL) {
				json_object_set_new(
				    btn_obj, "callback_data",
				    json_string(btn->callback_data));
			}
			json_array_append_new(row_array, btn_obj);
		}
		json_array_append_new(keyboard_array, row_array);
	}
	json_object_set_new(root, "inline_keyboard", keyboard_array);
	char *out = json_dumps(root, JSON_COMPACT);
	json_decref(root);
	return out;
}

json_t *noccio_inline_keyboard_to_json(noccio_inline_kb_markup_t *keyboard)
{
	if (keyboard == NULL || keyboard->inline_keyboard == NULL) {
		return NULL;
	}

	json_t *root = json_object();
	if (!root)
		return NULL;

	json_t *keyboard_array = json_array();
	if (!keyboard_array) {
		json_decref(root);
		return NULL;
	}

	for (int i = 0; i < keyboard->rows_count; i++) {
		json_t *row_array = json_array();
		for (int j = 0; j < keyboard->cols_on_row[i]; j++) {
			noccio_inline_kb_button_t *btn =
			    &keyboard->inline_keyboard[i][j];
			json_t *btn_obj = json_object();
			if (btn->text != NULL) {
				json_object_set_new(btn_obj, "text",
						    json_string(btn->text));
			} else {
				json_object_set_new(btn_obj, "text",
						    json_string(""));
			}
			if (btn->callback_data != NULL) {
				json_object_set_new(
				    btn_obj, "callback_data",
				    json_string(btn->callback_data));
			}
			json_array_append_new(row_array, btn_obj);
		}
		json_array_append_new(keyboard_array, row_array);
	}
	json_object_set_new(root, "inline_keyboard", keyboard_array);
	return root;
}

char *noccio_article_to_string(noccio_queryres_arr_t *article)
{
	if (article == NULL || article->count <= 0) {
		return NULL;
	}

	json_t *root_array = json_array();
	if (!root_array) {
		return NULL;
	}
	for (int i = 0; i < article->count; i++) {
		noccio_queryres_article_t *curr = &article->arr[i];
		json_t *art_obj = json_object();
		if (!art_obj) {
			continue;
		}

		json_object_set_new(art_obj, "type", json_string(curr->type));

		json_object_set_new(art_obj, "id", json_string(curr->id));

		json_object_set_new(art_obj, "title", json_string(curr->title));

		if (curr->thumbnail_url)
			json_object_set_new(art_obj, "thumbnail_url",
					    json_string(curr->thumbnail_url));
		json_t *imc_obj = json_object();
		if (curr->input_message_content->message_text != NULL) {
			json_object_set_new(
			    imc_obj, "message_text",
			    json_string(
				curr->input_message_content->message_text));
		} else {
			json_object_set_new(imc_obj, "message_text",
					    json_string(""));
		}
		json_object_set_new(art_obj, "input_message_content", imc_obj);

		if (curr->reply_markup != NULL) {
			json_t *keyboard_obj =
			    noccio_inline_keyboard_to_json(curr->reply_markup);

			if (keyboard_obj != NULL) {
				json_object_set_new(art_obj, "reply_markup",
						    keyboard_obj);
			}
		}
		json_array_append_new(root_array, art_obj);
	}

	char *out = json_dumps(root_array, JSON_COMPACT);

	json_decref(root_array);

	return out;
}

char *noccio_imvideo_to_string(noccio_input_media_video_t *media)
{
	if (!media)
		return NULL;

	json_t *root = json_object();
	if (!root)
		return NULL;
	json_object_set_new(root, "type", json_string("video"));
	json_object_set_new(root, "media", json_string(media->media));
	if (media->caption) {
		json_object_set_new(root, "caption",
				    json_string(media->caption));
	}
	char *out = json_dumps(root, JSON_COMPACT);
	l_trace("media out: %s", out);
	json_decref(root);
	return out;
}