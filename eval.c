
#include "eval.h"
#include "builtins.h"

Env* env_new(Env* parent) {
    Env* e = safe_alloc(sizeof(Env));
    e->names  = safe_alloc(sizeof(char*) * 256);
    e->values = safe_alloc(sizeof(Node*) * 256);
    e->count  = 0;
    e->parent = parent;
    return e;
}

void env_set(Env* e, const char* name, Node* value) {
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->names[i], name) == 0) {
            e->values[i] = value;
            return;
        }
    }
    e->names[e->count] = str_dup(name);
    e->values[e->count] = value;
    e->count++;
}

Node* env_get(Env* e, const char* name) {
    for (int i = 0; i < e->count; i++)
        if (strcmp(e->names[i], name) == 0) return e->values[i];
    if (e->parent) return env_get(e->parent, name);
    return node_nil();
}

// Таблица встроенных функций
static Node* call_builtin(const char* name, Node** args, int count) {
    // PRU
    if (strcmp(name, "пру_загрузи") == 0)     return builtin_pru_load(args, count);
    if (strcmp(name, "пру_проверь") == 0)     return builtin_pru_check(args, count);
    if (strcmp(name, "пру_инфо") == 0)        return builtin_pru_info(args, count);
    
    // Файлы
    if (strcmp(name, "прочитай") == 0)        return builtin_read_file(args, count);
    if (strcmp(name, "запиши") == 0)          return builtin_write_file(args, count);
    if (strcmp(name, "добавь") == 0)          return builtin_append_file(args, count);
    if (strcmp(name, "файл_есть") == 0)       return builtin_file_exists(args, count);
    if (strcmp(name, "размер_файла") == 0)    return builtin_file_size(args, count);
    if (strcmp(name, "удали_файл") == 0)      return builtin_delete_file(args, count);
    if (strcmp(name, "список_папки") == 0)    return builtin_list_dir(args, count);
    if (strcmp(name, "копируй_файл") == 0)    return builtin_copy_file(args, count);
    
    // Текстовые форматы
    if (strcmp(name, "строки_файла") == 0)    return builtin_read_lines(args, count);
    if (strcmp(name, "счёт_строк") == 0)      return builtin_count_lines(args, count);
    if (strcmp(name, "поиск") == 0)           return builtin_grep(args, count);
    
    // CSV
    if (strcmp(name, "csv_читай") == 0)       return builtin_csv_parse(args, count);
    if (strcmp(name, "csv_получи") == 0)      return builtin_csv_get_cell(args, count);
    if (strcmp(name, "csv_счёт_рядов") == 0)  return builtin_csv_row_count(args, count);
    
    // JSON
    if (strcmp(name, "json_ключи_внутр") == 0) return builtin_json_keys(args, count);
    if (strcmp(name, "json_получи") == 0)     return builtin_json_get(args, count);
    
    // XML
    if (strcmp(name, "xml_парс") == 0)        return builtin_xml_parse(args, count);
    if (strcmp(name, "xml_тег") == 0)         return builtin_xml_get_tag(args, count);
    
    // INI
    if (strcmp(name, "ini_читай") == 0)       return builtin_ini_parse(args, count);
    if (strcmp(name, "ini_значение") == 0)    return builtin_ini_get(args, count);
    
    // YAML
    if (strcmp(name, "yaml_парс") == 0)       return builtin_yaml_parse(args, count);
    
    // Бинарные
    if (strcmp(name, "байты") == 0)           return builtin_read_bytes(args, count);
    if (strcmp(name, "метаданные") == 0)      return builtin_get_metadata(args, count);
    if (strcmp(name, "это_формат") == 0)      return builtin_is_format(args, count);
    
    // Строки
    if (strcmp(name, "длина") == 0)           return builtin_length(args, count);
    if (strcmp(name, "верх_регистр") == 0)    return builtin_upper(args, count);
    if (strcmp(name, "низ_регистр") == 0)     return builtin_lower(args, count);
    if (strcmp(name, "обрежь") == 0)          return builtin_trim(args, count);
    if (strcmp(name, "заменить") == 0)        return builtin_replace(args, count);
    if (strcmp(name, "разделить") == 0)       return builtin_split(args, count);
    if (strcmp(name, "соединить") == 0)       return builtin_join(args, count);
    
    // Цвета
    if (strcmp(name, "кр") == 0)    return builtin_red(args, count);
    if (strcmp(name, "зел") == 0)   return builtin_green(args, count);
    if (strcmp(name, "жёл") == 0)   return builtin_yellow(args, count);
    if (strcmp(name, "син") == 0)   return builtin_blue(args, count);
    if (strcmp(name, "фиол") == 0)  return builtin_magenta(args, count);
    if (strcmp(name, "гол") == 0)   return builtin_cyan(args, count);
    if (strcmp(name, "жир") == 0)   return builtin_bold(args, count);
    
    // Системные
    if (strcmp(name, "тип_данных") == 0)    return builtin_type_of(args, count);
    if (strcmp(name, "ввод") == 0)          return builtin_input(args, count);
    if (strcmp(name, "в_число") == 0)       return builtin_int_cast(args, count);
    if (strcmp(name, "рандом") == 0)        return builtin_random(args, count);
    if (strcmp(name, "время_сейчас") == 0)  return builtin_time_now(args, count);
    if (strcmp(name, "сон_мс") == 0)        return builtin_sleep_ms(args, count);
    
#ifdef HAS_CURL
    // телеapi
    if (strcmp(name, "телеapi_инит") == 0)            return builtin_tg_init(args, count);
    if (strcmp(name, "телеapi_отправь") == 0)         return builtin_tg_send(args, count);
    if (strcmp(name, "телеapi_ответь") == 0)          return builtin_tg_reply(args, count);
    if (strcmp(name, "телеapi_редактируй") == 0)      return builtin_tg_edit(args, count);
    if (strcmp(name, "телеapi_удали") == 0)           return builtin_tg_delete(args, count);
    if (strcmp(name, "телеapi_перешли") == 0)         return builtin_tg_forward(args, count);
    if (strcmp(name, "телеapi_копируй") == 0)         return builtin_tg_copy(args, count);
    if (strcmp(name, "телеapi_закрепи") == 0)         return builtin_tg_pin(args, count);
    if (strcmp(name, "телеapi_открепи") == 0)         return builtin_tg_unpin(args, count);
    if (strcmp(name, "телеapi_фото") == 0)            return builtin_tg_photo(args, count);
    if (strcmp(name, "телеapi_видео") == 0)           return builtin_tg_video(args, count);
    if (strcmp(name, "телеapi_аудио") == 0)           return builtin_tg_audio(args, count);
    if (strcmp(name, "телеapi_голос") == 0)           return builtin_tg_voice(args, count);
    if (strcmp(name, "телеapi_документ") == 0)        return builtin_tg_document(args, count);
    if (strcmp(name, "телеapi_стикер") == 0)          return builtin_tg_sticker(args, count);
    if (strcmp(name, "телеapi_анимация") == 0)        return builtin_tg_animation(args, count);
    if (strcmp(name, "телеapi_клавиша") == 0)         return builtin_tg_button(args, count);
    if (strcmp(name, "телеapi_инлайн_клавиша") == 0)  return builtin_tg_inline_button(args, count);
    if (strcmp(name, "телеapi_урл_клавиша") == 0)     return builtin_tg_url_button(args, count);
    if (strcmp(name, "телеapi_опрос") == 0)           return builtin_tg_poll(args, count);
    if (strcmp(name, "телеapi_викторина") == 0)       return builtin_tg_quiz(args, count);
    if (strcmp(name, "телеapi_стоп_опрос") == 0)      return builtin_tg_stop_poll(args, count);
    if (strcmp(name, "телеapi_локация") == 0)         return builtin_tg_location(args, count);
    if (strcmp(name, "телеapi_контакт") == 0)         return builtin_tg_contact(args, count);
    if (strcmp(name, "телеapi_дайс") == 0)            return builtin_tg_dice(args, count);
    if (strcmp(name, "телеapi_кик") == 0)             return builtin_tg_kick(args, count);
    if (strcmp(name, "телеapi_бан") == 0)             return builtin_tg_ban(args, count);
    if (strcmp(name, "телеapi_разбан") == 0)          return builtin_tg_unban(args, count);
    if (strcmp(name, "телеapi_мут") == 0)             return builtin_tg_mute(args, count);
    if (strcmp(name, "телеapi_размут") == 0)          return builtin_tg_unmute(args, count);
    if (strcmp(name, "телеapi_админ") == 0)           return builtin_tg_promote(args, count);
    if (strcmp(name, "телеapi_разжаловать") == 0)     return builtin_tg_demote(args, count);
    if (strcmp(name, "телеapi_ограничить") == 0)      return builtin_tg_restrict(args, count);
    if (strcmp(name, "телеapi_инфо") == 0)            return builtin_tg_get_me(args, count);
    if (strcmp(name, "телеapi_чат") == 0)             return builtin_tg_get_chat(args, count);
    if (strcmp(name, "телеapi_участник") == 0)        return builtin_tg_get_chat_member(args, count);
    if (strcmp(name, "телеapi_участников") == 0)      return builtin_tg_get_chat_members_count(args, count);
    if (strcmp(name, "телеapi_админы") == 0)          return builtin_tg_get_administrators(args, count);
    if (strcmp(name, "телеapi_обновления") == 0)      return builtin_tg_get_updates(args, count);
    if (strcmp(name, "телеapi_вебхук") == 0)          return builtin_tg_set_webhook(args, count);
    if (strcmp(name, "телеapi_убрать_вебхук") == 0)   return builtin_tg_delete_webhook(args, count);
    if (strcmp(name, "телеapi_инфо_вебхук") == 0)     return builtin_tg_get_webhook_info(args, count);
    if (strcmp(name, "телеapi_фото_юзера") == 0)      return builtin_tg_get_user_photos(args, count);
    if (strcmp(name, "телеapi_файл") == 0)            return builtin_tg_get_file(args, count);
    if (strcmp(name, "телеapi_скачай") == 0)          return builtin_tg_download_file(args, count);
    if (strcmp(name, "телеapi_команды") == 0)         return builtin_tg_set_commands(args, count);
    if (strcmp(name, "телеapi_мои_команды") == 0)     return builtin_tg_get_commands(args, count);
    if (strcmp(name, "телеapi_убрать_команды") == 0)  return builtin_tg_delete_commands(args, count);
    if (strcmp(name, "телеapi_жирный") == 0)          return builtin_tg_bold(args, count);
    if (strcmp(name, "телеapi_курсив") == 0)          return builtin_tg_italic(args, count);
    if (strcmp(name, "телеapi_код") == 0)             return builtin_tg_code(args, count);
    if (strcmp(name, "телеapi_пре") == 0)             return builtin_tg_pre(args, count);
    if (strcmp(name, "телеapi_ссылка") == 0)          return builtin_tg_link(args, count);
    if (strcmp(name, "телеapi_упоминание") == 0)      return builtin_tg_mention(args, count);
    if (strcmp(name, "телеapi_спойлер") == 0)         return builtin_tg_spoiler(args, count);
    if (strcmp(name, "телеapi_цитата") == 0)          return builtin_tg_quote(args, count);
#endif
    
    return NULL;
}

