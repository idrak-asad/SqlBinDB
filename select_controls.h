
// ====================================================================
// 5. SELECT DATA (Məlumatları Oxumaq)
// ====================================================================
void selectData(const char *tableName) {
    if (strlen(current_db_path) == 0) {
        printf("XETA: Evvelce bir verilener bazasina qoshulun!\n");
        return;
    }

    char tableFilePath[256];
    snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

    FILE *file = fopen(tableFilePath, "rb");
    if (!file) {
        printf("Error: '%s' cadvali oxunarken xeta!\n", tableName);
        return;
    }

    DBHeader header;
    fread(&header, sizeof(DBHeader), 1, file);

    ColumnConfig configs[MAX_COLUMNS + 1];
    fread(configs, sizeof(ColumnConfig), header.columnCount, file);

    printf("\n=== CADVEL DATA: %s ===\n", tableName);

    // Başlıqları çıxarırıq
    for (int i = 1; i < header.columnCount; i++) {
        printf("%-15s\t", configs[i].columnName);
    }
    printf("\n--------------------------------------------------\n");

    uint8_t *rowBuffer = (uint8_t *)malloc(header.rowSize);

    // Sətirləri yalnız mövcud rowCount qədər oxuyuruq
    for (uint32_t r = 0; r < header.rowCount; r++) {
        fread(rowBuffer, header.rowSize, 1, file);

        uint8_t isDeleted = rowBuffer[0];
        if (isDeleted == 1) continue; // Soft-delete filtri

        int offset = 1;

        for (int i = 1; i < header.columnCount; i++) {
            if (strcmp(configs[i].columnType, "uint32_t") == 0 || strcmp(configs[i].columnType, "int") == 0) {
                uint32_t val;
                memcpy(&val, rowBuffer + offset, 4);
                printf("%-15u\t", val);
                offset += 4;
            }
            else if (strcmp(configs[i].columnType, "uint8_t") == 0) {
                uint8_t val;
                memcpy(&val, rowBuffer + offset, 1);
                printf("%-15u\t", val);
                offset += 1;
            }
            else if (strncmp(configs[i].columnType, "char(", 5) == 0) {
                int len = configs[i].byteSize;
                char *strStr = (char *)malloc(len + 1);
                memcpy(strStr, rowBuffer + offset, len);
                strStr[len] = '\0';
                printf("%-15s\t", strStr);
                offset += len;
                free(strStr);
            }
        }
        printf("\n");
    }

    free(rowBuffer);
    fclose(file);
    printf("==================================================\n\n");
}

// ====================================================================
// 6. DEBUG SELECT STAR (Bütün Binar Strukturu Görmək Üçün)
// ====================================================================
void debugSelectStar(const char *tableName) {
    if (strlen(current_db_path) == 0) {
        printf("XETA: Evvelce bir verilener bazasina qoshulun!\n");
        return;
    }

    char tableFilePath[256];
    snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

    FILE *file = fopen(tableFilePath, "rb");
    if (!file) {
        printf("Error: '%s' cadvali tapilmadi!\n", tableName);
        return;
    }

    DBHeader header;
    fread(&header, sizeof(DBHeader), 1, file);

    ColumnConfig configs[MAX_COLUMNS + 1];
    fread(configs, sizeof(ColumnConfig), header.columnCount, file);

    printf("\n==================================================\n");
    printf("     RAW BINARY DUMP (CADVEL: %s)\n", tableName);
    printf("==================================================\n");
    printf("Hazirki Real Satir Sayi (RowCount): %u\n", header.rowCount);
    printf("Maksimum Satir Limiti (MaxRows)  : %u\n", header.maxRows);
    printf("Novbeti Yazilacaq İndex (NextRow) : %u\n", header.nextRowIndex);
    printf("Bir Satrin Olcusu (RowSize)       : %u byte\n", header.rowSize);
    printf("--------------------------------------------------\n\n");

    uint8_t *rowBuffer = (uint8_t *)malloc(header.rowSize);

    for (uint32_t r = 0; r < header.rowCount; r++) {
        fread(rowBuffer, header.rowSize, 1, file);

        printf("SATIR #%d\n", r);
        uint8_t isDeleted = rowBuffer[0];
        printf("[Bayt 00] -> is_deleted: %u (%s)\n", isDeleted, (isDeleted == 1) ? "SILINIB" : "AKTIV");

        int offset = 1;
        for (int i = 1; i < header.columnCount; i++) {
            printf("[Bayt %02d] -> Sutun: %-12s | Deyer: ", offset, configs[i].columnName);

            if (strcmp(configs[i].columnType, "uint32_t") == 0 || strcmp(configs[i].columnType, "int") == 0) {
                uint32_t val;
                memcpy(&val, rowBuffer + offset, 4);
                printf("%u\n", val);
                offset += 4;
            }
            else if (strcmp(configs[i].columnType, "uint8_t") == 0) {
                uint8_t val;
                memcpy(&val, rowBuffer + offset, 1);
                printf("%u\n", val);
                offset += 1;
            }
            else if (strncmp(configs[i].columnType, "char(", 5) == 0) {
                int len = configs[i].byteSize;
                char *strStr = (char *)malloc(len + 1);
                memcpy(strStr, rowBuffer + offset, len);
                strStr[len] = '\0';
                printf("\"%s\"\n", strStr);
                offset += len;
                free(strStr);
            }
        }
        printf("\n");
    }

    free(rowBuffer);
    fclose(file);
    printf("==================================================\n");
}




