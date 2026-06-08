
#ifndef INDEX_CONTROLS_H
#define INDEX_CONTROLS_H

// #include "add_controls.h"

// İstifadəçinin manual olaraq istənilən sütuna indeks əlavə etməsi üçün funksiya
bool createIndex(const char *tableName, const char *columnName) {
    uint8_t tId = getTableIndexByName(tableName);
    uint8_t cId = getColumnIdByName(tId, columnName);
    if (tId == 0 || cId == 0) return false;

    char idxMetaPath[256];
    snprintf(idxMetaPath, sizeof(idxMetaPath), "%s/metadata/indexes.db", current_db_path);
    if (isColumnIndexed(tId, cId, NULL)) return true;

    FILE *f = fopen(idxMetaPath, "ab+");
    if (!f) return false;

    IndexMeta meta;
    meta.is_deleted = 0;
    meta.table_id = tId;
    meta.column_id = cId;
    snprintf(meta.index_name, sizeof(meta.index_name), "%s_%s.idx", tableName, columnName);

    fwrite(&meta, sizeof(IndexMeta), 1, f);
    fclose(f);

    char idxFilePath[256];
    snprintf(idxFilePath, sizeof(idxFilePath), "%s/tables/%s", current_db_path, meta.index_name);
    FILE *fIdx = fopen(idxFilePath, "wb");
    if (fIdx) fclose(fIdx);

    return true;
}

// Cədvəllər arası əlaqə qurulanda AVTOMATİK indeksləmə
bool createRelationWithAutoIndex(const char *parentTable, const char *parentCol, const char *childTable, const char *childCol) {
    // Öncə əlaqəni yaradırıq
    bool res = createRelation(parentTable, parentCol, childTable, childCol);
    if (res) {
        // HƏM valideyn həm uşaq sütunları avtomatik indekslənir!
        createIndex(parentTable, parentCol);
        createIndex(childTable, childCol);
    }
    return res;
}

// ====================================================================
// CRUD OPERASİYALARININ İNDEKS DƏSTƏKLİ GÜNCƏL VERSİYALARI
// ====================================================================

// INSERT: Hər dəfə yeni data yazılanda indeks faylına qeyd əlavə edilir
// Relyasiya və İndeks dəstəkli Tam Insert funksiyası
// bool insertRowsWithIndex(const char *tableName, void *dataPointer[], int dataCount) {
//     uint8_t thisTableId = getTableIndexByName(tableName);

//     // ====================================================================
//     // 1. FOREIGN KEY INTEGRITY CHECK (Valideyn Cədvəldə ID Yoxlanışı)
//     // ====================================================================
//     char relPath[256];
//     snprintf(relPath, sizeof(relPath), "%s/metadata/relations.db", current_db_path);
    
//     FILE *fRel = fopen(relPath, "rb");
//     if (fRel) {
//         CompactRelation rel;
//         while (fread(&rel, sizeof(CompactRelation), 1, fRel)) {
//             if (rel.is_deleted == 0 && rel.child_table_id == thisTableId) {
                
//                 // Biz child cədvəlik! Daxil edilən xarici ID dəyərini oxuyuruq
//                 uint32_t insertedFkVal = *(uint32_t *)dataPointer[rel.child_col_id - 1];

//                 // Valideyn cədvəlin faylını açırıq (Sadəlik üçün "users" fərz edirik)
//                 char parentTableName[64] = "users"; 
//                 char parentTableFilePath[256];
//                 snprintf(parentTableFilePath, sizeof(parentTableFilePath), "%s/tables/%s.db", current_db_path, parentTableName);

//                 FILE *fParent = fopen(parentTableFilePath, "rb");
//                 if (!fParent) {
//                     printf("XETA: Valideyn cedvel fayli (%s.db) tapilmadi!\n", parentTableName);
//                     fclose(fRel);
//                     return false;
//                 }

//                 DBHeader pHeader;
//                 fread(&pHeader, sizeof(DBHeader), 1, fParent);
//                 fseek(fParent, sizeof(DBHeader) + (sizeof(ColumnConfig) * pHeader.columnCount), SEEK_SET);

//                 uint8_t pRowBuffer[256];
//                 bool idExistsInParent = false;

//                 // Valideyn cədvəli sətir-sətir yoxlayırıq
//                 for (uint32_t pi = 0; pi < pHeader.rowCount; pi++) {
//                     fread(pRowBuffer, pHeader.rowSize, 1, fParent);
//                     if (pRowBuffer[0] == 1) continue; // Soft-delete olanları keçirik

//                     uint32_t parentIdVal = *(uint32_t *)(pRowBuffer + 1); // ID sütunu ofset 1
//                     if (parentIdVal == insertedFkVal) {
//                         idExistsInParent = true;
//                         break;
//                     }
//                 }
//                 fclose(fParent);

