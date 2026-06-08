
// table_controls.h
#ifndef TABLE_CONTROLS_H
#define TABLE_CONTROLS_H

// #include "add_controls.h"

bool createTable(char *tableName, char *columnNames[], char *columnTypes[],
                 char *Constraints[], bool reCreate, int max_rows);
// bool dropTable(const char *tableName);
bool dropTable(const char *tableName, int hardDrop);
void selectTables(const char *tableName);

// ====================================================================
// 2. DROP TABLE
// ====================================================================
bool dropTable(const char *tableName, int hardDrop)
{
  if (hardDrop == 0)
  {
    printf("Melumat: hardDrop = 0 oldugu ucun silinme legv edildi.\n");
    return false;
  }

  if (strlen(current_db_path) == 0)
    return false;

  // 1. Sıra nömrəsini (Index) tapırıq
  uint8_t targetTableIndex = getTableIndexByName(tableName);
  if (targetTableIndex == 0)
  {
    printf("Xeta: '%s' adli cedvel tapilmadi.\n", tableName);
    return false;
  }

  char tableFilePath[256], tablesMetaPath[256], relPath[256];
  snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);
  snprintf(tablesMetaPath, sizeof(tablesMetaPath), "%s/metadata/tables.db", current_db_path);
  snprintf(relPath, sizeof(relPath), "%s/metadata/relations.db", current_db_path);

  // 2. CASCADE DROP: Əlaqəli uşaq cədvəlləri tap və sil
  FILE *fRel = fopen(relPath, "rb+");
  if (fRel)
  {
    CompactRelation rel;
    long offset = 0;
    while (fread(&rel, sizeof(CompactRelation), 1, fRel))
    {
      // Əgər bu cədvəl başqa cədvəlin "valideynidirsə"
      if (rel.is_deleted == 0 && rel.parent_table_id == targetTableIndex)
      {
        char childTableName[MAX_NAME_LEN];
        // Index əsaslı ad tapma funksiyası
        if (getTableNameByIndex(rel.child_table_id, childTableName))
        {
          printf("CASCADE: '%s' silindiyi ucun bagli olan '%s' (Index:%d) silinir...\n",
                 tableName, childTableName, rel.child_table_id);
          dropTable(childTableName, 1); // Rekursiv silinmə
        }

        // Relasiyanı "deleted" kimi işarələ
        fseek(fRel, offset, SEEK_SET);
        uint8_t delFlag = 1;
        fwrite(&delFlag, 1, 1, fRel);
      }
      offset += sizeof(CompactRelation);
    }
    fclose(fRel);
  }

  // 3. Metadata: Cədvəli "soft-delete" et
  FILE *fTables = fopen(tablesMetaPath, "rb+");
  if (fTables)
  {
    CompactTableMeta tMeta;
    long offset = 0;
    uint8_t currentIndex = 0;

    while (fread(&tMeta, sizeof(CompactTableMeta), 1, fTables))
    {
      currentIndex++;
      if (tMeta.is_deleted == 0 && currentIndex == targetTableIndex)
      {
        fseek(fTables, offset, SEEK_SET);
        uint8_t delFlag = 1;
        fwrite(&delFlag, 1, 1, fTables); // İndeksə uyğun sətri sil
        printf("Sistem: '%s' (Index:%d) uğurla silindi.\n", tableName, targetTableIndex);
        break;
      }
      offset += sizeof(CompactTableMeta);
    }
    fclose(fTables);
  }

  return true;
}

