// #define MASTER_DIR "/sqlBinDB"
// #define MASTER_FILE "/sqlBinDB/master_dbs.db"

#if defined(TARGET_PLATFORM_ESP32)
    #define MASTER_DIR "/sqlBinDB"
    #define MASTER_FILE "/sqlBinDB/master_dbs.db"
#else
    // Windows və Linux üçün (Qarşısında '/' olmadan)
    #define MASTER_DIR "sqlBinDB"
    #define MASTER_FILE "sqlBinDB/master_dbs.db"
#endif

// FUNKSİYA PROTOTİPLƏRİ
void initSystem();
bool createDb(const char *DbName, const char *DbPsw, bool reCreate);
bool dropDb(char *DbName, char *DbPsw);
bool connectDb(char *DbName, char *DbPsw);
bool disConnectDb(char *DbName);
void selectDb(char *DbName);
bool helperCheckDbExists(char *DbName, char *outPsw, long *outOffset);

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
    #if defined(TARGET_PLATFORM_ESP32)
        // 1. LittleFS-i rəsmi olaraq rədd edilmədən başladırıq
        if(!LittleFS.begin(true)){
            printf("XETA: LittleFS mount edile bilmedi!\n");
            return;
        }

        // 2. Qovluğu yoxlayırıq və LittleFS vasitəsilə mütləq yaradırıq
        // (VFS səviyyəsində açılması üçün önündəki /littlefs hissəsini təmizləyirik)
        if (!LittleFS.exists("/sqlBinDB")) {
            LittleFS.mkdir("/sqlBinDB");
        }

        // 3. Əgər master fayl ümumiyyətlə yoxdursa, əvvəlcə daxili metodla yaradıb bağlayırıq
        if (!LittleFS.exists("/sqlBinDB/master_dbs.db")) {
            File fWrite = LittleFS.open("/sqlBinDB/master_dbs.db", FILE_WRITE);
            if(fWrite) {
                fWrite.close();
                printf("MASTER FAYLI ILKIN OLARAQ DISKDE YARADILDI.\n");
            }
        }
    #else
        // Windows/Linux üçün köhnə standart qovluq yaratma
        platform_create_dir(MASTER_DIR);
    #endif

    // 4. İndi standart fopen rəsmi VFS yolu ilə rədd edilmədən faylı aça biləcək
    FILE *f = fopen(MASTER_FILE, "rb");
    if(!f){
        // Əgər fayl boşdursa və ya "rb" ilə açılmırsa, yazma+oxuma rejimində ("wb+") sıfırdan açırıq
        f = fopen(MASTER_FILE, "wb+");
        if(!f){
            printf("Xeta: master registry acilmadi! (fopen hələ də NULL qayıtdı)\n");
            return;
        }
    }

    fclose(f);
    printf("Baza strukturu ugurla hazirlandi.\n");
}

