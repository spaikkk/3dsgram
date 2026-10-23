#ifndef TYPES_HEADER
#define TYPES_HEADER

#include <jansson.h>

struct noccio_date {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
};
typedef struct noccio_user {
	int64_t id;
	char *first_name;
	char *last_name;
	char *username;
	int is_bot;
} noccio_user_t;

typedef struct noccio_message_entity {
	char *url;
	struct noccio_user *user;
	char *language;
	char *custom_emoji_id;
	char *date_time_format;
	char *type;
	int offset;
	int lenght; //(utf-16 code units)
	int unix_time;
} noccio_message_entity_t;

typedef struct noccio_photo_size {
	char *file_id;
	char *file_unique_id;
	int width;
	int height;
	int file_size;
} noccio_photo_size_t;

typedef struct noccio_audio {
	char *file_id;
	char *file_unique_id;
	char *performer;
	char *title;
	char *file_name;
	struct noccio_photo_size *thumbnail;
	int duration;
	int file_size;
} noccio_audio_t;

typedef struct noccio_document {
	char *file_id;
	char *file_unique_id;
	struct noccio_photo_size *thumbnail;
	char *file_name;
	char *mime_type;
	int file_size;
} noccio_document_t;

typedef struct noccio_video {
	char *file_id;
	char *file_unique_id;
	struct noccio_photo_size *thumbnail;
	char *file_name;
	char *mime_type;
	int width;
	int height;
	int duration;
	int start_timestamp;
	int file_size;
} noccio_video_t;

typedef struct noccio_animation {
	char *file_id;
	char *file_unique_id;
	struct noccio_photo_size *thumbnail;
	char *file_name;
	char *mime_type;
	int width;
	int height;
	int duration;
	int file_size;
} noccio_animation_t;

typedef struct noccio_voice {
	char *file_id;
	char *file_unique_id;
	char *mime_type;
	int duration;
	int file_size;
} noccio_voice_t;

typedef struct noccio_inline_kb_button {
	char *text;
	char *callback_data;
} noccio_inline_kb_button_t;

typedef struct noccio_inline_kb_markup {
	noccio_inline_kb_button_t **inline_keyboard;
	int rows_count;
	int *cols_on_row;
} noccio_inline_kb_markup_t;

typedef enum {
	NOCCIO_CHAT_TYPE_PRIVATE,
	NOCCIO_CHAT_TYPE_GROUP,
	NOCCIO_CHAT_TYPE_SUPERGROUP,
	NOCCIO_CHAT_TYPE_CHANNEL
} noccio_chat_type_e;
typedef struct noccio_chat {
	int64_t id;
	char *title;
	char *username;
	char *description;
	noccio_chat_type_e type;
	int is_direct_messages;
} noccio_chat_t;

struct noccio_message_entity_arr {
	int count;
	noccio_message_entity_t *arr;
};
struct noccio_photo_size_arr {
	int count;
	noccio_photo_size_t *arr;
};
typedef struct noccio_message {
	int64_t id;
	struct noccio_date date;
	struct noccio_chat *chat;
	struct noccio_user *from;
	struct noccio_message *reply_to_message;
	char *text;
	struct noccio_message_entity_arr entities;
	struct noccio_animation *animation;
	struct noccio_audio *audio;
	struct noccio_document *document;
	struct noccio_photo_size_arr photo;
	struct noccio_video *video;
	struct noccio_voice *voice;
	char *caption;
	// struct noccio_inline_kb_markup *reply_markup;
} noccio_message_t;

typedef struct noccio_inline_query {
	char *id;
	noccio_user_t *from;
	char *query;
	char *offset;
	char *chat_type;
} noccio_inline_query_t;

typedef struct noccio_callback_query {
	char *id;
	noccio_user_t *from;
	// maybeinaccessiblemessage_t
	char *inline_message_id;
	char *chat_instance;
	char *data;
} noccio_callback_query_t;

typedef struct noccio_chosen_inline_result {
	char *result_id;
	noccio_user_t *from;
	char *inline_message_id;
	char *query;
} noccio_chosen_inline_result_t;

typedef struct noccio_input_message_content {
	char *message_text;
} noccio_input_message_content_t;

typedef struct noccio_queryres_article {
	char *type;
	char *id;
	char *title;
	char *thumbnail_url;
	struct noccio_input_message_content *input_message_content;
	noccio_inline_kb_markup_t *reply_markup;
} noccio_queryres_article_t;
typedef struct noccio_queryres_arr {
	int count;
	noccio_queryres_article_t *arr;
} noccio_queryres_arr_t;

typedef struct noccio_input_media_video {
	char *type;
	char *media; // file_id string o percorso disco
	char *caption;
	int is_file;
	char *buffer;
	size_t buffer_size;
	char *filename;
} noccio_input_media_video_t;

typedef struct noccio_reactiontype_emoji {
	char *type;
	char *emoji;
} noccio_reactiontype_emoji;

noccio_message_t *noccio_message_init();
noccio_chat_t *noccio_chat_init();
noccio_user_t *noccio_user_init();
noccio_message_entity_t *noccio_message_entity_init();
noccio_photo_size_t *noccio_photo_size_init();
noccio_animation_t *noccio_animation_init();
noccio_audio_t *noccio_audio_init();
noccio_document_t *noccio_document_init();
noccio_video_t *noccio_video_init();
noccio_voice_t *noccio_voice_init();
noccio_inline_query_t *noccio_inline_query_init();
noccio_callback_query_t *noccio_callback_query_init();
noccio_chosen_inline_result_t *noccio_chosen_inline_result_init();
noccio_input_media_video_t *noccio_input_media_video_init();

int noccio_message_clear(noccio_message_t *msg);
int noccio_chat_clear(noccio_chat_t *chat);
int noccio_user_clear(noccio_user_t *user);
int noccio_message_entity_clear(noccio_message_entity_t *entity);
int noccio_photo_size_clear(noccio_photo_size_t *photosize);
int noccio_animation_clear(noccio_animation_t *anim);
int noccio_audio_clear(noccio_audio_t *audio);
int noccio_document_clear(noccio_document_t *document);
int noccio_video_clear(noccio_video_t *video);
int noccio_voice_clear(noccio_voice_t *voice);
int noccio_inline_query_clear(noccio_inline_query_t *query);
int noccio_callback_query_clear(noccio_callback_query_t *cb_query);
int noccio_chosen_inline_result_clear(noccio_chosen_inline_result_t *res);
int noccio_input_media_video_clear(noccio_input_media_video_t *media);

char *noccio_inline_keyboard_to_string(noccio_inline_kb_markup_t *keyboard);
char *noccio_article_to_string(noccio_queryres_arr_t *article);
char *noccio_imvideo_to_string(noccio_input_media_video_t *media);

#endif