
// data_controls.h
#ifndef DATA_CONTROLS_H
#define DATA_CONTROLS_H

// #include "add_controls.h"
// #include "index_controls.h"
// 4 parametr: masa adı, sütunlar stringi, dəyərlər stringi, sütun sayı
int insertRows(const char *tableName, const char *colsStr, const char *valsStr, int colCount);
uint8_t deleteRowsByIndices(const char *tableName, uint32_t *rowIndices, uint8_t indicesCount);

// int insertRows(const char *tableName, void *dataPointer[], int dataCount)
// {

// #if defined(TARGET_PLATFORM_ESP32)
//     File file = openTable(tableName, "r+");

//     if (!file)
//     {
//         return 0;
//     }
//     Serial.println("\n[UĞURLU] Fayl LittleFS tərəfindən uğurla açıldı. Yazma prosesi başlayır...");
// #else
//     FILE *file = openTable(tableName, "rb+"); // "rb+" binar oxuma/yazma üçün
//     if (!file)
//         return 0;
//     printf("\n[UĞURLU] Fayl LittleFS tərəfindən uğurla açıldı. Yazma prosesi başlayır...\n");
// #endif

//     DBHeader header;
//     // file.read((uint8_t *)&header, sizeof(DBHeader));
//     // #if defined(TARGET_PLATFORM_ESP32)
//     //     file.read((uint8_t *)&header, sizeof(DBHeader));
//     // #else
//     //     fread(&header, sizeof(DBHeader), 1, file);
//     // #endif

//     ColumnConfig configs[MAX_COLUMNS + 1];
// // file.read((uint8_t *)configs, sizeof(ColumnConfig) * header.columnCount);
// #if defined(TARGET_PLATFORM_ESP32)
//     file.read((uint8_t *)&header, sizeof(DBHeader));
//     file.read((uint8_t *)configs, sizeof(ColumnConfig) * header.columnCount);
// #else
//     fread(&header, sizeof(DBHeader), 1, file);
//     fread(configs, sizeof(ColumnConfig), header.columnCount, file);
// #endif

//     uint8_t rowBuffer[512];
//     memset(rowBuffer, 0, header.rowSize);
//     rowBuffer[0] = 0; // is_deleted = 0

//     int pointerIdx = 0;
//     int currentOffset = 1;

//     // 🌟 1. BU DƏYİŞƏN ARTIQ ID YOX, SIRA NÖMRƏSİNİ QAYTARMAQ ÜÇÜN İSTİFADƏ OLUNACAQ
//     uint32_t rowIndex = 0;

// // Serial.println("Data İnsert prosesis başlayır");
// #if defined(TARGET_PLATFORM_ESP32)
//     Serial.println("\nData İnsert prosesis başlayır");
// #else
//     printf("\nData İnsert prosesis başlayır\n");
// #endif

//     for (int i = 1; i < header.columnCount; i++)
//     {
//         // 1. AUTO INCREMENT YOXLANILMASI
//         if (configs[i].constraints & FLAG_AUTO_INCREMENT)
//         {
//             // Serial.println("İncriment edilir");
// #if defined(TARGET_PLATFORM_ESP32)
//             Serial.println("\nİncriment edilir");
// #else
//             printf("\nİncriment edilir\n");
// #endif
//             header.last_inserted_id++;
//             uint32_t autoId = header.last_inserted_id;

//             // ID-ni hələ də fayla yazırıq (baza strukturu pozulmasın deyə)
//             memcpy(rowBuffer + currentOffset, &autoId, sizeof(uint32_t));
//             currentOffset += sizeof(uint32_t);
//             // Serial.println("İncriment edildi");
// #if defined(TARGET_PLATFORM_ESP32)
//             Serial.println("\nİncriment edildi");
// #else
//             printf("\nİncriment edildi\n");
// #endif
//             continue;
//         }

//         if (pointerIdx >= dataCount || dataPointer[pointerIdx] == NULL)
//         {
//             // Serial.println("row count test edilir");
// #if defined(TARGET_PLATFORM_ESP32)
//             Serial.println("\nrow count test edilir");
// #else
//             printf("\nrow count test edilir\n");
// #endif
//             currentOffset += configs[i].dataSize;
//             pointerIdx++;
//             // Serial.println("row count test edildi");
// #if defined(TARGET_PLATFORM_ESP32)
//             Serial.println("\nrow count test edildi");
// #else
//             printf("\nrow count test edildi\n");
// #endif
//             continue;
//         }

//         // 2. BUFFERƏ KOPIYALAMA
//         if (configs[i].typeID == TYPE_INT || configs[i].typeID == TYPE_UINT32)
//         {
//             memcpy(rowBuffer + currentOffset, dataPointer[pointerIdx], 4);
//             currentOffset += 4;
//         }
//         else if (configs[i].typeID == TYPE_UINT8)
//         {
//             memcpy(rowBuffer + currentOffset, dataPointer[pointerIdx], 1);
//             currentOffset += 1;
//         }
//         else if (configs[i].typeID == TYPE_FLOAT)
//         {
//             memcpy(rowBuffer + currentOffset, dataPointer[pointerIdx], 4);
//             currentOffset += 4;
//         }
//         else if (configs[i].typeID == TYPE_FIXED_POINT)
//         {
//             memcpy(rowBuffer + currentOffset, dataPointer[pointerIdx], 2);
//             currentOffset += 2;
//         }
//         else if (configs[i].typeID == TYPE_CHAR2)
//         {
//             char tempStr[MAX_CHAR] = {0};
//             strncpy(tempStr, (const char *)dataPointer[pointerIdx], configs[i].dataSize - 1);
//             memcpy(rowBuffer + currentOffset, tempStr, configs[i].dataSize);
//             currentOffset += configs[i].dataSize;
//         }
//         else if (configs[i].typeID == TYPE_VARCHAR2)
//         {
//             char varcharPath[256];
//             snprintf(varcharPath, sizeof(varcharPath), "%s/tables/%s.varchardb", current_db_path, tableName);

//             // File vFile = LittleFS.open(varcharPath, "a+");
//             uint32_t vOffset = 0;
//             // if (vFile)
//             // {
//             // vOffset = vFile.size();
//             const char *userStr = (const char *)dataPointer[pointerIdx];
//             uint16_t strLen = strlen(userStr);
//             //     vFile.write((uint8_t *)&strLen, sizeof(uint16_t));
//             //     vFile.write((const uint8_t *)userStr, strLen);
//             //     vFile.close();
//             // // }