// ===================================================================
// CREATE DATABASE (Verilənlər Bazası Yaradılması - Tam ESP32/VFS Uyğun)
// ===================================================================
bool createDb(const char *DbName, const char *DbPsw, bool reCreate) {
    char checkPsw[18];
    long checkOffset;

    // 1. Baza sistemdə artıq mövcuddurmu deyə yoxlayırıq
    if (helperCheckDbExists((char *)DbName, checkPsw, &checkOffset)) {
        if (!reCreate) {
            printf("XETA: '%s' adinda verilenebler bazasi artıq sistemde movcuddur!\n", DbName);
            return false;
        } else {
            printf("Melumat: '%s' bazasi mövcuddur, RECREATE (yenidən yaratma) aktivdir.\n", DbName);
        }
    }

    // 2. Platformaya uyğun olaraq qovluq yollarını və daxili strukturları təyin edirik
    #if defined(TARGET_PLATFORM_ESP32)
        // ESP32 daxili VFS (fopen) üçün mütləq /littlefs prefiksini tələb edir
        snprintf(current_db_path, sizeof(current_db_path), "/littlefs/%s", DbName);
        
        // LittleFS-in özünün tanıması və qovluq yaratması üçün xam yollar (önündə /littlefs olmadan)
        char rawDbDir[64], rawMetaDir[64], rawTablesDir[64];
        snprintf(rawDbDir, sizeof(rawDbDir), "/%s", DbName);
        snprintf(rawMetaDir, sizeof(rawMetaDir), "/%s/metadata", DbName);
        snprintf(rawTablesDir, sizeof(rawTablesDir), "/%s/tables", DbName);
        
        // Qovluqları növbə ilə təhlükəsiz yaradırıq
        if (!LittleFS.exists(rawDbDir))    LittleFS.mkdir(rawDbDir);
        if (!LittleFS.exists(rawMetaDir))  LittleFS.mkdir(rawMetaDir);
        if (!LittleFS.exists(rawTablesDir)) LittleFS.mkdir(rawTablesDir);
    #else
        // Windows və ya Linux mühiti üçün standart nisbi yollar
        snprintf(current_db_path, sizeof(current_db_path), "%s", DbName);
        char metaPath[256], tablesPath[256];
        snprintf(metaPath, sizeof(metaPath), "%s/metadata", current_db_path);
        snprintf(tablesPath, sizeof(tablesPath), "%s/tables", current_db_path);
        
        platform_create_dir(current_db_path);
        platform_create_dir(metaPath);
        platform_create_dir(tablesPath);
    #endif

    // 3. Verilənlər bazasının əsas struktur metadatalarını (tables.db və columns.db) ilkin binar olaraq yaradırıq
    char tablesMetaPath[256], columnsMetaPath[256];
    snprintf(tablesMetaPath, sizeof(tablesMetaPath), "%s/metadata/tables.db", current_db_path);
    snprintf(columnsMetaPath, sizeof(columnsMetaPath), "%s/metadata/columns.db", current_db_path);

    // "wb" rejimi faylı tam sıfırlayır və yenisini yazmaq üçün açır (Artıq fopen NULL qayıtmayacaq)
    FILE *fT = fopen(tablesMetaPath, "wb");
    FILE *fC = fopen(columnsMetaPath, "wb");

    if (!fT || !fC) {
        printf("XETA: Verilənlər bazasının daxili metadatası yaradıla bilmədi! (fopen xətası)\n");
        if (fT) fclose(fT);
        if (fC) fclose(fC);
        return false;
    }
    fclose(fT);
    fclose(fC);

    // Əlavə olaraq münasibətlər (relations.db) və indekslər (indexes.db) metadatalarını yaradaq
    char relMetaPath[256], idxMetaPath[256];
    snprintf(relMetaPath, sizeof(relMetaPath), "%s/metadata/relations.db", current_db_path);
    snprintf(idxMetaPath, sizeof(idxMetaPath), "%s/metadata/indexes.db", current_db_path);
    
    FILE *fR = fopen(relMetaPath, "wb");
    FILE *fI = fopen(idxMetaPath, "wb");
    if (fR) fclose(fR);
    if (fI) fclose(fI);

    // 4. Yeni bazanın qeydiyyatını mərkəzi reyestrə (master_dbs.db) əlavə edirik və ya yeniləyirik
    FILE *fMaster = fopen(MASTER_FILE, "rb+");
    if (!fMaster) {
        // Əgər master faylı hər hansı səbəbdən yoxdursa, "wb+" rejimi ilə yaradırıq
        fMaster = fopen(MASTER_FILE, "wb+");
    }

    if (!fMaster) {
        printf("XETA: Mərkəzi qeydiyyat faylı (master_dbs.db) açıla bilmədi!\n");
        return false;
    }

    DBRegistry reg;
    memset(&reg, 0, sizeof(DBRegistry));
    reg.is_deleted = 0;
    strncpy(reg.db_name, DbName, sizeof(reg.db_name) - 1);
    strncpy(reg.db_password, DbPsw, sizeof(reg.db_password) - 1);

    if (reCreate && helperCheckDbExists((char *)DbName, checkPsw, &checkOffset)) {
        // Əgər RECREATE ediriksə, köhnə kaydın yerləşdiyi offsetə gedib birbaşa üzərinə yazırıq
        fseek(fMaster, checkOffset, SEEK_SET);
        fwrite(&reg, sizeof(DBRegistry), 1, fMaster);
        printf("Məlumat: Reyestrdəki köhnə '%s' qeydi yeniləndi.\n", DbName);
    } else {
        // Tamamilə yeni bazadırsa, mərkəzi qeydiyyat faylının sonuna (Append) əlavə edirik
        fseek(fMaster, 0, SEEK_END);
        fwrite(&reg, sizeof(DBRegistry), 1, fMaster);
        printf("Məlumat: Yeni '%s' bazası reyestrə uğurla əlavə edildi.\n", DbName);
    }

    fclose(fMaster);

    // Cari aktiv qoşulma adını təyin edirik
    strncpy(current_db_name, DbName, sizeof(current_db_name) - 1);
    printf("UĞURLU: '%s' Verilənlər Bazası rəsmi olaraq yaradıldı və qoşuldu!\n", DbName);
    
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
    disConnectDb(DbName);
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
  char savedPsw[18];
  long offset;
  if (!helperCheckDbExists(DbName, savedPsw, &offset)) {
    printf("Xeta: '%s' adinda verilenebler bazasi tapilmadi!\n", DbName);
    return false;
  }

  if (strcmp(savedPsw, DbPsw) != 0) {
    printf("Xeta: Parol yalnisdir! Baglanti rədd edildi.\n", DbName);
    return false;
  }

  strncpy(current_db_name, DbName, 17);
  snprintf(current_db_path, sizeof(current_db_path), "%s/%s", MASTER_DIR,
           DbName);
  printf("Ugurlu: '%s' bazasina baglanildi.\n", DbName);
  return true;
}

// ====================================================================
// DISCONNECT DATABASE
// ====================================================================
bool disConnectDb(char *DbName) {
  if (strlen(current_db_name) == 0 || strcmp(current_db_name, DbName) != 0) {
    printf("Xeberdarlig: Siz onsuz da '%s' bazasina bagli deyilsiniz.\n",
           DbName);
    return false;
  }
  memset(current_db_path, 0, sizeof(current_db_path));
  memset(current_db_name, 0, sizeof(current_db_name));
  printf("Ugurlu: '%s' verilenebler bazasindan ayrildiniz.\n", DbName);
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
        printf("%-20s\t%-20s\n", reg.db_name, reg.db_psw);
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
bool helperCheckDbExists(char *DbName, char *outPsw, long *outOffset) {
  FILE *f = fopen(MASTER_FILE, "rb");
  if (!f)
    return false;

  DBRegistry reg;
  long currentOffset = 0;
  while (fread(&reg, sizeof(DBRegistry), 1, f)) {
    if (reg.is_deleted == 0 && strcmp(reg.db_name, DbName) == 0) {
      if (outPsw)
        strcpy(outPsw, reg.db_psw);
      if (outOffset)
        *outOffset = currentOffset;
      fclose(f);
      return true; // Tapıldı və aktivdir
    }
    currentOffset += sizeof(DBRegistry);
  }
  fclose(f);
  return false; // Tapılmadı və ya silinib
}

// #endif