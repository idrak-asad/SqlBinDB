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
bool createDb(const char *DbName, const char *DbPsw, bool reCreate);
bool dropDb(char *DbName, char *DbPsw);
bool connectDb(const char *DbName, const char *DbPsw);
bool disConnectDb();
void selectDb(const char *DbName);

void initSystem() {
    Serial.println("\n--- [SqlBinDB] Sistem Başladılır ---");

    #if defined(TARGET_PLATFORM_ESP32)
        Serial.println("Aktiv Platforma: ESP32 (LittleFS)");
        
        if (!LittleFS.begin(true)) {
            Serial.println("XƏTA: LittleFS mount edilə bilmədi!");
            return;
        }
        
        // LittleFS obyekti daxili idarəetmədə '/littlefs' prefiksini özü idarə edir
        if (!LittleFS.exists(LFS_DIR)) {
            if (LittleFS.mkdir(LFS_DIR)) {
                Serial.println("Mərkəzi qovluq yaradıldı: " LFS_DIR);
            } else {
                Serial.println("XƏTA: Qovluq yaradıla bilmədi!");
            }
        }

        // Master binar qeydiyyat faylı yoxdursa, sıfırdan binar rejimdə yaradaq
        FILE *fCheck = fopen(MASTER_FILE, "rb");
        if (!fCheck) {
            FILE *fCreate = fopen(MASTER_FILE, "wb");
            if (fCreate) {
                Serial.println("Master verilənlər bazası faylı ilkin olaraq yaradıldı.");
                fclose(fCreate);
            } else {
                Serial.println("XƏTA: Master verilənlər bazası binar faylı yaradıla bilmədi!");
            }
        } else {
            fclose(fCheck);
        }

    #elif defined(TARGET_PLATFORM_WINDOWS)
        printf("Aktiv Platforma: WINDOWS\n");
        _mkdir(MASTER_DIR); 
    #elif defined(TARGET_PLATFORM_LINUX)
        printf("Aktiv Platforma: LINUX\n");
        mkdir(MASTER_DIR, 0777); 
    #endif

    Serial.println("-----------------------------------\n");
}

// ===================================================================
// CREATE DATABASE
// ===================================================================
bool createDb(const char *DbName, const char *DbPsw, bool reCreate) {
    initSystem();
    
    // "rb+" mövcut faylı oxuyub-yazmaq üçündür. Əgər yoxdursa yuxarıda yaradılıb.
    FILE *f = fopen(MASTER_FILE, "rb+");
    if (!f) {
        Serial.println("XƏTA: createDb master faylı 'rb+' rejimində aça bilmədi!");
        return false;
    }

    DBRegistry reg;
    bool exists = false;
    long offset = 0;

    while (fread(&reg, sizeof(DBRegistry), 1, f)) {
        if (reg.is_deleted == 0 && strcmp(reg.db_name, DbName) == 0) {
            exists = true;
            if (!reCreate) {
                fclose(f);
                return true; 
            }
            break;
        }
        offset += sizeof(DBRegistry);
    }

    if (exists && reCreate) {
        fseek(f, offset, SEEK_SET);
        uint8_t del = 1;
        fwrite(&del, 1, 1, f); 
    }

    // Yeni bazanı mərkəzi qeydiyyata yazırıq
    fseek(f, 0, SEEK_END);
    DBRegistry newDb = {0};
    newDb.is_deleted = 0; // Təhlükəsizlik üçün sıfırlayırıq
    strncpy(newDb.db_name, DbName, 17);
    strncpy(newDb.db_password, DbPsw, 17);
    
    if (fwrite(&newDb, sizeof(DBRegistry), 1, f) == 1) {
        Serial.printf("[Uğurlu] '%s' bazası qeydiyyat faylına yazıldı.\n", DbName);
    } else {
        Serial.println("XƏTA: Baza məlumatları binar fayla yazıla bilmədi!");
    }
    fclose(f);

    // Fiziki alt qovluq strukturlarının qurulması
    char path[256];
#if defined(TARGET_PLATFORM_ESP32)
    snprintf(path, sizeof(path), "%s/%s", LFS_DIR, DbName); LittleFS.mkdir(path);
    snprintf(path, sizeof(path), "%s/%s/tables", LFS_DIR, DbName); LittleFS.mkdir(path);
    snprintf(path, sizeof(path), "%s/%s/metadata", LFS_DIR, DbName); LittleFS.mkdir(path);
#else
    snprintf(path, sizeof(path), "%s/%s", MASTER_DIR, DbName); mkdir(path, 0777);
    snprintf(path, sizeof(path), "%s/%s/tables", MASTER_DIR, DbName); mkdir(path, 0777);
    snprintf(path, sizeof(path), "%s/%s/metadata", MASTER_DIR, DbName); mkdir(path, 0777);
#endif

    // Metadata fayllarının sıfırdan yaradılması (fopen rəsmi mount ilə)
    snprintf(path, sizeof(path), "%s/%s/metadata/tables.db", MASTER_DIR, DbName);
    FILE *f1 = fopen(path, "wb"); if (f1) fclose(f1);
    snprintf(path, sizeof(path), "%s/%s/metadata/columns.db", MASTER_DIR, DbName);
    FILE *f2 = fopen(path, "wb"); if (f2) fclose(f2);
    snprintf(path, sizeof(path), "%s/%s/metadata/relations.db", MASTER_DIR, DbName);
    FILE *f3 = fopen(path, "wb"); if (f3) fclose(f3);
    snprintf(path, sizeof(path), "%s/%s/metadata/indexes.db", MASTER_DIR, DbName);
    FILE *f4 = fopen(path, "wb"); if (f4) fclose(f4);

    return true;
}

