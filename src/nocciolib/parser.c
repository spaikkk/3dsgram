#include "parser.h"
#include "jansson.h"
#include "types.h"
#include "utility/logger.h"
#include <string.h>
#include <time.h>

/*
    Helper macros for jansson
*/
#define streq(a, b) strcmp(a, b) == 0

#define json_get_string(object, target, field) \
	do { \
		json_t *type_obj = json_object_get(object, field); \
		if (type_obj) { \
			target = (char *)json_string_value(type_obj); \
			if (!target) \
				l_error("Aku apa? Aku suges") \
		} \
	} while (0);

#define json_dup_string(object, target, field) \
	do { \
		json_t *type_obj = json_object_get(object, field); \
		if (type_obj) { \
			target = strdup(json_string_value(type_obj)); \
		} \
	} while (0);

#define json_get_integer(object, target, field) \
	do { \
		json_t *type_obj = json_object_get(object, field); \
		if (type_obj) { \
			target = json_integer_value(type_obj); \
		} \
	} while (0);

#define json_get_bool(object, target, field) \
	do { \
		json_t *type_obj = json_object_get(object, field); \
		if (type_obj) { \
			target = json_boolean_value(type_obj); \
		} \
	} while (0);

static struct noccio_date noccio_date_from_ts(time_t unix_time);
static noccio_chat_type_e noccio_chat_type_get(const char *str);

int noccio_parse_chat(json_t *obj, noccio_chat_t *chat)
{
	if (!chat)
		return -1;

	json_get_integer(obj, chat->id, "id");

	char *chat_type_str;
	json_get_string(obj, chat_type_str, "type");
	chat->type = noccio_chat_type_get(chat_type_str);

	// optional fields
	json_dup_string(obj, chat->title, "title");

	json_dup_string(obj, chat->username, "username");

	json_dup_string(obj, chat->description, "description");

	json_get_bool(obj, chat->is_direct_messages, "is_direct_messages");

	return 0;
}

int noccio_parse_message(json_t *obj, noccio_message_t *msg)
{
	if (!msg)
		return -1;

	json_get_integer(obj, msg->id, "message_id");

	time_t unix_date;
	json_get_integer(obj, unix_date, "date");
	msg->date = noccio_date_from_ts(unix_date);

	json_t *chat_obj = json_object_get(obj, "chat");
	if (chat_obj) {
		msg->chat = noccio_chat_init();
		noccio_parse_chat(chat_obj, msg->chat);
	}

	// optional fields
	json_t *user_obj = json_object_get(obj, "from");
	if (user_obj) {
		msg->from = noccio_user_init();
		noccio_parse_user(user_obj, msg->from);
	}

	json_t *reply_to_msg_obj = json_object_get(obj, "reply_to_message");
	if (reply_to_msg_obj) {
		msg->reply_to_message = noccio_message_init();
		noccio_parse_message(reply_to_msg_obj, msg->reply_to_message);
	}

	json_dup_string(obj, msg->text, "text");

	json_t *entites_obj = json_object_get(obj, "entities");
	if (entites_obj && json_is_array(entites_obj)) {
		msg->entities.count = json_array_size(entites_obj);
		msg->entities.arr = calloc(msg->entities.count,
					   sizeof(noccio_message_entity_t));
		for (int i = 0; i < msg->entities.count; ++i) {
			json_t *entity = json_array_get(entites_obj, i);
			noccio_parse_message_entity(entity,
						    &msg->entities.arr[i]);
		}
	}

	json_t *anim_obj = json_object_get(obj, "animation");
	if (anim_obj) {
		msg->animation = noccio_animation_init();
		noccio_parse_animation(anim_obj, msg->animation);
	}

	json_t *audio_obj = json_object_get(obj, "audio");
	if (audio_obj) {
		msg->audio = noccio_audio_init();
		noccio_parse_audio(audio_obj, msg->audio);
	}

	json_t *document_obj = json_object_get(obj, "document");
	if (document_obj) {
		msg->document = noccio_document_init();
		noccio_parse_document(document_obj, msg->document);
	}

	// json_t *photo_obj = json_object_get(obj, "photo");

	json_t *video_obj = json_object_get(obj, "video");
	if (video_obj) {
		msg->video = noccio_video_init();
		noccio_parse_video(video_obj, msg->video);
	}

	json_t *voice_obj = json_object_get(obj, "voice");
	if (voice_obj) {
		msg->voice = noccio_voice_init();
		noccio_parse_voice(voice_obj, msg->voice);
	}

	json_t *photo_obj = json_object_get(obj, "photo");
	if (photo_obj && json_is_array(photo_obj)) {
		msg->photo.count = json_array_size(photo_obj);
		msg->photo.arr =
		    calloc(msg->photo.count, sizeof(noccio_photo_size_t));
		for (int i = 0; i < msg->photo.count; ++i) {
			json_t *photo = json_array_get(photo_obj, i);
			noccio_parse_photo_size(photo, &msg->photo.arr[i]);
		}
	}

	json_dup_string(obj, msg->caption, "caption");

	return 0;
}

