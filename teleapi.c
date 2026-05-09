#include "builtins.h"
#include <curl/curl.h>

// Глобальный токен бота
static char tg_token[256] = "";
static char tg_base_url[512] = "";

// Вспомогательная функция для HTTP запросов
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t real_size = size * nmemb;
    char** response = (char**)userp;
    
    char* ptr = realloc(*response, strlen(*response) + real_size + 1);
    if (!ptr) return 0;
    
    *response = ptr;
    memcpy(&((*response)[strlen(*response)]), contents, real_size);
    (*response)[strlen(*response) + real_size] = 0;
    
    return real_size;
}

static char* tg_request(const char* method, const char* params) {
    CURL* curl = curl_easy_init();
    if (!curl) return str_dup("{}");
    
    char url[1024];
    snprintf(url, sizeof(url), "%s/%s", tg_base_url, method);
    
    char* response = calloc(1, 1);
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    return response;
}

// ============ ИНИЦИАЛИЗАЦИЯ ============
Node* builtin_tg_init(Node** args, int count) {
    if (count < 1) return node_bool(0);
    
    strncpy(tg_token, args[0]->str_val, 255);
    snprintf(tg_base_url, sizeof(tg_base_url), 
             "https://api.telegram.org/bot%s", tg_token);
    
    return node_bool(1);
}

