#include "builtins.h"
#include "parser.h"
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifdef HAS_CURL
#include <curl/curl.h>
#endif

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define BOLD    "\x1b[1m"
#define RESET   "\x1b[0m"

// ============================================================
//  PRU — РОДНОЙ ФОРМАТ RUPYTHAN
// ============================================================

Node* builtin_pru_load(Node** args, int count) {
    if (count < 1) return node_nil();
    char* filename = args[0]->str_val;
    
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "💀 Ошибка: .pru файл '%s' не найден\n", filename);
        return node_nil();
    }
    
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);
    
    char* src = safe_alloc(size + 1);
    fread(src, 1, size, f);
    src[size] = '\0';
    fclose(f);
    
    return node_string(src);
}

Node* builtin_pru_check(Node** args, int count) {
    if (count < 1) return node_bool(0);
    char* filename = args[0]->str_val;
    
    FILE* f = fopen(filename, "rb");
    if (!f) return node_bool(0);
    
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);
    
    char* src = safe_alloc(size + 1);
    fread(src, 1, size, f);
    src[size] = '\0';
    fclose(f);
    
    Node* ast = parse(src);
    free(src);
    
    return ast ? node_bool(1) : node_bool(0);
}

Node* builtin_pru_info(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char* filename = args[0]->str_val;
    
    FILE* f = fopen(filename, "rb");
    if (!f) return node_string(str_dup("Файл не найден"));
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    
    char* src = safe_alloc(size + 1);
    fread(src, 1, size, f);
    src[size] = '\0';
    fclose(f);
    
    int lines = 1, funcs = 0;
    for (int i = 0; src[i]; i++) {
        if (src[i] == '\n') lines++;
    }
    
    char* tmp = str_dup(src);
    char* token = strtok(tmp, " \n\t");
    while (token) {
        if (strcmp(token, "фн") == 0) funcs++;
        token = strtok(NULL, " \n\t");
    }
    free(tmp);
    free(src);
    
    char buf[512];
    snprintf(buf, sizeof(buf), 
        "📄 Файл: %s\n📏 Строк: %d\n⚙️ Функций: %d\n💾 Размер: %ld байт",
        filename, lines, funcs, size);
    return node_string(str_dup(buf));
}

// ============================================================
//  БАЗОВЫЕ ФАЙЛОВЫЕ ОПЕРАЦИИ
// ============================================================

Node* builtin_read_file(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char* filename = args[0]->str_val;
    
    FILE* f = fopen(filename, "rb");
    if (!f) return node_string(str_dup(""));
    
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);
    
    char* buf = safe_alloc(size + 1);
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    
    return node_string(buf);
}

Node* builtin_write_file(Node** args, int count) {
    if (count < 2) return node_bool(0);
    FILE* f = fopen(args[0]->str_val, "wb");
    if (!f) return node_bool(0);
    fprintf(f, "%s", args[1]->str_val);
    fclose(f);
    return node_bool(1);
}

Node* builtin_append_file(Node** args, int count) {
    if (count < 2) return node_bool(0);
    FILE* f = fopen(args[0]->str_val, "ab");
    if (!f) return node_bool(0);
    fprintf(f, "%s", args[1]->str_val);
    fclose(f);
    return node_bool(1);
}

Node* builtin_file_exists(Node** args, int count) {
    if (count < 1) return node_bool(0);
    FILE* f = fopen(args[0]->str_val, "r");
    if (f) { fclose(f); return node_bool(1); }
    return node_bool(0);
}

Node* builtin_file_size(Node** args, int count) {
    if (count < 1) return node_number(0);
    struct stat st;
    if (stat(args[0]->str_val, &st) == 0)
        return node_number((int)st.st_size);
    return node_number(-1);
}

Node* builtin_delete_file(Node** args, int count) {
    if (count < 1) return node_bool(0);
    return remove(args[0]->str_val) == 0 ? node_bool(1) : node_bool(0);
}

Node* builtin_list_dir(Node** args, int count) {
    char* path = count > 0 ? args[0]->str_val : ".";
    struct stat st;
    
    if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
        char buf[512];
        snprintf(buf, sizeof(buf), "📄 %s (%ld байт)", path, (long)st.st_size);
        return node_string(str_dup(buf));
    }
    
    DIR* d = opendir(path);
    if (!d) return node_string(str_dup(""));
    
    char buf[4096] = "";
    struct dirent* dir;
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;
        strcat(buf, dir->d_name);
        strcat(buf, "\n");
    }
    closedir(d);
    return node_string(str_dup(buf));
}

Node* builtin_copy_file(Node** args, int count) {
    if (count < 2) return node_bool(0);
    FILE* src = fopen(args[0]->str_val, "rb");
    if (!src) return node_bool(0);
    FILE* dst = fopen(args[1]->str_val, "wb");
    if (!dst) { fclose(src); return node_bool(0); }
    
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0)
        fwrite(buf, 1, n, dst);
    
    fclose(src); fclose(dst);
    return node_bool(1);
}

// ============================================================
//  ТЕКСТОВЫЕ ФОРМАТЫ
// ============================================================

