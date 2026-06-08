// db_controls.h
#ifndef DB_CONTROLS_H
#define DB_CONTROLS_H

#if defined(TARGET_PLATFORM_ESP32)
#include "LittleFS.h"
// ESP32-də fopen funksiyası üçün rəsmi mount nöqtəsi mütləq əlavə edilməlidir
#define MASTER_DIR "/littlefs/sqlBinDB"
#define MASTER_FILE "/littlefs/sqlBinDB/master_dbs.db"

// Qovluq yaradılarkən LittleFS daxili sistemi üçün təmiz yol makrosu
#define LFS_DIR "/sqlBinDB"
#define LFS_FILE "/sqlBinDB/master_dbs.db"
#else
#include <sys/stat.h>
#include <sys/types.h>
#ifdef TARGET_PLATFORM_WINDOWS
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <unistd.h>
#endif
#define MASTER_DIR "sqlBinDB"
#define MASTER_FILE "sqlBinDB/master_dbs.db"
#endif

// FUNKSİYA PROTOTİPLƏRİ
bool helperCheckDbExists(const char *DbName, char *outPsw, long *outOffset);
void initSystem();
bool createDb(const char *DbName, const char *DbPsw);
bool dropDb(char *DbName, char *DbPsw);
bool connectDb(const char *DbName, const char *DbPsw);
bool disConnectDb();
void selectDb(const char *DbName);
bool executeDropSystem(const char *path);

void initSystem()
{

#if defined(TARGET_PLATFORM_ESP32)
    Serial.println("\n--- [SqlBinDB] Sistem Başladılır ---");
    Serial.println("Aktiv Platforma: ESP32 (LittleFS)");

    if (!LittleFS.begin(true))
    {
        Serial.println("XƏTA: LittleFS mount edilə bilmədi!");
        return;
    }

    // LittleFS obyekti daxili idarəetmədə '/littlefs' prefiksini özü idarə edir
    if (!LittleFS.exists(LFS_DIR))
    {
        if (LittleFS.mkdir(LFS_DIR))
        {
            Serial.println("Mərkəzi qovluq yaradıldı: " LFS_DIR);
        }
        else
        {
            Serial.println("XƏTA: Qovluq yaradıla bilmədi!");
        }
    }

    // Master binar qeydiyyat faylı yoxdursa, sıfırdan binar rejimdə yaradaq
    FILE *fCheck = fopen(MASTER_FILE, "rb");
    if (!fCheck)
    {
        FILE *fCreate = fopen(MASTER_FILE, "wb");
        if (fCreate)
        {
            Serial.println("Master verilənlər bazası faylı ilkin olaraq yaradıldı.");
            fclose(fCreate);
        }
        else
        {
            Serial.println("XƏTA: Master verilənlər bazası binar faylı yaradıla bilmədi!");
        }
    }
    else
    {
        fclose(fCheck);
    }
    Serial.println("-----------------------------------\n");
#elif defined(TARGET_PLATFORM_WINDOWS)
    // Serial.println("\n--- [SqlBinDB] Sistem Başladılır ---");
    printf("\n--- [SqlBinDB] Sistem Başladılır ---\n");
    printf("Aktiv Platforma: WINDOWS\n");
    _mkdir(MASTER_DIR);
    printf("-----------------------------------\n");
#elif defined(TARGET_PLATFORM_LINUX)
    printf("\n--- [SqlBinDB] Sistem Başladılır ---\n");
    printf("Aktiv Platforma: LINUX\n");
    mkdir(MASTER_DIR, 0777);
    printf("-----------------------------------\n");
#endif
}

// ===================================================================
// CREATE DATABASE
// ===================================================================
// bool createDb(const char *DbName, const char *DbPsw)
// {
//     initSystem();