// #if defined(TARGET_PLATFORM_ESP32)
//             // ESP32 üçün (Mövcud kodunuz)
//             File vFile = LittleFS.open(varcharPath, "a+");
//             vOffset = vFile.size();
//             vFile.write((uint8_t *)&strLen, sizeof(uint16_t));
//             vFile.write((const uint8_t *)userStr, strLen);
//             vFile.close();

// #elif defined(TARGET_PLATFORM_WINDOWS)
//             // Windows üçün (Standart C fopen istifadəsi)
//             FILE *vFile = fopen(varcharPath, "ab+"); // "ab+" binar əlavə etmə rejimi
//             if (vFile)
//             {
//                 fseek(vFile, 0, SEEK_END);
//                 vOffset = ftell(vFile); // Faylın sonu offset-dir

//                 fwrite(&strLen, sizeof(uint16_t), 1, vFile);
//                 fwrite(userStr, strLen, 1, vFile);
//                 fclose(vFile);
//             }
// #endif
//             memcpy(rowBuffer + currentOffset, &vOffset, sizeof(uint32_t));
//             currentOffset += sizeof(uint32_t);
//         }

//         pointerIdx++;
//     }
// // Serial.println("Data Yazıldı");
// #if defined(TARGET_PLATFORM_ESP32)
//     Serial.println("\nData Yazıldı");
// #else
//     printf("\nData Yazıldı\n");
// #endif

//     // 🌟 2. YENİ SƏTRİN FAYL DAXİLİNDƏKİ SIRA NÖMRƏSİNİ (İNDEKSİNİ) HESABLAYIB YADDA SAXLAYIRIQ
//     rowIndex = header.rowCount % header.maxRows;

//     // 3. DAİRƏVİ SİSTEMƏ UYGUN YAZMA NÖQTƏSİNİN HESABLANMASI
//     long writeOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount) +
//                        (rowIndex * header.rowSize); // Düsturu rowIndex ilə qısaltdıq

// // if (file.seek(writeOffset, SeekSet))
// #if defined(TARGET_PLATFORM_ESP32)
//     if (file.seek(writeOffset, SeekSet))
// #else
//     if (fseek(file, writeOffset, SEEK_SET))
//     // ; // Standart C funksiyası
// #endif
//     {
// #if defined(TARGET_PLATFORM_ESP32)
//         file.write(rowBuffer, header.rowSize);
// #else
//         fwrite(rowBuffer, header.rowSize, 1, file); // Standart C funksiyası
// #endif
//     }
//     else
//     {
// #if defined(TARGET_PLATFORM_ESP32)
//         file.close();
// #else
//         fclose(file);
// #endif
//         return false;
//     }
// // Serial.println("Data Save edilir");
// #if defined(TARGET_PLATFORM_ESP32)
//     Serial.println("\nData Save edilir");
// #else
//     printf("\nData Save edilir\n");
// #endif

//     if (header.rowCount < header.maxRows)
//     {
//         header.rowCount++;
//     }

//     // if (file.seek(0, SeekSet))
//     // {
//     //     file.write((uint8_t *)&header, sizeof(DBHeader));
//     // }

//     // #elif defined(TARGET_PLATFORM_WINDOWS)

//     // #if defined(TARGET_PLATFORM_ESP32)

// #if defined(TARGET_PLATFORM_ESP32)
//     file.seek(0, SeekSet);
//     file.read((uint8_t *)&header, sizeof(DBHeader));
//     file.flush();
//     file.close();
// #else
//     fseek(file, 0, SEEK_SET);
//     fread(&header, sizeof(DBHeader), 1, file);
//     fflush(file); // Standart C funksiyası
//     fclose(file);
// #endif

//     // 🌟 3. AUTO INCREMENT ID YOX, BAZADAKI REAL SIRA INDEKSİNİ QAYTARIRIQ
//     return rowIndex;
// }

// int insertRows(const char *tableName, const char *colsStr, const char *valsStr, int colCount) {
//     // 1. Faylı aç
// #if defined(TARGET_PLATFORM_ESP32)
//     File file = openTable(tableName, "r+");
// #else
//     FILE *file = fopen(tableName, "rb+"); // "openTable" yerinə fopen istifadə etdik
// #endif
//     if (!file) return 0;

//     DBHeader header;
//     ColumnConfig configs[MAX_COLUMNS + 1];

//     // Header və Configləri oxu
// #if defined(TARGET_PLATFORM_ESP32)
//     file.read((uint8_t *)&header, sizeof(DBHeader));
//     file.read((uint8_t *)configs, sizeof(ColumnConfig) * header.columnCount);
// #else
//     fread(&header, sizeof(DBHeader), 1, file);
//     fread(configs, sizeof(ColumnConfig), header.columnCount, file);
// #endif

//     uint8_t rowBuffer[512] = {0};

//     // 🌟 DÜZƏLİŞ 1: is_deleted bayrağını "0" et (Aktiv sətir)
//     rowBuffer[0] = 0;

//     char colsCopy[256], valsCopy[256];
//     strncpy(colsCopy, colsStr, 256);
//     strncpy(valsCopy, valsStr, 256);

//     char *colTok = strtok(colsCopy, ",");
//     char *valTok = strtok(valsCopy, ",");

//     while (colTok != NULL && valTok != NULL) {
//         // Trim
//         while (*colTok == ' ') colTok++;
//         while (*valTok == ' ') valTok++;

//         for (int i = 0; i < header.columnCount; i++) {
//             // 🌟 DÜZƏLİŞ 2: Sütun adlarını müqayisə et (Boşluqları nəzərə almaq üçün)
//             // Əgər bazada adlar "sensors_name"dirsə, bu düzgün işləyəcək
//             if (strcmp(configs[i].columnName, colTok) == 0) {

//                 size_t offset = 0;
//                 for(int k = 0; k < i; k++) offset += configs[k].dataSize;

