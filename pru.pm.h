#ifndef PRU_PM_H
#define PRU_PM_H

// Пакетный менеджер PRU
void pru_install(const char* package_name);
void pru_remove(const char* package_name);
void pru_list();
void pru_update();
void pru_init();
void pru_search(const char* query);
void pru_info(const char* package_name);

// Реестр пакетов
typedef struct {
    char* name;
    char* version;
    char* description;
    char* author;
} PkgInfo;

void pkg_registry_init();
PkgInfo* pkg_find(const char* name);
void pkg_install_local(const char* path);

#endif