// ============ БАЗОВЫЕ ФУНКЦИИ ============
Node* builtin_tg_send(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[2048];
    snprintf(params, sizeof(params), 
             "chat_id=%s&text=%s", 
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("sendMessage", params);
    return node_string(response);
}

Node* builtin_tg_reply(Node** args, int count) {
    if (count < 3) return node_string(str_dup("{}"));
    
    char params[2048];
    snprintf(params, sizeof(params), 
             "chat_id=%s&text=%s&reply_to_message_id=%s",
             args[0]->str_val, args[1]->str_val, args[2]->str_val);
    
    char* response = tg_request("sendMessage", params);
    return node_string(response);
}

Node* builtin_tg_edit(Node** args, int count) {
    if (count < 3) return node_string(str_dup("{}"));
    
    char params[2048];
    snprintf(params, sizeof(params), 
             "chat_id=%s&message_id=%s&text=%s",
             args[0]->str_val, args[1]->str_val, args[2]->str_val);
    
    char* response = tg_request("editMessageText", params);
    return node_string(response);
}

Node* builtin_tg_delete(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[512];
    snprintf(params, sizeof(params), 
             "chat_id=%s&message_id=%s",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("deleteMessage", params);
    return node_string(response);
}

Node* builtin_tg_forward(Node** args, int count) {
    if (count < 3) return node_string(str_dup("{}"));
    
    char params[1024];
    snprintf(params, sizeof(params), 
             "chat_id=%s&from_chat_id=%s&message_id=%s",
             args[0]->str_val, args[1]->str_val, args[2]->str_val);
    
    char* response = tg_request("forwardMessage", params);
    return node_string(response);
}

Node* builtin_tg_copy(Node** args, int count) {
    if (count < 3) return node_string(str_dup("{}"));
    
    char params[1024];
    snprintf(params, sizeof(params), 
             "chat_id=%s&from_chat_id=%s&message_id=%s",
             args[0]->str_val, args[1]->str_val, args[2]->str_val);
    
    char* response = tg_request("copyMessage", params);
    return node_string(response);
}

Node* builtin_tg_pin(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[512];
    snprintf(params, sizeof(params), 
             "chat_id=%s&message_id=%s",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("pinChatMessage", params);
    return node_string(response);
}

Node* builtin_tg_unpin(Node** args, int count) {
    if (count < 1) return node_string(str_dup("{}"));
    
    char params[256];
    snprintf(params, sizeof(params), "chat_id=%s", args[0]->str_val);
    
    char* response = tg_request("unpinChatMessage", params);
    return node_string(response);
}

// ============ МЕДИА ============
Node* builtin_tg_photo(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[1024];
    snprintf(params, sizeof(params), 
             "chat_id=%s&photo=%s",
             args[0]->str_val, args[1]->str_val);
    
    if (count >= 3) {
        strcat(params, "&caption=");
        strcat(params, args[2]->str_val);
    }
    
    char* response = tg_request("sendPhoto", params);
    return node_string(response);
}

Node* builtin_tg_video(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[1024];
    snprintf(params, sizeof(params), 
             "chat_id=%s&video=%s",
             args[0]->str_val, args[1]->str_val);
    
    if (count >= 3) {
        strcat(params, "&caption=");
        strcat(params, args[2]->str_val);
    }
    
    char* response = tg_request("sendVideo", params);
    return node_string(response);
}

Node* builtin_tg_audio(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[1024];
    snprintf(params, sizeof(params), 
             "chat_id=%s&audio=%s",
             args[0]->str_val, args[1]->str_val);
    
    if (count >= 3) {
        strcat(params, "&title=");
        strcat(params, args[2]->str_val);
    }
    
    char* response = tg_request("sendAudio", params);
    return node_string(response);
}

Node* builtin_tg_voice(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[1024];
    snprintf(params, sizeof(params), 
             "chat_id=%s&voice=%s",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("sendVoice", params);
    return node_string(response);
}

Node* builtin_tg_document(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[1024];
    snprintf(params, sizeof(params), 
             "chat_id=%s&document=%s",
             args[0]->str_val, args[1]->str_val);
    
    if (count >= 3) {
        strcat(params, "&caption=");
        strcat(params, args[2]->str_val);
    }
    
    char* response = tg_request("sendDocument", params);
    return node_string(response);
}

Node* builtin_tg_sticker(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[512];
    snprintf(params, sizeof(params), 
             "chat_id=%s&sticker=%s",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("sendSticker", params);
    return node_string(response);
}

Node* builtin_tg_animation(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[1024];
    snprintf(params, sizeof(params), 
             "chat_id=%s&animation=%s",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("sendAnimation", params);
    return node_string(response);
}

Node* builtin_tg_video_note(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[512];
    snprintf(params, sizeof(params), 
             "chat_id=%s&video_note=%s",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("sendVideoNote", params);
    return node_string(response);
}

Node* builtin_tg_media_group(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[2048];
    snprintf(params, sizeof(params), 
             "chat_id=%s&media=%s",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("sendMediaGroup", params);
    return node_string(response);
}

// ============ КЛАВИАТУРЫ ============
Node* builtin_tg_keyboard(Node** args, int count) {
    if (count < 1) return node_string(str_dup("[]"));
    
    char params[2048];
    snprintf(params, sizeof(params), 
             "{\"keyboard\":%s,\"resize_keyboard\":true}", 
             args[0]->str_val);
    
    return node_string(str_dup(params));
}

Node* builtin_tg_inline_keyboard(Node** args, int count) {
    if (count < 1) return node_string(str_dup("[]"));
    
    char params[2048];
    snprintf(params, sizeof(params), 
             "{\"inline_keyboard\":%s}", 
             args[0]->str_val);
    
    return node_string(str_dup(params));
}

Node* builtin_tg_remove_keyboard(Node** args, int count) {
    return node_string(str_dup("{\"remove_keyboard\":true}"));
}

Node* builtin_tg_button(Node** args, int count) {
    if (count < 1) return node_string(str_dup("{}"));
    
    char params[512];
    snprintf(params, sizeof(params), 
             "{\"text\":\"%s\"}", 
             args[0]->str_val);
    
    return node_string(str_dup(params));
}

Node* builtin_tg_inline_button(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[512];
    snprintf(params, sizeof(params), 
             "{\"text\":\"%s\",\"callback_data\":\"%s\"}", 
             args[0]->str_val, args[1]->str_val);
    
    return node_string(str_dup(params));
}

Node* builtin_tg_url_button(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[1024];
    snprintf(params, sizeof(params), 
             "{\"text\":\"%s\",\"url\":\"%s\"}", 
             args[0]->str_val, args[1]->str_val);
    
    return node_string(str_dup(params));
}

Node* builtin_tg_callback_button(Node** args, int count) {
    return builtin_tg_inline_button(args, count);
}

// ============ ОПРОСЫ ============
Node* builtin_tg_poll(Node** args, int count) {
    if (count < 3) return node_string(str_dup("{}"));
    
    char params[2048];
    snprintf(params, sizeof(params), 
             "chat_id=%s&question=%s&options=%s",
             args[0]->str_val, args[1]->str_val, args[2]->str_val);
    
    char* response = tg_request("sendPoll", params);
    return node_string(response);
}

Node* builtin_tg_quiz(Node** args, int count) {
    if (count < 4) return node_string(str_dup("{}"));
    
    char params[2048];
    snprintf(params, sizeof(params), 
             "chat_id=%s&question=%s&options=%s&correct_option_id=%s&type=quiz",
             args[0]->str_val, args[1]->str_val, 
             args[2]->str_val, args[3]->str_val);
    
    char* response = tg_request("sendPoll", params);
    return node_string(response);
}

Node* builtin_tg_stop_poll(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[512];
    snprintf(params, sizeof(params), 
             "chat_id=%s&message_id=%s",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("stopPoll", params);
    return node_string(response);
}

// ============ ГЕОЛОКАЦИЯ И КОНТАКТЫ ============
Node* builtin_tg_location(Node** args, int count) {
    if (count < 3) return node_string(str_dup("{}"));
    
    char params[512];
    snprintf(params, sizeof(params), 
             "chat_id=%s&latitude=%s&longitude=%s",
             args[0]->str_val, args[1]->str_val, args[2]->str_val);
    
    char* response = tg_request("sendLocation", params);
    return node_string(response);
}

Node* builtin_tg_venue(Node** args, int count) {
    if (count < 5) return node_string(str_dup("{}"));
    
    char params[1024];
    snprintf(params, sizeof(params), 
             "chat_id=%s&latitude=%s&longitude=%s&title=%s&address=%s",
             args[0]->str_val, args[1]->str_val, args[2]->str_val,
             args[3]->str_val, args[4]->str_val);
    
    char* response = tg_request("sendVenue", params);
    return node_string(response);
}

Node* builtin_tg_contact(Node** args, int count) {
    if (count < 3) return node_string(str_dup("{}"));
    
    char params[512];
    snprintf(params, sizeof(params), 
             "chat_id=%s&phone_number=%s&first_name=%s",
             args[0]->str_val, args[1]->str_val, args[2]->str_val);
    
    char* response = tg_request("sendContact", params);
    return node_string(response);
}

Node* builtin_tg_dice(Node** args, int count) {
    if (count < 1) return node_string(str_dup("{}"));
    
    char params[256];
    snprintf(params, sizeof(params), "chat_id=%s", args[0]->str_val);
    
    if (count >= 2) {
        strcat(params, "&emoji=");
        strcat(params, args[1]->str_val);
    }
    
    char* response = tg_request("sendDice", params);
    return node_string(response);
}

// ============ АДМИН-КОМАНДЫ ============
Node* builtin_tg_kick(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[512];
    snprintf(params, sizeof(params), 
             "chat_id=%s&user_id=%s",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("kickChatMember", params);
    return node_string(response);
}

Node* builtin_tg_ban(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[512];
    snprintf(params, sizeof(params), 
             "chat_id=%s&user_id=%s",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("banChatMember", params);
    return node_string(response);
}

Node* builtin_tg_unban(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[512];
    snprintf(params, sizeof(params), 
             "chat_id=%s&user_id=%s&only_if_banned=true",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("unbanChatMember", params);
    return node_string(response);
}

Node* builtin_tg_mute(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[1024];
    snprintf(params, sizeof(params), 
             "chat_id=%s&user_id=%s&permissions={\"can_send_messages\":false}",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("restrictChatMember", params);
    return node_string(response);
}

Node* builtin_tg_unmute(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[1024];
    snprintf(params, sizeof(params), 
             "chat_id=%s&user_id=%s&permissions={\"can_send_messages\":true,\"can_send_media_messages\":true,\"can_send_other_messages\":true,\"can_add_web_page_previews\":true}",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("restrictChatMember", params);
    return node_string(response);
}

Node* builtin_tg_promote(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[1024];
    snprintf(params, sizeof(params), 
             "chat_id=%s&user_id=%s&can_change_info=true&can_delete_messages=true&can_invite_users=true&can_restrict_members=true&can_pin_messages=true&can_promote_members=true",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("promoteChatMember", params);
    return node_string(response);
}

Node* builtin_tg_demote(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[512];
    snprintf(params, sizeof(params), 
             "chat_id=%s&user_id=%s&can_change_info=false&can_delete_messages=false&can_invite_users=false&can_restrict_members=false&can_pin_messages=false&can_promote_members=false",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("promoteChatMember", params);
    return node_string(response);
}

Node* builtin_tg_restrict(Node** args, int count) {
    if (count < 3) return node_string(str_dup("{}"));
    
    char params[1024];
    snprintf(params, sizeof(params), 
             "chat_id=%s&user_id=%s&permissions=%s",
             args[0]->str_val, args[1]->str_val, args[2]->str_val);
    
    char* response = tg_request("restrictChatMember", params);
    return node_string(response);
}

Node* builtin_tg_set_title(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[512];
    snprintf(params, sizeof(params), 
             "chat_id=%s&title=%s",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("setChatTitle", params);
    return node_string(response);
}

Node* builtin_tg_set_description(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[1024];
    snprintf(params, sizeof(params), 
             "chat_id=%s&description=%s",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("setChatDescription", params);
    return node_string(response);
}

Node* builtin_tg_set_photo_chat(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[512];
    snprintf(params, sizeof(params), 
             "chat_id=%s&photo=%s",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("setChatPhoto", params);
    return node_string(response);
}

Node* builtin_tg_leave_chat(Node** args, int count) {
    if (count < 1) return node_string(str_dup("{}"));
    
    char params[256];
    snprintf(params, sizeof(params), "chat_id=%s", args[0]->str_val);
    
    char* response = tg_request("leaveChat", params);
    return node_string(response);
}

Node* builtin_tg_get_chat_member(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char params[512];
    snprintf(params, sizeof(params), 
             "chat_id=%s&user_id=%s",
             args[0]->str_val, args[1]->str_val);
    
    char* response = tg_request("getChatMember", params);
    return node_string(response);
}

Node* builtin_tg_get_chat_members_count(Node** args, int count) {
    if (count < 1) return node_string(str_dup("{}"));
    
    char params[256];
    snprintf(params, sizeof(params), "chat_id=%s", args[0]->str_val);
    
    char* response = tg_request("getChatMemberCount", params);
    return node_string(response);
}

Node* builtin_tg_get_administrators(Node** args, int count) {
    if (count < 1) return node_string(str_dup("{}"));
    
    char params[256];
    snprintf(params, sizeof(params), "chat_id=%s", args[0]->str_val);
    
    char* response = tg_request("getChatAdministrators", params);
    return node_string(response);
}

// ============ ВЕБХУКИ И ОБНОВЛЕНИЯ ============
Node* builtin_tg_get_updates(Node** args, int count) {
    char params[256] = "";
    
    if (count >= 1) {
        snprintf(params, sizeof(params), "offset=%s", args[0]->str_val);
    }
    
    char* response = tg_request("getUpdates", params);
    return node_string(response);
}

Node* builtin_tg_set_webhook(Node** args, int count) {
    if (count < 1) return node_string(str_dup("{}"));
    
    char params[1024];
    snprintf(params, sizeof(params), "url=%s", args[0]->str_val);
    
    char* response = tg_request("setWebhook", params);
    return node_string(response);
}

Node* builtin_tg_delete_webhook(Node** args, int count) {
    char* response = tg_request("deleteWebhook", "");
    return node_string(response);
}

Node* builtin_tg_get_webhook_info(Node** args, int count) {
    char* response = tg_request("getWebhookInfo", "");
    return node_string(response);
}

// ============ ИНФОРМАЦИЯ ============
Node* builtin_tg_get_me(Node** args, int count) {
    char* response = tg_request("getMe", "");
    return node_string(response);
}

Node* builtin_tg_get_chat(Node** args, int count) {
    if (count < 1) return node_string(str_dup("{}"));
    
    char params[256];
    snprintf(params, sizeof(params), "chat_id=%s", args[0]->str_val);
    
    char* response = tg_request("getChat", params);
    return node_string(response);
}

Node* builtin_tg_get_user_photos(Node** args, int count) {
    if (count < 1) return node_string(str_dup("{}"));
    
    char params[256];
    snprintf(params, sizeof(params), "user_id=%s", args[0]->str_val);
    
    char* response = tg_request("getUserProfilePhotos", params);
    return node_string(response);
}

Node* builtin_tg_get_file(Node** args, int count) {
    if (count < 1) return node_string(str_dup("{}"));
    
    char params[256];
    snprintf(params, sizeof(params), "file_id=%s", args[0]->str_val);
    
    char* response = tg_request("getFile", params);
    return node_string(response);
}

Node* builtin_tg_download_file(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    
    char url[1024];
    snprintf(url, sizeof(url), 
             "https://api.telegram.org/file/bot%s/%s",
             tg_token, args[0]->str_val);
    
    // Скачиваем файл
    CURL* curl = curl_easy_init();
    FILE* fp = fopen(args[1]->str_val, "wb");
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);
    
    return node_bool(1);
}

// ============ КОМАНДЫ БОТА ============
Node* builtin_tg_set_commands(Node** args, int count) {
    if (count < 1) return node_string(str_dup("{}"));
    
    char params[4096];
    snprintf(params, sizeof(params), "commands=%s", args[0]->str_val);
    
    char* response = tg_request("setMyCommands", params);
    return node_string(response);
}

Node* builtin_tg_get_commands(Node** args, int count) {
    char* response = tg_request("getMyCommands", "");
    return node_string(response);
}

Node* builtin_tg_delete_commands(Node** args, int count) {
    char* response = tg_request("deleteMyCommands", "");
    return node_string(response);
}

// ============ ФОРМАТИРОВАНИЕ ТЕКСТА ============
Node* builtin_tg_bold(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char buf[1024];
    snprintf(buf, sizeof(buf), "<b>%s</b>", args[0]->str_val);
    return node_string(str_dup(buf));
}

Node* builtin_tg_italic(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char buf[1024];
    snprintf(buf, sizeof(buf), "<i>%s</i>", args[0]->str_val);
    return node_string(str_dup(buf));
}

Node* builtin_tg_code(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char buf[1024];
    snprintf(buf, sizeof(buf), "<code>%s</code>", args[0]->str_val);
    return node_string(str_dup(buf));
}

Node* builtin_tg_pre(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char buf[1024];
    if (count >= 2)
        snprintf(buf, sizeof(buf), "<pre><code class=\"language-%s\">%s</code></pre>", args[1]->str_val, args[0]->str_val);
    else
        snprintf(buf, sizeof(buf), "<pre>%s</pre>", args[0]->str_val);
    return node_string(str_dup(buf));
}

Node* builtin_tg_link(Node** args, int count) {
    if (count < 2) return node_string(str_dup(""));
    char buf[1024];
    snprintf(buf, sizeof(buf), "<a href=\"%s\">%s</a>", args[1]->str_val, args[0]->str_val);
    return node_string(str_dup(buf));
}

Node* builtin_tg_mention(Node** args, int count) {
    if (count < 2) return node_string(str_dup(""));
    char buf[1024];
    snprintf(buf, sizeof(buf), "<a href=\"tg://user?id=%s\">%s</a>", args[1]->str_val, args[0]->str_val);
    return node_string(str_dup(buf));
}

Node* builtin_tg_spoiler(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char buf[1024];
    snprintf(buf, sizeof(buf), "<tg-spoiler>%s</tg-spoiler>", args[0]->str_val);
    return node_string(str_dup(buf));
}

Node* builtin_tg_quote(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char buf[1024];
    snprintf(buf, sizeof(buf), "<blockquote>%s</blockquote>", args[0]->str_val);
    return node_string(str_dup(buf));
}