//                 if (configs[i].typeID == TYPE_INT || configs[i].typeID == TYPE_UINT32) {
//                     int val = atoi(valTok);
//                     memcpy(rowBuffer + offset, &val, 4);
//                 }
//                 else if (configs[i].typeID == TYPE_UINT8) {
//                     uint8_t val = (uint8_t)atoi(valTok);
//                     memcpy(rowBuffer + offset, &val, 1);
//                 }
//                 else {
//                     strncpy((char*)(rowBuffer + offset), valTok, configs[i].dataSize - 1);
//                 }
//                 break;
//             }
//         }
//         colTok = strtok(NULL, ",");
//         valTok = strtok(NULL, ",");
//     }

//     // 4. Yazma nöqtəsini hesabla
//     uint32_t rowIndex = header.rowCount % header.maxRows;
//     long writeOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount) + (rowIndex * header.rowSize);

//     // 5. Fayla yaz
// #if defined(TARGET_PLATFORM_ESP32)
//     file.seek(writeOffset, SeekSet);
//     file.write(rowBuffer, header.rowSize);
//     if (header.rowCount < header.maxRows) header.rowCount++;
//     file.seek(0, SeekSet);
//     file.write((uint8_t *)&header, sizeof(DBHeader));
//     file.close();
// #else
//     fseek(file, writeOffset, SEEK_SET);
//     fwrite(rowBuffer, header.rowSize, 1, file);
//     if (header.rowCount < header.maxRows) header.rowCount++;
//     fseek(file, 0, SEEK_SET);
//     fwrite(&header, sizeof(DBHeader), 1, file);
//     fclose(file);
// #endif

//     return rowIndex;
// }

// int insertRows(const char *tableName, const char *colsStr, const char *valsStr, int colCount) {
//     FILE *file = fopen(tableName, "rb+");
//     if (!file) return -1;

//     DBHeader header;
//     ColumnConfig configs[MAX_COLUMNS + 1];

//     fread(&header, sizeof(DBHeader), 1, file);
//     fread(configs, sizeof(ColumnConfig), header.columnCount, file);

//     uint8_t rowBuffer[512] = {0};
//     rowBuffer[0] = 0; // is_deleted = 0 (Aktiv)

//     char colsCopy[256], valsCopy[256];
//     strncpy(colsCopy, colsStr, 256);
//     strncpy(valsCopy, valsStr, 256);

//     char *colTok = strtok(colsCopy, ",");
//     char *valTok = strtok(valsCopy, ",");

//     while (colTok != NULL && valTok != NULL) {
//         while (*colTok == ' ') colTok++;
//         while (*valTok == ' ') valTok++;

//         for (int i = 0; i < header.columnCount; i++) {
//             if (strcmp(configs[i].columnName, colTok) == 0) {
//                 // Sütunun ofsetini hesablayarkən configs-ləri cəmləyirik
//                 size_t offset = 0;
//                 for(int k = 0; k < i; k++) offset += configs[k].dataSize;

//                 if (configs[i].typeID == TYPE_INT || configs[i].typeID == TYPE_UINT32) {
//                     int val = atoi(valTok);
//                     memcpy(rowBuffer + offset, &val, 4);
//                 }
//                 else if (configs[i].typeID == TYPE_UINT8) {
//                     uint8_t val = (uint8_t)atoi(valTok);
//                     memcpy(rowBuffer + offset, &val, 1);
//                 }
//                 else {
//                     // String tipləri üçün
//                     strncpy((char*)(rowBuffer + offset), valTok, configs[i].dataSize);
//                 }
//                 break;
//             }
//         }
//         colTok = strtok(NULL, ",");
//         valTok = strtok(NULL, ",");
//     }

//     uint32_t rowIndex = header.rowCount % header.maxRows;
//     long writeOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount) + (rowIndex * header.rowSize);

//     fseek(file, writeOffset, SEEK_SET);
//     fwrite(rowBuffer, header.rowSize, 1, file);

//     header.rowCount++;
//     fseek(file, 0, SEEK_SET);
//     fwrite(&header, sizeof(DBHeader), 1, file);
//     fclose(file);

//     return rowIndex;
// }

int insertRows(const char *tableName, const char *colsStr, const char *valsStr, int colCount)
{
    printf("[DEBUG]: Insert əməliyyatı başlayır: %s\n", tableName);
    printf("[DEBUG]: Gələn Sütunlar: %s | Dəyərlər: %s\n", colsStr, valsStr);

    // 1. Cədvəl ID-ni tap
    uint8_t tableId = getTableIndexByName(tableName);
    if (tableId == 0)
    {
        printf("[XƏTA]: '%s' cədvəli tapılmadı!\n", tableName);
        return -1;
    }

    // 2. Metadata-dan sütun strukturlarını yüklə
    ColumnConfig configs[MAX_COLUMNS];
    int actualColCount = loadConfigsForTable(tableId, configs);
    printf("[DEBUG]: Metadata-dan %d sütun konfiqurasiyası yükləndi.\n", actualColCount);

    // 3. Row buffer-i hazırla
    uint8_t rowBuffer[512] = {0};
    rowBuffer[0] = 0; // is_deleted = 0

    // 4. Stringləri parçala
    char colsCopy[256], valsCopy[256];
    strncpy(colsCopy, colsStr, 256);
    strncpy(valsCopy, valsStr, 256);

    char *saveptr1, *saveptr2;
    char *colTok = strtok_r(colsCopy, ",", &saveptr1);
    char *valTok = strtok_r(valsCopy, ",", &saveptr2);

    int processedCount = 0;
    while (colTok != NULL && valTok != NULL && processedCount < colCount)
    {
        while (*colTok == ' ')
            colTok++;
        while (*valTok == ' ')
            valTok++;

        bool found = false;
        // ... Dövrün içərisi ...
        for (int i = 0; i < actualColCount; i++)
        {
            if (strcmp(configs[i].columnName, colTok) == 0)
            {
                found = true;

                // Ofset hesablaması: 1 bayt (is_deleted) + əvvəlki sütunların cəmi
                size_t offset = 1;
                for (int k = 0; k < i; k++)
                    offset += configs[k].dataSize;

                printf("[DEBUG]: Sütun: %s (Tip: %d, Ölçü: %d) -> Dəyər: %s\n",
                       configs[i].columnName, configs[i].typeID, configs[i].dataSize, valTok);

                // Tipə uyğun yazma
                switch (configs[i].typeID)
                {
                case TYPE_INT:
                case TYPE_UINT32:
                case TYPE_TIMESTAMP:
                {
                    uint32_t val = (uint32_t)atoi(valTok);
                    memcpy(rowBuffer + offset, &val, 4);
                }
                break;
                case TYPE_UINT8:
                {
                    uint8_t val = (uint8_t)atoi(valTok);
                    memcpy(rowBuffer + offset, &val, 1);
                }
                break;
                case TYPE_FLOAT:
                {
                    float val = (float)atof(valTok);
                    memcpy(rowBuffer + offset, &val, 4);
                }
                break;
                case TYPE_FIXED_POINT:
                {
                    int16_t val = (int16_t)(atof(valTok) * 100.0f);
                    memcpy(rowBuffer + offset, &val, 2);
                }
                break;
                case TYPE_CHAR2:
                {
                    strncpy((char *)(rowBuffer + offset), valTok, configs[i].dataSize);
                }
                break;
                default:
                    printf("[XƏTA]: Naməlum tip ID: %d\n", configs[i].typeID);
                }
                break;
            }
        }

        if (!found)
        {
            printf("[XƏTA]: Sütun tapılmadı: '%s'. SQL Parseri yoxlayın!\n", colTok);
        }

        colTok = strtok_r(NULL, ",", &saveptr1);
        valTok = strtok_r(NULL, ",", &saveptr2);
        processedCount++;
    }

    // 5. Fayla yaz
    char path[256];
    snprintf(path, sizeof(path), "%s/tables/%s.db", current_db_path, tableName);
    FILE *file = fopen(path, "rb+");
    if (!file)
        return -1;

    DBHeader header;
    fread(&header, sizeof(DBHeader), 1, file);

    uint32_t rowIndex = header.rowCount % header.maxRows;
    long writeOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount) + (rowIndex * header.rowSize);

    fseek(file, writeOffset, SEEK_SET);
    fwrite(rowBuffer, header.rowSize, 1, file);

    header.rowCount++;
    fseek(file, 0, SEEK_SET);
    fwrite(&header, sizeof(DBHeader), 1, file);

    fclose(file);
    printf("[UĞURLU]: Məlumat %d-ci sıraya yazıldı.\n", rowIndex);
    return rowIndex;
}

