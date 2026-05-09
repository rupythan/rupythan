#include "pru_pm.h"
#include <sys/stat.h>
#include <unistd.h>

#define PRU_HOME_DIR "/data/data/com.termux/files/home/.rupythan"
#define PRU_PKG_DIR  "/data/data/com.termux/files/home/.rupythan/packages"
#define PRU_REGISTRY "/data/data/com.termux/files/home/.rupythan/registry.pru"

// ============================================================
//  ИНИЦИАЛИЗАЦИЯ
// ============================================================

void pru_init() {
    mkdir(PRU_HOME_DIR, 0755);
    mkdir(PRU_PKG_DIR, 0755);
    
    // Создаём реестр если нет
    FILE* f = fopen(PRU_REGISTRY, "r");
    if (!f) {
        f = fopen(PRU_REGISTRY, "w");
        fprintf(f, "# Реестр пакетов Rupythan\n\n");
        fclose(f);
    } else {
        fclose(f);
    }
}

// ============================================================
//  УСТАНОВКА ПАКЕТА
// ============================================================

void pru_install(const char* package_name) {
    printf("📦 Установка пакета: %s\n", package_name);
    
    // Проверяем локальный файл
    char local_path[512];
    snprintf(local_path, sizeof(local_path), "%s/%s.руп", ".", package_name);
    
    FILE* f = fopen(local_path, "r");
    if (f) {
        fclose(f);
        printf("   Найден локальный файл: %s\n", local_path);
        pkg_install_local(local_path);
        return;
    }
    
    // Проверяем ./packages/
    snprintf(local_path, sizeof(local_path), "packages/%s.руп", package_name);
    f = fopen(local_path, "r");
    if (f) {
        fclose(f);
        printf("   Найден пакет: %s\n", local_path);
        pkg_install_local(local_path);
        return;
    }
    
    // Скачиваем из интернета (заглушка)
    printf("   🌐 Поиск в репозитории Rupythan...\n");
    
    // Здесь можно добавить system("curl ...") или libcurl
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "curl -s -o %s/%s.руп https://pru.rupythan.org/packages/%s.руп 2>/dev/null",
             PRU_PKG_DIR, package_name, package_name);
    
    int result = system(cmd);
    if (result == 0) {
        printf("   ✅ Пакет '%s' установлен!\n", package_name);
        
        // Добавляем в реестр
        FILE* reg = fopen(PRU_REGISTRY, "a");
        if (reg) {
            fprintf(reg, "%s | v1.0 | Пользовательский пакет | unknown\n", package_name);
            fclose(reg);
        }
    } else {
        printf("   💀 Пакет '%s' не найден в репозитории\n", package_name);
        printf("   Подсказка: создай файл %s.руп в папке packages/\n", package_name);
    }
}

// ============================================================
//  УСТАНОВКА ЛОКАЛЬНОГО ПАКЕТА
// ============================================================

void pkg_install_local(const char* path) {
    char dest[512];
    char* name = strrchr(path, '/');
    if (!name) name = (char*)path;
    else name++;
    
    snprintf(dest, sizeof(dest), "%s/%s", PRU_PKG_DIR, name);
    
    // Копируем файл
    FILE* src = fopen(path, "rb");
    if (!src) {
        printf("   💀 Не могу прочитать %s\n", path);
        return;
    }
    
    FILE* dst = fopen(dest, "wb");
    if (!dst) {
        fclose(src);
        printf("   💀 Не могу записать в %s\n", dest);
        return;
    }
    
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0)
        fwrite(buf, 1, n, dst);
    
    fclose(src); fclose(dst);
    
    printf("   ✅ Пакет '%s' установлен в %s\n", name, PRU_PKG_DIR);
    
    // Добавляем в реестр
    FILE* reg = fopen(PRU_REGISTRY, "a");
    if (reg) {
        fprintf(reg, "%s | v1.0 | Локальный пакет | user\n", name);
        fclose(reg);
    }
}

// ============================================================
//  УДАЛЕНИЕ ПАКЕТА
// ============================================================

