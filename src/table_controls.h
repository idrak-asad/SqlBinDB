
// table_controls.h
#ifndef TABLE_CONTROLS_H
#define TABLE_CONTROLS_H

// #include "add_controls.h"



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
        printf("Melumat: hardDrop = 0 oldugu ucun silinme legv edildi.\n");
        return false;
    }

    if (strlen(current_db_path) == 0) return false;
    uint8_t targetTableId = getTableIdByName(tableName);
    if (targetTableId == 0) return false;

    char tableFilePath[256], tablesMetaPath[256], relPath[256];
    snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);
    snprintf(tablesMetaPath, sizeof(tablesMetaPath), "%s/metadata/tables.db", current_db_path);
    snprintf(relPath, sizeof(relPath), "%s/metadata/relations.db", current_db_path);

    // CASCADE DROP MEXANİZMİ (Dinamik Uşaq Cədvəllərin Tapılıb Silinməsi)
    FILE *fRel = fopen(relPath, "rb+");
    if (fRel) {
        CompactRelation rel;
        long offset = 0;
        while (fread(&rel, sizeof(CompactRelation), 1, fRel)) {
            if (rel.is_deleted == 0 && rel.parent_table_id == targetTableId) {
                char childTableName[MAX_NAME_LEN];
                if (getTableNameById(rel.child_table_id, childTableName)) {
                    printf("CASCADE: '%s' silindiyi ucun ona bagli olan '%s' cedveli de silinir...\n", tableName, childTableName);
                    dropTable(childTableName, 1); // Rekursiv olaraq uşaq cədvəli silirik
                }
                fseek(fRel, offset, SEEK_SET);
                uint8_t delFlag = 1;
                fwrite(&delFlag, 1, 1, fRel);
                fseek(fRel, offset + sizeof(CompactRelation), SEEK_SET);
            }
            offset += sizeof(CompactRelation);
        }
        fclose(fRel);
    }

    // Cədvəlin metadatada soft-delete edilməsi
    FILE *fTables = fopen(tablesMetaPath, "rb+");
    if (fTables) {
        CompactTableMeta tMeta;
        long offset = 0;
        while (fread(&tMeta, sizeof(CompactTableMeta), 1, fTables)) {
            if (tMeta.is_deleted == 0 && tMeta.table_id == targetTableId) {
                fseek(fTables, offset, SEEK_SET);
                uint8_t delFlag = 1;
                fwrite(&delFlag, 1, 1, fTables);
                break;
            }
            offset += sizeof(CompactTableMeta);
        }
        fclose(fTables);
    }

    // Fiziki faylların (`.db`, `.varchardb`) diskdən tamamilə silinməsi
    remove(tableFilePath);
    
    char varcharPath[256];
    snprintf(varcharPath, sizeof(varcharPath), "%s/tables/%s.varchardb", current_db_path, tableName);
    remove(varcharPath);

    // İndekslərin təmizlənməsi (index_controls.h funksiyası çağrılır)
    void resetTableIndexes(const char *tableName);
    resetTableIndexes(tableName);

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
                 char *Constraints[], bool reCreate, int max_rows) {
    if (strlen(current_db_path) == 0) return false;

    char tableFilePath[256];
    snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

    // Əgər reCreate false-dursa və cədvəl varsa toxunma
    FILE *chk = fopen(tableFilePath, "rb");
    if (chk && !reCreate) { fclose(chk); return true; }
    if (chk) fclose(chk);

    FILE *file = fopen(tableFilePath, "wb+");
    if (!file) return false;

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

    while (columnNames[colIdx - 1] != NULL && colIdx <= MAX_COLUMNS) {
        strncpy(configs[colIdx].columnName, columnNames[colIdx - 1], MAX_NAME_LEN);
        
        // Məhdudiyyətlərin binar maskaya çevrilməsi
        configs[colIdx].constraints = 0;
        if (Constraints && Constraints[colIdx - 1]) {
            if (strstr(Constraints[colIdx - 1], "PRIMARY KEY")) configs[colIdx].constraints |= FLAG_PRIMARY_KEY;
            if (strstr(Constraints[colIdx - 1], "NOT NULL"))    configs[colIdx].constraints |= FLAG_NOT_NULL;
            if (strstr(Constraints[colIdx - 1], "UNIQUE"))      configs[colIdx].constraints |= FLAG_UNIQUE;
            if (strstr(Constraints[colIdx - 1], "AUTO_INCREMENT")) configs[colIdx].constraints |= FLAG_AUTO_INCREMENT;
        }

        // Tiplərin tənzimlənməsi və ölçülərinin hesablanması
        if (strcmp(columnTypes[colIdx - 1], "INT") == 0 || (configs[colIdx].constraints & FLAG_AUTO_INCREMENT)) {
            configs[colIdx].typeID = TYPE_INT;
            configs[colIdx].dataSize = 4;
        } else if (strcmp(columnTypes[colIdx - 1], "FLOAT") == 0) {
            configs[colIdx].typeID = TYPE_FLOAT;
            configs[colIdx].dataSize = 4;
        } else if (strcmp(columnTypes[colIdx - 1], "FIXED_POINT") == 0) {
            configs[colIdx].typeID = TYPE_FIXED_POINT;
            configs[colIdx].dataSize = 2; // Cəmi 2 bayt yer tutur!
        } else if (strcmp(columnTypes[colIdx - 1], "DATETIME") == 0) {
            configs[colIdx].typeID = TYPE_DATETIME;
            configs[colIdx].dataSize = sizeof(BinaryDateTime); // 8 bayt
        } else if (strcmp(columnTypes[colIdx - 1], "TIMESTAMP") == 0) {
            configs[colIdx].typeID = TYPE_TIMESTAMP;
            configs[colIdx].dataSize = 4; // 4 bayt Unix Epoch saniyəsi
        } else if (strcmp(columnTypes[colIdx - 1], "CHAR2") == 0) {
            configs[colIdx].typeID = TYPE_CHAR2;
            configs[colIdx].dataSize = MAX_CHAR;
        } else if (strcmp(columnTypes[colIdx - 1], "VARCHAR2") == 0) {
            configs[colIdx].typeID = TYPE_VARCHAR2;
            configs[colIdx].dataSize = 4; // Mərkəzi cədvəldə sadəcə 4 bayt fayl pointeri tutur!
        } else {
            configs[colIdx].typeID = TYPE_UINT8;
            configs[colIdx].dataSize = 1;
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

    // Mərkəzi metadataya cədvəli qeyd edək
    char tablesMetaPath[256];
    snprintf(tablesMetaPath, sizeof(tablesMetaPath), "%s/metadata/tables.db", current_db_path);
    FILE *fMeta = fopen(tablesMetaPath, "ab+");
    if (fMeta) {
        CompactTableMeta tMeta = {0, (uint8_t)(rand() % 254 + 1)};
        strncpy(tMeta.table_name, tableName, MAX_NAME_LEN);
        fwrite(&tMeta, sizeof(CompactTableMeta), 1, fMeta);
        fclose(fMeta);
    }

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
             cMeta.data_size);
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


#endif