// uint8_t deleteRows(const char *tableName, char *whereColumnsName[], void *whereColumnsData[], const char *whereOperators[], int hardDelete)
// {
//     if (strlen(current_db_path) == 0)
//         return 0;

//     char tableFilePath[256];
//     snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

//     // "r+" həm oxumaq, həm də üzərinə yaza bilmək üçün mütləqdir

//     DBHeader header;
// // file.read((uint8_t *)&header, sizeof(DBHeader));
// #if defined(TARGET_PLATFORM_ESP32)
//     File file = LittleFS.open(tableFilePath, "r+");
//     if (!file)
//         return 0;
//     file.read((uint8_t *)&header, sizeof(DBHeader));
// #else
//     FILE *file = fopen(tableFilePath, "rb+"); // Windows üçün binar açılma
//     if (!file)
//         return false;
//     fread(&header, sizeof(DBHeader), 1, file);
// #endif

//     ColumnConfig configs[MAX_COLUMNS + 1];
// #if defined(TARGET_PLATFORM_ESP32)
//     file.read((uint8_t *)configs, sizeof(ColumnConfig) * header.columnCount);
// #else
//     fread(configs, sizeof(ColumnConfig), header.columnCount, file);
// #endif

//     uint8_t rowBuffer[512];
//     uint32_t deletedCount = 0;
//     long startOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);

//     // İlk öncə cədvəlin ID-sini alırıq (Relyasiyaları yoxlamaq üçün)
//     uint8_t currentTableId = getTableIndexByName(tableName);

//     for (uint32_t r = 0; r < header.rowCount; r++)
//     {
//         long currentBlockOffset = startOffset + (r * header.rowSize);
// #if defined(TARGET_PLATFORM_ESP32)
//         if (!file.seek(currentBlockOffset, SeekSet))
// #else
//         fseek(file, currentBlockOffset, SEEK_SET);
// #endif
//             continue;

// #if defined(TARGET_PLATFORM_ESP32)
//         if (file.read(rowBuffer, header.rowSize) != header.rowSize)
// #else
//         if (fread(rowBuffer, header.rowSize, 1, file) != 1)
// #endif
//             continue;

//         if (rowBuffer[0] == 1)
//             continue; // Artıq silinmiş sətirdirsə, keç

//         bool matches = true;
//         if (whereColumnsName != NULL)
//         {
//             int w = 0;
//             while (whereColumnsName[w] != NULL)
//             {
//                 int colIdx = getColumnIndexInConfig(configs, header.columnCount, whereColumnsName[w]);
//                 if (colIdx == -1)
//                 {
//                     matches = false;
//                     break;
//                 }

//                 int colOffset = getColumnOffsetInRow(configs, header.columnCount, colIdx);

//                 bool conditionMet = helperCheckCondition(rowBuffer + colOffset, configs[colIdx].typeID, whereColumnsData[w], whereOperators[w]);
//                 if (!conditionMet)
//                 {
//                     matches = false;
//                     break;
//                 }
//                 w++;
//             }
//         }

//         // Əgər silinmək istənən sətir tapıldısa:
//         if (matches)
//         {
//             bool allowDelete = true;

//             // ====================================================================
//             // RELYASİYA / ƏLAQƏLİ CƏDVƏL YOXlANIŞI
//             // ====================================================================
//             if (currentTableId != 0 && (hardDelete == 0 || hardDelete == 1))
//             {
//                 char relPath[256];
//                 snprintf(relPath, sizeof(relPath), "%s/metadata/relations.db", current_db_path);
// #if defined(TARGET_PLATFORM_ESP32)
//                 File fRel = LittleFS.open(relPath, "r");
// #else
//                 FILE *fRel = fopen(relPath, "rb"); // Windows üçün 'rb'
// #endif
//                 if (fRel)
//                 {
//                     CompactRelation rel;
// #if defined(TARGET_PLATFORM_ESP32)
//                     while (fRel.read((uint8_t *)&rel, sizeof(CompactRelation)) == sizeof(CompactRelation))
// #else
//                     while (fread(&rel, sizeof(CompactRelation), 1, fRel) == 1)
// #endif
//                     {
//                         if (rel.is_deleted == 0 && rel.parent_table_id == currentTableId)
//                         {

//                             char childTableName[MAX_NAME_LEN];
//                             getTableNameByIndex(rel.child_table_id, childTableName);

