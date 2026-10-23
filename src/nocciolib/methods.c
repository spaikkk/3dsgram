#include "methods.h"
#include "net/httpClient.h"
#include "parser.h"
#include "types.h"
#include "utility/logger.h"
#include <stdint.h>
#include <stdio.h>

int noccio_get_full_chat(const char *token, int64_t id, char **res)
{
	char chat_id_str[32];
	snprintf(chat_id_str, 32, "%ld", id);
	HttpArgs args[1] = {{.name = "chat_id",
			     .value = chat_id_str,
			     .mime_type = "text/plain",
			     .is_disk_file = 0}};
	Url url = init_url("getChat", token, NULL);
	make_request(&url, args, 1, res);
	if (!*res) {
		l_error("Waspadalah! id: %ld", id);
		return -1;
	}
	return 0;
}

int noccio_send_message(const char *token, int64_t chat_id,
			char *text, noccio_inline_kb_markup_t *keyboard,
			char *parse_mode, noccio_message_t *cb)
{
	int args_total = 2;
	if (keyboard)
		args_total++;
	if (parse_mode)
		args_total++;

	Url url = init_url("sendMessage", token, NULL);
	HttpArgs args[args_total];
	int args_counter = 0;

	char chat_id_str[32];
	args[args_counter].name = "chat_id";
	snprintf(chat_id_str, sizeof(chat_id_str), "%ld", chat_id);
	args[args_counter].value = chat_id_str;
	args[args_counter].mime_type = "text/plain";
	args[args_counter].is_disk_file = 0;
	args[args_counter].data_size = 0;
	args[args_counter].filename = NULL;
	args_counter++;

	args[args_counter].name = "text";
	args[args_counter].value = text;
	args[args_counter].mime_type = "text/plain";
	args[args_counter].is_disk_file = 0;
	args[args_counter].data_size = 0;
	args[args_counter].filename = NULL;
	args_counter++;

	char *kb_json = NULL;
	if (keyboard) {
		kb_json = noccio_inline_keyboard_to_string(keyboard);
		l_trace("KEYBOARD: %s", kb_json);
		args[args_counter].name = "reply_markup";
		args[args_counter].value = kb_json;
		args[args_counter].mime_type = "text/plain";
		args[args_counter].is_disk_file = 0;
		args[args_counter].data_size = 0;
		args[args_counter].filename = NULL;
		args_counter++;
	}
	if (parse_mode) {
		args[args_counter].name = "parse_mode";
		args[args_counter].value = parse_mode;
		args[args_counter].mime_type = "text/plain";
		args[args_counter].is_disk_file = 0;
		args[args_counter].data_size = 0;
		args[args_counter].filename = NULL;
		args_counter++;
	}

	char *response = NULL;
	int r =
	    make_request(&url, args, args_total, &response);
	if (cb && r == 0) {
		json_t *msg_root = json_loads(response, 0, NULL);
		json_t *msg_obj = json_object_get(msg_root, "result");
		noccio_parse_message(msg_obj, cb);
		json_decref(msg_root);
	}
	l_trace("RISPOSTA SENDMESSAGE: %s", response);
	free(response);
	if (keyboard) {
		free(kb_json);
	}
	return r;
}

int noccio_answer_inline_query(const char *token,
			       char *inline_query_id,
			       noccio_queryres_arr_t *results)
{
	l_trace("DENTRO ANSWER INLINE QUERY");
	int args_total = 2;
	Url url = init_url("answerInlineQuery", token, NULL);
	HttpArgs args[args_total];

	args[0].name = "inline_query_id";
	args[0].value = inline_query_id;
	args[0].mime_type = "text/plain";
	args[0].is_disk_file = 0;
	args[0].data_size = 0;
	args[0].filename = NULL;

	char *result_string = noccio_article_to_string(results);
	args[1].name = "results";
	args[1].value = result_string;
	args[1].mime_type = "text/plain";
	args[1].is_disk_file = 0;
	args[1].data_size = 0;
	args[1].filename = NULL;

	char *response = NULL;
	int r =
	    make_request(&url, args, args_total, &response);
	l_trace("Risposta answerInlineQuery: %s", response);
	free(response);
	free(result_string);
	return r;
}

int noccio_edit_message_media(const char *token,
			      noccio_input_media_video_t *media,
			      int64_t chat_id, int64_t message_id,
			      char *inline_message_id)
{
	int args_total = 1;
	int is_inline_message = 0;
	if (inline_message_id) {
		is_inline_message = 1;
		args_total++;
	} else {
		args_total += 2;
	}

	Url url = init_url("editMessageMedia", token, NULL);
	HttpArgs args[args_total];
	char chat_id_str[32], message_id_str[32];

	char *inputmedia_str = noccio_imvideo_to_string(media);
	if (!inputmedia_str)
		return -1;
	args[0] = (HttpArgs){.name = "media",
			     .value = inputmedia_str,
			     .mime_type = "text/plain",
			     .is_disk_file = 0};
	if (is_inline_message) {
		args[1] = (HttpArgs){.name = "inline_message_id",
				     .value = inline_message_id,
				     .mime_type = "text/plain",
				     .is_disk_file = 0};
	} else {
		snprintf(chat_id_str, sizeof(chat_id_str), "%ld", chat_id);
		snprintf(message_id_str, sizeof(message_id_str), "%ld",
			 message_id);
		args[1] = (HttpArgs){.name = "chat_id",
				     .value = chat_id_str,
				     .mime_type = "text/plain",
				     .is_disk_file = 0};
		args[2] = (HttpArgs){.name = "message_id",
				     .value = message_id_str,
				     .mime_type = "text/plain",
				     .is_disk_file = 0};
	}

	char *response = NULL;
	int r =
	    make_request(&url, args, args_total, &response);
	l_trace("RISPOSTA FROCI: %s", response);
	free(response);
	free(inputmedia_str);
	return r;
}

