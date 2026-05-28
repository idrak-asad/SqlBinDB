// ====================================================================
// 4. INSERT ROWS (Dairəvi və İtkisiz Model)
// ====================================================================
// bool insertRows(const char *tableName, void *dataPointer[], int dataCount) {
//     if (strlen(current_db_path) == 0) {
//         printf("XETA: Evvelce bir verilener bazasina qoshulun!\n");
//         return false;
//     }

//     char tableFilePath[256];
//     snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

//     // 1. Cədvəl faylını oxuma və yazma rejimində açırıq
//     FILE *file = fopen(tableFilePath, "rb+");
//     if (!file) {
//         printf("Error: '%s' cadvali tapilmadi!\n", tableName);
//         return false;
//     }

//     DBHeader header;
//     fread(&header, sizeof(DBHeader), 1, file);

//     ColumnConfig configs[MAX_COLUMNS + 1];
//     fread(configs, sizeof(ColumnConfig), header.columnCount, file);

//     // Sxem üzrə sütun sayı yoxlanışı (Gizli "is_deleted" sütunu çıxılmaqla)
//     if (dataCount != (header.columnCount - 1)) {
//         printf("Error: Sutun sayi uygun gelmir! Gozlenilen: %d, Gelen: %d\n", header.columnCount - 1, dataCount);
//         fclose(file);
//         return false;
//     }

//     uint8_t thisTableId = getTableIdByName(tableName);

//     // 2. FOREIGN KEY (REFERENTIAL INTEGRITY) YOXLANIŞI
//     char relPath[256];
//     snprintf(relPath, sizeof(relPath), "%s/metadata/relations.db", current_db_path);

//     FILE *fRel = fopen(relPath, "rb");
//     if (fRel) {
//         CompactRelation rel;
//         // Əgər bu cədvəl hər hansı bir əlaqədə "Child" (Asılı) tərəfdirsə:
//         while (fread(&rel, sizeof(CompactRelation), 1, fRel)) {
//             if (rel.is_deleted == 0 && rel.child_table_id == thisTableId) {

//                 // Daxil edilən xarici ID-ni tapırıq (rel.child_col_id - 1 indeksindədir)
//                 uint32_t insertedFkVal = *(uint32_t *)dataPointer[rel.child_col_id - 1];

//                 // Fərz edək ki, valideyn cədvəlin adı "users"-dir (tables.db-dən də oxuna bilər)
//                 char parentTableName[64] = "users";
//                 char parentTableFilePath[256];
//                 snprintf(parentTableFilePath, sizeof(parentTableFilePath), "%s/tables/%s.db", current_db_path, parentTableName);

//                 FILE *fParent = fopen(parentTableFilePath, "rb");
//                 if (!fParent) {
//                     printf("XETA: Valideyn cedvel fayli (%s.db) tapilmadi!\n", parentTableName);
//                     fclose(fRel);
//                     fclose(file);
//                     return false;
//                 }

//                 DBHeader pHeader;
//                 fread(&pHeader, sizeof(DBHeader), 1, fParent);

//                 // Sətirlərin başladığı bayt ünvanına keçirik
//                 fseek(fParent, sizeof(DBHeader) + (sizeof(ColumnConfig) * pHeader.columnCount), SEEK_SET);

//                 uint8_t pRowBuffer[256];
//                 bool idExistsInParent = false;

//                 // Valideyn cədvəli sətir-sətir diskdən oxuyub ID-ni axtarırıq (ESP32 RAM dostu)
//                 for (uint32_t pi = 0; pi < pHeader.rowCount; pi++) {
//                     fread(pRowBuffer, pHeader.rowSize, 1, fParent);

//                     if (pRowBuffer[0] == 1) continue; // Silinmiş valideyn sətirləri keçirik

//                     // Fərz edirik ki, ID həmişə ilk real data sütunudur (Ofset = 1)
//                     uint32_t parentIdVal = *(uint32_t *)(pRowBuffer + 1);

//                     if (parentIdVal == insertedFkVal) {
//                         idExistsInParent = true; // Tapıldı!
//                         break;
//                     }
//                 }
//                 fclose(fParent);

//                 // Əgər valideyn cədvəldə bu ID yoxdursa və ya silinibsə, INSERT-i bloklayırıq!
//                 if (!idExistsInParent) {
//                     printf("FOREIGN KEY INTEGRITY VIOLATION: '%s' cedveline data daxil edile bilmez! ", tableName);
//                     printf("Cunki '%s' cedvelinde id = %u olan aktiv qeyd yoxdur!\n", parentTableName, insertedFkVal);
//                     fclose(fRel);
//                     fclose(file);
//                     return false;
//                 }
//             }
//         }
//         fclose(fRel);
//     }