//                             char childKeyColumnName[MAX_NAME_LEN] = "";
//                             if (!getColumnNameById(rel.child_table_id, rel.child_col_id, childKeyColumnName))
//                                 continue;

//                             // Parent cədvəlin ID dəyərini götürürük (id adətən 1-ci ofsetdə yerləşir)
//                             uint32_t parentIdVal = 0;
//                             memcpy(&parentIdVal, rowBuffer + 1, 4);

//                             // Alt cədvəldə bu ID-yə bağlı data olub-olmadığını yoxlamaq üçün filtr qururuq
//                             char *childWhereCols[] = {childKeyColumnName, NULL};
//                             void *childWhereData[] = {&parentIdVal};
//                             const char *childWhereOps[] = {"=", NULL};

//                             // Alt cədvəldəki dataların sayını yoxlayırıq
//                             uint8_t childRowsCount = selectWhere(childTableName, (const char **)childWhereCols, childWhereData, childWhereOps);

//                             if (childRowsCount > 0)
//                             {
//                                 if (hardDelete == 0)
//                                 {
// // 🛑 REJİM 0: Alt cədvəldə data var, silməyə icazə yoxdur!
// #if defined(TARGET_PLATFORM_ESP32)
//                                     Serial.printf("[MƏHDUDİYYƏT] '%s' cədvəlində əlaqəli data olduğu üçün '%s' silinə bilməz!\n", childTableName, tableName);
// #else
//                                     printf("[MƏHDUDİYYƏT] '%s' cədvəlində əlaqəli data olduğu üçün '%s' silinə bilməz!\n", childTableName, tableName); // Windows/Linux üçün standart
// #endif
//                                     // Serial.printf("[MƏHDUDİYYƏT] '%s' cədvəlində əlaqəli data olduğu üçün '%s' silinə bilməz!\n", childTableName, tableName);
//                                     allowDelete = false;
//                                     break;
//                                 }
//                                 else if (hardDelete == 1)
//                                 {
// // 🔄 REJİM 1: CASCADE - Alt cədvəldəki bağlı dataları da silirik
// #if defined(TARGET_PLATFORM_ESP32)
//                                     Serial.printf("[CASCADE] '%s' silindiyi üçün '%s' cədvəlindəki əlaqəli sətirlər də silinir...\n", tableName, childTableName);
// #else
//                                     printf("[CASCADE] '%s' silindiyi üçün '%s' cədvəlindəki əlaqəli sətirlər də silinir...\n", tableName, childTableName); // Windows/Linux üçün standart
// #endif
//                                     // Serial.printf("[CASCADE] '%s' silindiyi üçün '%s' cədvəlindəki əlaqəli sətirlər də silinir...\n", tableName, childTableName);
//                                     deleteRows(childTableName, childWhereCols, childWhereData, childWhereOps, 1);
//                                 }
//                             }
//                         }
//                     }
// #if defined(TARGET_PLATFORM_ESP32)
//                     fRel.close();
// #else
//                     fclose(fRel);
// #endif
//                 }
//             }

//             // 🛑 Əgər hardDelete = 0 şərti pozulubsa, bu sətri silmədən növbəti sətirlərə keçirik
//             if (!allowDelete)
//                 continue;

//             // ====================================================================
//             // REAL SİLİNMƏ ƏMƏLİYYATI (is_deleted = 1)
//             // ====================================================================
//             // REJİM 2 daxil olmaqla, əgər qadağa yoxdursa, əsas cədvəldəki sətir silinir
//             rowBuffer[0] = 1;

// #if defined(TARGET_PLATFORM_ESP32)
//             if (file.seek(currentBlockOffset, SeekSet))
//             {
//                 file.write(rowBuffer, header.rowSize);
// #else
//             if (fseek(file, currentBlockOffset, SEEK_SET) == 0)
//             {
//                 fwrite(rowBuffer, header.rowSize, 1, file);
// #endif
//                 deletedCount++;
//             }
//         }
//     }

// #if defined(TARGET_PLATFORM_ESP32)
//     file.close();
// #else
//     fclose(file);
// #endif
//     return deletedCount;
// }

// WHERE şərtlərini yoxlayan köməkçi
bool checkWhereConditions(uint8_t *rowBuffer, ColumnConfig *configs, int colCount, char *cols[], void *data[], const char *ops[])
{
    if (cols == NULL || cols[0] == NULL)
        return true; // WHERE yoxdursa, hamısını sil
    int w = 0;
    printf("  3");
    while (cols[w] != NULL)
    {
        printf("4");
        int colIdx = getColumnIndexInConfig(configs, colCount, cols[w]);
        if (colIdx == -1)
            return false;
        printf("5");
        int colOffset = getColumnOffsetInRow(configs, colCount, colIdx);
        if (!helperCheckCondition(rowBuffer + colOffset, configs[colIdx].typeID, data[w], ops[w]))
            return false;
        w++;
        printf("6");
    }
    return true;
}

// Relyasiyaları idarə edən (Cascade/Restrict)
bool handleRelationalConstraints(const char *parentTable, uint8_t *rowBuffer, int hardDelete, uint8_t parentTableId)
{
    // relations.db-ni aç və əlaqəli sətirləri yoxla (əvvəlki kodunuzdakı kimi)
    // Əgər hardDelete == 1 (CASCADE) olarsa:
    // deleteRows(childTableName, childWhereCols, childWhereData, childWhereOps, 1);
    // Əgər hardDelete == 0 (RESTRICT) və childRowsCount > 0 olarsa: return false;
    return true;
}

// Fiziki silinməni icra edən
bool performPhysicalDelete(FILE *file, long offset, uint8_t *rowBuffer, size_t rowSize)
{
    rowBuffer[0] = 1; // Sətiri silinmiş kimi işarələ
    fseek(file, offset, SEEK_SET);
    return fwrite(rowBuffer, rowSize, 1, file) == 1;
}

// uint8_t deleteRows(const char *tableName, char *whereColumnsName[], void *whereColumnsData[], const char *whereOperators[], int hardDelete) {
//     printf("\n[DELETE ROWS START]: Cədvəl: '%s' | Rejim: %d\n", tableName, hardDelete);

//     char tableFilePath[256];
//     snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

//     FILE *file = fopen(tableFilePath, "rb+");
//     if (!file) {
//         printf("[ERROR]: '%s' faylı açıla bilmədi!\n", tableFilePath);
//         return 0;
//     }