Node* builtin_read_lines(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char* data = args[0]->str_val;
    char buf[4096] = "";
    char* lines = str_dup(data);
    char* line = strtok(lines, "\n");
    int num = 1;
    
    while (line) {
        char tmp[512];
        snprintf(tmp, sizeof(tmp), "%d: %s\n", num++, line);
        strcat(buf, tmp);
        line = strtok(NULL, "\n");
    }
    free(lines);
    return node_string(str_dup(buf));
}

Node* builtin_count_lines(Node** args, int count) {
    if (count < 1) return node_number(0);
    char* data = args[0]->str_val;
    int lines = 1;
    for (int i = 0; data[i]; i++)
        if (data[i] == '\n') lines++;
    return node_number(lines);
}

Node* builtin_grep(Node** args, int count) {
    if (count < 2) return node_string(str_dup(""));
    char* data = args[0]->str_val;
    char* pattern = args[1]->str_val;
    char buf[4096] = "";
    
    char* lines = str_dup(data);
    char* line = strtok(lines, "\n");
    
    while (line) {
        if (strstr(line, pattern)) {
            strcat(buf, line);
            strcat(buf, "\n");
        }
        line = strtok(NULL, "\n");
    }
    free(lines);
    return node_string(str_dup(buf));
}

// ============================================================
//  CSV
// ============================================================

Node* builtin_csv_parse(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char* data = args[0]->str_val;
    char buf[8192] = "";
    int in_quotes = 0;
    
    for (int i = 0; data[i]; i++) {
        if (data[i] == '"') in_quotes = !in_quotes;
        else if (data[i] == ',' && !in_quotes)
            strcat(buf, " │ ");
        else if (data[i] == '\n' && !in_quotes)
            strcat(buf, " │\n");
        else {
            char tmp[2] = {data[i], 0};
            strcat(buf, tmp);
        }
    }
    
    return node_string(str_dup(buf));
}

Node* builtin_csv_get_cell(Node** args, int count) {
    if (count < 3) return node_string(str_dup(""));
    char* data = args[0]->str_val;
    int target_row = args[1]->int_val;
    int target_col = args[2]->int_val;
    
    int row = 0, col = 0;
    int in_quotes = 0;
    char cell[256] = "";
    int ci = 0;
    
    for (int i = 0; data[i]; i++) {
        if (data[i] == '"') { in_quotes = !in_quotes; continue; }
        
        if (data[i] == ',' && !in_quotes) {
            if (row == target_row && col == target_col) {
                cell[ci] = 0;
                return node_string(str_dup(cell));
            }
            col++;
            ci = 0;
        }
        else if (data[i] == '\n' && !in_quotes) {
            if (row == target_row && col == target_col) {
                cell[ci] = 0;
                return node_string(str_dup(cell));
            }
            row++;
            col = 0;
            ci = 0;
        }
        else {
            if (ci < 255) cell[ci++] = data[i];
        }
    }
    
    return node_string(str_dup(""));
}

Node* builtin_csv_row_count(Node** args, int count) {
    if (count < 1) return node_number(0);
    char* data = args[0]->str_val;
    int rows = 1;
    for (int i = 0; data[i]; i++)
        if (data[i] == '\n') rows++;
    return node_number(rows);
}

// ============================================================
//  JSON
// ============================================================

Node* builtin_json_parse(Node** args, int count) {
    if (count < 1) return node_nil();
    char* json = args[0]->str_val;
    char buf[4096] = "";
    int indent = 0;
    
    for (int i = 0; json[i]; i++) {
        if (json[i] == '{' || json[i] == '[') {
            strcat(buf, "\n");
            for (int j = 0; j < indent; j++) strcat(buf, "  ");
            char tmp[2] = {json[i], 0};
            strcat(buf, tmp);
            strcat(buf, "\n");
            indent++;
        }
        else if (json[i] == '}' || json[i] == ']') {
            indent--;
            strcat(buf, "\n");
            for (int j = 0; j < indent; j++) strcat(buf, "  ");
            char tmp[2] = {json[i], 0};
            strcat(buf, tmp);
        }
        else if (json[i] == ',') {
            strcat(buf, ",\n");
            for (int j = 0; j < indent; j++) strcat(buf, "  ");
        }
        else if (json[i] == ':') {
            strcat(buf, " : ");
        }
        else if (json[i] != ' ' && json[i] != '\n' && json[i] != '\t') {
            char tmp[2] = {json[i], 0};
            strcat(buf, tmp);
        }
    }
    
    return node_string(str_dup(buf));
}

Node* builtin_json_get(Node** args, int count) {
    if (count < 2) return node_string(str_dup(""));
    char* json = args[0]->str_val;
    char* key = args[1]->str_val;
    
    char search[300];
    snprintf(search, sizeof(search), "\"%s\"", key);
    
    char* pos = strstr(json, search);
    if (!pos) return node_string(str_dup(""));
    
    pos = strchr(pos + strlen(search), ':');
    if (!pos) return node_string(str_dup(""));
    pos++;
    
    while (*pos == ' ' || *pos == '\t') pos++;
    
    char value[512] = "";
    int vi = 0;
    int in_quotes = 0;
    
    if (*pos == '"') { pos++; in_quotes = 1; }
    
    while (*pos && vi < 510) {
        if (in_quotes && *pos == '"') break;
        if (!in_quotes && (*pos == ',' || *pos == '}' || *pos == ']')) break;
        value[vi++] = *pos++;
    }
    value[vi] = 0;
    
    return node_string(str_dup(value));
}