bool dropTableHard(const char *tableName)
{
  if (strlen(current_db_path) == 0)
    return false;

  // 1. Sıra nömrəsini (Index) tapırıq
  uint8_t targetTableIndex = getTableIndexByName(tableName);
  if (targetTableIndex == 0)
  {
    printf("Xeta: '%s' adli cedvel tapilmadi.\n", tableName);
    return false;
  }

  char tableFilePath[256], tablesMetaPath[256], relPath[256], colMetaPath[256];
  snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);
  snprintf(tablesMetaPath, sizeof(tablesMetaPath), "%s/metadata/tables.db", current_db_path);
  snprintf(relPath, sizeof(relPath), "%s/metadata/relations.db", current_db_path);
  snprintf(colMetaPath, sizeof(colMetaPath), "%s/metadata/columns.db", current_db_path);

  // 2. CASCADE DROP: Əlaqəli uşaq cədvəlləri tap və sil
  FILE *fRel = fopen(relPath, "rb+");
  if (fRel)
  {
    CompactRelation rel;
    long offset = 0;
    while (fread(&rel, sizeof(CompactRelation), 1, fRel))
    {
      // Əgər bu cədvəl başqa cədvəlin "valideynidirsə"
      if (rel.is_deleted == 0 && rel.parent_table_id == targetTableIndex)
      {
        char childTableName[MAX_NAME_LEN];
        // Index əsaslı ad tapma funksiyası
        if (getTableNameByIndex(rel.child_table_id, childTableName))
        {
          printf("CASCADE: '%s' silindiyi ucun bagli olan '%s' (Index:%d) silinir...\n",
                 tableName, childTableName, rel.child_table_id);
          dropTable(childTableName, 1); // Rekursiv silinmə
        }

        // Relasiyanı "deleted" kimi işarələ
        fseek(fRel, offset, SEEK_SET);
        uint8_t delFlag = 1;
        fwrite(&delFlag, 1, 1, fRel);
      }
      offset += sizeof(CompactRelation);
    }
    fclose(fRel);
  }

  // 3. Metadata: Cədvəli "soft-delete" et
  FILE *fTables = fopen(tablesMetaPath, "rb+");
  if (fTables)
  {
    CompactTableMeta tMeta;
    long offset = 0;
    uint8_t currentIndex = 0;

    while (fread(&tMeta, sizeof(CompactTableMeta), 1, fTables))
    {
      currentIndex++;
      if (tMeta.is_deleted == 0 && currentIndex == targetTableIndex)
      {
        fseek(fTables, offset, SEEK_SET);
        uint8_t delFlag = 1;
        fwrite(&delFlag, 1, 1, fTables); // İndeksə uyğun sətri sil
        printf("Sistem: '%s' (Index:%d) uğurla silindi.\n", tableName, targetTableIndex);
        break;
      }
      offset += sizeof(CompactTableMeta);
    }
    fclose(fTables);
  }

  FILE *fCols = fopen(colMetaPath, "rb+");
if (fCols) {
    // CompactColumnMeta cMeta;
    // long offset = 0;
    // int count = 0;
    // while (count<10) {
    //   fread(&cMeta, sizeof(CompactColumnMeta), 1, fCols);
    //   count++;
    //     // Cədvəl ID-si uyğundursa və hələ silinməyibsə
    //     printf("baxılır '%s' table name.\n", cMeta.table_id);
    //     if (cMeta.is_deleted == 0 && cMeta.table_id == targetTableIndex) {
    //         cMeta.is_deleted = 1; // Struct-ı yenilə
    //         fseek(fCols, offset, SEEK_SET);
    //         fwrite(&cMeta, sizeof(CompactColumnMeta), 1, fCols); // Bütün strukturu yaz
    //         printf("CASCADE: Columns metadata-dan '%s' silindi.\n", cMeta.table_id);
    //     }
    //     offset += sizeof(CompactColumnMeta);
    // }
    // fclose(fCols);

    CompactColumnMeta cMeta;
long offset = 0;
// Faylın əvvəlinə qayıtdığımızdan əmin olun
fseek(fCols, 0, SEEK_SET);

// Faylı strukturu oxuyaraq dövrə salırıq
while (fread(&cMeta, sizeof(CompactColumnMeta), 1, fCols) == 1) {
    
    // Debug: Sütun adını deyil, yalnız ID-ləri çap edirik
    printf("Baxılır: Table ID: %d, Type ID: %d\n", cMeta.table_id, cMeta.type_id);

    if (cMeta.is_deleted == 0 && cMeta.table_id == targetTableIndex) {
        cMeta.is_deleted = 1; 

        // Fayl göstəricisini cari strukturun başladığı yerə çəkirik
        fseek(fCols, offset, SEEK_SET);
        
        // Yenilənmiş strukturu bütövlükdə yazırıq
        fwrite(&cMeta, sizeof(CompactColumnMeta), 1, fCols);
        
        // Yazdıqdan sonra fayl göstəricisini növbəti strukturun başına qaytarırıq
        fseek(fCols, offset + sizeof(CompactColumnMeta), SEEK_SET);
        
        printf("CASCADE: Columns metadata-dan Table ID: %d olan sütun silindi.\n", cMeta.table_id);
    }
    
    offset += sizeof(CompactColumnMeta);
}
fclose(fCols);
}
  if (remove(tableFilePath) == 0)
  {
    printf("Sistem: '%s.db' faylı fiziki olaraq silindi.\n", tableName);
  }
  else
  {
    printf("Xəta: '%s.db' faylı fiziki olaraq silinə bilmədi!\n", tableName);
  }

  return true;
}