//     DBHeader header;
//     fread(&header, sizeof(DBHeader), 1, file);
//     ColumnConfig configs[MAX_COLUMNS + 1];
//     fread(configs, sizeof(ColumnConfig), header.columnCount, file);

//     uint8_t rowBuffer[512];
//     uint32_t deletedCount = 0;
//     long startOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);
//     uint8_t currentTableId = getTableIndexByName(tableName);

//     for (uint32_t r = 0; r < header.rowCount; r++) {

//         long currentBlockOffset = startOffset + (r * header.rowSize);
//         fseek(file, currentBlockOffset, SEEK_SET);
//         if (fread(rowBuffer, header.rowSize, 1, file) != 1 || rowBuffer[0] == 1) continue;

//         // WHERE şərtinin yoxlanılması
//         bool matches = checkWhereConditions(rowBuffer, configs, header.columnCount, whereColumnsName, whereColumnsData, whereOperators);

//         if (matches) {
//             printf("[DEBUG]: Sətir %d silinməyə uyğun gəldi.\n", r);

//             // 1. Relyasiya (Cascade/Restrict) yoxlanışı
//             if (!handleRelationalConstraints(tableName, rowBuffer, hardDelete, currentTableId)) {
//                 printf("[SKIP]: Silinməyə icazə verilmədi (RESTRICT).\n");
//                 continue;
//             }

//             // 2. Fiziki silinmə (is_deleted = 1)
//             if (performPhysicalDelete(file, currentBlockOffset, rowBuffer, header.rowSize)) {
//                 deletedCount++;
//                 printf("[SUCCESS]: Sətir %d uğurla silindi.\n", r);
//             }
//         }
//     }
//     fclose(file);
//     return deletedCount;
// }

uint8_t deleteRows(const char *tableName, char *whereColumnsName[], void *whereColumnsData[], const char *whereOperators[], int hardDelete)
{

    // whereColumnsName[1]= NULL;
    // whereColumnsData[1]= NULL;
    // whereOperators[1]= NULL;
    printf("\n--- [DEBUG: Condition Lists] ---\n");
    // printf("Ümumi şərt sayı: %d\n", 2);

    // for (int i = 0; i < 2; i++) {
    //     // Hər elementi yoxlayaraq çap edirik
    //     printf("Element [%d]:\n", i);
    //     printf("  Sütun (cols): '%s'\n", (whereColumnsName[i] != NULL) ? whereColumnsName[i] : "NULL");
    //     printf("  Operator (ops): '%s'\n", (whereOperators[i] != NULL) ? whereOperators[i] : "NULL");
    //     printf("  Dəyər (data): '%s'\n", (whereColumnsData[i] != NULL) ? (char *)whereColumnsData[i] : "NULL");
    // }
    // printf("--------------------------------\n");
    selectAndDelete(tableName, whereColumnsName, whereColumnsData, whereOperators);
    // uint32_t foundRowIndices[10];
    // memset(foundRowIndices, 0, sizeof(foundRowIndices));
    // uint8_t sensorscount = selectWhereIndex(tableName, whereColumnsName, whereColumnsData,
    //                                         whereOperators, foundRowIndices, 10);

    // if (sensorscount > 0)
    // {
    // Serial.print("Fayldakı sıra nömrələri (Indices): [");
    //     printf("Fayldakı sıra nömrələri (Indices): [");
    //     for (uint8_t i = 0; i < sensorscount; i++)
    //     {
    //         //   Serial.print(foundRowIndices[i]);
    //         //   printf(foundRowIndices[i]);
    //         printf("Index: %u\n", foundRowIndices[i]);
    //         if (i < sensorscount - 1)
    //         {
    //             // Serial.print(", "); // Elementlərin arasına vergül qoyuruq
    //             printf(", ");
    //         }
    //     }
    //     // Serial.println("]");
    //     printf("]");

    //     uint8_t deleted =
    //         deleteRowsByIndices(tableName, foundRowIndices, sensorscount);
    //     // Serial.printf("Uğurla silinən sətir sayı: %d\n", deleted);
    //     printf("Uğurla silinən sətir sayı: %d\n", deleted);
    // }
    // else
    // {
    //     // Serial.println(
    //     //     "foundRowIndices siyahısı boşdur (Heç bir uyğun sətir tapılmadı).");
    //     printf("foundRowIndices siyahısı boşdur (Heç bir uyğun sətir tapılmadı).");
    // }
    return true;
}

