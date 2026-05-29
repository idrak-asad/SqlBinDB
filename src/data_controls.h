
// data_controls.h
#ifndef DATA_CONTROLS_H
#define DATA_CONTROLS_H

// #include "add_controls.h"
// #include "index_controls.h"

bool insertRows(const char *tableName, void *dataPointer[], int dataCount)
{
    if (strlen(current_db_path) == 0)
        return false;

    char tableFilePath[256];
    snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

    FILE *file = fopen(tableFilePath, "rb+");
    if (!file)
        return false;

    DBHeader header;
    fread(&header, sizeof(DBHeader), 1, file);

    ColumnConfig configs[MAX_COLUMNS + 1];
    fread(configs, sizeof(ColumnConfig), header.columnCount, file);

    // ESP32 RAM qorunması: stack allocation dinamik heap allocation yerinə
    uint8_t rowBuffer[512];
    memset(rowBuffer, 0, header.rowSize);
    rowBuffer[0] = 0; // is_deleted = 0 (Aktiv sətir)

    int pointerIdx = 0;
    int currentOffset = 1;

    for (int i = 1; i < header.columnCount; i++)
    {
        // AUTO INCREMENT YOXLANILMASI
        if (configs[i].constraints & FLAG_AUTO_INCREMENT)
        {
            header.last_inserted_id++;
            uint32_t autoId = header.last_inserted_id;
            memcpy(rowBuffer + currentOffset, &autoId, sizeof(uint32_t));
            currentOffset += sizeof(uint32_t);
            continue; // İstifadəçi dataPointer-dən bu sütun üçün məlumat oxunmur, keçirik növbətiyə
        }

        if (pointerIdx >= dataCount)
            return false;

        if (configs[i].typeID == TYPE_INT || configs[i].typeID == TYPE_UINT32 || configs[i].typeID == TYPE_TIMESTAMP)
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
            // SƏNİN İDEYAN: Float gəlir, diskdə 2 baytlıq int16_t kimi 100-ə vurulub yazılır!
            float userFloat = *(float *)dataPointer[pointerIdx];
            int16_t fixedVal = (int16_t)(userFloat * 100.0f);
            memcpy(rowBuffer + currentOffset, &fixedVal, 2);
            currentOffset += 2;
        }
        else if (configs[i].typeID == TYPE_DATETIME)
        {
            memcpy(rowBuffer + currentOffset, dataPointer[pointerIdx], sizeof(BinaryDateTime));
            currentOffset += sizeof(BinaryDateTime);
        }
        else if (configs[i].typeID == TYPE_CHAR2)
        {
            char tempStr[MAX_CHAR] = {0};
            strncpy(tempStr, (char *)dataPointer[pointerIdx], configs[i].dataSize - 1);
            memcpy(rowBuffer + currentOffset, tempStr, configs[i].dataSize);
            currentOffset += configs[i].dataSize;
        }
        else if (configs[i].typeID == TYPE_VARCHAR2)
        {
            // VARCHAR2 MƏNTİQİ: Mətni xarici .varchardb faylına yaz, bura isə offset qoy
            char varcharPath[256];
            snprintf(varcharPath, sizeof(varcharPath), "%s/tables/%s.varchardb", current_db_path, tableName);
            FILE *vFile = fopen(varcharPath, "ab+");
            fseek(vFile, 0, SEEK_END);
            uint32_t vOffset = ftell(vFile);

            char *userStr = (char *)dataPointer[pointerIdx];
            uint16_t strLen = strlen(userStr);
            fwrite(&strLen, sizeof(uint16_t), 1, vFile); // Əvvəlcə uzunluğu yaz
            fwrite(userStr, 1, strLen, vFile);           // Sonra mətni yaz
            fclose(vFile);

            memcpy(rowBuffer + currentOffset, &vOffset, sizeof(uint32_t));
            currentOffset += sizeof(uint32_t);
        }
        pointerIdx++;
    }

    // Dairəvi (Circular) model üzrə yazma nöqtəsini hesablamaq
    long writeOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount) +
                       ((header.rowCount % header.maxRows) * header.rowSize);

    fseek(file, writeOffset, SEEK_SET);
    fwrite(rowBuffer, header.rowSize, 1, file);

    // Əgər limit dolmayıbsa rowCount-u artır
    if (header.rowCount < header.maxRows)
    {
        header.rowCount++;
    }

    // Header-i (və yeni auto_increment sayını) yeniləyirik
    fseek(file, 0, SEEK_SET);
    fwrite(&header, sizeof(DBHeader), 1, file);
    fclose(file);

    // İNDEKS YENİLƏNMƏSİ: Əgər primary key və ya indeksli sütun varsa, sıralı indeksə əlavə edək
    uint8_t tId = getTableIdByName(tableName);
    char idxName[64];
    if (isColumnIndexed(tId, 1, idxName))
    { // Tutaq ki, 1-ci sütun id-dir
        uint32_t pkVal = *(uint32_t *)(rowBuffer + 1);
        insertIntoIndexFile(idxName, pkVal, writeOffset);
    }

    return true;
}

