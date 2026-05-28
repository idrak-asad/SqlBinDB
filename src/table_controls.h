

bool createTable(char *tableName, char *columnNames[], char *columnTypes[],
                 char *Constraints[], bool reCreate, int max_rows);
// bool dropTable(const char *tableName);
bool dropTable(const char *tableName, int hardDrop);
void selectTables(char *tableName);


// ====================================================================
// 2. DROP TABLE
// ====================================================================
bool dropTable(const char *tableName, int hardDrop) {
  if (hardDrop == 0) {
    printf(
        "Melumat: hardDrop = 0 oldugu ucun cedvelin silinmesi legv edildi.\n");
    return false;
  }

  if (strlen(current_db_path) == 0)
    return false;

  uint8_t targetTableId = getTableIdByName(tableName);
  if (targetTableId == 0)
    return false;

  char tableFilePath[256], tablesMetaPath[256], relPath[256];
  snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db",
           current_db_path, tableName);
  snprintf(tablesMetaPath, sizeof(tablesMetaPath), "%s/metadata/tables.db",
           current_db_path);
  snprintf(relPath, sizeof(relPath), "%s/metadata/relations.db",
           current_db_path);

  // QAYDA 1: CASCADE DROP (hardDrop == 1)
  if (hardDrop == 1) {
    FILE *fRel = fopen(relPath, "rb+");
    if (fRel) {
      CompactRelation rel;
      long offset = 0;
      while (fread(&rel, sizeof(CompactRelation), 1, fRel)) {
        if (rel.is_deleted == 0 && rel.parent_table_id == targetTableId) {
          // Bu cədvələ bağlı bir child cədvəl var! Onu da tamamilə silirik
          // (Zəncirvari Drop) Real proqramda child_table_id-yə görə cədvəlin
          // adı tables.db-dən tapılıb bura ötürülməlidir Nümunə olaraq uşaq
          // cədvəli də silirik:
          dropTable("devices", 1);

          // Relyasiyanın özünü də silinmiş elan edirik
          fseek(fRel, offset, SEEK_SET);
          uint8_t delFlag = 1;
          fwrite(&delFlag, 1, 1, fRel);
          fseek(fRel, offset + sizeof(CompactRelation), SEEK_SET);
        }
        offset += sizeof(CompactRelation);
      }
      fclose(fRel);
    }
  }

  // QAYDA 2: ORPHAN DROP (hardDrop == 2) -> Uşaq cədvəllərə toxunmuruq

  // tables.db-dən cədvəlin özünü soft-delete edirik
  FILE *tMetaFile = fopen(tablesMetaPath, "rb+");
  if (tMetaFile) {
    CompactTableMeta tMeta;
    long offset = 0;
    while (fread(&tMeta, sizeof(CompactTableMeta), 1, tMetaFile)) {
      if (tMeta.is_deleted == 0 && strcmp(tMeta.table_name, tableName) == 0) {
        fseek(tMetaFile, offset, SEEK_SET);
        uint8_t delFlag = 1;
        fwrite(&delFlag, 1, 1, tMetaFile);
        break;
      }
      offset += sizeof(CompactTableMeta);
    }
    fclose(tMetaFile);
  }

  // Fiziki faylı diskdən silirik
  remove(tableFilePath);
  printf("Ugurlu: '%s' cedveli sistemden silindi (Mode: %d).\n", tableName,
         hardDrop);
  return true;
}


// ====================================================================
// 1. CREATE TABLE
// ====================================================================
/**
 * Binar relyasiyalı verilənlər bazasında yeni bir cədvəl (fayl) yaradır.
 * Windows, Linux və ESP32 platformalarını tam dəstəkləyir.
 */