//     // "rb+" mövcut faylı oxuyub-yazmaq üçündür. Əgər yoxdursa yuxarıda yaradılıb.
//     FILE *f = fopen(MASTER_FILE, "rb+");
//     if (!f)
//     {
// // Serial.println("XƏTA: createDb master faylı 'rb+' rejimində aça bilmədi!");
// #if defined(TARGET_PLATFORM_ESP32)
//         Serial.println("\nXƏTA: createDb master faylı 'rb+' rejimində aça bilmədi!");
// #else
//         printf("\nXƏTA: createDb master faylı 'rb+' rejimində aça bilmədi!\n");
// #endif
//         return false;
//     }

//     DBRegistry reg;
//     bool exists = false;
//     long offset = 0;

//     while (fread(&reg, sizeof(DBRegistry), 1, f))
//     {
//         if (reg.is_deleted == 0 && strcmp(reg.db_name, DbName) == 0)
//         {
//             exists = true;
//             // if (!reCreate) {
//             //     fclose(f);
//             //     return true;
//             // }
//             break;
//         }
//         offset += sizeof(DBRegistry);
//     }

//     // if (exists && reCreate) {
//     //     fseek(f, offset, SEEK_SET);
//     //     uint8_t del = 1;
//     //     fwrite(&del, 1, 1, f);
//     // }

//     // Yeni bazanı mərkəzi qeydiyyata yazırıq
//     fseek(f, 0, SEEK_END);
//     DBRegistry newDb = {0};
//     newDb.is_deleted = 0; // Təhlükəsizlik üçün sıfırlayırıq
//     strncpy(newDb.db_name, DbName, 17);
//     strncpy(newDb.db_password, DbPsw, 17);

//     if (fwrite(&newDb, sizeof(DBRegistry), 1, f) == 1)
//     {
// #if defined(TARGET_PLATFORM_ESP32)
//         Serial.printf("[Uğurlu] '%s' bazası qeydiyyat faylına yazıldı.\n", DbName);
// #else
//         printf("\n[Uğurlu] '%s' bazası qeydiyyat faylına yazıldı.\n", DbName);
// #endif
//     }
//     else
//     {
// // Serial.println("XƏTA: Baza məlumatları binar fayla yazıla bilmədi!");
// #if defined(TARGET_PLATFORM_ESP32)
//         Serial.println("\nXƏTA: Baza məlumatları binar fayla yazıla bilmədi!");
// #else
//         printf("\nXƏTA: Baza məlumatları binar fayla yazıla bilmədi!\n");
// #endif
//     }
//     fclose(f);

//     // Fiziki alt qovluq strukturlarının qurulması
//     char path[256];
// #if defined(TARGET_PLATFORM_ESP32)
//     snprintf(path, sizeof(path), "%s/%s", LFS_DIR, DbName);
//     LittleFS.mkdir(path);
//     snprintf(path, sizeof(path), "%s/%s/tables", LFS_DIR, DbName);
//     LittleFS.mkdir(path);
//     snprintf(path, sizeof(path), "%s/%s/metadata", LFS_DIR, DbName);
//     LittleFS.mkdir(path);
// #else
//     snprintf(path, sizeof(path), "%s/%s", MASTER_DIR, DbName);
//     mkdir(path, 0777);
//     snprintf(path, sizeof(path), "%s/%s/tables", MASTER_DIR, DbName);
//     mkdir(path, 0777);
//     snprintf(path, sizeof(path), "%s/%s/metadata", MASTER_DIR, DbName);
//     mkdir(path, 0777);
// #endif

//     // Metadata fayllarının sıfırdan yaradılması (fopen rəsmi mount ilə)
//     snprintf(path, sizeof(path), "%s/%s/metadata/tables.db", MASTER_DIR, DbName);
//     FILE *f1 = fopen(path, "wb");
//     if (f1)
//         fclose(f1);
//     snprintf(path, sizeof(path), "%s/%s/metadata/columns.db", MASTER_DIR, DbName);
//     FILE *f2 = fopen(path, "wb");
//     if (f2)
//         fclose(f2);
//     snprintf(path, sizeof(path), "%s/%s/metadata/relations.db", MASTER_DIR, DbName);
//     FILE *f3 = fopen(path, "wb");
//     if (f3)
//         fclose(f3);
//     snprintf(path, sizeof(path), "%s/%s/metadata/indexes.db", MASTER_DIR, DbName);
//     FILE *f4 = fopen(path, "wb");
//     if (f4)
//         fclose(f4);