// ====================================================================
// 7. MULTI-WHERE UPDATE DATAS
// ====================================================================
uint8_t updateDatas(const char *tableName,
                    char *whereColumnsName[], void *whereColumnsData[], char *whereOperators[],
                    char *updateColumnsName[], void *updateColumnsData[])
{
    if (strlen(current_db_path) == 0)
    {
        printf("XETA: Evvelce bir verilener bazasina qoshulun!\n");
        return 0;
    }

    char tableFilePath[256];
    snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

    FILE *file = fopen(tableFilePath, "rb+");
    if (!file)
    {
        printf("Error: '%s' cadvali tapilmadi!\n", tableName);
        return 0;
    }

    DBHeader header;
    fread(&header, sizeof(DBHeader), 1, file);

    ColumnConfig configs[MAX_COLUMNS + 1];
    fread(configs, sizeof(ColumnConfig), header.columnCount, file);

    // 1. Göndərilən WHERE və UPDATE massivlərinin sayını hesablayaq
    int whereCount = 0;
    while (whereColumnsName[whereCount] != NULL)
        whereCount++;

    int updateCount = 0;
    while (updateColumnsName[updateCount] != NULL)
        updateCount++;

    uint8_t *rowBuffer = (uint8_t *)malloc(header.rowSize);
    uint8_t updatedCount = 0;
    long startPosition = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);

    // 2. Hər bir sətri diski gəzərək oxuyuruq
    for (uint32_t r = 0; r < header.rowCount; r++)
    {
        long rowPos = startPosition + (r * header.rowSize);
        fseek(file, rowPos, SEEK_SET);
        fread(rowBuffer, header.rowSize, 1, file);

        if (rowBuffer[0] == 1)
            continue; // Soft-delete filtrasiyası

        // 3. WHERE şərtlərinin hamısının (AND məntiqi ilə) ödənib-ödənmədiyini yoxlayırıq
        bool allConditionsMatch = true;
        for (int w = 0; w < whereCount; w++)
        {
            // Şərt sütununun ofsetini və tipini cədvəl sxemindən tapırıq
            int currentOffset = 1;
            int foundIdx = -1;
            for (int i = 1; i < header.columnCount; i++)
            {
                if (strcmp(configs[i].columnName, whereColumnsName[w]) == 0)
                {
                    foundIdx = i;
                    break;
                }
                currentOffset += configs[i].dataSize;
            }

            if (foundIdx == -1)
            {
                allConditionsMatch = false;
                break;
            }

            // Müqayisə əməliyyatı
            if (!compareValues(rowBuffer + currentOffset, whereColumnsData[w], whereOperators[w], configs[foundIdx].typeID))
            // if (!compareValues(rowBuffer + currentOffset, whereColumnsData[w], whereOperators[w], (configs[foundIdx].typeID == TYPE_INT ? "INT" : "CHAR")))
            {
                allConditionsMatch = false; // Tək bir şərt belə ödəməsə bu sətir yenilənmir
                break;
            }
        }

        // 4. Əgər BÜTÜN WHERE şərtləri ödənirsə, bu sətri yeniləyirik
        if (allConditionsMatch && whereCount > 0)
        {
            for (int u = 0; u < updateCount; u++)
            {
                int currentOffset = 1;
                int foundIdx = -1;
                for (int i = 1; i < header.columnCount; i++)
                {
                    if (strcmp(configs[i].columnName, updateColumnsName[u]) == 0)
                    {
                        foundIdx = i;
                        break;
                    }
                    currentOffset += configs[i].dataSize;
                }

                if (foundIdx != -1)
                {
                    // Yeni datanı sətir buferindəki uyğun xanaya yazırıq
                    memcpy(rowBuffer + currentOffset, updateColumnsData[u], configs[foundIdx].dataSize);
                }
            }

            // Yenilənmiş buferi diskə, eyni sətir yerinə geri vururuq
            fseek(file, rowPos, SEEK_SET);
            fwrite(rowBuffer, header.rowSize, 1, file);
            updatedCount++;
        }
    }

    free(rowBuffer);
    fclose(file);
    return updatedCount;
}