bool createTable(char *tableName, char *columnNames[], char *columnTypes[], char *Constraints[], bool reCreate, int max_rows) {
    if (strlen(current_db_path) == 0) {
        printf("XETA: Evvelce bir verilener bazasina qoshulun (connectDb)!\n");
        return false;
    }

    char tablePath[256];
    // Platformaya uyğun olaraq düzgün fayl yolunu generasiya edirik
    platform_format_path(tablePath, sizeof(tablePath), current_db_path, "tables", tableName);
    strcat(tablePath, ".db");

    // Əgər reCreate false-dursa və fayl artıq mövcuddursa, yenidən yaratmırıq (Datalar qorunur)
    if (!reCreate) {
        FILE *checkFile = fopen(tablePath, "rb");
        if (checkFile) {
            printf("Melumat: '%s' cedveli artıq mövcuddur. (reCreate = false, datalar qorundu)\n", tableName);
            fclose(checkFile);
            return true;
        }
    }

    // Cədvəl binar faylını yazma rejimində açırıq
    FILE *file = fopen(tablePath, "wb+");
    if (!file) {
        printf("XETA: Cedvel fayli yaradila bilmedi: %s\n", tablePath);
        return false;
    }

    // Gələn sütunların sayını hesablayaq
    int inputColumnCount = 0;
    while (columnNames[inputColumnCount] != NULL) {
        inputColumnCount++;
    }

    if (inputColumnCount > MAX_COLUMNS) {
        printf("XETA: Maksimum sutun sayi (%d) ashildi!\n", MAX_COLUMNS);
        fclose(file);
        return false;
    }

    // ====================================================================
    // 1. SÜTUNLARIN SXEMİNİN (SCHEMA) VƏ BAYT ÖLÇÜLƏRİNİN HESABLANMASI
    // ====================================================================
    ColumnConfig configs[MAX_COLUMNS + 1];
    uint16_t totalRowSize = 0;

    // A) Bütün cədvəllərin ən başına GİZLİ bir "is_deleted" sütunu qoyuruq (Soft-delete üçün)
    strcpy(configs[0].columnName, "is_deleted");
    strcpy(configs[0].columnType, "uint8_t");
    configs[0].byteSize = 1; // 1 bayt (0 = aktiv, 1 = silinib)
    totalRowSize += configs[0].byteSize;

    // B) İstifadəçinin göndərdiyi sütunları analiz edirik
    for (int i = 0; i < inputColumnCount; i++) {
        int targetIdx = i + 1; // 0-cı indeks gizli sütundur
        
        strncpy(configs[targetIdx].columnName, columnNames[i], 31);
        configs[targetIdx].columnName[31] = '\0';
        
        strncpy(configs[targetIdx].columnType, columnTypes[i], 15);
        configs[targetIdx].columnType[15] = '\0';

        // Tipə görə binar yaddaş ölçüsünü təyin edirik
        if (strcmp(columnTypes[i], "uint32_t") == 0 || strcmp(columnTypes[i], "int") == 0) {
            configs[targetIdx].byteSize = 4;
        } else if (strcmp(columnTypes[i], "uint8_t") == 0 || strcmp(columnTypes[i], "bool") == 0) {
            configs[targetIdx].byteSize = 1;
        } else if (strcmp(columnTypes[i], "float") == 0) {
            configs[targetIdx].byteSize = 4;
        } else if (strncmp(columnTypes[i], "char(", 5) == 0) {
            // "char(10)" kimi tiplərdən mötərizə daxilindəki ölçünü çıxarırıq
            int charSize = atoi(columnTypes[i] + 5);
            configs[targetIdx].byteSize = (charSize > 0) ? charSize : 10;
        } else {
            // Standart olaraq 4 bayt təyin edirik
            configs[targetIdx].byteSize = 4;
        }

        totalRowSize += configs[targetIdx].byteSize;
    }

    // Toplam real sütun sayı (Gizli sütun + Giriş sütunları)
    int totalColumnCount = inputColumnCount + 1;

    // ====================================================================
    // 2. CƏDVƏLİN BİNAR BAŞLIĞININ (DBHeader) HAZIRLANMASI
    // ====================================================================
    DBHeader header;
    header.columnCount = totalColumnCount;
    header.rowSize = totalRowSize;
    header.maxRows = max_rows;
    header.rowCount = 0;
    header.nextRowIndex = 0; // Dairəvi bufer üçün yazma mövqeyi başlanğıcı

    // ====================================================================
    // 3. DATANIN DİSKƏ (FÖLDERƏ) YAZILMASI
    // ====================================================================
    // A) Əvvəlcə DBHeader strukturunu yazırıq
    fwrite(&header, sizeof(DBHeader), 1, file);

    // B) Sonra sütun konfiqurasiyalarını (Sxemi) yazırıq
    fwrite(configs, sizeof(ColumnConfig), totalColumnCount, file);

    // C) Boş sətirlər üçün diskdə öncədən yer ayırırıq (Pre-allocation)
    // Bu addım ESP32-də Flash yaddaşın fraqmentasiya olmasının qarşısını alır.
    uint8_t *emptyRow = (uint8_t *)calloc(1, totalRowSize);
    if (emptyRow) {
        for (int i = 0; i < max_rows; i++) {
            fwrite(emptyRow, totalRowSize, 1, file);
        }
        free(emptyRow);
    }

    fclose(file);

    // ====================================================================
    // 4. METADATA QEYDİYYATI (tables.db yenilənməsi)
    // ====================================================================
    // Hər yeni cədvəl yarananda onun adını və unikal ID-sini metadata bazasına qeyd edirik
    char tablesMetaPath[256];
    platform_format_path(tablesMetaPath, sizeof(tablesMetaPath), current_db_path, "metadata", "tables.db");

    FILE *fMeta = fopen(tablesMetaPath, "ab+");
    if (fMeta) {
        // Mövcud cədvəl sayını tapıb yeni ID veririk
        fseek(fMeta, 0, SEEK_END);
        long fileSize = ftell(fMeta);
        uint8_t newTableId = (fileSize / 33) + 1; // Hər qeyd 33 baytdır (1 bayt ID + 32 bayt ad)

        uint8_t activeFlag = 1; // 1 = Aktiv cədvəl
        fwrite(&activeFlag, 1, 1, fMeta);
        fwrite(&newTableId, 1, 1, fMeta);
        
        char nameBuf[31] = {0};
        strncpy(nameBuf, tableName, 30);
        fwrite(nameBuf, 31, 1, fMeta);
        
        fclose(fMeta);
    }

    printf("Ugurlu: '%s' cedveli binar olaraq yaradildi. (Setir olchusu: %d bayt, Maksimum setir: %d)\n", tableName, totalRowSize, max_rows);
    return true;
}


