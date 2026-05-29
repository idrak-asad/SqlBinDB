// db_controls.h
#ifndef DB_CONTROLS_H
#define DB_CONTROLS_H

// #include "add_controls.h"


// #define MASTER_DIR "/sqlBinDB"
// #define MASTER_FILE "/sqlBinDB/master_dbs.db"

#if defined(TARGET_PLATFORM_ESP32)
    #include "LittleFS.h"
    #define MASTER_DIR "/sqlBinDB"
    #define MASTER_FILE "/sqlBinDB/master_dbs.db"
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
void initSystem();
bool createDb(const char *DbName, const char *DbPsw, bool reCreate);
bool dropDb(char *DbName, char *DbPsw);
bool connectDb(char *DbName, char *DbPsw);
bool disConnectDb();
void selectDb(char *DbName);
// bool helperCheckDbExists(char *DbName, char *outPsw, long *outOffset);
bool helperCheckDbExists(const char *DbName, char *outPsw, long *outOffset)

// Qlobal aktiv qoşulma yolu
// char current_db_path[128] = "";

// Sistemi ilk dəfə işə salanda ana qovluğu yaradır
// void initSystem() {

//     // LittleFS mount
//     if(!LittleFS.begin(true)){
//         printf("LittleFS mount xetasi!\n");
//         return;
//     }

//     // əsas qovluq
//     platform_create_dir(MASTER_DIR);

//     // master faylı yoxdursa yarat
//     FILE *f = fopen(MASTER_FILE, "rb");

//     if(!f){

//         f = fopen(MASTER_FILE, "wb");

//         if(!f){
//             printf("MASTER FILE yaradila bilmedi!\n");
//             return;
//         }

//         printf("MASTER FILE yaradildi.\n");
//     }

//     if(f)
//         fclose(f);
// }



void initSystem() {
    Serial.println("\n--- [SqlBinDB] Sistem Başladılır ---");

    #if defined(TARGET_PLATFORM_ESP32)
        Serial.println("Aktiv Platforma: ESP32 (LittleFS)");
        
        // ESP32 üçün LittleFS mount əməliyyatı
        if (!LittleFS.begin(true)) {
            Serial.println("XƏTA: LittleFS mount edilə bilmədi!");
            return;
        }
        
        // Qovluğun mövcudluğunu yoxlayıb yoxdursa yaradırıq
        if (!LittleFS.exists(MASTER_DIR)) {
            if (LittleFS.mkdir(MASTER_DIR)) {
                Serial.println("Mərkəzi qovluq yaradıldı: " MASTER_DIR);
            } else {
                Serial.println("XƏTA: Qovluq yaradıla bilmədi!");
            }
        }
    #elif defined(TARGET_PLATFORM_WINDOWS)
        printf("Aktiv Platforma: WINDOWS\n");
        _mkdir(MASTER_DIR); // Windows üçün qovluq yaratmaq
    #elif defined(TARGET_PLATFORM_LINUX)
        printf("Aktiv Platforma: LINUX\n");
        mkdir(MASTER_DIR, 0777); // Linux üçün qovluq yaratmaq
    #else
        printf("Aktiv Platforma: TƏYİN OLUNMAMIŞ/BİLİNMƏYƏN\n");
    #endif

    Serial.println("-----------------------------------\n");
}

