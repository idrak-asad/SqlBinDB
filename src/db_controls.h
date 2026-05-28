#define MASTER_DIR "sqlBinDB"
#define MASTER_FILE "sqlBinDB/master_dbs.db"

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
  _mkdir(MASTER_DIR);
  // Əgər fayl yoxdursa, yaradıb bağlasın
  FILE *f = fopen(MASTER_FILE, "ab");
  if (f)
    fclose(f);
}

// ====================================================================
// CREATE DATABASE
// ====================================================================
bool createDb(const char *dbName, const char *password, bool reCreate) {
    // 1. Əsas verilənlər bazası qovluğunu platformaya uyğun yaradırıq
    platform_create_dir(dbName);

    // 2. Alt metadata və tables qovluqlarını yaradırıq
    char metaFolder[256], tablesFolder[256];
    snprintf(metaFolder, sizeof(metaFolder), "%s/metadata", dbName);
    snprintf(tablesFolder, sizeof(tablesFolder), "%s/tables", dbName);
    
    platform_create_dir(metaFolder);
    platform_create_dir(tablesFolder);

    // 3. Master master_dbs.db qeydiyyat faylını yaradırıq
    char masterPath[256];
    platform_format_path(masterPath, sizeof(masterPath), dbName, "", "master_dbs.db");
    
    FILE *f = fopen(masterPath, "ab+");
    if(f) fclose(f);
    
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