int noccio_parse_user(json_t *obj, noccio_user_t *user)
{
	if (!user)
		return -1;

	json_get_integer(obj, user->id, "id");

	json_get_integer(obj, user->is_bot, "is_bot");

	json_dup_string(obj, user->first_name, "first_name");

	l_trace("id: %ld; is bot: %d; first name: %s", user->id, user->is_bot,
		user->first_name);

	// optional fields
	json_dup_string(obj, user->last_name, "last_name");

	json_dup_string(obj, user->username, "username");

	return 0;
}

int noccio_parse_message_entity(json_t *obj, noccio_message_entity_t *entity)
{
	if (!entity)
		return -1;

	json_dup_string(obj, entity->type, "type");

	json_get_integer(obj, entity->offset, "offset");

	json_get_integer(obj, entity->lenght, "lenght");

	// optional fields
	json_dup_string(obj, entity->url, "url");

	json_dup_string(obj, entity->language, "language");

	json_dup_string(obj, entity->custom_emoji_id, "custom_emoji_id");

	json_dup_string(obj, entity->date_time_format, "date_time_format");

	json_t *user_obj = json_object_get(obj, "user");
	if (user_obj) {
		entity->user = noccio_user_init();
		noccio_parse_user(user_obj, entity->user);
	}

	return 0;
}

int noccio_parse_photo_size(json_t *obj, noccio_photo_size_t *photosize)
{
	if (!photosize)
		return -1;

	json_dup_string(obj, photosize->file_id, "file_id");

	json_dup_string(obj, photosize->file_unique_id, "file_unique_id");

	json_get_integer(obj, photosize->width, "width");

	json_get_integer(obj, photosize->height, "height");

	// optional field
	json_get_integer(obj, photosize->file_size, "file_size");

	return 0;
}

int noccio_parse_animation(json_t *obj, noccio_animation_t *anim)
{
	if (!anim)
		return -1;

	json_dup_string(obj, anim->file_id, "file_id");

	json_dup_string(obj, anim->file_unique_id, "file_unique_id");

	json_get_integer(obj, anim->width, "width");

	json_get_integer(obj, anim->height, "height");

	json_get_integer(obj, anim->duration, "duration");

	// optional fields
	json_t *photo_obj = json_object_get(obj, "thumbnail");
	if (photo_obj) {
		anim->thumbnail = noccio_photo_size_init();
		noccio_parse_photo_size(photo_obj, anim->thumbnail);
	}

	json_get_integer(obj, anim->file_size, "file_size");

	json_dup_string(obj, anim->mime_type, "mime_type");

	json_dup_string(obj, anim->file_name, "file_name");

	return 0;
}

int noccio_parse_audio(json_t *obj, noccio_audio_t *audio)
{
	if (!audio)
		return 1;

	json_dup_string(obj, audio->file_id, "file_id");

	json_dup_string(obj, audio->file_unique_id, "file_unique_id");

	json_get_integer(obj, audio->duration, "duration");

	// optional fields
	json_dup_string(obj, audio->performer, "performer");

	json_dup_string(obj, audio->title, "title");

	json_dup_string(obj, audio->file_name, "file_name");

	json_get_integer(obj, audio->file_size, "file_size");

	json_t *thumbnail_obj = json_object_get(obj, "thumbnail");
	if (thumbnail_obj) {
		audio->thumbnail = noccio_photo_size_init();
		noccio_parse_photo_size(thumbnail_obj, audio->thumbnail);
	}

	return 0;
}