// ====================================================================
// 1. CREATE TABLE
// ====================================================================
/**
 * Binar relyasiyalı verilənlər bazasında yeni bir cədvəl (fayl) yaradır.
 * Windows, Linux və ESP32 platformalarını tam dəstəkləyir.
 */
bool createTable(char *tableName, char *columnNames[], char *columnTypes[],
                 char *Constraints[], bool reCreate, int max_rows)
{
  if (strlen(current_db_path) == 0)
    return false;

  char tableFilePath[256];
  snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

  // Əgər reCreate false-dursa və cədvəl varsa toxunma
  FILE *chk = fopen(tableFilePath, "rb");
  if (chk && !reCreate)
  {
    fclose(chk);
    return true;
  }
  if (chk)
    fclose(chk);

  FILE *file = fopen(tableFilePath, "wb+");
  if (!file)
    return false;

  DBHeader header;
  header.maxRows = max_rows;
  header.rowCount = 0;
  header.last_inserted_id = 0; // İnitial auto_increment

  ColumnConfig configs[MAX_COLUMNS + 1];

  // 0-cı sütun hər zaman is_deleted qoruyucusudur
  strcpy(configs[0].columnName, "is_deleted");
  configs[0].typeID = TYPE_UINT8;
  configs[0].dataSize = 1;
  configs[0].constraints = 0;

  int colIdx = 1;
  uint16_t calculatedRowSize = 1; // is_deleted üçün 1 bayt ilə başlayır

  while (columnNames[colIdx - 1] != NULL && colIdx <= MAX_COLUMNS)
  {
    strncpy(configs[colIdx].columnName, columnNames[colIdx - 1], MAX_NAME_LEN);

    // Məhdudiyyətlərin binar maskaya çevrilməsi
    configs[colIdx].constraints = 0;
    if (Constraints && Constraints[colIdx - 1])
    {
      if (strstr(Constraints[colIdx - 1], "PRIMARY KEY"))
        configs[colIdx].constraints |= FLAG_PRIMARY_KEY;
      if (strstr(Constraints[colIdx - 1], "NOT NULL"))
        configs[colIdx].constraints |= FLAG_NOT_NULL;
      if (strstr(Constraints[colIdx - 1], "UNIQUE"))
        configs[colIdx].constraints |= FLAG_UNIQUE;
      if (strstr(Constraints[colIdx - 1], "AUTO_INCREMENT"))
        configs[colIdx].constraints |= FLAG_AUTO_INCREMENT;
    }

    // Tiplərin tənzimlənməsi və ölçülərinin hesablanması
    if (strcmp(columnTypes[colIdx - 1], "INT") == 0 || (configs[colIdx].constraints & FLAG_AUTO_INCREMENT))
    {
      configs[colIdx].typeID = TYPE_INT;
      configs[colIdx].dataSize = 4;
    }
    else if (strcmp(columnTypes[colIdx - 1], "FLOAT") == 0)
    {
      configs[colIdx].typeID = TYPE_FLOAT;
      configs[colIdx].dataSize = 4;
    }
    else if (strcmp(columnTypes[colIdx - 1], "FIXED_POINT") == 0)
    {
      configs[colIdx].typeID = TYPE_FIXED_POINT;
      configs[colIdx].dataSize = 2; // Cəmi 2 bayt yer tutur!
    }
    else if (strcmp(columnTypes[colIdx - 1], "DATETIME") == 0)
    {
      configs[colIdx].typeID = TYPE_DATETIME;
      configs[colIdx].dataSize = sizeof(BinaryDateTime); // 8 bayt
    }
    else if (strcmp(columnTypes[colIdx - 1], "TIMESTAMP") == 0)
    {
      configs[colIdx].typeID = TYPE_TIMESTAMP;
      configs[colIdx].dataSize = 4; // 4 bayt Unix Epoch saniyəsi
    }
    else if (strncmp(columnTypes[colIdx - 1], "CHAR2", 5) == 0)
    {
      configs[colIdx].typeID = TYPE_CHAR2;

      int customSize;
      // columnTypes-ın daxilindən "CHAR2(%d)" formatında rəqəmi oxumağa çalışır
      if (sscanf(columnTypes[colIdx - 1], "CHAR2(%d)", &customSize) == 1)
      {
        configs[colIdx].dataSize = customSize;
      }
      else
      {
        // Əgər mötərizədə rəqəm tapılmasa (məsələn, sadəcə "CHAR2" göndərilibsə), default olaraq MAX_CHAR yazır
        configs[colIdx].dataSize = MAX_CHAR;
// Serial.println("char uzunluğu maxsimum seçildi");
#if defined(TARGET_PLATFORM_ESP32)
        Serial.println("\nchar uzunluğu maxsimum seçildi");
#else
        printf("\nchar uzunluğu maxsimum seçildi\n");
#endif
      }
    }
    else if (strcmp(columnTypes[colIdx - 1], "VARCHAR2") == 0)
    {
      configs[colIdx].typeID = TYPE_VARCHAR2;
      configs[colIdx].dataSize = 4; // Mərkəzi cədvəldə sadəcə 4 bayt fayl pointeri tutur!
    }
    else
    {
      configs[colIdx].typeID = TYPE_UINT8;
      configs[colIdx].dataSize = 1;
// Serial.println("type təyin edilmədi. standart təyin edildi.");
#if defined(TARGET_PLATFORM_ESP32)
      Serial.println("\ntype təyin edilmədi. standart təyin edildi.");
#else
      printf("\ntype təyin edilmədi. standart təyin edildi.\n");
#endif
    }

    calculatedRowSize += configs[colIdx].dataSize;
    colIdx++;
  }

  header.columnCount = colIdx;
  header.rowSize = calculatedRowSize;

  // Binar olaraq strukturu disklərə basırıq
  fwrite(&header, sizeof(DBHeader), 1, file);
  fwrite(configs, sizeof(ColumnConfig), header.columnCount, file);
  fclose(file);

  // ... (əvvəlki kod: header və configs yazıldıqdan sonra) ...
  fclose(file);

  // 1. Tables.db-yə qeydiyyat (sqlBinDB/metadata/tables.db)
  // --- METADATA YAZMA PROSESİ ---

  // 1. Tables.db faylından hazırkı cədvəl sayını tap (ID = say + 1)
  char tablesMetaPath[256];
  snprintf(tablesMetaPath, sizeof(tablesMetaPath), "%s/metadata/tables.db", current_db_path);

  // ID hesablamaq üçün fayl ölçüsünü yoxlayırıq
  FILE *fTmp = fopen(tablesMetaPath, "rb");
  uint8_t newTableId = 1;
  if (fTmp)
  {
    fseek(fTmp, 0, SEEK_END);
    long size = ftell(fTmp);
    newTableId = (uint8_t)((size / sizeof(CompactTableMeta)) + 1);
    fclose(fTmp);
  }

  // 2. Tables.db-yə yaz
  FILE *fTable = fopen(tablesMetaPath, "ab+");
  if (fTable)
  {
    CompactTableMeta tMeta = {0};
    tMeta.is_deleted = 0;
    // tMeta.table_id = newTableId; // Ardıcıl ID
    strncpy(tMeta.table_name, tableName, 32);
    tMeta.col_count = header.columnCount - 1;
    tMeta.max_rows = header.maxRows;

    fwrite(&tMeta, sizeof(CompactTableMeta), 1, fTable);
    fclose(fTable);
  }

  // 3. Columns.db-yə yaz (Eyni ID ilə əlaqələndiririk)
  char columnsMetaPath[256];
  snprintf(columnsMetaPath, sizeof(columnsMetaPath), "%s/metadata/columns.db", current_db_path);
  FILE *fCol = fopen(columnsMetaPath, "ab+");
  if (fCol)
  {
    for (int i = 1; i < header.columnCount; i++)
    {
      CompactColumnMeta cMeta = {0};
      cMeta.is_deleted = 0;
      cMeta.table_id = newTableId; // tables.db-də yaratdığımız ID
      strncpy(cMeta.column_name, configs[i].columnName, MAX_NAME_LEN);
      cMeta.type_id = configs[i].typeID;
      cMeta.data_size = configs[i].dataSize;
      cMeta.constraints = configs[i].constraints;

      fwrite(&cMeta, sizeof(CompactColumnMeta), 1, fCol);
    }
    fclose(fCol);
  }

  return true;
}