void pru_remove(const char* package_name) {
    printf("🗑️  Удаление пакета: %s\n", package_name);
    
    char path[512];
    snprintf(path, sizeof(path), "%s/%s.руп", PRU_PKG_DIR, package_name);
    
    if (remove(path) == 0) {
        printf("   ✅ Пакет '%s' удалён\n", package_name);
    } else {
        // Пробуем без .руп
        snprintf(path, sizeof(path), "%s/%s", PRU_PKG_DIR, package_name);
        if (remove(path) == 0) {
            printf("   ✅ Пакет '%s' удалён\n", package_name);
        } else {
            printf("   💀 Пакет '%s' не найден\n", package_name);
        }
    }
}

// ============================================================
//  СПИСОК ПАКЕТОВ
// ============================================================

void pru_list() {
    printf("📋 Установленные пакеты Rupythan:\n");
    printf("   Домашняя папка: %s\n\n", PRU_PKG_DIR);
    
    DIR* d = opendir(PRU_PKG_DIR);
    if (!d) {
        printf("   (пусто)\n");
        return;
    }
    
    int count = 0;
    struct dirent* dir;
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            continue;
        
        // Определяем тип файла
        char* ext = strrchr(dir->d_name, '.');
        char icon[8] = "📄";
        
        if (ext) {
            if (strcmp(ext, ".руп") == 0 || strcmp(ext, ".pru") == 0) strcpy(icon, "🦊");
            else if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0) strcpy(icon, "⚙️");
            else if (strcmp(ext, ".json") == 0) strcpy(icon, "📊");
            else if (strcmp(ext, ".csv") == 0) strcpy(icon, "📈");
            else if (strcmp(ext, ".txt") == 0) strcpy(icon, "📝");
        }
        
        printf("   %s %s\n", icon, dir->d_name);
        count++;
    }
    closedir(d);
    
    if (count == 0) printf("   (пусто)\n");
    printf("\n   Всего: %d пакетов\n", count);
}

// ============================================================
//  ОБНОВЛЕНИЕ ЯЗЫКА
// ============================================================

void pru_update() {
    printf("🔄 Проверка обновлений Rupythan...\n");
    
    // Проверяем наличие git
    if (system("which git > /dev/null 2>&1") != 0) {
        printf("   💀 Git не установлен. Выполни: pkg install git\n");
        return;
    }
    
    // Если проект в git
    if (access(".git", F_OK) == 0) {
        printf("   📂 Найден git-репозиторий\n");
        system("git pull 2>/dev/null");
        printf("   ✅ Исходный код обновлён!\n");
        printf("   🔨 Пересобери: make && cp rupythan ~/bin/\n");
        return;
    }
    
    // Если нет git — предлагаем скачать
    printf("   🌐 Скачивание последней версии...\n");
    printf("   Выполни вручную:\n");
    printf("   cd ~ && rm -rf rupythan\n");
    printf("   git clone https://github.com/rupythan/rupythan.git\n");
    printf("   cd rupythan && make && cp rupythan ~/bin/\n");
}

// ============================================================
//  ПОИСК ПАКЕТОВ
// ============================================================

void pru_search(const char* query) {
    printf("🔍 Поиск пакетов: '%s'\n", query);
    printf("   Поиск в локальных пакетах...\n");
    
    DIR* d = opendir(PRU_PKG_DIR);
    if (d) {
        struct dirent* dir;
        while ((dir = readdir(d)) != NULL) {
            if (strstr(dir->d_name, query))
                printf("   📄 %s\n", dir->d_name);
        }
        closedir(d);
    }
    
    printf("   🌐 Поиск в репозитории...\n");
    printf("   (интернет-репозиторий в разработке)\n");
}

// ============================================================
//  ИНФОРМАЦИЯ О ПАКЕТЕ
// ============================================================

void pru_info(const char* package_name) {
    printf("📦 Информация о пакете: %s\n", package_name);
    
    char path[512];
    snprintf(path, sizeof(path), "%s/%s.руп", PRU_PKG_DIR, package_name);
    
    struct stat st;
    if (stat(path, &st) == 0) {
        printf("   📏 Размер: %ld байт\n", (long)st.st_size);
        printf("   📂 Путь: %s\n", path);
        
        // Читаем первые строки для описания
        FILE* f = fopen(path, "r");
        if (f) {
            char line[256];
            printf("   📝 Содержимое:\n");
            int i = 0;
            while (fgets(line, sizeof(line), f) && i < 5) {
                printf("     %s", line);
                i++;
            }
            fclose(f);
        }
    } else {
        printf("   💀 Пакет не найден\n");
    }
}