Node* builtin_json_keys(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char* json = args[0]->str_val;
    char buf[2048] = "";
    
    for (int i = 0; json[i]; i++) {
        if (json[i] == '"') {
            i++;
            int start = i;
            while (json[i] && json[i] != '"') i++;
            int len = i - start;
            if (len > 0 && len < 100) {
                int j = i + 1;
                while (json[j] == ' ' || json[j] == '\t') j++;
                if (json[j] == ':') {
                    char key[128];
                    strncpy(key, json + start, len);
                    key[len] = 0;
                    strcat(buf, key);
                    strcat(buf, "\n");
                }
            }
        }
    }
    
    return node_string(str_dup(buf));
}

// ============================================================
//  XML
// ============================================================

Node* builtin_xml_parse(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char* xml = args[0]->str_val;
    char buf[4096] = "";
    int indent = 0;
    int in_tag = 0;
    
    for (int i = 0; xml[i]; i++) {
        if (xml[i] == '<') {
            in_tag = 1;
            if (xml[i+1] == '/') indent--;
            strcat(buf, "\n");
            for (int j = 0; j < indent; j++) strcat(buf, "  ");
        }
        
        char tmp[2] = {xml[i], 0};
        strcat(buf, tmp);
        
        if (xml[i] == '>' && in_tag) {
            in_tag = 0;
            if (xml[i-1] != '/') indent++;
        }
    }
    
    return node_string(str_dup(buf));
}

Node* builtin_xml_get_tag(Node** args, int count) {
    if (count < 2) return node_string(str_dup(""));
    char* xml = args[0]->str_val;
    char* tag = args[1]->str_val;
    
    char open_tag[128], close_tag[128];
    snprintf(open_tag, sizeof(open_tag), "<%s>", tag);
    snprintf(close_tag, sizeof(close_tag), "</%s>", tag);
    
    char* start = strstr(xml, open_tag);
    if (!start) {
        snprintf(open_tag, sizeof(open_tag), "<%s ", tag);
        start = strstr(xml, open_tag);
    }
    if (!start) return node_string(str_dup(""));
    
    start = strchr(start, '>');
    if (!start) return node_string(str_dup(""));
    start++;
    
    char* end = strstr(start, close_tag);
    if (!end) return node_string(str_dup(""));
    
    int len = end - start;
    char* content = safe_alloc(len + 1);
    strncpy(content, start, len);
    content[len] = 0;
    
    return node_string(content);
}

// ============================================================
//  INI
// ============================================================

Node* builtin_ini_parse(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char* ini = args[0]->str_val;
    char buf[4096] = "";
    char section[128] = "";
    
    char* lines = str_dup(ini);
    char* line = strtok(lines, "\n");
    
    while (line) {
        if (strlen(line) == 0 || line[0] == ';' || line[0] == '#') {
            line = strtok(NULL, "\n");
            continue;
        }
        
        if (line[0] == '[') {
            int len = strlen(line);
            strncpy(section, line + 1, len - 2);
            section[len - 2] = 0;
            char tmp[256];
            snprintf(tmp, sizeof(tmp), "\n📁 [%s]\n", section);
            strcat(buf, tmp);
        }
        else {
            char* eq = strchr(line, '=');
            if (eq) {
                *eq = 0;
                char tmp[512];
                snprintf(tmp, sizeof(tmp), "  🔑 %s = %s\n", line, eq + 1);
                strcat(buf, tmp);
            }
        }
        
        line = strtok(NULL, "\n");
    }
    
    return node_string(str_dup(buf));
}

Node* builtin_ini_get(Node** args, int count) {
    if (count < 3) return node_string(str_dup(""));
    char* ini = args[0]->str_val;
    char* section = args[1]->str_val;
    char* key = args[2]->str_val;
    
    char search[128];
    snprintf(search, sizeof(search), "[%s]", section);
    
    char* sec = strstr(ini, search);
    if (!sec) return node_string(str_dup(""));
    
    char* line_start = strchr(sec, '\n');
    if (!line_start) return node_string(str_dup(""));
    
    while (line_start) {
        line_start++;
        char* line_end = strchr(line_start, '\n');
        if (!line_end) line_end = line_start + strlen(line_start);
        
        if (*line_start == '[') break;
        
        char line[256];
        int len = line_end - line_start;
        if (len > 255) len = 255;
        strncpy(line, line_start, len);
        line[len] = 0;
        
        char* eq = strchr(line, '=');
        if (eq) {
            *eq = 0;
            while (*line == ' ') line++;
            char* val = eq + 1;
            while (*val == ' ') val++;
            
            if (strcmp(line, key) == 0)
                return node_string(str_dup(val));
        }
        
        line_start = line_end;
        if (!*line_start) break;
    }
    
    return node_string(str_dup(""));
}

// ============================================================
//  YAML
// ============================================================

Node* builtin_yaml_parse(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char* yaml = args[0]->str_val;
    char buf[4096] = "";
    
    char* lines = str_dup(yaml);
    char* line = strtok(lines, "\n");
    
    while (line) {
        int indent = 0;
        while (line[indent] == ' ') indent++;
        
        for (int j = 0; j < indent; j++) strcat(buf, " ");
        
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = 0;
            char tmp[512];
            snprintf(tmp, sizeof(tmp), "🔑 %s → %s\n", line, colon + 2);
            strcat(buf, tmp);
        }
        else if (line[indent] == '-') {
            char tmp[512];
            snprintf(tmp, sizeof(tmp), "  📌 %s\n", line + indent + 2);
            strcat(buf, tmp);
        }
        
        line = strtok(NULL, "\n");
    }
    
    return node_string(str_dup(buf));
}