//     return true;
// }

bool createDb(const char *DbName, const char *DbPsw)
{
    initSystem();

    FILE *f = fopen(MASTER_FILE, "rb+");
    if (!f)
        f = fopen(MASTER_FILE, "wb+"); // Əgər fayl yoxdursa yarat

    DBRegistry reg;
    long freeSlotOffset = -1; // Boş yerin yeri
    bool exists = false;

    // 1. Qeydiyyat faylını yoxla: Mövcuddursa tap, yoxdursa ilk "silinmiş" yeri tap
    while (fread(&reg, sizeof(DBRegistry), 1, f))
    {
        if (reg.is_deleted == 0 && strcmp(reg.db_name, DbName) == 0)
        {
            exists = true; // Baza artıq mövcuddur
            break;
        }
        if (reg.is_deleted == 1 && freeSlotOffset == -1)
        {
            freeSlotOffset = ftell(f) - sizeof(DBRegistry);
        }
    }

    if (exists)
    {
        printf("XƏTA: '%s' adlı baza artıq mövcuddur.\n", DbName);
        fclose(f);
        return false;
    }

    // 2. Yeni məlumatı hazırlıq (ya boş slot, ya da fayl sonu)
    DBRegistry newDb = {0};
    newDb.is_deleted = 0;
    strncpy(newDb.db_name, DbName, 17);
    strncpy(newDb.db_password, DbPsw, 17);

    if (freeSlotOffset != -1)
    {
        fseek(f, freeSlotOffset, SEEK_SET); // Tapılmış boş yerə get
    }
    else
    {
        fseek(f, 0, SEEK_END); // Faylın sonuna get
    }

    fwrite(&newDb, sizeof(DBRegistry), 1, f);
    fclose(f);

    // 3. Fiziki qovluğu tam silib yenidən yarat (Təmiz başlanğıc)
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", MASTER_DIR, DbName);

    // Qovluğu rekursiv silmə funksiyası burada çağırılmalıdır (məsələn: remove_directory(path))
    // Aşağıda sadə silmə və yaratma:
    remove(path); // Qovluq boşdursa silər, dolu olduqda rekursiv silmə lazımdır
    mkdir(path, 0777);

    // Alt qovluqları yarat
    snprintf(path, sizeof(path), "%s/%s/tables", MASTER_DIR, DbName);
    mkdir(path, 0777);
    snprintf(path, sizeof(path), "%s/%s/metadata", MASTER_DIR, DbName);
    mkdir(path, 0777);

    // Metadata fayllarını resetlə
    const char *files[] = {"tables.db", "columns.db", "relations.db", "indexes.db"};
    for (int i = 0; i < 4; i++)
    {
        snprintf(path, sizeof(path), "%s/%s/metadata/%s", MASTER_DIR, DbName, files[i]);
        FILE *tmp = fopen(path, "wb");
        if (tmp)
            fclose(tmp);
    }

    return true;
}

// ====================================================================
// CONNECT DATABASE
// ====================================================================
bool connectDb(const char *DbName, const char *DbPsw)
{
    FILE *f = fopen(MASTER_FILE, "rb");
    if (!f)
    {
// Serial.println("XƏTA: connectDb master faylı oxumaq üçün aça bilmədi!");
#if defined(TARGET_PLATFORM_ESP32)
        Serial.println("\nXƏTA: connectDb master faylı oxumaq üçün aça bilmədi!");
#else
        printf("\nXƏTA: connectDb master faylı oxumaq üçün aça bilmədi!\n");
#endif
        return false;
    }

    DBRegistry reg;
    bool found = false;
    while (fread(&reg, sizeof(DBRegistry), 1, f))
    {
        if (reg.is_deleted == 0 && strcmp(reg.db_name, DbName) == 0 && strcmp(reg.db_password, DbPsw) == 0)
        {
            found = true;
            break;
        }
    }
    fclose(f);

    if (found)
    {
        strncpy(current_db_name, DbName, 17);
        snprintf(current_db_path, sizeof(current_db_path), "%s/%s", MASTER_DIR, DbName);
        return true;
    }
    return false;
}