// ====================================================================
// 3. SELECT TABLES
// ====================================================================
void selectTables(const char *tableName)
{
  if (strlen(current_db_path) == 0)
  {
    printf("XETA: Evvelce bir bazaya qoshulun!\n");
    return;
  }

  char tablesMetaPath[256], columnsMetaPath[256];
  snprintf(tablesMetaPath, sizeof(tablesMetaPath), "%s/metadata/tables.db", current_db_path);
  snprintf(columnsMetaPath, sizeof(columnsMetaPath), "%s/metadata/columns.db", current_db_path);

  // 1. Bütün cədvəlləri listələmək (tableName == "*")
  if (strcmp(tableName, "*") == 0)
  {
    FILE *f = fopen(tablesMetaPath, "rb");
    if (!f)
    {
      printf("Bu bazada cadval yaradilmariq.\n");
      return;
    }

    printf("\n=== BAZASINDAKI CADVELLER ===\n");
    printf("%-10s \t %-20s \t %-10s \t %-10s\n", "INDEX", "NAME", "COLS", "MAX ROWS");
    printf("-----------------------------------------------------------------\n");

    CompactTableMeta tMeta;
    uint8_t row_number = 0;
    while (fread(&tMeta, sizeof(CompactTableMeta), 1, f))
    {
      row_number++; // Fiziki sıra nömrəsi (Index)
      if (tMeta.is_deleted == 0)
      {
        printf("%-10d \t %-20s \t %-10d \t %-10d\n", row_number, tMeta.table_name, tMeta.col_count, tMeta.max_rows);
      }
    }
    fclose(f);
    printf("================================================================-\n\n");
    return;
  }

  // 2. Müəyyən bir cədvəlin sxemini göstərmək
  FILE *fTables = fopen(tablesMetaPath, "rb");
  if (!fTables)
    return;

  CompactTableMeta tMeta;
  uint8_t targetTableIndex = 0;
  uint8_t currentIndex = 0;

  // Cədvəlin indeksini tapırıq
  while (fread(&tMeta, sizeof(CompactTableMeta), 1, fTables))
  {
    currentIndex++;
    if (tMeta.is_deleted == 0 && strcmp(tMeta.table_name, tableName) == 0)
    {
      targetTableIndex = currentIndex;
      break;
    }
  }
  fclose(fTables);

  if (targetTableIndex == 0)
  {
    printf("XETA: '%s' adinda cadval tapilmadi!\n", tableName);
    return;
  }

  // 3. İndeksə uyğun sütunları çap et
  FILE *fCols = fopen(columnsMetaPath, "rb");
  if (!fCols)
    return;

  printf("\n=== CADVEL SXEMI: %s (Table Index: %d) ===\n", tableName, targetTableIndex);
  printf("%-20s \t %-8s \t %-6s \t %-30s\n", "COLUMN NAME", "TYPE ID", "SIZE", "CONSTRAINTS");
  printf("---------------------------------------------------------------------\n");

  CompactColumnMeta cMeta;
  while (fread(&cMeta, sizeof(CompactColumnMeta), 1, fCols))
  {
    // cMeta.table_id artıq Index-i saxlayır
    if (cMeta.is_deleted == 0 && cMeta.table_id == targetTableIndex)
    {
      printf("%-20s \t %-8d \t %-6d \t [ ", cMeta.column_name, cMeta.type_id, cMeta.data_size);
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
  printf("=====================================================================\n\n");
}

#endif