// ====================================================================
// 9. MULTI-WHERE SELECT DATA
// ====================================================================
uint8_t selectWhere(const char *tableName, char *whereColumnsName[], void *whereColumnsData[], char *whereOperators[]) {
    if (strlen(current_db_path) == 0) {
        printf("XETA: Evvelce bir verilener bazasina qoshulun!\n");
        return 0;
    }

    char tableFilePath[256];
    snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

    FILE *file = fopen(tableFilePath, "rb");
    if (!file) {
        printf("Error: '%s' cadvali tapilmadi!\n", tableName);
        return 0;
    }

    DBHeader header;
    fread(&header, sizeof(DBHeader), 1, file);

    ColumnConfig configs[MAX_COLUMNS + 1];
    fread(configs, sizeof(ColumnConfig), header.columnCount, file);

    // 1. WHERE şərtlərinin sayını hesablayaq
    int whereCount = 0;
    while (whereColumnsName[whereCount] != NULL) whereCount++;

    printf("\n=== FILTERED SELECT: %s ===\n", tableName);

    // Cədvəlin sütun başlıqlarını ekrana çıxarırıq (Gizli sütun hariç)
    for (int i = 1; i < header.columnCount; i++) {
        printf("%-15s\t", configs[i].columnName);
    }
    printf("\n--------------------------------------------------\n");

    // ESP32 optimizasiyası: Dinamik malloc yerinə stack-də sabit ölçülü bufer (Maks 256 byte sətir üçün)
    uint8_t rowBuffer[256]; 
    if (header.rowSize > 256) {
        printf("XETA: Satir olcusu desteklenen buferden boyukdur!\n");
        fclose(file);
        return 0;
    }

    uint8_t matchCount = 0;
    long startPosition = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);

    // 2. Sətirləri diskdən parça-parça (tək-tək) oxuyuruq
    for (uint32_t r = 0; r < header.rowCount; r++) {
        long rowPos = startPosition + (r * header.rowSize);
        fseek(file, rowPos, SEEK_SET);
        fread(rowBuffer, header.rowSize, 1, file);

        if (rowBuffer[0] == 1) continue; // Soft-delete filtri

        // 3. Bütün şərtlərin ödənib-ödənmədiyini yoxlayırıq (AND məntiqi)
        bool allConditionsMatch = true;
        for (int w = 0; w < whereCount; w++) {
            int currentOffset = 1;
            int foundIdx = -1;
            
            for (int i = 1; i < header.columnCount; i++) {
                if (strcmp(configs[i].columnName, whereColumnsName[w]) == 0) {
                    foundIdx = i;
                    break;
                }
                currentOffset += configs[i].byteSize;
            }

            if (foundIdx == -1) {
                allConditionsMatch = false;
                break;
            }

            // `compareValues` köməkçi funksiyamız ilə binar müqayisə edirik
            if (!compareValues(rowBuffer + currentOffset, whereColumnsData[w], whereOperators[w], configs[foundIdx].columnType)) {
                allConditionsMatch = false;
                break;
            }
        }

        // 4. Əgər bütün WHERE şərtləri ödənirsə, sətri ekrana yazdırırıq
        if (allConditionsMatch || whereCount == 0) {
            int offset = 1;
            for (int i = 1; i < header.columnCount; i++) {
                if (strcmp(configs[i].columnType, "uint32_t") == 0 || strcmp(configs[i].columnType, "int") == 0) {
                    uint32_t val;
                    memcpy(&val, rowBuffer + offset, 4);
                    printf("%-15u\t", val);
                    offset += 4;
                }
                else if (strcmp(configs[i].columnType, "uint8_t") == 0) {
                    uint8_t val = rowBuffer[offset];
                    printf("%-15u\t", val);
                    offset += 1;
                }
                else if (strncmp(configs[i].columnType, "char(", 5) == 0) {
                    int len = configs[i].byteSize;
                    char strStr[64] = {0}; // Müvəqqəti string buferi
                    memcpy(strStr, rowBuffer + offset, len);
                    printf("%-15s\t", strStr);
                    offset += len;
                }
            }
            printf("\n");
            matchCount++;
        }
    }

    fclose(file);
    printf("--------------------------------------------------\n");
    printf("Tapilan sater sayi: %d\n==================================================\n\n", matchCount);
    return matchCount;
}