//                 // Əgər valideyn cədvəldə bu ID yoxdursa, İNSERT-İ BLOKLAYIRIQ!
//                 if (!idExistsInParent) {
//                     printf("FOREIGN KEY INTEGRITY VIOLATION: '%s' cedveline data daxil edile bilmez! ", tableName);
//                     printf("Cunki '%s' cedvelinde id = %u olan aktiv qeyd yoxdur!\n", parentTableName, insertedFkVal);
//                     fclose(fRel);
//                     return false;
//                 }
//             }
//         }
//         fclose(fRel);
//     }

//     // ====================================================================
//     // 2. DATANIN DİSKƏ YAZILMASI
//     // ====================================================================
//     // Datanı .db faylına fiziki olaraq vururuq (Və Auto-increment işləyir)
//     bool status = insertRows(tableName, dataPointer, dataCount); 
//     if (!status) return false;

//     // ====================================================================
//     // 3. AVTOMATİK BİNAR İNDEKS YENİLƏNMƏSİ (.idx faylları)
//     // ====================================================================
//     char tableFilePath[256];
//     snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);
//     FILE *file = fopen(tableFilePath, "rb");
//     if (!file) return false;

//     DBHeader header;
//     fread(&header, sizeof(DBHeader), 1, file);
//     fclose(file);

//     // Son yazılan sətirin fiziki bayt mövqeyini (File Position) hesablayırıq
//     uint32_t lastIdx = (header.nextRowIndex == 0) ? header.maxRows - 1 : header.nextRowIndex - 1;
//     uint32_t writePosition = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount) + (lastIdx * header.rowSize);

//     // Cədvəlin hansı sütunlarının indeksi varsa, hamısını yeniləyirik
//     for (uint8_t c = 1; c < header.columnCount; c++) {
//         char idxName[32];
//         if (isColumnIndexed(thisTableId, c, idxName)) {
//             char idxPath[256];
//             snprintf(idxPath, sizeof(idxPath), "%s/tables/%s", current_db_path, idxName);
            
//             FILE *fIdx = fopen(idxPath, "ab+");
//             if (fIdx) {
//                 IndexEntry entry;
//                 entry.key_value = *(uint32_t *)dataPointer[c - 1]; // İndekslənən uint32_t dəyər
//                 entry.file_offset = writePosition;                    // Sətrin fayldakı yeri
                
//                 fwrite(&entry, sizeof(IndexEntry), 1, fIdx);
//                 fclose(fIdx);
//             }
//         }
//     }

//     printf("Ugurlu: 1 satir '%s' cedvaline daxil edildi (Auto-Increment ve Indeksler yenilendi).\n", tableName);
//     return true;
// }



// SELECT WHERE: Əgər şərt qoyulan sütunun indeksi varsa, O(1) ilə birbaşa nöqtəyə sıçrayır!
uint8_t selectWhereWithIndex(const char* tableName, const char* whereColumnsName[], void* whereColumnsData[], const char* whereOperators[]) {
    uint8_t tId = getTableIndexByName(tableName);
    uint8_t cId = getColumnIdByName(tId, whereColumnsName[0]); // İlk şərt sütununa baxırıq
    char idxName[32];

    // ƏGƏR İNDEKS YOXDURSA: Köhnə sətir-sətir yavaş axtarış funksiyasına yönləndirir
    if (!isColumnIndexed(tId, cId, idxName) || strcmp(whereOperators[0], "=") != 0) {
        return selectWhere(tableName, whereColumnsName, whereColumnsData, whereOperators);
    }

    // ƏGƏR İNDEKS VARSA: Sürətli axtarış işə düşür!
    printf("\n⚡ [İNDEKS OPTİMİZASİYASI İŞƏ DÜŞDÜ - %s] ⚡\n", idxName);
    char idxPath[256];
    snprintf(idxPath, sizeof(idxPath), "%s/tables/%s", current_db_path, idxName);
    FILE *fIdx = fopen(idxPath, "rb");
    if (!idxPath) return 0;

    uint32_t targetVal = *(uint32_t *)whereColumnsData[0];
    uint32_t targetFilePos = 0;
    IndexEntry entry;

    // İndeks faylını sürətlə darayırıq (Çox kiçik fayldır)
    while (fread(&entry, sizeof(IndexEntry), 1, fIdx)) {
        if (entry.key_value == targetVal) {
            targetFilePos = entry.file_offset;
            break;
        }
    }
    fclose(fIdx);

    if (targetFilePos == 0) {
        printf("Məlumat tapılmadı (İndeks boş qaytardı).\n");
        return 0;
    }

    // Doğrudan hədəf sətirə tullanırıq (Bütün cədvəli gəzmək yoxdur!)
    char tableFilePath[256];
    snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);
    FILE *file = fopen(tableFilePath, "rb");
    if (!file) return 0;

    DBHeader header;
    fread(&header, sizeof(DBHeader), 1, file);
    ColumnConfig configs[MAX_COLUMNS + 1];
    fread(configs, sizeof(ColumnConfig), header.columnCount, file);

    uint8_t rowBuffer[256];
    fseek(file, targetFilePos, SEEK_SET);
    fread(rowBuffer, header.rowSize, 1, file);
    fclose(file);

    // Ekrana çıxarma mexanizmi
    if (rowBuffer[0] == 0) { // Silinməyibsə
        int offset = 1;
        for (int i = 1; i < header.columnCount; i++) {
            if (configs[i].typeID==TYPE_UINT32) {
                uint32_t val; memcpy(&val, rowBuffer + offset, 4);
                printf("%s: %u \t", configs[i].columnName, val);
                offset += 4;
            }
        }
        printf("\n");
        return selectWhere(tableName, whereColumnsName, whereColumnsData, whereOperators);
        // return 1;
    }
    return 0;
}