Node* eval(Node* n, Env* env) {
    if (!n) return node_nil();

    switch (n->type) {
        // ============ ЛИТЕРАЛЫ ============
        case NODE_NUMBER:
        case NODE_FLOAT:
        case NODE_STRING:
        case NODE_BOOL:
        case NODE_NIL:
            return n;

        // ============ ПЕРЕМЕННАЯ ============
        case NODE_IDENT:
            return env_get(env, n->str_val);

        // ============ БИНАРНЫЕ ОПЕРАЦИИ ============
        case NODE_BINOP: {
            Node* l = eval(n->left, env);
            Node* r = eval(n->right, env);

            // Конкатенация строк
            if (l->type == NODE_STRING || r->type == NODE_STRING) {
                char buf[4096];
                char ls[128] = "", rs[128] = "";
                if (l->type == NODE_STRING) snprintf(ls, sizeof(ls), "%s", l->str_val);
                else if (l->type == NODE_NUMBER) snprintf(ls, sizeof(ls), "%d", l->int_val);
                else if (l->type == NODE_FLOAT) snprintf(ls, sizeof(ls), "%g", l->float_val);
                else if (l->type == NODE_BOOL) snprintf(ls, sizeof(ls), "%s", l->int_val ? "истина" : "ложь");
                if (r->type == NODE_STRING) snprintf(rs, sizeof(rs), "%s", r->str_val);
                else if (r->type == NODE_NUMBER) snprintf(rs, sizeof(rs), "%d", r->int_val);
                else if (r->type == NODE_FLOAT) snprintf(rs, sizeof(rs), "%g", r->float_val);
                else if (r->type == NODE_BOOL) snprintf(rs, sizeof(rs), "%s", r->int_val ? "истина" : "ложь");
                snprintf(buf, sizeof(buf), "%s%s", ls, rs);
                return node_string(str_dup(buf));
            }

            // Float операции
            if (l->type == NODE_FLOAT || r->type == NODE_FLOAT) {
                double a = (l->type == NODE_FLOAT) ? l->float_val : (double)l->int_val;
                double b = (r->type == NODE_FLOAT) ? r->float_val : (double)r->int_val;
                switch (n->op) {
                    case TOK_PLUS:  return node_float(a + b);
                    case TOK_MINUS: return node_float(a - b);
                    case TOK_STAR:  return node_float(a * b);
                    case TOK_SLASH: return b != 0 ? node_float(a / b) : node_nil();
                    case TOK_EQEQ:  return node_bool(a == b);
                    case TOK_NEQ:   return node_bool(a != b);
                    case TOK_LT:    return node_bool(a < b);
                    case TOK_GT:    return node_bool(a > b);
                    case TOK_LEQ:   return node_bool(a <= b);
                    case TOK_GEQ:   return node_bool(a >= b);
                    default:        return node_nil();
                }
            }

            // Целочисленные операции
            int a = l->int_val, b = r->int_val;
            switch (n->op) {
                case TOK_PLUS:  return node_number(a + b);
                case TOK_MINUS: return node_number(a - b);
                case TOK_STAR:  return node_number(a * b);
                case TOK_SLASH: return b != 0 ? node_number(a / b) : node_nil();
                case TOK_EQEQ:  return node_bool(a == b);
                case TOK_NEQ:   return node_bool(a != b);
                case TOK_LT:    return node_bool(a < b);
                case TOK_GT:    return node_bool(a > b);
                case TOK_LEQ:   return node_bool(a <= b);
                case TOK_GEQ:   return node_bool(a >= b);
                // Логические операторы
                case TOK_AND:   return node_bool(a && b);
                case TOK_OR:    return node_bool(a || b);
                default:        return node_nil();
            }
        }

        // ============ ПРИСВАИВАНИЕ ============
        case NODE_LET:
        case NODE_ASSIGN: {
            Node* val = eval(n->value, env);
            env_set(env, n->str_val, val);
            return val;
        }

        // ============ УСЛОВИЕ ============
        case NODE_IF: {
            Node* cond = eval(n->cond, env);
            if (cond->int_val) return eval(n->body, env);
            if (n->else_body)  return eval(n->else_body, env);
            return node_nil();
        }

        // ============ ЦИКЛ ПОКА ============
        // ИСКАТЬ: case NODE_WHILE:
        case NODE_WHILE: {
            Node* last = node_nil();
            while (1) {
                Node* cond = eval(n->cond, env);
                if (!cond->int_val) break;
                last = eval(n->body, env);
                // Обработка break/continue
                if (last && last->type == NODE_BREAK) { last = node_nil(); break; }
                if (last && last->type == NODE_CONTINUE) { last = node_nil(); continue; }
            }
            return last;
        }

        // ============ ЦИКЛ ДЛЯ ============
        case NODE_FOR: {
            Node* iter = eval(n->iterable, env);
            Node* last = node_nil();

            if (iter->type == NODE_LIST) {
                for (int i = 0; i < iter->item_count; i++) {
                    env_set(env, n->str_val, iter->items[i]);
                    last = eval(n->body, env);
                    if (last && last->type == NODE_BREAK) { last = node_nil(); break; }
                    if (last && last->type == NODE_CONTINUE) { last = node_nil(); continue; }
                }
            }
            else if (iter->type == NODE_NUMBER || iter->type == NODE_FLOAT) {
                int max = (iter->type == NODE_FLOAT) ? (int)iter->float_val : iter->int_val;
                for (int i = 0; i < max; i++) {
                    env_set(env, n->str_val, node_number(i));
                    last = eval(n->body, env);
                    if (last && last->type == NODE_BREAK) { last = node_nil(); break; }
                    if (last && last->type == NODE_CONTINUE) { last = node_nil(); continue; }
                }
            }

            return last;
        }

        // ============ ВЫВОД ============
        case NODE_PRINT: {
            Node* v = eval(n->value, env);
            switch (v->type) {
                case NODE_STRING: printf("%s\n", v->str_val); break;
                case NODE_NUMBER: printf("%d\n", v->int_val); break;
                case NODE_FLOAT:  printf("%g\n", v->float_val); break;
                case NODE_BOOL:   printf("%s\n", v->int_val ? "истина" : "ложь"); break;
                case NODE_NIL:    printf("ничто\n"); break;
                default:          printf("%d\n", v->int_val);
            }
            return v;
        }

        // ============ ФУНКЦИЯ (объявление) ============
        case NODE_FN_DEF:
        case NODE_LAMBDA: {
            env_set(env, n->str_val, n);
            return n;
        }

        // ============ ВЫЗОВ ФУНКЦИИ ============
        case NODE_FN_CALL: {
            if (n->fn->type == NODE_IDENT) {
                char* name = n->fn->str_val;
                Node** evaled_args = safe_alloc(sizeof(Node*) * (n->arg_count + 1));
                for (int i = 0; i < n->arg_count; i++)
                    evaled_args[i] = eval(n->args[i], env);

                Node* result = call_builtin(name, evaled_args, n->arg_count);
                if (result) return result;
            }

            Node* f = eval(n->fn, env);
            if (!f || (f->type != NODE_FN_DEF && f->type != NODE_LAMBDA)) return node_nil();

            Env* call_env = env_new(env);
            for (int i = 0; i < f->param_count && i < n->arg_count; i++) {
                Node* arg = eval(n->args[i], env);
                env_set(call_env, f->params[i]->str_val, arg);
            }
            return eval(f->body, call_env);
        }

        // ============ БЛОК ============
        case NODE_BLOCK: {
            Node* last = node_nil();
            for (int i = 0; i < n->count; i++) {
                last = eval(n->stmts[i], env);
                if (last && (last->type == NODE_RETURN || last->type == NODE_BREAK || last->type == NODE_CONTINUE))
                    return last;
            }
            return last;
        }

        // ============ ВОЗВРАТ ============
        // ИСКАТЬ: case NODE_RETURN:
        case NODE_RETURN:
            return eval(n->value, env);

        // ============ BREAK ============
        // ИСКАТЬ: case NODE_BREAK:
        case NODE_BREAK:
            return n;

        // ============ CONTINUE ============
        // ИСКАТЬ: case NODE_CONTINUE:
        case NODE_CONTINUE:
            return n;

        // ============ СПИСОК ============
        case NODE_LIST:
            return n;

        // ============ СЛОВАРЬ ============
        // ИСКАТЬ: case NODE_DICT:
        case NODE_DICT:
            return n;

        // ============ TRY/CATCH ============
        // ИСКАТЬ: case NODE_TRY_CATCH:
        case NODE_TRY_CATCH: {
            Node* result = eval(n->body, env);
            // Если результат — исключение (THROW), ловим
            if (result && result->type == NODE_THROW) {
                fprintf(stderr, "🔄 Поймано исключение: %s\n", result->str_val ? result->str_val : "неизвестно");
                return eval(n->catch_body, env);
            }
            return result;
        }

        // ============ THROW ============
        // ИСКАТЬ: case NODE_THROW:
        case NODE_THROW: {
            fprintf(stderr, "💀 Исключение: %s\n", n->str_val ? n->str_val : "неизвестная ошибка");
            return n;
        }

        // ============ SWITCH/CASE ============
        // ИСКАТЬ: case NODE_SWITCH:
        case NODE_SWITCH: {
            Node* val = eval(n->value, env);
            for (int i = 0; i < n->case_count; i++) {
                Node* case_node = n->cases[i];
                Node* case_val = eval(case_node->value, env);
                
                int match = 0;
                if (case_val->type == NODE_NUMBER && val->type == NODE_NUMBER)
                    match = (case_val->int_val == val->int_val);
                else if (case_val->type == NODE_FLOAT && val->type == NODE_FLOAT)
                    match = (case_val->float_val == val->float_val);
                else if (case_val->type == NODE_STRING && val->type == NODE_STRING)
                    match = (strcmp(case_val->str_val, val->str_val) == 0);
                else if (case_val->type == NODE_BOOL && val->type == NODE_BOOL)
                    match = (case_val->int_val == val->int_val);
                
                if (match) return eval(case_node->body, env);
            }
            if (n->default_case) return eval(n->default_case, env);
            return node_nil();
        }

        // ============ ИМПОРТ ============
        // ИСКАТЬ: case NODE_IMPORT:
        case NODE_IMPORT: {
            char path[512];
            snprintf(path, sizeof(path), "модули/%s.руп", n->str_val);
            FILE* f = fopen(path, "r");
            if (f) {
                fseek(f, 0, SEEK_END);
                long sz = ftell(f);
                rewind(f);
                char* src = safe_alloc(sz + 1);
                fread(src, 1, sz, f);
                src[sz] = 0;
                fclose(f);
                Node* ast = parse(src);
                if (ast) eval(ast, env);
            }
            return node_nil();
        }

        default:
            return node_nil();
    }
}

