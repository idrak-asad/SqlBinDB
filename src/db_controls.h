#define MASTER_DIR "/sqlBinDB"
#define MASTER_FILE "/sqlBinDB/master_dbs.db"

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
void initSystem() {

    // LittleFS mount
    if(!LittleFS.begin(true)){
        printf("LittleFS mount xetasi!\n");
        return;
    }

    // əsas qovluq
    platform_create_dir(MASTER_DIR);

    // master faylı yoxdursa yarat
    FILE *f = fopen(MASTER_FILE, "rb");

    if(!f){

        f = fopen(MASTER_FILE, "wb");

        if(!f){
            printf("MASTER FILE yaradila bilmedi!\n");
            return;
        }

        printf("MASTER FILE yaradildi.\n");
    }

    if(f)
        fclose(f);
}

// ====================================================================
// CREATE DATABASE
// ====================================================================
bool createDb(const char *dbName, const char *password, bool reCreate) {

    // əvvəl yoxla database artıq varmı
    char dummyPsw[18];
    long dummyOffset;

    if(helperCheckDbExists((char*)dbName, dummyPsw, &dummyOffset)){
        printf("Xeta: '%s' bazasi artıq movcuddur!\n", dbName);
        return false;
    }

    // əsas sistem qovluğu
    platform_create_dir(MASTER_DIR);

    // database qovluğu
    char dbPath[256];
    snprintf(dbPath, sizeof(dbPath), "%s/%s", MASTER_DIR, dbName);

    platform_create_dir(dbPath);

    // metadata qovluğu
    char metaFolder[256];
    snprintf(metaFolder, sizeof(metaFolder), "%s/metadata", dbPath);

    platform_create_dir(metaFolder);

    // tables qovluğu
    char tablesFolder[256];
    snprintf(tablesFolder, sizeof(tablesFolder), "%s/tables", dbPath);

    platform_create_dir(tablesFolder);

    // registry əlavə et
    DBRegistry reg;
    memset(&reg, 0, sizeof(DBRegistry));

    reg.is_deleted = 0;

    strncpy(reg.db_name, dbName, sizeof(reg.db_name)-1);
    strncpy(reg.db_psw, password, sizeof(reg.db_psw)-1);

    FILE *master = fopen(MASTER_FILE, "ab");

    if(!master){
        printf("Xeta: master registry acilmadi!\n");
        return false;
    }

    fwrite(&reg, sizeof(DBRegistry), 1, master);

    fclose(master);

    printf("Baza strukturu ugurla hazirlandi (%s)\n", dbName);

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