// ====================================================================
// İNDEKSLƏRİN SIFIRLANMASI (DROP və ya RECREATE)
// ====================================================================

// DROP TABLE və ya RECREATE zamanı indeksləri təmizləyən funksiya
void resetTableIndexes(const char *tableName) {
    uint8_t tId = getTableIndexByName(tableName);
    if (tId == 0) return;

    char idxMetaPath[256];
    snprintf(idxMetaPath, sizeof(idxMetaPath), "%s/metadata/indexes.db", current_db_path);
    FILE *f = fopen(idxMetaPath, "rb+");
    if (!f) return;

    IndexMeta meta;
    long offset = 0;
    while (fread(&meta, sizeof(IndexMeta), 1, f)) {
        if (meta.table_id == tId) {
            // 1. Mərkəzi metadatada soft delete edirik
            fseek(f, offset, SEEK_SET);
            uint8_t delFlag = 1;
            fwrite(&delFlag, 1, 1, f);
            fseek(f, offset + sizeof(IndexMeta), SEEK_SET);

            // 2. Fiziki .idx faylını diskdən tamamilə silirik
            char idxPath[256];
            snprintf(idxPath, sizeof(idxPath), "%s/tables/%s", current_db_path, meta.index_name);
            remove(idxPath);
        }
        offset += sizeof(IndexMeta);
    }
    fclose(f);
    printf("'%s' cədvəlinə aid bütün indekslər sıfırlandı və təmizləndi.\n", tableName);
}

// dropTable-ı bu şəkildə yeniləyirik:
bool dropTableWithIndex(const char *tableName, int hardDrop) {
    // Öncə cədvələ aid indeksləri sıfırlayırıq
    resetTableIndexes(tableName);
    // Sonra cədvəli tamamilə silirik
    return dropTable(tableName, hardDrop);
}


// İndeks faylına sıralı məlumat yazmaq (Insertion Sort məntiqi ilə binar faylda)
void insertIntoIndexFile(const char *idxName, uint32_t keyValue, uint32_t offsetValue) {
    char idxPath[256];
    snprintf(idxPath, sizeof(idxPath), "%s/tables/%s", current_db_path, idxName);
    
    FILE *f = fopen(idxPath, "rb+");
    if (!f) {
        f = fopen(idxPath, "wb+"); // Yoxdursa yarat
        if (!f) return;
    }

    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    int count = fileSize / sizeof(IndexEntry);

    IndexEntry *entries = (IndexEntry *)malloc((count + 1) * sizeof(IndexEntry));
    if (count > 0) {
        fseek(f, 0, SEEK_SET);
        fread(entries, sizeof(IndexEntry), count, f);
    }

    // Yeni elementi sıralı yerinə yerləşdirək
    int i = count - 1;
    while (i >= 0 && entries[i].key_value > keyValue) {
        entries[i + 1] = entries[i];
        i--;
    }
    entries[i + 1].key_value = keyValue;
    entries[i + 1].file_offset = offsetValue;

    // Fayla geri yazaq
    freopen(idxPath, "wb", f);
    fwrite(entries, sizeof(IndexEntry), count + 1, f);
    fclose(f);
    free(entries);
}


// BİNAR AXTARIŞ (Binary Search) - İndeks sayəsində diski saniyələr içində tarayır
long binarySearchInIndex(const char *idxName, uint32_t targetKey) {
    char idxPath[256];
    snprintf(idxPath, sizeof(idxPath), "%s/tables/%s", current_db_path, idxName);
    FILE *f = fopen(idxPath, "rb");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    int low = 0;
    int high = (fileSize / sizeof(IndexEntry)) - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        fseek(f, mid * sizeof(IndexEntry), SEEK_SET);
        IndexEntry entry;
        fread(&entry, sizeof(IndexEntry), 1, f);

        if (entry.key_value == targetKey) {
            fclose(f);
            return (long)entry.file_offset; // Tapılan sətirin real data faylındakı offseti
        }
        if (entry.key_value < targetKey) low = mid + 1;
        else high = mid - 1;
    }

    fclose(f);
    return -1; // Tapılmadı
}


#endif