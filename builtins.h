#ifndef BUILTINS_H
#define BUILTINS_H
#include "eval.h"

// ============ PRU — РОДНОЙ ФОРМАТ ============
Node* builtin_pru_load(Node** args, int count);
Node* builtin_pru_check(Node** args, int count);
Node* builtin_pru_info(Node** args, int count);

// ============ БАЗОВЫЕ ФАЙЛЫ ============
Node* builtin_read_file(Node** args, int count);
Node* builtin_write_file(Node** args, int count);
Node* builtin_append_file(Node** args, int count);
Node* builtin_file_exists(Node** args, int count);
Node* builtin_file_size(Node** args, int count);
Node* builtin_delete_file(Node** args, int count);
Node* builtin_list_dir(Node** args, int count);
Node* builtin_copy_file(Node** args, int count);

// ============ ТЕКСТОВЫЕ ФОРМАТЫ (TXT, LOG, PY) ============
Node* builtin_read_lines(Node** args, int count);
Node* builtin_count_lines(Node** args, int count);
Node* builtin_grep(Node** args, int count);

// ============ CSV ============
Node* builtin_csv_parse(Node** args, int count);
Node* builtin_csv_get_cell(Node** args, int count);
Node* builtin_csv_row_count(Node** args, int count);

// ============ JSON ============
Node* builtin_json_parse(Node** args, int count);
Node* builtin_json_get(Node** args, int count);
Node* builtin_json_keys(Node** args, int count);

// ============ XML ============
Node* builtin_xml_parse(Node** args, int count);
Node* builtin_xml_get_tag(Node** args, int count);

// ============ INI ============
Node* builtin_ini_parse(Node** args, int count);
Node* builtin_ini_get(Node** args, int count);

// ============ YAML (простой) ============
Node* builtin_yaml_parse(Node** args, int count);

// ============ БИНАРНЫЕ ФОРМАТЫ (PNG, JPG, MP3, MP4, ZIP) ============
Node* builtin_read_bytes(Node** args, int count);
Node* builtin_write_bytes(Node** args, int count);
Node* builtin_get_metadata(Node** args, int count);
Node* builtin_is_format(Node** args, int count);

// ============ СТРОКИ ============
Node* builtin_length(Node** args, int count);
Node* builtin_upper(Node** args, int count);
Node* builtin_lower(Node** args, int count);
Node* builtin_trim(Node** args, int count);
Node* builtin_split(Node** args, int count);
Node* builtin_join(Node** args, int count);
Node* builtin_replace(Node** args, int count);

// ============ ЦВЕТА ============
Node* builtin_red(Node** args, int count);
Node* builtin_green(Node** args, int count);
Node* builtin_yellow(Node** args, int count);
Node* builtin_blue(Node** args, int count);
Node* builtin_magenta(Node** args, int count);
Node* builtin_cyan(Node** args, int count);
Node* builtin_bold(Node** args, int count);

// ============ СИСТЕМНЫЕ ============
Node* builtin_type_of(Node** args, int count);
Node* builtin_input(Node** args, int count);
Node* builtin_int_cast(Node** args, int count);
Node* builtin_random(Node** args, int count);
Node* builtin_time_now(Node** args, int count);
Node* builtin_sleep_ms(Node** args, int count);

// ============ телеapi — Telegram Bot API ============
// Базовые
Node* builtin_tg_init(Node** args, int count);
Node* builtin_tg_send(Node** args, int count);
Node* builtin_tg_reply(Node** args, int count);
Node* builtin_tg_edit(Node** args, int count);
Node* builtin_tg_delete(Node** args, int count);
Node* builtin_tg_forward(Node** args, int count);
Node* builtin_tg_copy(Node** args, int count);
Node* builtin_tg_pin(Node** args, int count);
Node* builtin_tg_unpin(Node** args, int count);

// Медиа
Node* builtin_tg_photo(Node** args, int count);
Node* builtin_tg_video(Node** args, int count);
Node* builtin_tg_audio(Node** args, int count);
Node* builtin_tg_voice(Node** args, int count);
Node* builtin_tg_document(Node** args, int count);
Node* builtin_tg_sticker(Node** args, int count);
Node* builtin_tg_animation(Node** args, int count);
Node* builtin_tg_video_note(Node** args, int count);
Node* builtin_tg_media_group(Node** args, int count);

// Клавиатуры
Node* builtin_tg_keyboard(Node** args, int count);
Node* builtin_tg_inline_keyboard(Node** args, int count);
Node* builtin_tg_remove_keyboard(Node** args, int count);
Node* builtin_tg_button(Node** args, int count);
Node* builtin_tg_inline_button(Node** args, int count);
Node* builtin_tg_url_button(Node** args, int count);
Node* builtin_tg_callback_button(Node** args, int count);

// Инлайн-режим
Node* builtin_tg_answer_inline(Node** args, int count);
Node* builtin_tg_inline_article(Node** args, int count);
Node* builtin_tg_inline_photo(Node** args, int count);
Node* builtin_tg_inline_video(Node** args, int count);

// Опросы и викторины
Node* builtin_tg_poll(Node** args, int count);
Node* builtin_tg_quiz(Node** args, int count);
Node* builtin_tg_stop_poll(Node** args, int count);

// Геолокация и контакты
Node* builtin_tg_location(Node** args, int count);
Node* builtin_tg_venue(Node** args, int count);
Node* builtin_tg_contact(Node** args, int count);
Node* builtin_tg_dice(Node** args, int count);

// Админ-команды
Node* builtin_tg_kick(Node** args, int count);
Node* builtin_tg_ban(Node** args, int count);
Node* builtin_tg_unban(Node** args, int count);
Node* builtin_tg_mute(Node** args, int count);
Node* builtin_tg_unmute(Node** args, int count);
Node* builtin_tg_promote(Node** args, int count);
Node* builtin_tg_demote(Node** args, int count);
Node* builtin_tg_restrict(Node** args, int count);
Node* builtin_tg_set_title(Node** args, int count);
Node* builtin_tg_set_description(Node** args, int count);
Node* builtin_tg_set_photo_chat(Node** args, int count);
Node* builtin_tg_leave_chat(Node** args, int count);
Node* builtin_tg_get_chat_member(Node** args, int count);
Node* builtin_tg_get_chat_members_count(Node** args, int count);
Node* builtin_tg_get_administrators(Node** args, int count);

// Вебхуки и обновления
Node* builtin_tg_get_updates(Node** args, int count);
Node* builtin_tg_set_webhook(Node** args, int count);
Node* builtin_tg_delete_webhook(Node** args, int count);
Node* builtin_tg_get_webhook_info(Node** args, int count);

// Информация
Node* builtin_tg_get_me(Node** args, int count);
Node* builtin_tg_get_chat(Node** args, int count);
Node* builtin_tg_get_user_photos(Node** args, int count);
Node* builtin_tg_get_file(Node** args, int count);
Node* builtin_tg_download_file(Node** args, int count);

// Команды бота
Node* builtin_tg_set_commands(Node** args, int count);
Node* builtin_tg_get_commands(Node** args, int count);
Node* builtin_tg_delete_commands(Node** args, int count);

// Форматирование текста
Node* builtin_tg_bold(Node** args, int count);
Node* builtin_tg_italic(Node** args, int count);
Node* builtin_tg_code(Node** args, int count);
Node* builtin_tg_pre(Node** args, int count);
Node* builtin_tg_link(Node** args, int count);
Node* builtin_tg_mention(Node** args, int count);
Node* builtin_tg_spoiler(Node** args, int count);
Node* builtin_tg_quote(Node** args, int count);

void register_builtins(Env* env);
#endif