// ===================================================================
// LITTLEFS UYĞUN: updateRows
// ===================================================================
uint8_t updateRows(const char *tableName, char *whereColumnsName[], void *whereColumnsData[], const char *whereOperators[], char *setColumnsName[], void *setColumnsData[])
{
    if (strlen(current_db_path) == 0 || setColumnsName == NULL)
        return 0;

    char tableFilePath[256];
    snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

// File file = LittleFS.open(tableFilePath, "r+");
#if defined(TARGET_PLATFORM_ESP32)
    File file = LittleFS.open(tableFilePath, "r+");
#else
    FILE *file = fopen(tableFilePath, "rb"); // Windows üçün 'rb'
#endif
    if (!file)
        return 0;

    DBHeader header;
// file.read((uint8_t *)&header, sizeof(DBHeader));
#if defined(TARGET_PLATFORM_ESP32)
    file.read((uint8_t *)&header, sizeof(DBHeader));
#else
    fread(&header, sizeof(DBHeader), 1, file);
#endif

    ColumnConfig configs[MAX_COLUMNS + 1];
    // file.read((uint8_t *)configs, sizeof(ColumnConfig) * header.columnCount);
    DB_FILE_READ(file, configs, sizeof(ColumnConfig) * header.columnCount);

    uint8_t rowBuffer[512];
    uint32_t updatedCount = 0;
    long startOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);

    for (uint32_t r = 0; r < header.rowCount; r++)
    {
        long currentBlockOffset = startOffset + (r * header.rowSize);

        // if (!file.seek(currentBlockOffset, SeekSet))
        if (!DB_FILE_SEEK(file, currentBlockOffset))
            continue;
        // if (file.read(rowBuffer, header.rowSize) != header.rowSize)
        if (myRead(file, rowBuffer, header.rowSize) != header.rowSize)

            continue;

        if (rowBuffer[0] == 1)
            continue;

        bool matches = true;
        if (whereColumnsName != NULL)
        {
            int w = 0;
            while (whereColumnsName[w] != NULL)
            {
                int colIdx = getColumnIndexInConfig(configs, header.columnCount, whereColumnsName[w]);
                if (colIdx == -1)
                {
                    matches = false;
                    break;
                }

                int colOffset = getColumnOffsetInRow(configs, header.columnCount, colIdx);

                // DÜZƏLİŞ: .type yerinə .typeID yazıldı!
                if (!helperCheckCondition(rowBuffer + colOffset, configs[colIdx].typeID, whereColumnsData[w], whereOperators[w]))
                {
                    matches = false;
                    break;
                }
                w++;
            }
        }

        if (matches)
        {
            int s = 0;
            while (setColumnsName[s] != NULL)
            {
                int colIdx = getColumnIndexInConfig(configs, header.columnCount, setColumnsName[s]);
                if (colIdx != -1)
                {
                    int colOffset = getColumnOffsetInRow(configs, header.columnCount, colIdx);

                    // DÜZƏLİŞ: .type yerinə .typeID yazıldı!
                    if (configs[colIdx].typeID == TYPE_INT)
                    {
                        *(int32_t *)(rowBuffer + colOffset) = *(int32_t *)setColumnsData[s];
                    }
                    else if (configs[colIdx].typeID == TYPE_UINT32)
                    {
                        *(uint32_t *)(rowBuffer + colOffset) = *(uint32_t *)setColumnsData[s];
                    }
                    else if (configs[colIdx].typeID == TYPE_FLOAT)
                    {
                        *(float *)(rowBuffer + colOffset) = *(float *)setColumnsData[s];
                    }
                    else if (configs[colIdx].typeID == TYPE_CHAR2)
                    {
                        memset(rowBuffer + colOffset, 0, configs[colIdx].dataSize);
                        strncpy((char *)(rowBuffer + colOffset), (char *)setColumnsData[s], configs[colIdx].dataSize - 1);
                    }
                }
                s++;
            }

            // if (file.seek(currentBlockOffset, SeekSet))
            if (!DB_FILE_SEEK(file, currentBlockOffset))
            {
                // file.write(rowBuffer, header.rowSize);
                DB_FILE_WRITE(file, rowBuffer, header.rowSize);
                updatedCount++;
            }
        }
    }

#if defined(TARGET_PLATFORM_ESP32)
    file.close();
#else
    fclose(file);
#endif
    return updatedCount;
}

uint8_t deleteRowsByIndices(const char *tableName, uint32_t *rowIndices, uint8_t indicesCount)
{
    if (strlen(current_db_path) == 0 || indicesCount == 0 || rowIndices == NULL)
        return 0;

    // Standart C-nin fopen kilidini tam buraxması üçün 5ms gözləyirik (File Handle Release)

// File file = openTable(tableName, "r+");
#ifdef TARGET_PLATFORM_ESP32
    delay(5);
    File file = openTable(tableName, "r+");
#else
    Sleep(5);
    FILE *file = fopen(tableName, "rb+"); // Windows üçün
#endif
    if (!file)
        return 0;

    DBHeader header;
    if (!DB_READ_HEADER(file, &header))
    {
        DB_CLOSE_FILE(file); // Platformadan asılı olmayaraq faylı bağlayır
        return 0;            // Xəta kodu qaytar
    }

    long startOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);
    uint8_t deletedCount = 0;
    uint8_t deleteFlag = 1; // is_deleted = 1 edirik

    for (uint8_t i = 0; i < indicesCount; i++)
    {
        uint32_t targetRow = rowIndices[i];

        if (targetRow >= header.rowCount)
            continue;

        // Tam olaraq hədəf sətrin ilk baytının (is_deleted) yerləşdiyi ofset
        long currentBlockOffset = startOffset + (targetRow * header.rowSize);

        // SeekSet makrosu ilə birbaşa sətir başına keçirik
        // if (file.seek(currentBlockOffset, SeekSet))
        // if (!FILE_SEEK(file, currentBlockOffset))
        // {
        //     // size_t written = file.write(&deleteFlag, 1);
        //     size_t written = FILE_WRITE(file, &deleteFlag, 1);

        //     if (written > 0)
        //     {
        //         deletedCount++;
        //         // Dəyişikliyi dərhal diskə məcburi yazırıq
        //         // file.flush();
        //         FILE_FLUSH(file);
        //         // Serial.printf("[KÖMƏKÇİ] Sıra No [%d] uğurla silindi (Ofset: %ld)\n", targetRow, currentBlockOffset);
        //         LOG_PRINT("[KÖMƏKÇİ] Sıra No [%d] uğurla silindi (Ofset: %ld)\n", targetRow, currentBlockOffset);
        //     }
        // }
        if (DB_FILE_SEEK(file, currentBlockOffset) == 0) // Əgər uğurlu olarsa (0 qaytararsa)
        {
            size_t written = DB_FILE_WRITE(file, &deleteFlag, 1);

            if (written > 0)
            {
                deletedCount++;
                DB_FILE_FLUSH(file);
                DB_LOG_PRINT("[KÖMƏKÇİ] Sıra No [%d] uğurla silindi\n", targetRow);
            }
        }
        else
        {
            // Əgər seek uğursuz olarsa, bura düşəcək
            DB_LOG_PRINT("[XƏTA] Seek uğursuz oldu: %d\n", targetRow);
        }
    }

#if defined(TARGET_PLATFORM_ESP32)
    file.close();
#else
    fclose(file);
#endif
    return deletedCount;
}

// Sıra nömrəsinə (r) görə sətir datasını ekrana çıxarır və ya buferə köçürür
bool readRowByIndex(const char *tableName, uint32_t rowIndex, uint8_t *outRowBuffer)
{
    if (strlen(current_db_path) == 0)
        return false;

    char tableFilePath[256];
    snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

    FILE *file = fopen(tableFilePath, "rb");
    if (!file)
        return false;

    DBHeader header;
    fread(&header, sizeof(DBHeader), 1, file);

    if (rowIndex >= header.rowCount)
    {
        fclose(file);
        return false;
    }

    long startOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);
    long targetOffset = startOffset + (rowIndex * header.rowSize);

    fseek(file, targetOffset, SEEK_SET);

    uint8_t localBuffer[512];
    fread(localBuffer, header.rowSize, 1, file);
    fclose(file);

    if (localBuffer[0] == 1)
    {
        // Sətir silinibsə oxumağa icazə vermə
        return false;
    }

    if (outRowBuffer != NULL)
    {
        memcpy(outRowBuffer, localBuffer, header.rowSize);
    }

    // Konsolda göstərək
    printf("Row Index [%d] oxundu: ", rowIndex);
    // (İstəyə görə burada configs oxunub sütun tipinə görə print də edilə bilər)
    return true;
}