int noccio_parse_document(json_t *obj, noccio_document_t *document)
{
	if (!document)
		return -1;

	json_dup_string(obj, document->file_id, "file_id");

	json_dup_string(obj, document->file_unique_id, "file_unique_id");

	// optional fields
	json_t *thumbnail_obj = json_object_get(obj, "thumbnail");
	if (thumbnail_obj) {
		document->thumbnail = noccio_photo_size_init();
		noccio_parse_photo_size(thumbnail_obj, document->thumbnail);
	}

	json_get_integer(obj, document->file_size, "file_size");

	json_dup_string(obj, document->file_name, "file_name");

	json_dup_string(obj, document->mime_type, "mime_type");

	return 0;
}

int noccio_parse_video(json_t *obj, noccio_video_t *video)
{
	if (!video)
		return -1;

	json_dup_string(obj, video->file_id, "file_id");

	json_dup_string(obj, video->file_unique_id, "file_unique_id");

	json_get_integer(obj, video->width, "width");

	json_get_integer(obj, video->height, "height");

	json_get_integer(obj, video->duration, "duration");

	// optional fields
	json_t *thumbnail_obj = json_object_get(obj, "thumbnail");
	if (thumbnail_obj) {
		video->thumbnail = noccio_photo_size_init();
		noccio_parse_photo_size(thumbnail_obj, video->thumbnail);
	}

	json_get_integer(obj, video->start_timestamp, "start_timestamp");

	json_dup_string(obj, video->file_name, "file_name");

	json_dup_string(obj, video->mime_type, "mime_type");

	json_get_integer(obj, video->file_size, "file_size");

	return 0;
}

int noccio_parse_voice(json_t *obj, noccio_voice_t *voice)
{
	if (!voice)
		return -1;

	json_dup_string(obj, voice->file_id, "file_id");

	json_dup_string(obj, voice->file_unique_id, "file_unique_id");

	json_get_integer(obj, voice->duration, "duration");

	// optional fields
	json_dup_string(obj, voice->mime_type, "mime_type");

	json_get_integer(obj, voice->file_size, "file_size");

	return 0;
}

int noccio_parse_inline_query(json_t *obj, noccio_inline_query_t *query)
{
	if (!query)
		return -1;

	json_dup_string(obj, query->id, "id");

	json_t *user_obj = json_object_get(obj, "from");
	if (user_obj) {
		query->from = noccio_user_init();
		noccio_parse_user(user_obj, query->from);
	}

	json_dup_string(obj, query->query, "query");

	json_dup_string(obj, query->offset, "offset");

	// optional fields
	json_dup_string(obj, query->chat_type, "chat_type");

	return 0;
}

int noccio_parse_callback_query(json_t *obj, noccio_callback_query_t *cb_query)
{
	if (!cb_query)
		return -1;

	json_dup_string(obj, cb_query->id, "id");

	json_t *user_obj = json_object_get(obj, "from");
	if (user_obj) {
		cb_query->from = noccio_user_init();
		noccio_parse_user(user_obj, cb_query->from);
	}

	// optional fields
	json_dup_string(obj, cb_query->inline_message_id, "inline_message_id");

	json_dup_string(obj, cb_query->chat_instance, "chat_instance");

	json_dup_string(obj, cb_query->data, "data");

	return 0;
}

int noccio_parse_chosen_inline_result(json_t *obj,
				      noccio_chosen_inline_result_t *res)
{
	if (!res)
		return -1;

	json_dup_string(obj, res->result_id, "result_id");

	json_t *user_obj = json_object_get(obj, "from");
	if (user_obj) {
		res->from = noccio_user_init();
		noccio_parse_user(user_obj, res->from);
	}

	// optional fields
	json_dup_string(obj, res->inline_message_id, "inline_message_id");

	json_dup_string(obj, res->query, "query");

	return 0;
}

// ts (this shrimp) frying me
static struct noccio_date noccio_date_from_ts(time_t unix_time)
{
	struct tm *t = localtime(&unix_time);
	return (struct noccio_date){.year = t->tm_year + 1900,
				    .month = t->tm_mon + 1,
				    .day = t->tm_mday,
				    .hour = t->tm_hour,
				    .minute = t->tm_min,
				    .second = t->tm_sec};
}

static noccio_chat_type_e noccio_chat_type_get(const char *str)
{
	if (streq(str, "private"))
		return NOCCIO_CHAT_TYPE_PRIVATE;
	else if (streq(str, "group"))
		return NOCCIO_CHAT_TYPE_GROUP;
	else if (streq(str, "supergroup"))
		return NOCCIO_CHAT_TYPE_SUPERGROUP;
	else
		return NOCCIO_CHAT_TYPE_CHANNEL;
}