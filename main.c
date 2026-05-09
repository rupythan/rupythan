#include "parser.h"
#include "eval.h"
#include "update.h"
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#ifdef HAS_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#define MODULES_DIR "/data/data/com.termux/files/home/rupythan/модули"

static const char* builtin_modules[] = {
    "json", "csv", "xml", "ini", "yaml",
    "http", "база_данных", "матем", "строки",
    "файлы", "система", "цвета", "время",
    "шифрование", "сеть", "графика", "звук",
    NULL
};

void list_modules() {
    printf("\n📦 Установленные модули Rupythan:\n\n");
    for (int i = 0; builtin_modules[i]; i++) {
        printf("  📚 %s\n", builtin_modules[i]);
    }
    printf("\n");
}

void install_module(const char* name) {
    for (int i = 0; builtin_modules[i]; i++) {
        if (strcmp(builtin_modules[i], name) == 0) {
            printf("✅ Модуль '%s' уже установлен (встроенный)\n", name);
            return;
        }
    }
    
    mkdir(MODULES_DIR, 0755);
    
    char path[512];
    snprintf(path, sizeof(path), "%s/%s.руп", MODULES_DIR, name);
    
    FILE* f = fopen(path, "w");
    if (f) {
        fprintf(f, "# Модуль: %s\n", name);
        fprintf(f, "выведи(\"Модуль %s загружен!\")\n", name);
        fclose(f);
        printf("✅ Модуль '%s' установлен\n", name);
    } else {
        printf("💀 Ошибка установки '%s'\n", name);
    }
}

void remove_module(const char* name) {
    for (int i = 0; builtin_modules[i]; i++) {
        if (strcmp(builtin_modules[i], name) == 0) {
            printf("⚠️ Модуль '%s' — встроенный, удалить нельзя\n", name);
            return;
        }
    }
    
    char path[512];
    snprintf(path, sizeof(path), "%s/%s.руп", MODULES_DIR, name);
    
    if (remove(path) == 0)
        printf("🗑️ Модуль '%s' удалён\n", name);
    else
        printf("💀 Модуль '%s' не найден\n", name);
}

void run_file(const char* filename) {
    if (strcmp(filename, "справка") == 0 || strcmp(filename, "--help") == 0 || strcmp(filename, "-h") == 0) {
        print_help();
        return;
    }
    
    if (strcmp(filename, "список") == 0 || strcmp(filename, "list") == 0) {
        list_modules();
        return;
    }
    
    struct stat st;
    if (stat(filename, &st) != 0) {
        fprintf(stderr, "💀 Ошибка: файл '%s' не найден\n", filename);
        return;
    }
    
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "💀 Ошибка: не могу открыть '%s'\n", filename);
        return;
    }
    
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);
    
    char* src = safe_alloc(size + 1);
    fread(src, 1, size, f);
    src[size] = '\0';
    fclose(f);
    
    Node* ast = parse(src);
    if (ast) eval_program(ast);
    free(src);
}

void run_repl() {
    Env* global = env_new(NULL);
    register_builtins(global);
    
    printf("🦊 Rupythan v2.0 — интерактивный режим\n");
    printf("   Введи 'выход' для выхода\n");
    printf("   Введи 'справка' для подсказки\n");
    printf("   Введи 'список' для списка модулей\n\n");
    
#ifdef HAS_READLINE
    while (1) {
        char* line = readline("руп> ");
        if (!line) break;
        
        if (strcmp(line, "выход") == 0 || strcmp(line, "exit") == 0) {
            free(line);
            break;
        }
        if (strcmp(line, "справка") == 0) { print_help(); free(line); continue; }
        if (strcmp(line, "список") == 0) { list_modules(); free(line); continue; }
        if (strlen(line) == 0) { free(line); continue; }
        
        add_history(line);
        
        Node* ast = parse(line);
        if (ast) {
            Node* result = eval(ast, global);
            if (result && result->type == NODE_NUMBER)
                printf("= %d\n", result->int_val);
            else if (result && result->type == NODE_FLOAT)
                printf("= %g\n", result->float_val);
            else if (result && result->type == NODE_STRING)
                printf("= \"%s\"\n", result->str_val);
        }
        free(line);
    }
#else
    char line[1024];
    while (1) {
        printf("руп> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;
        
        line[strcspn(line, "\n")] = 0;
        
        if (strcmp(line, "выход") == 0 || strcmp(line, "exit") == 0) break;
        if (strcmp(line, "справка") == 0) { print_help(); continue; }
        if (strcmp(line, "список") == 0) { list_modules(); continue; }
        if (strlen(line) == 0) continue;
        
        Node* ast = parse(line);
        if (ast) {
            Node* result = eval(ast, global);
            if (result && result->type == NODE_NUMBER)
                printf("= %d\n", result->int_val);
            else if (result && result->type == NODE_FLOAT)
                printf("= %g\n", result->float_val);
            else if (result && result->type == NODE_STRING)
                printf("= \"%s\"\n", result->str_val);
        }
    }
#endif
    printf("Пока! 🦊\n");
}

int main(int argc, char** argv) {
    mkdir(MODULES_DIR, 0755);
    
    if (argc == 1) {
        run_repl();
    }
    else if (argc == 3 && strcmp(argv[1], "install") == 0) {
        install_module(argv[2]);
    }
    else if (argc == 3 && strcmp(argv[1], "remove") == 0) {
        remove_module(argv[2]);
    }
    else if (argc == 2 && strcmp(argv[1], "list") == 0) {
        list_modules();
    }
    else if (argc == 2 && strcmp(argv[1], "update") == 0) {
        self_update();
    }
    else if (argc == 2) {
        run_file(argv[1]);
    }
    else if (argc == 3 && strcmp(argv[1], "-с") == 0) {
        Node* ast = parse(argv[2]);
        if (ast) eval_program(ast);
    }
    else {
        printf("🦊 Rupythan v2.0\n\n");
        printf("Команды:\n");
        printf("  rupythan              — REPL\n");
        printf("  rupythan файл.руп     — запустить файл\n");
        printf("  rupythan -с \"код\"     — выполнить строку\n");
        printf("  rupythan install имя  — установить модуль\n");
        printf("  rupythan remove имя   — удалить модуль\n");
        printf("  rupythan list         — список модулей\n");
        printf("  rupythan update       — обновить язык\n");
        printf("  rupythan справка      — документация\n");
    }
    return 0;
}