#include <stdlib.h> // Desktop platformalarında system() funksiyası üçün

#if defined(TARGET_PLATFORM_ESP32)
// ====================================================================
// ESP32 ÜÇÜN STACK-I QORUYAN VƏ ÇÖKMƏYƏN SİLMƏ MEXANİZMİ
// ====================================================================

// Rekursiv silmə funksiyamız (dəyişmədi, sadəcə ayrıca task daxilində çağırılacaq)
void helperDeleteDirSafe(const char *dirPath)
{
    while (true)
    {
        File dir = LittleFS.open(dirPath, "r");
        if (!dir)
            break;

        if (!dir.isDirectory())
        {
            dir.close();
            LittleFS.remove(dirPath);
            return;
        }

        File file = dir.openNextFile();
        if (!file)
        {
            dir.close();
            break;
        }

        String entryPath = file.path();
        if (entryPath.length() == 0)
        {
            entryPath = String(dirPath) + "/" + file.name();
        }
        bool isSubDir = file.isDirectory();

        file.close();
        dir.close();

        if (isSubDir)
        {
            helperDeleteDirSafe(entryPath.c_str());
        }
        else
        {
            LittleFS.remove(entryPath.c_str());
        }
    }
    LittleFS.rmdir(dirPath);
}

// FreeRTOS Taskına ötürüləcək parametrlər üçün struktur
typedef struct
{
    char dirPath[256];
    SemaphoreHandle_t doneSemaphore;
} DeleteTaskParam;

// Müvəqqəti yaradılacaq və geniş yaddaşda (Stack) işləyəcək Task funksiyası
void helperDeleteDirTask(void *pvParameters)
{
    DeleteTaskParam *param = (DeleteTaskParam *)pvParameters;
    if (param != NULL)
    {
        // Ağır fayl sistemi əməliyyatını burda rahat icra edirik (Stak limiti problemi yoxdur)
        helperDeleteDirSafe(param->dirPath);

        if (param->doneSemaphore != NULL)
        {
            xSemaphoreGive(param->doneSemaphore); // Ana dövrə işin bitdiyini xəbər veririk
        }
    }
    vTaskDelete(NULL); // Task özünü yaddaşdan silir
}
#endif