// ===================================================================
// CREATE DATABASE (Verilənlər Bazası Yaradılması - Tam ESP32/VFS Uyğun)
// ===================================================================
bool createDb(const char *DbName, const char *DbPsw, bool reCreate) {
    initSystem();
    
    // Bazadan öncə mövcud olub-olmadığını yoxlayaq
    FILE *f = fopen(MASTER_FILE, "rb+");
    if (!f) return false;

    DBRegistry reg;
    bool exists = false;
    long offset = 0;

    while (fread(&reg, sizeof(DBRegistry), 1, f)) {
        if (reg.is_deleted == 0 && strcmp(reg.db_name, DbName) == 0) {
            exists = true;
            if (!reCreate) {
                fclose(f);
                return true; // Artıq var və sıfırlamaq istəmirik
            }
            break;
        }
        offset += sizeof(DBRegistry);
    }

    if (exists && reCreate) {
        fseek(f, offset, SEEK_SET);
        uint8_t del = 1;
        fwrite(&del, 1, 1, f); // Köhnə qeydi soft-delete edirik
    }

    // Yeni bazanı mərkəzi qeydiyyata yazaq
    fseek(f, 0, SEEK_END);
    DBRegistry newDb = {0};
    strncpy(newDb.db_name, DbName, 17);
    strncpy(newDb.db_password, DbPsw, 17);
    fwrite(&newDb, sizeof(DBRegistry), 1, f);
    fclose(f);

    // Fiziki alt qovluq strukturlarını yaradaq (POSIX formatında)
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", MASTER_DIR, DbName);
#if defined(TARGET_PLATFORM_ESP32)
    LittleFS.mkdir(path);
    snprintf(path, sizeof(path), "%s/%s/tables", MASTER_DIR, DbName); LittleFS.mkdir(path);
    snprintf(path, sizeof(path), "%s/%s/metadata", MASTER_DIR, DbName); LittleFS.mkdir(path);
#else
    mkdir(path, 0777);
    snprintf(path, sizeof(path), "%s/%s/tables", MASTER_DIR, DbName); mkdir(path, 0777);
    snprintf(path, sizeof(path), "%s/%s/metadata", MASTER_DIR, DbName); mkdir(path, 0777);
#endif

    // Metadata fayllarını sıfırdan yaradaq
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
// DROP DATABASE
// ====================================================================
bool dropDb(char *DbName, char *DbPsw) {
  char savedPsw[18];
  long offset;
  if (!helperCheckDbExists(DbName, savedPsw, &offset)) {
    printf("Xeta: '%s' adinda bir bazaya rast gelinmedi!\n", DbName);
    return false;
  }

  if (strcmp(savedPsw, DbPsw) != 0) {
    printf("Xeta: Parol sehvdir! '%s' bazasi siline bilmez.\n", DbName);
    return false;
  }

  // 1. Mərkəzi siyahıda Soft Delete (is_deleted = 1) edirik
  FILE *f = fopen(MASTER_FILE, "rb+");
  if (f) {
    fseek(f, offset, SEEK_SET);
    uint8_t deleteFlag = 1;
    fwrite(&deleteFlag, 1, 1, f); // Sadəcə ilk baytı 1 edirik
    fclose(f);
  }

  // Qoşuludurca bağlantını qırırıq
  if (strcmp(current_db_name, DbName) == 0) {
    disConnectDb();
  }

  // 2. Fiziki qovluğu təmizləyirik
  char command[256];
  snprintf(command, sizeof(command), "rmdir /s /q \"%s\\%s\" >nul 2>nul",
           MASTER_DIR, DbName);
  system(command);

  printf("Ugurlu: '%s' bazasi mərkezi siyahidan və diskden tamamilə silindi.\n",
         DbName);
  return true;
}

// ====================================================================
// CONNECT DATABASE
// ====================================================================
bool connectDb(char *DbName, char *DbPsw) {
    FILE *f = fopen(MASTER_FILE, "rb");
    if (!f) return false;

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
// DISCONNECT DATABASE
// ====================================================================
bool disConnectDb() {
    memset(current_db_name, 0, sizeof(current_db_name));
    memset(current_db_path, 0, sizeof(current_db_path));
    return true;
}

// ====================================================================
// SELECT DATABASE (Sənin istədiyin xüsusi məntiq)
// ====================================================================
void selectDb(char *DbName) {
  // VARIANT 1: DbName == "*" olduqda hamısını siyahılayır
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

  // VARIANT 2: Hər hansı bir tək simvoldursa (məsələn: ".", "?", v.s.) qoşulu
  // olanı göstərir
  if (strlen(DbName) == 1) {
    if (strlen(current_db_name) == 0) {
      printf("Hazirda aktiv qosulma yoxdur (Aktiv DB: NULL).\n");
    } else {
      printf("Hazirda qosulu olan Verilenebler Bazasi: [%s]\n",
             current_db_name);
    }
    return;
  }

  // VARIANT 3: Tam ad yazılıbsa varlığını yoxlayır
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
  // LittleFS ilə faylı oxumaq üçün açırıq
  File f = LittleFS.open(MASTER_FILE, "r");
  if (!f || f.isDirectory()) {
      return false;
  }

  DBRegistry reg;
  long currentOffset = 0;
  bool found = false;

  // fread əvəzinə f.read istifadə edirik
  while (f.read((uint8_t*)&reg, sizeof(DBRegistry)) == sizeof(DBRegistry)) {
      if (reg.is_deleted == 0 && strcmp(reg.db_name, DbName) == 0) {
          // if (outPsw) strcpy(outPsw, reg.db_psw);
          if (outPsw) strcpy(outPsw, reg.db_pass);
          if (outOffset) *outOffset = currentOffset;
          found = true;
          break;
      }
      currentOffset += sizeof(DBRegistry);
  }
  
  f.close(); // fclose əvəzinə close()
  return found;
}

#endif