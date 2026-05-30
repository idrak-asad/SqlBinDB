
// data_controls.h
#ifndef DATA_CONTROLS_H
#define DATA_CONTROLS_H

// #include "add_controls.h"
// #include "index_controls.h"

int insertRows(const char *tableName, void *dataPointer[], int dataCount)
{
    File file = openTable(tableName, "r+");

    if (!file)
    {
        // openTable daxilində artıq [KRİTİK XƏTA] loqları çıxır
        return 0; // Xəta halında ID = 0 qayıdır
    }

    Serial.println("[UĞURLU] Fayl LittleFS tərəfindən uğurla açıldı. Yazma prosesi başlayır...");

    DBHeader header;
    file.read((uint8_t *)&header, sizeof(DBHeader));

    ColumnConfig configs[MAX_COLUMNS + 1];
    file.read((uint8_t *)configs, sizeof(ColumnConfig) * header.columnCount);

    // ESP32 RAM qorunması üçün stack allocation
    uint8_t rowBuffer[512];
    memset(rowBuffer, 0, header.rowSize);
    rowBuffer[0] = 0; // is_deleted = 0 (Aktiv sətir)

    int pointerIdx = 0;
    int currentOffset = 1; // is_deleted baytından sonra başlayır
    uint32_t return_id = 0;
    Serial.println("Data İnsert prosesis başlayır");
    for (int i = 1; i < header.columnCount; i++)
    {

        // 1. AUTO INCREMENT YOXLANILMASI
        if (configs[i].constraints & FLAG_AUTO_INCREMENT)
        {
            Serial.println("İncriment edilir");
            header.last_inserted_id++;
            uint32_t autoId = header.last_inserted_id;
            return_id = autoId;
            memcpy(rowBuffer + currentOffset, &autoId, sizeof(uint32_t));
            currentOffset += sizeof(uint32_t);
            // Auto increment sütunu üçün dataPointer-dən məlumat oxunmur, pointerIdx ARTMIR!
            Serial.println("İncriment edildi");
            continue;
        }

        // Əgər göndərilən dataların sayı sütun sayından azdırsa crash-ın qarşısını almaq üçün qoruma
        if (pointerIdx >= dataCount || dataPointer[pointerIdx] == NULL)
        {
            Serial.println("row count test edilir");
            // Əgər data çatışmırsa, boşluq buraxıb offset-i irəli çəkirik
            currentOffset += configs[i].dataSize;
            pointerIdx++;
            Serial.println("row count test edildi");
            continue;
        }

        // 2. SİZİN SİSTEMİN REAL TİPLƏRİNƏ GÖRƏ BUFFERƏ KOPIYALAMA (typeID istifadə edilir)
        if (configs[i].typeID == TYPE_INT || configs[i].typeID == TYPE_UINT32)
        {
            memcpy(rowBuffer + currentOffset, dataPointer[pointerIdx], 4);
            currentOffset += 4;
        }
        else if (configs[i].typeID == TYPE_UINT8)
        {
            memcpy(rowBuffer + currentOffset, dataPointer[pointerIdx], 1);
            currentOffset += 1;
        }
        else if (configs[i].typeID == TYPE_FLOAT)
        {
            memcpy(rowBuffer + currentOffset, dataPointer[pointerIdx], 4);
            currentOffset += 4;
        }
        else if (configs[i].typeID == TYPE_FIXED_POINT)
        {
            // Sizin RAM qoruyan 2 baytlıq tipiniz
            memcpy(rowBuffer + currentOffset, dataPointer[pointerIdx], 2);
            currentOffset += 2;
        }
        else if (configs[i].typeID == TYPE_CHAR2)
        {
            // Sabit ölçülü mətn (Sizin configs[i].dataSize uzunluğunda)
            char tempStr[MAX_CHAR] = {0};
            strncpy(tempStr, (const char *)dataPointer[pointerIdx], configs[i].dataSize - 1);
            memcpy(rowBuffer + currentOffset, tempStr, configs[i].dataSize);
            currentOffset += configs[i].dataSize;
        }
        else if (configs[i].typeID == TYPE_VARCHAR2)
        {
            // Dinamik varchar sistemi (.varchardb faylına yazır)
            char varcharPath[256];
            snprintf(varcharPath, sizeof(varcharPath), "%s/tables/%s.varchardb", current_db_path, tableName);

            File vFile = LittleFS.open(varcharPath, "a+"); // Əgər yoxdursa yaradır, varsa sonuna əlavə edir
            uint32_t vOffset = 0;
            if (vFile)
            {
                vOffset = vFile.size();
                const char *userStr = (const char *)dataPointer[pointerIdx];
                uint16_t strLen = strlen(userStr);
                vFile.write((uint8_t *)&strLen, sizeof(uint16_t));
                vFile.write((const uint8_t *)userStr, strLen);
                vFile.close();
            }
            // Əsas faylda .varchardb-dəki yerin ofsetini (pointer) saxlayırıq
            memcpy(rowBuffer + currentOffset, &vOffset, sizeof(uint32_t));
            currentOffset += sizeof(uint32_t);
        }

        pointerIdx++; // Növbəti ötürülən məlumata keçid
    }
    Serial.println("Data Yazıldı");
    // 3. DAİRƏVİ (CIRCULAR) SİSTEMƏ UYGUN YAZMA NÖQTƏSİNİN HESABLANMASI
    // Sizin data_controls.h-dakı orijinal riyazi modeliniz:
    long writeOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount) +
                       ((header.rowCount % header.maxRows) * header.rowSize);

    if (file.seek(writeOffset, SeekSet))
    {
        file.write(rowBuffer, header.rowSize);
    }
    else
    {
        file.close();
        return false;
    }
    Serial.println("Data Save edilir");
    // Əgər cədvəlin limiti (maxRows) dolmayıbsa ümumi sətir sayını artırırıq
    if (header.rowCount < header.maxRows)
    {
        header.rowCount++;
    }

    // Başlığı (yeni rowCount və last_inserted_id-ni) faylın əvvəlinə yenidən yazırıq
    if (file.seek(0, SeekSet))
    {
        file.write((uint8_t *)&header, sizeof(DBHeader));
    }

    file.flush();
    file.close();
    return return_id;
}