void selectJoin(const char *parentTable, const char *childTable, const char *parentKey, const char *childKey) {
    if (strlen(current_db_path) == 0) return;

    char pPath[256], cPath[256];
    snprintf(pPath, sizeof(pPath), "%s/tables/%s.db", current_db_path, parentTable);
    snprintf(cPath, sizeof(cPath), "%s/tables/%s.db", current_db_path, childTable);

    FILE *pInst = fopen(pPath, "rb");
    FILE *cInst = fopen(cPath, "rb");
    if (!pInst || !cInst) { 
        if(pInst) fclose(pInst); if(cInst) fclose(cInst); return; 
    }

    DBHeader pHead, cHead;
    fread(&pHead, sizeof(DBHeader), 1, pInst);
    fread(&cHead, sizeof(DBHeader), 1, cInst);

    ColumnConfig pConfigs[16], cConfigs[16];
    fread(pConfigs, sizeof(ColumnConfig), pHead.columnCount, pInst);
    fread(cConfigs, sizeof(ColumnConfig), cHead.columnCount, cInst);

    // Açarların ofsetlərini təyin edirik
    int pKeyOffset = 1, cKeyOffset = 1; // 0 is_deleted-dir
    for (int i = 1; i < pHead.columnCount; i++) {
        if (strcmp(pConfigs[i].columnName, parentKey) == 0) break;
        pKeyOffset += pConfigs[i].byteSize;
    }
    for (int i = 1; i < cHead.columnCount; i++) {
        if (strcmp(cConfigs[i].columnName, childKey) == 0) break;
        cKeyOffset += cConfigs[i].byteSize;
    }

    printf("\n=== JOIN RESULT: %s + %s ===\n", parentTable, childTable);
    printf("%-15s \t %-15s\n", "PARENT DATA", "CHILD DATA");
    printf("-----------------------------------------\n");

    uint8_t pBuffer[128], cBuffer[128];
    long pStart = sizeof(DBHeader) + (sizeof(ColumnConfig) * pHead.columnCount);
    long cStart = sizeof(DBHeader) + (sizeof(ColumnConfig) * cHead.columnCount);

    // SƏTİR-SƏTİR GƏZİNTİ (Stream Cross-Match)
    for (uint32_t pi = 0; pi < pHead.rowCount; pi++) {
        fseek(pInst, pStart + (pi * pHead.rowSize), SEEK_SET);
        fread(pBuffer, pHead.rowSize, 1, pInst);
        if (pBuffer[0] == 1) continue; // Soft-delete

        uint32_t pKeyValue = *(uint32_t *)(pBuffer + pKeyOffset);

        // Hər bir parent sətir üçün child sətirlərini tək-tək diskdən oxuyub yoxlayırıq (Yaddaşa tam qənaət)
        for (uint32_t ci = 0; ci < cHead.rowCount; ci++) {
            fseek(cInst, cStart + (ci * cHead.rowSize), SEEK_SET);
            fread(cBuffer, cHead.rowSize, 1, cInst);
            if (cBuffer[0] == 1) continue;

            uint32_t cKeyValue = *(uint32_t *)(cBuffer + cKeyOffset);

            // Əgər ID-lər bərabərdirsə (Match tapıldısa), ekrana çıxarırıq
            if (pKeyValue == cKeyValue) {
                // Sadəlik üçün ilk dataları ekrana çıxarırıq
                printf("ID: %-11u \t Match Found!\n", pKeyValue);
            }
        }
    }

    fclose(pInst);
    fclose(cInst);
    printf("=========================================\n\n");
}