//     // 3. DATANIN BİNAR PAKETLƏNMƏSİ
//     uint8_t rowBuffer[256]; // Sabit bufer yaddaş fraqmentasiyasının qarşısını alır
//     int offset = 0;

//     // Ən başa aktiv sətir bayrağını yazırıq (is_deleted = 0)
//     uint8_t isDeleted = 0;
//     memcpy(rowBuffer + offset, &isDeleted, 1);
//     offset += 1;

//     // Sxemə uyğun olaraq dataları buferə köçürürük
//     for (int i = 0; i < dataCount; i++) {
//         int currentSize = configs[i + 1].byteSize;
//         memcpy(rowBuffer + offset, dataPointer[i], currentSize);
//         offset += currentSize;
//     }

//     // 4. DAİRƏVİ MƏNTİQLƏ YAZILMA NÖQTƏSİNİN HESABLANMASI VƏ YAZILMASI
//     long writePosition = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount) + (header.nextRowIndex * header.rowSize);
//     fseek(file, writePosition, SEEK_SET);
//     fwrite(rowBuffer, header.rowSize, 1, file);

//     // İndeksin növbəti mərhələ üçün fırladılması
//     header.nextRowIndex = (header.nextRowIndex + 1) % header.maxRows;

//     // Əgər cədvəl tam dolmayıbsa sətir sayını artırırıq
//     if (header.rowCount < header.maxRows) {
//         header.rowCount++;
//     }

//     // Yenilənmiş başlığı faylın əvvəlinə yazırıq
//     fseek(file, 0, SEEK_SET);
//     fwrite(&header, sizeof(DBHeader), 1, file);

//     fclose(file);
//     printf("Ugurlu: 1 satir '%s' cadvaline daxil edildi.\n", tableName);
//     return true;
// }