Cursor insertRowsWithDelete(const char *tableName, const char *colsStr, const char *valsStr, int colCount)
{
    Cursor cursor;
    cursor.lastOffset = 0;
    printf("[DEBUG]: Insert əməliyyatı başlayır: %s\n", tableName);
    printf("[DEBUG]: Gələn Sütunlar: %s | Dəyərlər: %s\n", colsStr, valsStr);

    // 1. Cədvəl ID-ni tap
    int tableId = getTableIndexByName(tableName);
    if (tableId == -1)
    {
        printf("[XƏTA]: '%s' cədvəli tapılmadı!\n", tableName);
        return cursor;
    }

    // 2. Metadata-dan sütun strukturlarını yüklə
    ColumnConfig configs[MAX_COLUMNS];
    int actualColCount = loadConfigsForTable(tableId, configs);
    printf("[DEBUG]: Metadata-dan %d sütun konfiqurasiyası yükləndi.\n", actualColCount);

    // 3. Row buffer-i hazırla
    uint8_t rowBuffer[512] = {0};
    rowBuffer[0] = 0; // is_deleted = 0

    // 4. Stringləri parçala
    char colsCopy[256], valsCopy[256];
    strncpy(colsCopy, colsStr, 256);
    strncpy(valsCopy, valsStr, 256);

    char *saveptr1, *saveptr2;
    char *colTok = strtok_r(colsCopy, ",", &saveptr1);
    char *valTok = strtok_r(valsCopy, ",", &saveptr2);

    int processedCount = 0;
    while (colTok != NULL && valTok != NULL && processedCount < colCount)
    {
        while (*colTok == ' ')
            colTok++;
        while (*valTok == ' ')
            valTok++;

        bool found = false;
        // ... Dövrün içərisi ...
        for (int i = 0; i < actualColCount; i++)
        {
            if (strcmp(configs[i].columnName, colTok) == 0)
            {
                found = true;

                // Ofset hesablaması: 1 bayt (is_deleted) + əvvəlki sütunların cəmi
                size_t offset = 1;
                for (int k = 0; k < i; k++)
                    offset += configs[k].dataSize;

                printf("[DEBUG]: Sütun: %s (Tip: %d, Ölçü: %d) -> Dəyər: %s\n",
                       configs[i].columnName, configs[i].typeID, configs[i].dataSize, valTok);

                // Tipə uyğun yazma
                switch (configs[i].typeID)
                {
                case TYPE_INT:
                case TYPE_UINT32:
                case TYPE_TIMESTAMP:
                {
                    uint32_t val = (uint32_t)atoi(valTok);
                    memcpy(rowBuffer + offset, &val, 4);
                }
                break;
                case TYPE_UINT8:
                {
                    uint8_t val = (uint8_t)atoi(valTok);
                    memcpy(rowBuffer + offset, &val, 1);
                }
                break;
                case TYPE_FLOAT:
                {
                    float val = (float)atof(valTok);
                    memcpy(rowBuffer + offset, &val, 4);
                }
                break;
                case TYPE_FIXED_POINT:
                {
                    int16_t val = (int16_t)(atof(valTok) * 100.0f);
                    memcpy(rowBuffer + offset, &val, 2);
                }
                break;
                case TYPE_CHAR2:
                {
                    strncpy((char *)(rowBuffer + offset), valTok, configs[i].dataSize);
                }
                break;
                default:
                    printf("[XƏTA]: Naməlum tip ID: %d\n", configs[i].typeID);
                }
                break;
            }
        }

        if (!found)
        {
            printf("[XƏTA]: Sütun tapılmadı: '%s'. SQL Parseri yoxlayın!\n", colTok);
        }

        colTok = strtok_r(NULL, ",", &saveptr1);
        valTok = strtok_r(NULL, ",", &saveptr2);
        processedCount++;
    }

    // 5. Fayla yaz
    char path[256];
    snprintf(path, sizeof(path), "%s/tables/%s.db", current_db_path, tableName);
    FILE *file = fopen(path, "rb+");
    if (!file)
        return cursor;

    DBHeader header;
    fread(&header, sizeof(DBHeader), 1, file);

    uint32_t targetRowIndex = 0;
    bool foundDeletedRow = false;

    // YENİ: Silinmiş sətir axtarışı (is_deleted == 1)
    long startDataPos = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);
    uint8_t statusByte;

    for (uint32_t r = 0; r < header.rowCount; r++)
    {
        fseek(file, startDataPos + (r * header.rowSize), SEEK_SET);
        fread(&statusByte, 1, 1, file);

        if (statusByte == 1) // Silinmiş sətir tapıldı
        {
            targetRowIndex = r;
            foundDeletedRow = true;
            break;
        }
    }
    // Əgər silinmiş sətir tapılmadısa, sonuncu sıranı götür
    if (!foundDeletedRow)
    {
        targetRowIndex = header.rowCount % header.maxRows;
    }

    // uint32_t rowIndex = header.rowCount % header.maxRows;
    long writeOffset = startDataPos + (targetRowIndex * header.rowSize);
    fseek(file, writeOffset, SEEK_SET);
    fwrite(rowBuffer, header.rowSize, 1, file);

    // long writeOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount) + (rowIndex * header.rowSize);

    // fseek(file, writeOffset, SEEK_SET);
    // fwrite(rowBuffer, header.rowSize, 1, file);

    if (!foundDeletedRow)
    {
        header.rowCount++;
        fseek(file, 0, SEEK_SET);
        fwrite(&header, sizeof(DBHeader), 1, file);
    }

    fclose(file);
    printf("[UĞURLU]: Məlumat %d-ci sıraya (deleted reuse: %s) yazıldı.\n",
           targetRowIndex, foundDeletedRow ? "BƏLİ" : "XEYR");
    cursor.lastOffset = targetRowIndex+1;
    return cursor;
}

#endif