// ====================================================================
// 3. SELECT TABLES
// ====================================================================
void selectTables(char *tableName) {
  if (strlen(current_db_path) == 0) {
    printf("XETA: Evvelce bir bazaya qoshulun!\n");
    return;
  }

  char tablesMetaPath[256], columnsMetaPath[256];
  snprintf(tablesMetaPath, sizeof(tablesMetaPath), "%s/metadata/tables.db",
           current_db_path);
  snprintf(columnsMetaPath, sizeof(columnsMetaPath), "%s/metadata/columns.db",
           current_db_path);

  if (strcmp(tableName, "*") == 0) {
    FILE *f = fopen(tablesMetaPath, "rb");
    if (!f) {
      printf("Bu bazada cadval yaradilmariq.\n");
      return;
    }
    printf("\n=== BAZASINDAKI CADVELLER ===\n");
    printf("%-10s \t %-20s \t %-10s \t %-10s\n", "ID", "NAME", "COLS",
           "MAX ROWS");
    printf(
        "-----------------------------------------------------------------\n");
    CompactTableMeta tMeta;
    while (fread(&tMeta, sizeof(CompactTableMeta), 1, f)) {
      if (tMeta.is_deleted == 0) {
        printf("%-10d \t %-20s \t %-10d \t %-10d\n", tMeta.table_id,
               tMeta.table_name, tMeta.col_count, tMeta.max_rows);
      }
    }
    fclose(f);
    printf("================================================================-"
           "\n\n");
    return;
  }

  if (strlen(tableName) == 1) {
    printf("Hazirda aktiv Yol: %s\n", current_db_path);
    return;
  }

  FILE *fTables = fopen(tablesMetaPath, "rb");
  if (!fTables)
    return;

  CompactTableMeta tMeta;
  uint8_t targetTableId = 0;
  while (fread(&tMeta, sizeof(CompactTableMeta), 1, fTables)) {
    if (tMeta.is_deleted == 0 && strcmp(tMeta.table_name, tableName) == 0) {
      targetTableId = tMeta.table_id;
      break;
    }
  }
  fclose(fTables);

  if (targetTableId == 0) {
    printf("XETA: '%s' adinda cadval tapilmadi!\n", tableName);
    return;
  }

  FILE *fCols = fopen(columnsMetaPath, "rb");
  if (!fCols)
    return;

  printf("\n=== CADVEL SXEMI: %s (Table ID: %d) ===\n", tableName,
         targetTableId);
  printf("%-20s \t %-8s \t %-6s \t %-30s\n", "COLUMN NAME", "TYPE ID", "SIZE",
         "CONSTRAINTS");
  printf("---------------------------------------------------------------------"
         "-\n");

  CompactColumnMeta cMeta;
  while (fread(&cMeta, sizeof(CompactColumnMeta), 1, fCols)) {
    if (cMeta.is_deleted == 0 && cMeta.table_id == targetTableId) {
      printf("%-20s \t %-8d \t %-6d \t [ ", cMeta.column_name, cMeta.type_id,
             cMeta.type_size);
      if (cMeta.constraints & FLAG_PRIMARY_KEY)
        printf("PRIMARY_KEY ");
      if (cMeta.constraints & FLAG_NOT_NULL)
        printf("NOT_NULL ");
      if (cMeta.constraints & FLAG_UNIQUE)
        printf("UNIQUE ");
      printf("]\n");
    }
  }
  fclose(fCols);
  printf("====================================================================="
         "=\n\n");
}