// ============================================================
//  БИНАРНЫЕ ФОРМАТЫ
// ============================================================

Node* builtin_read_bytes(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char* filename = args[0]->str_val;
    
    FILE* f = fopen(filename, "rb");
    if (!f) return node_string(str_dup(""));
    
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);
    
    char* buf = safe_alloc(size * 3 + 1);
    unsigned char byte;
    int pos = 0;
    
    for (size_t i = 0; i < size && i < 1024; i++) {
        fread(&byte, 1, 1, f);
        pos += snprintf(buf + pos, 4, "%02X ", byte);
        if ((i + 1) % 16 == 0) buf[pos++] = '\n';
    }
    buf[pos] = 0;
    fclose(f);
    
    return node_string(buf);
}

Node* builtin_write_bytes(Node** args, int count) {
    return builtin_write_file(args, count);
}

Node* builtin_get_metadata(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char* filename = args[0]->str_val;
    struct stat st;
    
    if (stat(filename, &st) != 0)
        return node_string(str_dup("Файл не найден"));
    
    char buf[512];
    snprintf(buf, sizeof(buf),
        "📄 Файл: %s\n📏 Размер: %ld байт\n🕐 Изменён: %s",
        filename, (long)st.st_size, ctime(&st.st_mtime));
    
    return node_string(str_dup(buf));
}

Node* builtin_is_format(Node** args, int count) {
    if (count < 2) return node_bool(0);
    char* filename = args[0]->str_val;
    char* ext = args[1]->str_val;
    
    char* dot = strrchr(filename, '.');
    if (!dot) return node_bool(0);
    
    return node_bool(strcmp(dot + 1, ext) == 0);
}

// ============================================================
//  СТРОКИ
// ============================================================

Node* builtin_length(Node** args, int count) {
    if (count < 1) return node_number(0);
    if (args[0]->type == NODE_STRING)
        return node_number(strlen(args[0]->str_val));
    return node_number(0);
}

Node* builtin_upper(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char* s = str_dup(args[0]->str_val);
    for (char* p = s; *p; p++)
        if (*p >= 'a' && *p <= 'z') *p -= 32;
    return node_string(s);
}

Node* builtin_lower(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char* s = str_dup(args[0]->str_val);
    for (char* p = s; *p; p++)
        if (*p >= 'A' && *p <= 'Z') *p += 32;
    return node_string(s);
}

Node* builtin_trim(Node** args, int count) {
    if (count < 1) return node_string(str_dup(""));
    char* s = args[0]->str_val;
    while (*s == ' ' || *s == '\t' || *s == '\n') s++;
    char* end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\n')) end--;
    *(end + 1) = 0;
    return node_string(str_dup(s));
}

Node* builtin_split(Node** args, int count) {
    if (count < 1) return node_list(NULL, 0);
    return node_string(str_dup(args[0]->str_val));
}

Node* builtin_join(Node** args, int count) {
    if (count < 2) return node_string(str_dup(""));
    return node_string(str_dup(args[0]->str_val));
}

Node* builtin_replace(Node** args, int count) {
    if (count < 3) return node_string(str_dup(""));
    char* s = str_dup(args[0]->str_val);
    char* from = args[1]->str_val;
    char* to = args[2]->str_val;
    
    char buf[4096] = "";
    char* pos = s;
    char* found;
    
    while ((found = strstr(pos, from)) != NULL) {
        strncat(buf, pos, found - pos);
        strcat(buf, to);
        pos = found + strlen(from);
    }
    strcat(buf, pos);
    free(s);
    
    return node_string(str_dup(buf));
}

// ============================================================
//  ЦВЕТА
// ============================================================

Node* builtin_red(Node** args, int count) {
    if (count < 1) return node_string(str_dup(RED));
    char buf[2048];
    snprintf(buf, sizeof(buf), "%s%s%s", RED, args[0]->str_val, RESET);
    return node_string(str_dup(buf));
}

Node* builtin_green(Node** args, int count) {
    if (count < 1) return node_string(str_dup(GREEN));
    char buf[2048];
    snprintf(buf, sizeof(buf), "%s%s%s", GREEN, args[0]->str_val, RESET);
    return node_string(str_dup(buf));
}

Node* builtin_yellow(Node** args, int count) {
    if (count < 1) return node_string(str_dup(YELLOW));
    char buf[2048];
    snprintf(buf, sizeof(buf), "%s%s%s", YELLOW, args[0]->str_val, RESET);
    return node_string(str_dup(buf));
}

Node* builtin_blue(Node** args, int count) {
    if (count < 1) return node_string(str_dup(BLUE));
    char buf[2048];
    snprintf(buf, sizeof(buf), "%s%s%s", BLUE, args[0]->str_val, RESET);
    return node_string(str_dup(buf));
}

Node* builtin_magenta(Node** args, int count) {
    if (count < 1) return node_string(str_dup(MAGENTA));
    char buf[2048];
    snprintf(buf, sizeof(buf), "%s%s%s", MAGENTA, args[0]->str_val, RESET);
    return node_string(str_dup(buf));
}