uint8_t deleteRows(const char *tableName, char *whereColumnsName[], void *whereColumnsData[], const char *whereOperators[], int hardDelete)
{
    if (strlen(current_db_path) == 0)
        return 0;

    char tableFilePath[256];
    snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

    // "r+" həm oxumaq, həm də üzərinə yaza bilmək üçün mütləqdir
    File file = LittleFS.open(tableFilePath, "r+");
    if (!file)
        return 0;

    DBHeader header;
    file.read((uint8_t *)&header, sizeof(DBHeader));

    ColumnConfig configs[MAX_COLUMNS + 1];
    file.read((uint8_t *)configs, sizeof(ColumnConfig) * header.columnCount);

    uint8_t rowBuffer[512];
    uint32_t deletedCount = 0;
    long startOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);

    // İlk öncə cədvəlin ID-sini alırıq (Relyasiyaları yoxlamaq üçün)
    uint8_t currentTableId = getTableIdByName(tableName);

    for (uint32_t r = 0; r < header.rowCount; r++)
    {
        long currentBlockOffset = startOffset + (r * header.rowSize);

        if (!file.seek(currentBlockOffset, SeekSet))
            continue;
        if (file.read(rowBuffer, header.rowSize) != header.rowSize)
            continue;

        if (rowBuffer[0] == 1)
            continue; // Artıq silinmiş sətirdirsə, keç

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

                bool conditionMet = helperCheckCondition(rowBuffer + colOffset, configs[colIdx].typeID, whereColumnsData[w], whereOperators[w]);
                if (!conditionMet)
                {
                    matches = false;
                    break;
                }
                w++;
            }
        }

        // Əgər silinmək istənən sətir tapıldısa:
        if (matches)
        {
            bool allowDelete = true;

            // ====================================================================
            // RELYASİYA / ƏLAQƏLİ CƏDVƏL YOXlANIŞI
            // ====================================================================
            if (currentTableId != 0 && (hardDelete == 0 || hardDelete == 1))
            {
                char relPath[256];
                snprintf(relPath, sizeof(relPath), "%s/metadata/relations.db", current_db_path);

                File fRel = LittleFS.open(relPath, "r");
                if (fRel)
                {
                    CompactRelation rel;
                    while (fRel.read((uint8_t *)&rel, sizeof(CompactRelation)) == sizeof(CompactRelation))
                    {
                        if (rel.is_deleted == 0 && rel.parent_table_id == currentTableId)
                        {

                            char childTableName[MAX_NAME_LEN];
                            getTableNameById(rel.child_table_id, childTableName);

                            char childKeyColumnName[MAX_NAME_LEN] = "";
                            if (!getColumnNameById(rel.child_table_id, rel.child_col_id, childKeyColumnName))
                                continue;

                            // Parent cədvəlin ID dəyərini götürürük (id adətən 1-ci ofsetdə yerləşir)
                            uint32_t parentIdVal = 0;
                            memcpy(&parentIdVal, rowBuffer + 1, 4);

                            // Alt cədvəldə bu ID-yə bağlı data olub-olmadığını yoxlamaq üçün filtr qururuq
                            char *childWhereCols[] = {childKeyColumnName, NULL};
                            void *childWhereData[] = {&parentIdVal};
                            const char *childWhereOps[] = {"=", NULL};

                            // Alt cədvəldəki dataların sayını yoxlayırıq
                            uint8_t childRowsCount = selectWhere(childTableName, (const char **)childWhereCols, childWhereData, childWhereOps);

                            if (childRowsCount > 0)
                            {
                                if (hardDelete == 0)
                                {
                                    // 🛑 REJİM 0: Alt cədvəldə data var, silməyə icazə yoxdur!
                                    Serial.printf("[MƏHDUDİYYƏT] '%s' cədvəlində əlaqəli data olduğu üçün '%s' silinə bilməz!\n", childTableName, tableName);
                                    allowDelete = false;
                                    break;
                                }
                                else if (hardDelete == 1)
                                {
                                    // 🔄 REJİM 1: CASCADE - Alt cədvəldəki bağlı dataları da silirik
                                    Serial.printf("[CASCADE] '%s' silindiyi üçün '%s' cədvəlindəki əlaqəli sətirlər də silinir...\n", tableName, childTableName);
                                    deleteRows(childTableName, childWhereCols, childWhereData, childWhereOps, 1);
                                }
                            }
                        }
                    }
                    fRel.close();
                }
            }

            // 🛑 Əgər hardDelete = 0 şərti pozulubsa, bu sətri silmədən növbəti sətirlərə keçirik
            if (!allowDelete)
                continue;

            // ====================================================================
            // REAL SİLİNMƏ ƏMƏLİYYATI (is_deleted = 1)
            // ====================================================================
            // REJİM 2 daxil olmaqla, əgər qadağa yoxdursa, əsas cədvəldəki sətir silinir
            rowBuffer[0] = 1;
            if (file.seek(currentBlockOffset, SeekSet))
            {
                file.write(rowBuffer, header.rowSize);
                deletedCount++;
            }
        }
    }

    file.close();
    return deletedCount;
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

    File file = LittleFS.open(tableFilePath, "r+");
    if (!file)
        return 0;

    DBHeader header;
    file.read((uint8_t *)&header, sizeof(DBHeader));

    ColumnConfig configs[MAX_COLUMNS + 1];
    file.read((uint8_t *)configs, sizeof(ColumnConfig) * header.columnCount);

    uint8_t rowBuffer[512];
    uint32_t updatedCount = 0;
    long startOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);

    for (uint32_t r = 0; r < header.rowCount; r++)
    {
        long currentBlockOffset = startOffset + (r * header.rowSize);

        if (!file.seek(currentBlockOffset, SeekSet))
            continue;
        if (file.read(rowBuffer, header.rowSize) != header.rowSize)
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

            if (file.seek(currentBlockOffset, SeekSet))
            {
                file.write(rowBuffer, header.rowSize);
                updatedCount++;
            }
        }
    }

    file.close();
    return updatedCount;
}

uint8_t deleteRowsByIndices(const char *tableName, uint32_t *rowIndices, uint8_t indicesCount)
{
    if (strlen(current_db_path) == 0 || indicesCount == 0 || rowIndices == NULL)
        return 0;

    // Standart C-nin fopen kilidini tam buraxması üçün 5ms gözləyirik (File Handle Release)
    delay(5);

    File file = openTable(tableName, "r+");
    if (!file)
        return 0;

    DBHeader header;
    if (file.read((uint8_t *)&header, sizeof(DBHeader)) != sizeof(DBHeader))
    {
        file.close();
        return 0;
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
        if (file.seek(currentBlockOffset, SeekSet))
        {
            size_t written = file.write(&deleteFlag, 1);

            if (written > 0)
            {
                deletedCount++;
                // Dəyişikliyi dərhal diskə məcburi yazırıq
                file.flush();
                Serial.printf("[KÖMƏKÇİ] Sıra No [%d] uğurla silindi (Ofset: %ld)\n", targetRow, currentBlockOffset);
            }
        }
    }

    file.close();
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

#endif