// ====================================================================
// DROP DATABASE (TAM STABİL VƏ TƏHLÜKƏSİZ VARIANT)
// ====================================================================
bool dropDb(char *DbName, char *DbPsw)
{
    char savedPsw[18];
    long offset;

    // 1. Bazanın mövcudluğunu və parolunu yoxlayırıq
    if (!helperCheckDbExists(DbName, savedPsw, &offset))
    {
        printf("Xeta: '%s' adinda bir bazaya rast gelinmedi!\n", DbName);
        return false;
    }

    if (strcmp(savedPsw, DbPsw) != 0)
    {
        printf("Xeta: Parol sehvdir! '%s' bazasi siline bilmez.\n", DbName);
        return false;
    }

    // 2. Əgər silinən baza hal-hazırda aktiv qoşulmuş bazadırsa, əlaqəni kəsirik
    if (strcmp(current_db_name, DbName) == 0)
    {
        disConnectDb();
    }

    // 3. Mərkəzi qeydiyyat faylında (master_dbs.db) 'is_deleted' bayrağını 1 edirik
    FILE *f = fopen(MASTER_FILE, "rb+");
    if (f)
    {
        fseek(f, offset, SEEK_SET);
        uint8_t deleteFlag = 1;
        fwrite(&deleteFlag, 1, 1, f);
        fclose(f);
        printf("[Uqurlu] '%s' bazasi merkezi qeydiyyatdan silindi.\n", DbName);
    }
    else
    {
        printf("XETA: master fayli 'rb+' rejiminde acila bilmedi!\n");
        return false;
    }

    // 4. Fiziki qovluqların və daxili binar faylların silinməsi
    char pathBuffer[300];

#if defined(TARGET_PLATFORM_ESP32)
    snprintf(pathBuffer, sizeof(pathBuffer), "%s/%s", LFS_DIR, DbName);
    printf("Fiziki qovluq silinir (LittleFS Genis Task vasiteile): %s\n", pathBuffer);

    // loopTask-ın stakını qorumaq üçün FreeRTOS Semaphor və dinamik struktur yaradırıq
    SemaphoreHandle_t sem = xSemaphoreCreateBinary();
    DeleteTaskParam *param = (DeleteTaskParam *)malloc(sizeof(DeleteTaskParam));

    if (param != NULL && sem != NULL)
    {
        strncpy(param->dirPath, pathBuffer, sizeof(param->dirPath));
        param->doneSemaphore = sem;

        // 🔥 12 KB (12288 bayt) stack ölçüsü ilə yeni müstəqil task yaradırıq
        xTaskCreate(helperDeleteDirTask, "db_lfs_del_task", 12288, param, 5, NULL);

        // Əsas proqram (loopTask) burda bloklanır və gözləyir, stakı isə tamamilə təhlükəsiz qalır
        xSemaphoreTake(sem, portMAX_DELAY);

        vSemaphoreDelete(sem);
        free(param);
        printf("[Uqurlu] Fiziki qovluq ve alt binar senedler tam temizlendi.\n");
    }
    else
    {
        // Nadir halda RAM tam dolarsa (Fallback olaraq birbaşa çağırış)
        if (sem)
            vSemaphoreDelete(sem);
        if (param)
            free(param);
        helperDeleteDirSafe(pathBuffer);
    }

#elif defined(TARGET_PLATFORM_WINDOWS)
    snprintf(pathBuffer, sizeof(pathBuffer), "rmdir /s /q \"%s\\%s\"", MASTER_DIR, DbName);
    system(pathBuffer);
    printf("Fiziki qovluq silindi (Windows): %s/%s\n", MASTER_DIR, DbName);

#elif defined(TARGET_PLATFORM_LINUX)
    snprintf(pathBuffer, sizeof(pathBuffer), "rm -rf \"%s/%s\"", MASTER_DIR, DbName);
    system(pathBuffer);
    printf("Fiziki qovluq silindi (Linux): %s/%s\n", MASTER_DIR, DbName);
#endif

    return true;
}

// ====================================================================
// DISCONNECT DATABASE
// ====================================================================
bool disConnectDb()
{
    memset(current_db_name, 0, sizeof(current_db_name));
    memset(current_db_path, 0, sizeof(current_db_path));
    return true;
}

// ====================================================================
// SELECT DATABASE
// ====================================================================
void selectDb(const char *DbName)
{
    if (strcmp(DbName, "*") == 0)
    {
        FILE *f = fopen(MASTER_FILE, "rb");
        if (!f)
        {
            printf("Sistemde hec bir verilenebler bazasi tapilmadi.\n");
            return;
        }

        printf("\n=== SISTEMDEKI BUTUN VERILENLER BAZALARI ===\n");
        printf("%-20s\t%-20s\t%-20s\n", "DATABASE NAME", "PASSWORD", "Deleted");
        printf("--------------------------------------------\n");

        DBRegistry reg;
        int count = 0;
        while (fread(&reg, sizeof(DBRegistry), 1, f))
        {
            // if (reg.is_deleted == 0)
            // {
            printf("%-20s\t%-20s\t%-4d\n", reg.db_name, reg.db_password, reg.is_deleted);
            count++;
            // }
        }
        fclose(f);
        if (count == 0)
            printf("(Siyahı boşdur)\n");
        printf("============================================\n\n");
        return;
    }

    if (strlen(DbName) == 1)
    {
        if (strlen(current_db_name) == 0)
        {
            printf("Hazirda aktiv qosulma yoxdur (Aktiv DB: NULL).\n");
        }
        else
        {
            printf("Hazirda qosulu olan Verilenebler Bazasi: [%s]\n",
                   current_db_name);
        }
        return;
    }

    char dummyPsw[18];
    long dummyOffset;
    if (helperCheckDbExists(DbName, dummyPsw, &dummyOffset))
    {
        printf("Sorğu: '%s' adinda verilenebler bazasi sistemde movcuddur.\n",
               DbName);
    }
    else
    {
        printf("XETA: '%s' adinda bir verilenebler bazasi tapilmadi!\n", DbName);
    }
}