bool insertRows(const char *tableName, void *dataPointer[], int dataCount)
{
    if (strlen(current_db_path) == 0)
    {
        printf("XETA: Evvelce bir verilener bazasina qoshulun!\n");
        return false;
    }

    char tableFilePath[256];
    snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

    // Cədvəl faylını oxuma və yazma rejimində açırıq
    FILE *file = fopen(tableFilePath, "rb+");
    if (!file)
    {
        printf("Error: '%s' cadvali tapilmadi!\n", tableName);
        return false;
    }

    DBHeader header;
    fread(&header, sizeof(DBHeader), 1, file);

    ColumnConfig configs[MAX_COLUMNS + 1];
    fread(configs, sizeof(ColumnConfig), header.columnCount, file);

    // Sxem üzrə sütun sayı yoxlanışı (Gizli "is_deleted" sütunu çıxılmaqla)
    if (dataCount != (header.columnCount - 1))
    {
        printf("Error: Sutun sayi uygun gelmir! Gozlenilen: %d, Gelen: %d\n", header.columnCount - 1, dataCount);
        fclose(file);
        return false;
    }

    // ====================================================================
    // DATANIN BİNAR PAKETLƏNMƏSİ VƏ AUTO_INCREMENT MEXANİZMİ
    // ====================================================================
    uint8_t rowBuffer[256];
    int offset = 0;

    // Ən başa aktiv sətir bayrağını yazırıq (is_deleted = 0)
    uint8_t isDeleted = 0;
    memcpy(rowBuffer + offset, &isDeleted, 1);
    offset += 1;

    // Sxemə uyğun olaraq dataları buferə köçürürük
    for (int i = 0; i < dataCount; i++)
    {
        int currentSize = configs[i + 1].byteSize;

        // Əgər sütun "id"-dirsə və gələn dəyər 0-dırsa -> AUTO_INCREMENT işə düşür!
        if (strcmp(configs[i + 1].columnName, "id") == 0 && *(uint32_t *)dataPointer[i] == 0)
        {
            uint32_t autoId = header.rowCount + 1; // Sətir sayına əsasən yeni unikal ID
            memcpy(rowBuffer + offset, &autoId, currentSize);

            // dataPointer daxilindəki dəyəri də yeniləyirik ki, İndeks faylına doğru ID getsin!
            *(uint32_t *)dataPointer[i] = autoId;
        }
        else
        {
            // Normal qaydada istifadəçinin göndərdiyi datanı yazırıq
            memcpy(rowBuffer + offset, dataPointer[i], currentSize);
        }
        offset += currentSize;
    }

    // ====================================================================
    // DAİRƏVİ MƏNTİQLƏ YAZILMA NÖQTƏSİNİN HESABLANMASI
    // ====================================================================
    long writePosition = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount) + (header.nextRowIndex * header.rowSize);
    fseek(file, writePosition, SEEK_SET);
    fwrite(rowBuffer, header.rowSize, 1, file);

    // İndeksin növbəti mərhələ üçün fırladılması (Dairəvi Bufer)
    header.nextRowIndex = (header.nextRowIndex + 1) % header.maxRows;

    // Əgər cədvəl tam dolmayıbsa sətir sayını artırırıq
    if (header.rowCount < header.maxRows)
    {
        header.rowCount++;
    }

    // Yenilənmiş başlığı faylın əvvəlinə yazırıq
    fseek(file, 0, SEEK_SET);
    fwrite(&header, sizeof(DBHeader), 1, file);

    fclose(file);
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
                currentOffset += configs[i].byteSize;
            }

            if (foundIdx == -1)
            {
                allConditionsMatch = false;
                break;
            }

            // Müqayisə əməliyyatı
            if (!compareValues(rowBuffer + currentOffset, whereColumnsData[w], whereOperators[w], configs[foundIdx].columnType))
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
                    currentOffset += configs[i].byteSize;
                }

                if (foundIdx != -1)
                {
                    // Yeni datanı sətir buferindəki uyğun xanaya yazırıq
                    memcpy(rowBuffer + currentOffset, updateColumnsData[u], configs[foundIdx].byteSize);
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
uint8_t deleteRows(const char *tableName, char *whereColumnsName[], void *whereColumnsData[], char *whereOperators[], int hardDelete)
{
    // Qayda 0: Əgər mod 0-dırsa, proses tamamilə ləğv olunur
    if (hardDelete == 0)
    {
        printf("Melumat: hardDelete = 0 oldugu ucun silme prosesi legv edildi.\n");
        return 0;
    }

    if (strlen(current_db_path) == 0)
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

    int whereCount = 0;
    while (whereColumnsName[whereCount] != NULL)
        whereCount++;

    uint8_t rowBuffer[256];
    uint8_t deletedCount = 0;
    uint8_t thisTableId = getTableIdByName(tableName);
    long startPosition = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);

    for (uint32_t r = 0; r < header.rowCount; r++)
    {
        long rowPos = startPosition + (r * header.rowSize);
        fseek(file, rowPos, SEEK_SET);
        fread(rowBuffer, header.rowSize, 1, file);

        if (rowBuffer[0] == 1)
            continue;

        bool allConditionsMatch = true;
        for (int w = 0; w < whereCount; w++)
        {
            int currentOffset = 1;
            int foundIdx = -1;
            for (int i = 1; i < header.columnCount; i++)
            {
                if (strcmp(configs[i].columnName, whereColumnsName[w]) == 0)
                {
                    foundIdx = i;
                    break;
                }
                currentOffset += configs[i].byteSize;
            }

            if (foundIdx == -1 || !compareValues(rowBuffer + currentOffset, whereColumnsData[w], whereOperators[w], configs[foundIdx].columnType))
            {
                allConditionsMatch = false;
                break;
            }
        }

        // Şərtlər tam ödənilirsə silmə məntiqi işə düşür
        if (allConditionsMatch && whereCount > 0)
        {

            // QAYDA 1: HƏR İKİ TƏRƏFDƏN SİLMƏ (CASCADE - hardDelete == 1)
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
                        // Əgər biz valideyn cədvəliksə və uşağımız varsa:
                        if (rel.is_deleted == 0 && rel.parent_table_id == thisTableId)
                        {
                            // Bizdən silinən bu sətrin ID-sini (Açarını) götürürük (Fərz edək ki, id 1-ci sütundur)
                            uint32_t parentIdVal = *(uint32_t *)(rowBuffer + 1);

                            // İNDİ UŞAQ CƏDVƏLDƏN BU ID-YƏ BAĞLI OLANLARI DA SİLİRİK (Zəncirvari)
                            // Bu funksiya özü-özünü çağırır (Rekursiv Cascade Silmə)
                            // char *childWhereCols[] = {"user_id", NULL}; // Nümunə xarici sütun adı
                            // void *childWhereData[] = {&parentIdVal};
                            // char *childWhereOps[] = {"=", NULL};

                            const char *childWhereCols[] = {"user_id", NULL};
                            void *childWhereData[] = {&parentIdVal};
                            const char *childWhereOps[] = {"=", NULL};

                            // Uşaq cədvəlin adını təyin edib (Məs: devices) silirik:
                            deleteRows("devices", childWhereCols, childWhereData, childWhereOps, 1);
                        }
                    }
                    fclose(fRel);
                }
            }

            // QAYDA 2: QARŞI TƏRƏF HAVADA QALIR (hardDelete == 2) -> Uşaqlara toxunmuruq, birbaşa özümüzü silirik

            // Sətri soft-delete edirik
            uint8_t deleteFlag = 1;
            fseek(file, rowPos, SEEK_SET);
            fwrite(&deleteFlag, 1, 1, file);
            deletedCount++;
        }
    }

    fclose(file);
    return deletedCount;
}