Node* builtin_cyan(Node** args, int count) {
    if (count < 1) return node_string(str_dup(CYAN));
    char buf[2048];
    snprintf(buf, sizeof(buf), "%s%s%s", CYAN, args[0]->str_val, RESET);
    return node_string(str_dup(buf));
}

Node* builtin_bold(Node** args, int count) {
    if (count < 1) return node_string(str_dup(BOLD));
    char buf[2048];
    snprintf(buf, sizeof(buf), "%s%s%s", BOLD, args[0]->str_val, RESET);
    return node_string(str_dup(buf));
}

// ============================================================
//  СИСТЕМНЫЕ
// ============================================================

Node* builtin_type_of(Node** args, int count) {
    if (count < 1) return node_string(str_dup("ничто"));
    switch (args[0]->type) {
        case NODE_NUMBER: return node_string(str_dup("число"));
        case NODE_STRING: return node_string(str_dup("строка"));
        case NODE_BOOL:   return node_string(str_dup("логика"));
        case NODE_NIL:    return node_string(str_dup("ничто"));
        case NODE_LIST:   return node_string(str_dup("список"));
        default:          return node_string(str_dup("объект"));
    }
}

Node* builtin_input(Node** args, int count) {
    if (count > 0) printf("%s", args[0]->str_val);
    fflush(stdout);
    char buf[1024];
    if (fgets(buf, sizeof(buf), stdin)) {
        buf[strcspn(buf, "\n")] = 0;
        return node_string(str_dup(buf));
    }
    return node_string(str_dup(""));
}

Node* builtin_int_cast(Node** args, int count) {
    if (count < 1) return node_number(0);
    if (args[0]->type == NODE_NUMBER) return args[0];
    if (args[0]->type == NODE_STRING) return node_number(atoi(args[0]->str_val));
    return node_number(0);
}

Node* builtin_random(Node** args, int count) {
    if (count < 2) return node_number(0);
    int min = args[0]->int_val;
    int max = args[1]->int_val;
    return node_number(min + rand() % (max - min + 1));
}

Node* builtin_time_now(Node** args, int count) {
    return node_number((int)time(NULL));
}

Node* builtin_sleep_ms(Node** args, int count) {
    if (count < 1) return node_nil();
    int ms = args[0]->int_val;
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
    return node_nil();
}

// ============================================================
//  TELEGRAM API
// ============================================================

#ifdef HAS_CURL

static char tg_token[256] = "";
static char tg_base_url[512] = "";

static size_t tg_write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
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
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, tg_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    return response;
}

Node* builtin_tg_init(Node** args, int count) {
    if (count < 1) return node_bool(0);
    strncpy(tg_token, args[0]->str_val, 255);
    snprintf(tg_base_url, sizeof(tg_base_url), "https://api.telegram.org/bot%s", tg_token);
    return node_bool(1);
}

Node* builtin_tg_send(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[2048];
    snprintf(params, sizeof(params), "chat_id=%s&text=%s", args[0]->str_val, args[1]->str_val);
    char* response = tg_request("sendMessage", params);
    return node_string(response);
}

Node* builtin_tg_reply(Node** args, int count) {
    if (count < 3) return node_string(str_dup("{}"));
    char params[2048];
    snprintf(params, sizeof(params), "chat_id=%s&text=%s&reply_to_message_id=%s",
             args[0]->str_val, args[1]->str_val, args[2]->str_val);
    char* response = tg_request("sendMessage", params);
    return node_string(response);
}

Node* builtin_tg_edit(Node** args, int count) {
    if (count < 3) return node_string(str_dup("{}"));
    char params[2048];
    snprintf(params, sizeof(params), "chat_id=%s&message_id=%s&text=%s",
             args[0]->str_val, args[1]->str_val, args[2]->str_val);
    char* response = tg_request("editMessageText", params);
    return node_string(response);
}

Node* builtin_tg_delete(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[512];
    snprintf(params, sizeof(params), "chat_id=%s&message_id=%s", args[0]->str_val, args[1]->str_val);
    char* response = tg_request("deleteMessage", params);
    return node_string(response);
}

Node* builtin_tg_forward(Node** args, int count) {
    if (count < 3) return node_string(str_dup("{}"));
    char params[1024];
    snprintf(params, sizeof(params), "chat_id=%s&from_chat_id=%s&message_id=%s",
             args[0]->str_val, args[1]->str_val, args[2]->str_val);
    char* response = tg_request("forwardMessage", params);
    return node_string(response);
}

Node* builtin_tg_copy(Node** args, int count) {
    if (count < 3) return node_string(str_dup("{}"));
    char params[1024];
    snprintf(params, sizeof(params), "chat_id=%s&from_chat_id=%s&message_id=%s",
             args[0]->str_val, args[1]->str_val, args[2]->str_val);
    char* response = tg_request("copyMessage", params);
    return node_string(response);
}