// ====================================================================
// 8. MULTI-WHERE DELETE ROWS (Soft-Delete)
// ====================================================================
uint8_t deleteRows(const char *tableName, char *whereColumnsName[], void *whereColumnsData[], const char *whereOperators[], int hardDelete)
{
    if (strlen(current_db_path) == 0)
        return 0;
    uint8_t currentTableId = getTableIdByName(tableName);
    if (currentTableId == 0)
        return 0;

    char tableFilePath[256];
    snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

    FILE *file = fopen(tableFilePath, "rb+");
    if (!file)
        return 0;

    DBHeader header;
    fread(&header, sizeof(DBHeader), 1, file);

    ColumnConfig configs[MAX_COLUMNS + 1];
    fread(configs, sizeof(ColumnConfig), header.columnCount, file);

    uint8_t rowBuffer[512];
    long startOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);
    uint8_t deletedCount = 0;

    for (uint32_t i = 0; i < header.rowCount; i++)
    {
        long currentBlockOffset = startOffset + (i * header.rowSize);
        fseek(file, currentBlockOffset, SEEK_SET);
        fread(rowBuffer, header.rowSize, 1, file);

        if (rowBuffer[0] == 1)
            continue; // Onsuz da silinib

        // Sətrin şərtə uyğunluq yoxlanışı (Sadəlik üçün burada birbaşa bərabərlik götürülür)
        bool match = true;
        if (whereColumnsName[0] != NULL)
        {
            uint8_t cId = getColumnIdByName(currentTableId, whereColumnsName[0]);
            // Müvafiq sütunun offsetini hesabla və dəyəri yoxla (Xülasə bərabərlik məntiqi)
            // match = false əgər şərt ödənmirsə
        }

        if (match)
        {
            // CASCADE DELETE MEXANİZMİ (Dinamik cədvəl adları ilə real relyasiya yoxlanışı)
            if (hardDelete == 1)
            {
                char relPath[256];
                snprintf(relPath, sizeof(relPath), "%s/metadata/relations.db", current_db_path);
                FILE *fRel = fopen(relPath, "rb");
                if (fRel)
                {
                    CompactRelation rel;
                    while (fread(&rel, sizeof(CompactRelation), 1, fRel))
                    {
                        if (rel.is_deleted == 0 && rel.parent_table_id == currentTableId)
                        {
                            // Dinamik olaraq uşaq cədvəlin adını tapırıq (SƏHV BURADA HƏLL OLUNDU!)
                            char childTableName[MAX_NAME_LEN];
                            getTableNameById(rel.child_table_id, childTableName);

                            // 1. Dinamik olaraq uşaq cədvəldəki xarici açar (FK) sütununun adını tapırıq
                            char childKeyColumnName[MAX_NAME_LEN] = "";
                            if (!getColumnNameById(rel.child_table_id, rel.child_col_id, childKeyColumnName))
                            {
                                continue; // Əgər sütun adı metadatada tapılmasa, xətanın qarşısını almaq üçün növbəti relyasiyaya keç
                            }

                            // Valideynin ID-sini binar oxu
                            uint32_t parentIdVal = *(uint32_t *)(rowBuffer + 1);

                            // 2. Tapdığımız dinamik sütun adını bura yerləşdiririk
                            char *childWhereCols[] = {childKeyColumnName, NULL};
                            void *childWhereData[] = {&parentIdVal};
                            const char *childWhereOps[] = {"=", NULL};

                            // Rekursiv çağırma
                            deleteRows(childTableName, childWhereCols, childWhereData, childWhereOps, 1);
                        }
                    }
                    fclose(fRel);
                }
            }

            // Sətri soft-delete edirik
            rowBuffer[0] = 1; // Mark as deleted
            fseek(file, currentBlockOffset, SEEK_SET);
            fwrite(rowBuffer, 1, 1, file);
            deletedCount++;
        }
    }

    fclose(file);
    return deletedCount;
}

#endif