int noccio_edit_message_text(const char *token, char *text,
			     int64_t chat_id, int64_t message_id,
			     char *inline_message_id)
{
	int args_total = 1;
	int is_inline_message = 0;
	if (inline_message_id) {
		is_inline_message = 1;
		args_total++;
	} else {
		args_total += 2;
	}

	Url url = init_url("editMessageText", token, NULL);
	HttpArgs args[args_total];
	char chat_id_str[32], message_id_str[32];

	args[0] = (HttpArgs){.name = "text",
			     .value = text,
			     .mime_type = "text/plain",
			     .is_disk_file = 0};
	if (is_inline_message) {
		args[1] = (HttpArgs){.name = "inline_message_id",
				     .value = inline_message_id,
				     .mime_type = "text/plain",
				     .is_disk_file = 0};
	} else {
		snprintf(chat_id_str, sizeof(chat_id_str), "%ld", chat_id);
		snprintf(message_id_str, sizeof(message_id_str), "%ld",
			 message_id);
		args[1] = (HttpArgs){.name = "chat_id",
				     .value = chat_id_str,
				     .mime_type = "text/plain",
				     .is_disk_file = 0};
		args[2] = (HttpArgs){.name = "message_id",
				     .value = message_id_str,
				     .mime_type = "text/plain",
				     .is_disk_file = 0};
	}

	char *response = NULL;
	int r =
	    make_request(&url, args, args_total, &response);
	free(response);
	return r;
}

int noccio_send_video(const char *token, int64_t chat_id,
		      noccio_input_media_video_t *media, noccio_message_t *cb)
{
	int args_total = 2;
	if (media->caption)
		args_total++;

	Url url = init_url("sendVideo", token, NULL);
	HttpArgs args[args_total];

	char chat_id_str[32];
	args[0].name = "chat_id";
	snprintf(chat_id_str, sizeof(chat_id_str), "%ld", chat_id);
	args[0].value = chat_id_str;
	args[0].mime_type = "text/plain";
	args[0].is_disk_file = 0;
	args[0].data_size = 0;
	args[0].filename = NULL;

	args[1].name = "video";
	if (media->buffer && media->buffer_size > 0) {
		args[1].value = media->buffer;
		args[1].mime_type = "video/mp4";
		args[1].is_disk_file = 0;
		args[1].data_size = media->buffer_size;
		args[1].filename =
		    media->filename ? media->filename : "video.mp4";
	} else if (media->is_file) {
		args[1].value = media->media;
		args[1].mime_type = "video/mp4";
		args[1].is_disk_file = 1;
		args[1].data_size = 0;
		args[1].filename = NULL;
	} else {
		args[1].value = media->media;
		args[1].mime_type = "text/plain";
		args[1].is_disk_file = 0;
		args[1].data_size = 0;
		args[1].filename = NULL;
	}

	if (media->caption) {
		args[2].name = "caption";
		args[2].value = media->caption;
		args[2].mime_type = "text/plain";
		args[2].is_disk_file = 0;
		args[2].data_size = 0;
		args[2].filename = NULL;
	}

	char *response = NULL;
	int r =
	    make_request(&url, args, args_total, &response);
	if (cb && r == 0) {
		json_t *msg_root = json_loads(response, 0, NULL);
		json_t *msg_obj = json_object_get(msg_root, "result");
		noccio_parse_message(msg_obj, cb);
		json_decref(msg_root);
	}
	l_trace("RISPOSTA SEND_VIDEO: %s", response);
	free(response);
	return r;
}

int noccio_set_message_reaction(const char *token, int64_t chat_id,
				int64_t message_id, char *emoji)
{
	int args_total = 3;

	Url url = init_url("setMessageReaction", token, NULL);
	HttpArgs args[args_total];

	char chat_id_str[32];
	args[0].name = "chat_id";
	snprintf(chat_id_str, sizeof(chat_id_str), "%ld", chat_id);
	args[0].value = chat_id_str;
	args[0].mime_type = "text/plain";
	args[0].is_disk_file = 0;
	args[0].data_size = 0;
	args[0].filename = NULL;

	char message_id_str[32];
	args[1].name = "message_id";
	snprintf(message_id_str, sizeof(message_id_str), "%ld", message_id);
	args[1].value = message_id_str;
	args[1].mime_type = "text/plain";
	args[1].is_disk_file = 0;
	args[1].data_size = 0;
	args[1].filename = NULL;

	char emoji_arr_string[128];
	snprintf(emoji_arr_string, sizeof(emoji_arr_string),
		 "[{\"type\":\"emoji\",\"emoji\":\"%s\"}]", emoji);
	args[2].name = "reaction";
	args[2].value = emoji_arr_string;
	args[2].mime_type = "text/plain";
	args[2].is_disk_file = 0;
	args[2].data_size = 0;
	args[2].filename = NULL;

	char *response = NULL;
	int r =
	    make_request(&url, args, args_total, &response);
	l_trace("RISPOSTA REACTION: %s", response);
	free(response);
	return r;
}