Node* builtin_tg_pin(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[512];
    snprintf(params, sizeof(params), "chat_id=%s&message_id=%s", args[0]->str_val, args[1]->str_val);
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

Node* builtin_tg_photo(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[1024];
    snprintf(params, sizeof(params), "chat_id=%s&photo=%s", args[0]->str_val, args[1]->str_val);
    if (count >= 3) { strcat(params, "&caption="); strcat(params, args[2]->str_val); }
    char* response = tg_request("sendPhoto", params);
    return node_string(response);
}

Node* builtin_tg_video(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[1024];
    snprintf(params, sizeof(params), "chat_id=%s&video=%s", args[0]->str_val, args[1]->str_val);
    if (count >= 3) { strcat(params, "&caption="); strcat(params, args[2]->str_val); }
    char* response = tg_request("sendVideo", params);
    return node_string(response);
}

Node* builtin_tg_audio(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[1024];
    snprintf(params, sizeof(params), "chat_id=%s&audio=%s", args[0]->str_val, args[1]->str_val);
    if (count >= 3) { strcat(params, "&title="); strcat(params, args[2]->str_val); }
    char* response = tg_request("sendAudio", params);
    return node_string(response);
}

Node* builtin_tg_voice(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[1024];
    snprintf(params, sizeof(params), "chat_id=%s&voice=%s", args[0]->str_val, args[1]->str_val);
    char* response = tg_request("sendVoice", params);
    return node_string(response);
}

Node* builtin_tg_document(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[1024];
    snprintf(params, sizeof(params), "chat_id=%s&document=%s", args[0]->str_val, args[1]->str_val);
    if (count >= 3) { strcat(params, "&caption="); strcat(params, args[2]->str_val); }
    char* response = tg_request("sendDocument", params);
    return node_string(response);
}

Node* builtin_tg_sticker(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[512];
    snprintf(params, sizeof(params), "chat_id=%s&sticker=%s", args[0]->str_val, args[1]->str_val);
    char* response = tg_request("sendSticker", params);
    return node_string(response);
}

Node* builtin_tg_animation(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[1024];
    snprintf(params, sizeof(params), "chat_id=%s&animation=%s", args[0]->str_val, args[1]->str_val);
    char* response = tg_request("sendAnimation", params);
    return node_string(response);
}

Node* builtin_tg_video_note(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[512];
    snprintf(params, sizeof(params), "chat_id=%s&video_note=%s", args[0]->str_val, args[1]->str_val);
    char* response = tg_request("sendVideoNote", params);
    return node_string(response);
}

Node* builtin_tg_media_group(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[2048];
    snprintf(params, sizeof(params), "chat_id=%s&media=%s", args[0]->str_val, args[1]->str_val);
    char* response = tg_request("sendMediaGroup", params);
    return node_string(response);
}

Node* builtin_tg_button(Node** args, int count) {
    if (count < 1) return node_string(str_dup("{}"));
    char params[512];
    snprintf(params, sizeof(params), "{\"text\":\"%s\"}", args[0]->str_val);
    return node_string(str_dup(params));
}

Node* builtin_tg_inline_button(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[512];
    snprintf(params, sizeof(params), "{\"text\":\"%s\",\"callback_data\":\"%s\"}", args[0]->str_val, args[1]->str_val);
    return node_string(str_dup(params));
}

Node* builtin_tg_url_button(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[1024];
    snprintf(params, sizeof(params), "{\"text\":\"%s\",\"url\":\"%s\"}", args[0]->str_val, args[1]->str_val);
    return node_string(str_dup(params));
}

Node* builtin_tg_callback_button(Node** args, int count) {
    return builtin_tg_inline_button(args, count);
}

Node* builtin_tg_poll(Node** args, int count) {
    if (count < 3) return node_string(str_dup("{}"));
    char params[2048];
    snprintf(params, sizeof(params), "chat_id=%s&question=%s&options=%s",
             args[0]->str_val, args[1]->str_val, args[2]->str_val);
    char* response = tg_request("sendPoll", params);
    return node_string(response);
}

Node* builtin_tg_quiz(Node** args, int count) {
    if (count < 4) return node_string(str_dup("{}"));
    char params[2048];
    snprintf(params, sizeof(params),
             "chat_id=%s&question=%s&options=%s&correct_option_id=%s&type=quiz",
             args[0]->str_val, args[1]->str_val, args[2]->str_val, args[3]->str_val);
    char* response = tg_request("sendPoll", params);
    return node_string(response);
}

Node* builtin_tg_stop_poll(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[512];
    snprintf(params, sizeof(params), "chat_id=%s&message_id=%s", args[0]->str_val, args[1]->str_val);
    char* response = tg_request("stopPoll", params);
    return node_string(response);
}

Node* builtin_tg_location(Node** args, int count) {
    if (count < 3) return node_string(str_dup("{}"));
    char params[512];
    snprintf(params, sizeof(params), "chat_id=%s&latitude=%s&longitude=%s",
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
    snprintf(params, sizeof(params), "chat_id=%s&phone_number=%s&first_name=%s",
             args[0]->str_val, args[1]->str_val, args[2]->str_val);
    char* response = tg_request("sendContact", params);
    return node_string(response);
}

Node* builtin_tg_dice(Node** args, int count) {
    if (count < 1) return node_string(str_dup("{}"));
    char params[256];
    snprintf(params, sizeof(params), "chat_id=%s", args[0]->str_val);
    if (count >= 2) { strcat(params, "&emoji="); strcat(params, args[1]->str_val); }
    char* response = tg_request("sendDice", params);
    return node_string(response);
}

Node* builtin_tg_kick(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[512];
    snprintf(params, sizeof(params), "chat_id=%s&user_id=%s", args[0]->str_val, args[1]->str_val);
    char* response = tg_request("kickChatMember", params);
    return node_string(response);
}

Node* builtin_tg_ban(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[512];
    snprintf(params, sizeof(params), "chat_id=%s&user_id=%s", args[0]->str_val, args[1]->str_val);
    char* response = tg_request("banChatMember", params);
    return node_string(response);
}

Node* builtin_tg_unban(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[512];
    snprintf(params, sizeof(params), "chat_id=%s&user_id=%s&only_if_banned=true",
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
    snprintf(params, sizeof(params), "chat_id=%s&user_id=%s&permissions=%s",
             args[0]->str_val, args[1]->str_val, args[2]->str_val);
    char* response = tg_request("restrictChatMember", params);
    return node_string(response);
}

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

Node* builtin_tg_get_chat_member(Node** args, int count) {
    if (count < 2) return node_string(str_dup("{}"));
    char params[512];
    snprintf(params, sizeof(params), "chat_id=%s&user_id=%s", args[0]->str_val, args[1]->str_val);
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

Node* builtin_tg_get_updates(Node** args, int count) {
    char params[256] = "";
    if (count >= 1) snprintf(params, sizeof(params), "offset=%s", args[0]->str_val);
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
    snprintf(url, sizeof(url), "https://api.telegram.org/file/bot%s/%s", tg_token, args[0]->str_val);
    CURL* curl = curl_easy_init();
    FILE* fp = fopen(args[1]->str_val, "wb");
    if (!fp) { curl_easy_cleanup(curl); return node_bool(0); }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);
    return node_bool(1);
}

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

#endif // HAS_CURL

// ============================================================
//  РЕГИСТРАЦИЯ ВСЕХ BUILTINS
// ============================================================

void register_builtins(Env* env) {
    srand(time(NULL));
    
    // PRU
    env_set(env, "пру_загрузи",     node_fn_def("пру_загрузи", NULL, 0, NULL));
    env_set(env, "пру_проверь",     node_fn_def("пру_проверь", NULL, 0, NULL));
    env_set(env, "пру_инфо",        node_fn_def("пру_инфо", NULL, 0, NULL));
    
    // Файлы
    env_set(env, "прочитай",        node_fn_def("прочитай", NULL, 0, NULL));
    env_set(env, "запиши",          node_fn_def("запиши", NULL, 0, NULL));
    env_set(env, "добавь",          node_fn_def("добавь", NULL, 0, NULL));
    env_set(env, "файл_есть",       node_fn_def("файл_есть", NULL, 0, NULL));
    env_set(env, "размер_файла",    node_fn_def("размер_файла", NULL, 0, NULL));
    env_set(env, "удали_файл",      node_fn_def("удали_файл", NULL, 0, NULL));
    env_set(env, "список_папки",    node_fn_def("список_папки", NULL, 0, NULL));
    env_set(env, "копируй_файл",    node_fn_def("копируй_файл", NULL, 0, NULL));
    
    // Текстовые форматы
    env_set(env, "строки_файла",    node_fn_def("строки_файла", NULL, 0, NULL));
    env_set(env, "счёт_строк",      node_fn_def("счёт_строк", NULL, 0, NULL));
    env_set(env, "поиск",           node_fn_def("поиск", NULL, 0, NULL));
    
    // CSV
    env_set(env, "csv_читай",       node_fn_def("csv_читай", NULL, 0, NULL));
    env_set(env, "csv_получи",      node_fn_def("csv_получи", NULL, 0, NULL));
    env_set(env, "csv_счёт_рядов",  node_fn_def("csv_счёт_рядов", NULL, 0, NULL));
    
    // JSON
    env_set(env, "json_ключи_внутр", node_fn_def("json_ключи_внутр", NULL, 0, NULL));
    env_set(env, "json_получи",     node_fn_def("json_получи", NULL, 0, NULL));
    
    // XML
    env_set(env, "xml_парс",        node_fn_def("xml_парс", NULL, 0, NULL));
    env_set(env, "xml_тег",         node_fn_def("xml_тег", NULL, 0, NULL));
    
    // INI
    env_set(env, "ini_читай",       node_fn_def("ini_читай", NULL, 0, NULL));
    env_set(env, "ini_значение",    node_fn_def("ini_значение", NULL, 0, NULL));
    
    // YAML
    env_set(env, "yaml_парс",       node_fn_def("yaml_парс", NULL, 0, NULL));
    
    // Бинарные
    env_set(env, "байты",           node_fn_def("байты", NULL, 0, NULL));
    env_set(env, "метаданные",      node_fn_def("метаданные", NULL, 0, NULL));
    env_set(env, "это_формат",      node_fn_def("это_формат", NULL, 0, NULL));
    
    // Строки
    env_set(env, "длина",           node_fn_def("длина", NULL, 0, NULL));
    env_set(env, "верх_регистр",    node_fn_def("верх_регистр", NULL, 0, NULL));
    env_set(env, "низ_регистр",     node_fn_def("низ_регистр", NULL, 0, NULL));
    env_set(env, "обрежь",          node_fn_def("обрежь", NULL, 0, NULL));
    env_set(env, "заменить",        node_fn_def("заменить", NULL, 0, NULL));
    
    // Цвета
    env_set(env, "кр",              node_fn_def("кр", NULL, 0, NULL));
    env_set(env, "зел",             node_fn_def("зел", NULL, 0, NULL));
    env_set(env, "жёл",             node_fn_def("жёл", NULL, 0, NULL));
    env_set(env, "син",             node_fn_def("син", NULL, 0, NULL));
    env_set(env, "фиол",            node_fn_def("фиол", NULL, 0, NULL));
    env_set(env, "гол",             node_fn_def("гол", NULL, 0, NULL));
    env_set(env, "жир",             node_fn_def("жир", NULL, 0, NULL));
    
    // Система
    env_set(env, "тип_данных",      node_fn_def("тип_данных", NULL, 0, NULL));
    env_set(env, "ввод",            node_fn_def("ввод", NULL, 0, NULL));
    env_set(env, "в_число",         node_fn_def("в_число", NULL, 0, NULL));
    env_set(env, "рандом",          node_fn_def("рандом", NULL, 0, NULL));
    env_set(env, "время_сейчас",    node_fn_def("время_сейчас", NULL, 0, NULL));
    env_set(env, "сон_мс",          node_fn_def("сон_мс", NULL, 0, NULL));
    
#ifdef HAS_CURL
    // телеapi
    env_set(env, "телеapi_инит",    node_fn_def("телеapi_инит", NULL, 0, NULL));
    env_set(env, "телеapi_отправь", node_fn_def("телеapi_отправь", NULL, 0, NULL));
    env_set(env, "телеapi_ответь",  node_fn_def("телеapi_ответь", NULL, 0, NULL));
    env_set(env, "телеapi_редактируй", node_fn_def("телеapi_редактируй", NULL, 0, NULL));
    env_set(env, "телеapi_удали",   node_fn_def("телеapi_удали", NULL, 0, NULL));
    env_set(env, "телеapi_перешли", node_fn_def("телеapi_перешли", NULL, 0, NULL));
    env_set(env, "телеapi_копируй", node_fn_def("телеapi_копируй", NULL, 0, NULL));
    env_set(env, "телеapi_закрепи", node_fn_def("телеapi_закрепи", NULL, 0, NULL));
    env_set(env, "телеapi_открепи", node_fn_def("телеapi_открепи", NULL, 0, NULL));
    env_set(env, "телеapi_фото",    node_fn_def("телеapi_фото", NULL, 0, NULL));
    env_set(env, "телеapi_видео",   node_fn_def("телеapi_видео", NULL, 0, NULL));
    env_set(env, "телеapi_аудио",   node_fn_def("телеapi_аудио", NULL, 0, NULL));
    env_set(env, "телеapi_голос",   node_fn_def("телеapi_голос", NULL, 0, NULL));
    env_set(env, "телеapi_документ", node_fn_def("телеapi_документ", NULL, 0, NULL));
    env_set(env, "телеapi_стикер",  node_fn_def("телеapi_стикер", NULL, 0, NULL));
    env_set(env, "телеapi_клавиша", node_fn_def("телеapi_клавиша", NULL, 0, NULL));
    env_set(env, "телеapi_инлайн_клавиша", node_fn_def("телеapi_инлайн_клавиша", NULL, 0, NULL));
    env_set(env, "телеapi_урл_клавиша", node_fn_def("телеapi_урл_клавиша", NULL, 0, NULL));
    env_set(env, "телеapi_опрос",   node_fn_def("телеapi_опрос", NULL, 0, NULL));
    env_set(env, "телеapi_викторина", node_fn_def("телеapi_викторина", NULL, 0, NULL));
    env_set(env, "телеapi_локация", node_fn_def("телеapi_локация", NULL, 0, NULL));
    env_set(env, "телеapi_контакт", node_fn_def("телеapi_контакт", NULL, 0, NULL));
    env_set(env, "телеapi_дайс",    node_fn_def("телеapi_дайс", NULL, 0, NULL));
    env_set(env, "телеapi_кик",     node_fn_def("телеapi_кик", NULL, 0, NULL));
    env_set(env, "телеapi_бан",     node_fn_def("телеapi_бан", NULL, 0, NULL));
    env_set(env, "телеapi_разбан",  node_fn_def("телеapi_разбан", NULL, 0, NULL));
    env_set(env, "телеapi_мут",     node_fn_def("телеapi_мут", NULL, 0, NULL));
    env_set(env, "телеapi_размут",  node_fn_def("телеapi_размут", NULL, 0, NULL));
    env_set(env, "телеapi_админ",   node_fn_def("телеapi_админ", NULL, 0, NULL));
    env_set(env, "телеapi_инфо",    node_fn_def("телеapi_инфо", NULL, 0, NULL));
    env_set(env, "телеapi_жирный",  node_fn_def("телеapi_жирный", NULL, 0, NULL));
    env_set(env, "телеapi_курсив",  node_fn_def("телеapi_курсив", NULL, 0, NULL));
    env_set(env, "телеapi_код",     node_fn_def("телеapi_код", NULL, 0, NULL));
    env_set(env, "телеapi_ссылка",  node_fn_def("телеapi_ссылка", NULL, 0, NULL));
    env_set(env, "телеapi_спойлер", node_fn_def("телеapi_спойлер", NULL, 0, NULL));
    env_set(env, "телеapi_цитата",  node_fn_def("телеapi_цитата", NULL, 0, NULL));
#endif
}