// ====================================================================
// CONNECT DATABASE
// ====================================================================
bool connectDb(const char *DbName, const char *DbPsw) {
    FILE *f = fopen(MASTER_FILE, "rb");
    if (!f) {
        Serial.println("XƏTA: connectDb master faylı oxumaq üçün aça bilmədi!");
        return false;
    }

    DBRegistry reg;
    bool found = false;
    while (fread(&reg, sizeof(DBRegistry), 1, f)) {
        if (reg.is_deleted == 0 && strcmp(reg.db_name, DbName) == 0 && strcmp(reg.db_password, DbPsw) == 0) {
            found = true;
            break;
        }
    }
    fclose(f);

    if (found) {
        strncpy(current_db_name, DbName, 17);
        snprintf(current_db_path, sizeof(current_db_path), "%s/%s", MASTER_DIR, DbName);
        return true;
    }
    return false;
}

// ====================================================================
// DROP DATABASE
// ====================================================================
#if defined(TARGET_PLATFORM_ESP32)
// ESP32-də LittleFS qovluğu silmək üçün öncə içinin boş olmasını tələb edir.
// Bu köməkçi funksiya rekursiv olaraq daxildəki bütün fayl və qovluqları təmizləyir.
void helperDeleteDirRecursive(const char *path) {
    File root = LittleFS.open(path);
    if (!root) return;
    if (!root.isDirectory()) {
        root.close();
        LittleFS.remove(path);
        return;
    }

    File file = root.openNextFile();
    while (file) {
        String entryPath = file.path();
        if (entryPath.length() == 0) {
            entryPath = String(path) + "/" + file.name();
        }

        if (file.isDirectory()) {
            file.close();
            helperDeleteDirRecursive(entryPath.c_str());
        } else {
            file.close();
            LittleFS.remove(entryPath.c_str());
        }
        file = root.openNextFile();
    }
    root.close();
    LittleFS.rmdir(path);
}
#endif