void eval_program(Node* ast) {
    Env* global = env_new(NULL);
    register_builtins(global);
    eval(ast, global);
}

void print_help() {
    printf("\n");
    printf("🦊 %sRupythan v2.0%s — русский язык программирования\n", BOLD, RESET);
    printf("\n");
    printf("%s📋 ПЕРЕМЕННЫЕ:%s\n", YELLOW, RESET);
    printf("  пусть имя = значение\n");
    printf("\n");
    printf("%s🖨️ ВЫВОД:%s\n", YELLOW, RESET);
    printf("  выведи(что)\n");
    printf("  выведи(зел(\"Успех!\"))\n");
    printf("\n");
    printf("%s⌨️ ВВОД:%s\n", YELLOW, RESET);
    printf("  ввод(\"подсказка: \")\n");
    printf("\n");
    printf("%s🔀 УСЛОВИЯ:%s\n", YELLOW, RESET);
    printf("  если условие\n");
    printf("    выведи(\"да\")\n");
    printf("  иначе\n");
    printf("    выведи(\"нет\")\n");
    printf("\n");
    printf("%s🔄 ЦИКЛЫ:%s\n", YELLOW, RESET);
    printf("  пока условие\n");
    printf("    выведи(счётчик)\n");
    printf("  для элемент из [1, 2, 3]\n");
    printf("    выведи(элемент)\n");
    printf("  стоп  — выйти из цикла\n");
    printf("  дальше — следующий шаг\n");
    printf("\n");
    printf("%s⚙️ ФУНКЦИИ:%s\n", YELLOW, RESET);
    printf("  фн имя(пар1, пар2)\n");
    printf("    верни пар1 + пар2\n");
    printf("  лямбда(х) х * 2\n");
    printf("\n");
    printf("%s🎯 ИСКЛЮЧЕНИЯ:%s\n", YELLOW, RESET);
    printf("  попробуй\n");
    printf("    опасный_код()\n");
    printf("  лови\n");
    printf("    выведи(\"Ошибка!\")\n");
    printf("  бросай(\"сообщение\")\n");
    printf("\n");
    printf("%s🔀 ВЫБОР:%s\n", YELLOW, RESET);
    printf("  выбор значение\n");
    printf("    случай 1\n");
    printf("      выведи(\"один\")\n");
    printf("    случай 2\n");
    printf("      выведи(\"два\")\n");
    printf("\n");
    printf("%s📂 ФАЙЛЫ:%s\n", YELLOW, RESET);
    printf("  прочитай(\"путь\") — читать файл\n");
    printf("  запиши(\"путь\", \"данные\") — записать\n");
    printf("\n");
    printf("%s📊 ФОРМАТЫ:%s\n", YELLOW, RESET);
    printf("  csv_читай(\"файл.csv\")\n");
    printf("  json_получи(json, \"ключ\")\n");
    printf("\n");
    printf("%s🎨 ЦВЕТА:%s\n", YELLOW, RESET);
    printf("  кр() зел() жёл() син() фиол() гол() жир()\n");
    printf("\n");
    printf("%s📦 ИМПОРТ:%s\n", YELLOW, RESET);
    printf("  импорт матем\n");
    printf("  импорт json\n");
    printf("\n");
    printf("Запуск: rupythan файл.руп\n");
    printf("REPL:   rupythan\n");
    printf("Справка: rupythan справка\n");
    printf("\n");
}