// ====================================================================
// HELPER: Baza adını mərkəzi binar faylda axtaran daxili funksiya
// ====================================================================
// bool helperCheckDbExists(const char *DbName, char *outPsw, long *outOffset)
// {
//     // LittleFS obyekti üçün rəsmi LFS_FILE (başında /littlefs olmayan) istifadə edilir
//     File f = LittleFS.open(LFS_FILE, "r");
//     if (!f || f.isDirectory())
//     {
//         return false;
//     }

//     DBRegistry reg;
//     long currentOffset = 0;
//     bool found = false;

//     while (f.read((uint8_t *)&reg, sizeof(DBRegistry)) == sizeof(DBRegistry))
//     {
//         if (reg.is_deleted == 0 && strcmp(reg.db_name, DbName) == 0)
//         {
//             if (outPsw)
//                 strcpy(outPsw, reg.db_password);
//             if (outOffset)
//                 *outOffset = currentOffset;
//             found = true;
//             break;
//         }
//         currentOffset += sizeof(DBRegistry);
//     }

//     f.close();
//     return found;
// }

bool helperCheckDbExists(const char *DbName, char *outPsw, long *outOffset)
{
#if defined(TARGET_PLATFORM_ESP32)
    File f = LittleFS.open(LFS_FILE, "r");
    if (!f || f.isDirectory())
        return false;

    DBRegistry reg;
    long currentOffset = 0;
    bool found = false;
    while (f.read((uint8_t *)&reg, sizeof(DBRegistry)) == sizeof(DBRegistry))
    {
        if (reg.is_deleted == 0 && strcmp(reg.db_name, DbName) == 0)
        {
            if (outPsw)
                strcpy(outPsw, reg.db_password);
            if (outOffset)
                *outOffset = currentOffset;
            found = true;
            break;
        }
        currentOffset += sizeof(DBRegistry);
    }
    f.close();
    return found;
#else
    // Windows və Linux üçün Standart C Fayl Sistemi
    FILE *f = fopen(MASTER_FILE, "rb");
    if (!f)
        return false;

    DBRegistry reg;
    long currentOffset = 0;
    bool found = false;
    while (fread(&reg, sizeof(DBRegistry), 1, f) == 1)
    {
        if (reg.is_deleted == 0 && strcmp(reg.db_name, DbName) == 0)
        {
            if (outPsw)
                strcpy(outPsw, reg.db_password);
            if (outOffset)
                *outOffset = currentOffset;
            found = true;
            break;
        }
        currentOffset += sizeof(DBRegistry);
    }
    fclose(f);
    return found;
#endif
}

#include <stdio.h>
#include <string.h>

#if defined(TARGET_PLATFORM_ESP32)
#include <LittleFS.h>
bool executeDropSystem(const char *path)
{
    File root = LittleFS.open(path);
    if (!root || !root.isDirectory())
        return false;

    File file = root.openNextFile();
    while (file)
    {
        String filePath = String(path) + "/" + file.name();
        if (file.isDirectory())
        {
            executeDropSystem(filePath.c_str());
        }
        else
        {
            LittleFS.remove(filePath.c_str());
        }
        file = root.openNextFile();
    }
    LittleFS.rmdir(path);
    return true;
}
#else
// Windows üçün (Windows-da qovluq silmək üçün sistem əmri daha praktikdir)
bool executeDropSystem(const char *path)
{
    char command[512];
    // 'rmdir /s /q' Windows-da qovluğu və içindəkiləri tamamilə silir
    snprintf(command, sizeof(command), "rmdir /s /q \"%s\"", path);
    system(command);
    return true;
}
#endif

#endif