bool dropDb(char *DbName, char *DbPsw) {
    char savedPsw[18];
    long offset;
    
    // 1. Bazanın mövcudluğunu və parolunu yoxlayırıq
    if (!helperCheckDbExists(DbName, savedPsw, &offset)) {
        Serial.printf("Xəta: '%s' adında bir bazaya rast gəlinmədi!\n", DbName);
        return false;
    }

    if (strcmp(savedPsw, DbPsw) != 0) {
        Serial.printf("Xəta: Parol səhvdir! '%s' bazası silinə bilməz.\n", DbName);
        return false;
    }

    // 2. Mərkəzi qeydiyyat faylında (master_dbs.db) 'is_deleted' bayrağını 1 edirik
    FILE *f = fopen(MASTER_FILE, "rb+");
    if (f) {
        fseek(f, offset, SEEK_SET);
        uint8_t deleteFlag = 1;
        fwrite(&deleteFlag, 1, 1, f); 
        fclose(f);
        Serial.printf("[Uğurlu] '%s' bazası mərkəzi qeydiyyatdan silindi.\n", DbName);
    } else {
        Serial.println("XƏTA: master faylı 'rb+' rejimində açıla bilmədi!");
        return false;
    }

    // 3. Fiziki qovluqların və daxili binar faylların tamamilə silinməsi
    char pathBuffer[300];
    
#if defined(TARGET_PLATFORM_ESP32)
    // ESP32 LittleFS üzərində rekursiv təmizləmə çağırılır
    snprintf(pathBuffer, sizeof(pathBuffer), "%s/%s", LFS_DIR, DbName);
    Serial.printf("Fiziki qovluq silinir (LittleFS): %s\n", pathBuffer);
    helperDeleteDirRecursive(pathBuffer);
    
#elif defined(TARGET_PLATFORM_WINDOWS)
    // Windows üçün qovluğu alt faylları ilə birgə silən OS əmri (/s /q)
    snprintf(pathBuffer, sizeof(pathBuffer), "rmdir /s /q \"%s\\%s\"", MASTER_DIR, DbName);
    system(pathBuffer);
    printf("Fiziki qovluq silindi (Windows): %s/%s\n", MASTER_DIR, DbName);
    
#elif defined(TARGET_PLATFORM_LINUX)
    // Linux üçün qovluğu alt faylları ilə birgə silən OS əmri (rm -rf)
    snprintf(pathBuffer, sizeof(pathBuffer), "rm -rf \"%s/%s\"", MASTER_DIR, DbName);
    system(pathBuffer);
    printf("Fiziki qovluq silindi (Linux): %s/%s\n", MASTER_DIR, DbName);
#endif

    // 4. Əgər silinən baza hal-hazırda aktiv qoşulmuş bazadırsa, əlaqəni kəsirik
    if (strcmp(current_db_name, DbName) == 0) {
        disConnectDb();
    }

    return true;
}

// ====================================================================
// DISCONNECT DATABASE
// ====================================================================
bool disConnectDb() {
    memset(current_db_name, 0, sizeof(current_db_name));
    memset(current_db_path, 0, sizeof(current_db_path));
    return true;
}

// ====================================================================
// SELECT DATABASE
// ====================================================================
void selectDb(const char *DbName) {
  if (strcmp(DbName, "*") == 0) {
    FILE *f = fopen(MASTER_FILE, "rb");
    if (!f) {
      printf("Sistemde hec bir verilenebler bazasi tapilmadi.\n");
      return;
    }

    printf("\n=== SISTEMDEKI BUTUN VERILENLER BAZALARI ===\n");
    printf("%-20s\t%-20s\n", "DATABASE NAME", "PASSWORD");
    printf("--------------------------------------------\n");

    DBRegistry reg;
    int count = 0;
    while (fread(&reg, sizeof(DBRegistry), 1, f)) {
      if (reg.is_deleted == 0) {
        printf("%-20s\t%-20s\n", reg.db_name, reg.db_password);
        count++;
      }
    }
    fclose(f);
    if (count == 0)
      printf("(Siyahı boşdur)\n");
    printf("============================================\n\n");
    return;
  }

  if (strlen(DbName) == 1) {
    if (strlen(current_db_name) == 0) {
      printf("Hazirda aktiv qosulma yoxdur (Aktiv DB: NULL).\n");
    } else {
      printf("Hazirda qosulu olan Verilenebler Bazasi: [%s]\n",
             current_db_name);
    }
    return;
  }

  char dummyPsw[18];
  long dummyOffset;
  if (helperCheckDbExists(DbName, dummyPsw, &dummyOffset)) {
    printf("Sorğu: '%s' adinda verilenebler bazasi sistemde movcuddur.\n",
           DbName);
  } else {
    printf("XETA: '%s' adinda bir verilenebler bazasi tapilmadi!\n", DbName);
  }
}

// ====================================================================
// HELPER: Baza adını mərkəzi binar faylda axtaran daxili funksiya
// ====================================================================
bool helperCheckDbExists(const char *DbName, char *outPsw, long *outOffset) {
  // LittleFS obyekti üçün rəsmi LFS_FILE (başında /littlefs olmayan) istifadə edilir
  File f = LittleFS.open(LFS_FILE, "r");
  if (!f || f.isDirectory()) {
      return false;
  }

  DBRegistry reg;
  long currentOffset = 0;
  bool found = false;

  while (f.read((uint8_t*)&reg, sizeof(DBRegistry)) == sizeof(DBRegistry)) {
      if (reg.is_deleted == 0 && strcmp(reg.db_name, DbName) == 0) {
          if (outPsw) strcpy(outPsw, reg.db_password);
          if (outOffset) *outOffset = currentOffset;
          found = true;
          break;
      }
      currentOffset += sizeof(DBRegistry);
  }
  
  f.close(); 
  return found;
}

#endif