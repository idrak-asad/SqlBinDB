// select_controls.h
#ifndef SELECT_CONTROLS_H
#define SELECT_CONTROLS_H

// #include "add_controls.h"

// ====================================================================
// 5. SELECT DATA (Məlumatları Oxumaq)
// ====================================================================
// 1. Sadə SELECT * FROM table
// void selectData(const char *tableName);
Cursor selectData(const char *tableName, Cursor *prevCursor);

// 2. Şərtli SELECT FROM table WHERE cond
uint8_t selectWhere(const char *tableName,
                    const char *whereColumnsName[],
                    void *whereColumnsData[],
                    // uint8_t whereDataTypes[], // Sütun tipləri (Mütləqdir!)
                    const char *whereOperators[]);

// 3. Sadə JOIN: SELECT * FROM t1 JOIN t2 ON t1.id = t2.t1_id
void selectJoinData(const char *parentTable, const char *childTable,
                    const char *parentCol, const char *childCol);

// 4. HƏM JOIN, HƏM WHERE: Kombinasiya edilmiş funksiya
// void selectJoinWhereData(const char *parentTable, const char *childTable,
//                          const char *parentCol, const char *childCol,
//                          const char *whereColumnsName[], void *whereColumnsData[],
//                          const char *whereOperators[], int conditionCount);

// void selectJoinWhereData(const char *parentTable, const char *childTable,
//                          const char *parentCol, const char *childCol,
//                          const char *whereColumnsName[], void *whereColumnsData[],
//                          const char *whereOperators[], int conditionCount)
// {
// }
// void parseJoinCondition(char *joinCond[], char *parentCol[], char *childCol[]);

// void parseJoinCondition(char *joinCond[], char *parentCol[], char *childCol[])
// {
// }

// void selectLineByLine(const char *tableName, int id)
// {

//     Cursor newCursor;
//     newCursor.count = 0;
//     newCursor.rowIndices = malloc(sizeof(uint32_t) * 10);
//     newCursor.isFinished = false;
//     FileHandle file = openTable(tableName, "r"); // openTable funksiyanız da FileHandle qaytarmalıdır
//     if (!file)
//     {
//         newCursor.isFinished = true;
//         return newCursor;
//     }

//     DBHeader header;

//     DBHeader header;
//     if (myRead(file, &header, sizeof(DBHeader)) != sizeof(DBHeader))
//     {
//         myClose(file);
//         newCursor.isFinished = true;
//         return newCursor;
//     }
//     // 3. Config-ləri oxu
//     ColumnConfig configs[MAX_COLUMNS];
//     myRead(file, configs, sizeof(ColumnConfig) * header.columnCount);

//     // 4. Axtarışın qaldığı yerdən davam et
//     long startOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);
//     uint32_t r = prevCursor->lastOffset;

//     printf("\n=== TABLE DATA: %s ===\n", tableName);
//     printf("%-10s\t", "Deleted?");
//     for (int i = 1; i < header.columnCount; i++)
//     {
//         printf("%-15s\t", configs[i].columnName);
//     }
//     printf("\n----------------------------------------------------------------------------------\n");

//     uint8_t rowBuffer[512];
//     long startOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);
//     uint32_t r = prevCursor->lastOffset;

//     if (!mySeek(file, startOffset + (r * header.rowSize)))
//     {
//         newCursor.isFinished = true;
//         myClose(file);
//         return newCursor;
//     }

//     while (newCursor.count < 10)
//     {

//         if (myRead(file, rowBuffer, header.rowSize) != header.rowSize)
//         {
//             newCursor.isFinished = true; // Fayl bitdi
//             break;
//         }

//         // is_deleted = 0 yoxlanışı (rowBuffer[0] silinmə baytınızdır)
//         if (rowBuffer[0] == 0)
//         {
//             // Şərt yoxlanışı (Sizin helper funksiyanız)
//             // if (helperCheckCondition(rowBuffer, configs, header))
//             // {
//             //     newCursor.rowIndices[newCursor.count++] = r;
//             // }
//         }
//         r++;

//         // if (rowBuffer[0] == 1)
//         //     continue; // Silinmiş sətirləri keçirik
//         printf("%-10d\t", rowBuffer[0]);
//         int currentOffset = 1;
//         for (int i = 1; i < header.columnCount; i++)
//         {
//             if (configs[i].typeID == TYPE_INT || configs[i].typeID == TYPE_UINT32)
//             {
//                 uint32_t val = *(uint32_t *)(rowBuffer + currentOffset);
//                 printf("%-15d\t", val);
//                 currentOffset += 4;
//             }
//             else if (configs[i].typeID == TYPE_TIMESTAMP)
//             {
//                 uint32_t ts = *(uint32_t *)(rowBuffer + currentOffset);
//                 printf("%-15u (TS)\t", ts);
//                 currentOffset += 4;
//             }
//             else if (configs[i].typeID == TYPE_UINT8)
//             {
//                 uint8_t val = *(uint8_t *)(rowBuffer + currentOffset);
//                 printf("%-15d\t", val);
//                 currentOffset += 1;
//             }
//             else if (configs[i].typeID == TYPE_FLOAT)
//             {
//                 float val = *(float *)(rowBuffer + currentOffset);
//                 printf("%-15.2f\t", val);
//                 currentOffset += 4;
//             }
//             else if (configs[i].typeID == TYPE_FIXED_POINT)
//             {
//                 // SƏNİN METODUN: Diskdəki 2 baytlıq tam ədədi userə çıxaranda 100-ə bölüb real float edirik!
//                 int16_t fixedVal = *(int16_t *)(rowBuffer + currentOffset);
//                 float realFloat = (float)fixedVal / 100.0f;
//                 printf("%-15.2f\t", realFloat);
//                 currentOffset += 2;
//             }
//             else if (configs[i].typeID == TYPE_DATETIME)
//             {
//                 BinaryDateTime dt;
//                 memcpy(&dt, rowBuffer + currentOffset, sizeof(BinaryDateTime));
//                 printf("%04d-%02d-%02d %02d:%02d:%02d\t", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
//                 currentOffset += sizeof(BinaryDateTime);
//             }
//             else if (configs[i].typeID == TYPE_CHAR2)
//             {
//                 // Stack allocation sayəsində yaddaş çökməsi/leak problemi həll olundu
//                 char tempStr[MAX_CHAR + 1] = {0};
//                 memcpy(tempStr, rowBuffer + currentOffset, configs[i].dataSize);
//                 printf("%-15s\t", tempStr);
//                 currentOffset += configs[i].dataSize;
//             }
//             else if (configs[i].typeID == TYPE_VARCHAR2)
//             {
//                 // VARCHAR2 OXUNMASI: Pointer offsetinə gedib ordan dynamic mətni oxuyuruq
//                 uint32_t vOffset = *(uint32_t *)(rowBuffer + currentOffset);
//                 char varcharPath[256];
//                 snprintf(varcharPath, sizeof(varcharPath), "%s/tables/%s.varchardb", current_db_path, tableName);

//                 // Dəyişənləri ümumi olaraq elan edirik ki, aşağıda xəta verməsin
//                 char vStr[256] = {0};
//                 uint16_t strLen = 0;

// #if defined(TARGET_PLATFORM_ESP32)
//                 File vFile = LittleFS.open(varcharPath, "r");
//                 if (vFile)
//                 {
//                     vFile.seek(vOffset, SeekSet);
//                     vFile.read((uint8_t *)&strLen, sizeof(uint16_t));
//                     if (strLen > 255)
//                         strLen = 255;
//                     vFile.read((uint8_t *)vStr, strLen);
//                     vFile.close();
//                 }
// #else
//                 FILE *vFile = fopen(varcharPath, "rb");
//                 if (vFile)
//                 {
//                     fseek(vFile, vOffset, SEEK_SET);
//                     if (fread(&strLen, sizeof(uint16_t), 1, vFile) == 1)
//                     {
//                         if (strLen > 255)
//                             strLen = 255;
//                         fread(vStr, 1, strLen, vFile);
//                     }
//                     fclose(vFile);
//                 }
// #endif
//                 printf("%-15s\t", vStr);
//                 currentOffset += sizeof(uint32_t);
//             }
//         }
//         printf("\n");
//     }
//     newCursor.lastOffset = r;
//     myClose(file);
//     return newCursor;
// }

bool fetchRowData(const char *tableName, uint32_t rowIndex, uint8_t *rowBuffer)
{
    char path[256];
    snprintf(path, sizeof(path), "%s/tables/%s.db", current_db_path, tableName);

    FILE *f = fopen(path, "rb");
    if (!f)
        return false;

    // 1. Header və Sütun konfiqurasiyalarını oxu
    DBHeader header;
    fread(&header, sizeof(DBHeader), 1, f);

    // Metadata-dan sonrakı başlanğıc nöqtəsi
    long startDataPos = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);

    // 2. İndeksə uyğun ofsetə keç (offset = start + ID * sətir_ölçüsü)
    fseek(f, startDataPos + (rowIndex * header.rowSize), SEEK_SET);

    // 3. Sətri oxu
    bool success = (fread(rowBuffer, header.rowSize, 1, f) == 1);

    fclose(f);
    return success;
}

Cursor selectData(const char *tableName, Cursor *prevCursor)
{
    Cursor newCursor;
    newCursor.count = 0;
    // newCursor.rowIndices = malloc(sizeof(uint32_t) * 10); // 10-luq batch
    newCursor.rowIndices = (uint32_t *)malloc(sizeof(uint32_t) * 100);
    newCursor.isFinished = false;

    // 1. Faylı aç (openTable sizin funksiyanızdır)
    // printf("Select oxunur: %d\n", 1);
    FileHandle file = openTable(tableName, "r");
    if (!file)
    {
        newCursor.isFinished = true;
        return newCursor;
    }

    // 2. Header-i oxu
    // printf("Select oxunur: %d\n", 2);
    DBHeader header;
    if (myRead(file, &header, sizeof(DBHeader)) != sizeof(DBHeader))
    {
        myClose(file);
        newCursor.isFinished = true;
        return newCursor;
    }

    // 3. Config-ləri oxu
    // printf("Select oxunur: %d\n", 3);
    ColumnConfig configs[MAX_COLUMNS];
    myRead(file, configs, sizeof(ColumnConfig) * header.columnCount);

    // printf("Select oxunur: %d\n", 4);
    // 4. Axtarışın qaldığı yerdən davam et
    long startOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);
    uint32_t r = prevCursor->lastOffset;
    // printf("Select oxunur: %d\n", 5);
    if (!mySeek(file, startOffset + (r * header.rowSize)))
    {
        newCursor.isFinished = true;
        myClose(file);
        return newCursor;
    }

    uint8_t rowBuffer[512]; // rowSize-a uyğun olmalıdır
    // printf("Select oxunur: %d\n", 6);
    // 5. 10-luq dövr (Pagination)
    while (newCursor.count < 10)
    {
        // printf("Select oxunur: %d\n", 7);
        if (myRead(file, rowBuffer, header.rowSize) != header.rowSize)
        {
            newCursor.isFinished = true; // Fayl bitdi
            break;
        }
        // printf("Select oxunur: %d\n", 8);
        // is_deleted = 0 yoxlanışı (rowBuffer[0] silinmə baytınızdır)
        // if (rowBuffer[0] == 0) {
        // // Şərt yoxlanışı (Sizin helper funksiyanız)
        // if (helperCheckCondition(rowBuffer, configs, header)) {
        newCursor.rowIndices[newCursor.count++] = r;
        // }
        // }
        r++;
        // newCursor.count++;
    }
    // printf("Select oxunur: %d\n", 9);
    newCursor.lastOffset = r; // Növbəti çağırış üçün qaldığımız sətiri yadda saxla

    myClose(file);
    return newCursor;
}

// void selectDataAll(const char *tableName) {
//     FILE *file = openTable(tableName, "r");
//     if (!file) {
//         printf("XETA: Fayl acilmadi: %s\n", tableName);
//         return;
//     }

//     DBHeader header;
//     fread(&header, sizeof(DBHeader), 1, file);
//     ColumnConfig configs[MAX_COLUMNS + 1];
//     fread(configs, sizeof(ColumnConfig), header.columnCount, file);

//     printf("\n=== TABLE DATA: %s ===\n", tableName);
//     for (int i = 1; i < header.columnCount; i++) printf("%-15s\t", configs[i].columnName);
//     printf("\n----------------------------------------------------------------------------------\n");

//     uint8_t rowBuffer[512];
//     long startOffset = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);

//     for (uint32_t r = 0; r < header.rowCount; r++) {
//         fseek(file, startOffset + (r * header.rowSize), SEEK_SET);
//         fread(rowBuffer, header.rowSize, 1, file);

//         if (rowBuffer[0] == 1) continue;

//         // 🌟 Məntiq: Sütunların ofsetini bir-bir cəmləyərək gedirik
//         size_t currentOffset = 0;
//         for (int i = 0; i < header.columnCount; i++) {
//             if (i > 0) { // İlk sütunu (is_deleted) çap etmirik
//                 if (configs[i].typeID == TYPE_INT || configs[i].typeID == TYPE_UINT32) {
//                     uint32_t val = *(uint32_t *)(rowBuffer + currentOffset);
//                     printf("%-15d\t", val);
//                 }
//                 else if (configs[i].typeID == TYPE_CHAR2) {
//                     char tempStr[32] = {0};
//                     memcpy(tempStr, rowBuffer + currentOffset, configs[i].dataSize);
//                     printf("%-15s\t", tempStr);
//                 }
//                 // Digər tiplər...
//             }
//             currentOffset += configs[i].dataSize; // Hər sütun qədər ofseti artırırıq
//         }
//         printf("\n");
//     }
//     fclose(file);
// }

// ====================================================================
// 6. DEBUG SELECT STAR (Bütün Binar Strukturu Görmək Üçün)
// ====================================================================
// void debugSelectStar(const char *tableName) {
//     // if (strlen(current_db_path) == 0) {
//     //     printf("XETA: Evvelce bir verilener bazasina qoshulun!\n");
//     //     return;
//     // }

//     // char tableFilePath[256];
//     // snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

//     // FILE *file = fopen(tableFilePath, "rb");
//     // if (!file) {
//     //     printf("Error: '%s' cadvali tapilmadi!\n", tableName);
//     //     return;
//     // }
//     File file = openTable(tableName, "r");

//     if (!file)
//     {
//         return 0;
//     }

//     DBHeader header;
//     fread(&header, sizeof(DBHeader), 1, file);

//     ColumnConfig configs[MAX_COLUMNS + 1];
//     // fread(configs, sizeof(ColumnConfig), header.columnCount, file);
//     file.read((uint8_t*)configs, sizeof(ColumnConfig) * header.columnCount);

//     printf("\n==================================================\n");
//     printf("     RAW BINARY DUMP (CADVEL: %s)\n", tableName);
//     printf("==================================================\n");
//     printf("Hazirki Real Satir Sayi (RowCount): %u\n", header.rowCount);
//     printf("Maksimum Satir Limiti (MaxRows)  : %u\n", header.maxRows);
//     printf("Novbeti Yazilacaq İndex (NextRow) : %u\n", header.nextRowIndex);
//     printf("Bir Satrin Olcusu (RowSize)       : %u byte\n", header.rowSize);
//     printf("--------------------------------------------------\n\n");

//     uint8_t *rowBuffer = (uint8_t *)malloc(header.rowSize);

//     for (uint32_t r = 0; r < header.rowCount; r++) {
//         // fread(rowBuffer, header.rowSize, 1, file);
//         file.read((uint8_t*)rowBuffer, header.rowSize);

//         printf("SATIR #%d\n", r);
//         uint8_t isDeleted = rowBuffer[0];
//         printf("[Bayt 00] -> is_deleted: %u (%s)\n", isDeleted, (isDeleted == 1) ? "SILINIB" : "AKTIV");

//         int offset = 1;
//         for (int i = 1; i < header.columnCount; i++) {
//             printf("[Bayt %02d] -> Sutun: %-12s | Deyer: ", offset, configs[i].columnName);

//             if (configs[i].typeID == TYPE_UINT32 || configs[i].typeID == TYPE_INT ) {
//                 uint32_t val;
//                 memcpy(&val, rowBuffer + offset, 4);
//                 printf("%u\n", val);
//                 offset += 4;
//             }
//             else if (configs[i].typeID == TYPE_UINT8 ) {
//                 uint8_t val;
//                 memcpy(&val, rowBuffer + offset, 1);
//                 printf("%u\n", val);
//                 offset += 1;
//             }
//             else if (configs[i].typeID == TYPE_CHAR2 ) {
//                 int len = configs[i].dataSize;
//                 char *strStr = (char *)malloc(len + 1);
//                 memcpy(strStr, rowBuffer + offset, len);
//                 strStr[len] = '\0';
//                 printf("\"%s\"\n", strStr);
//                 offset += len;
//                 free(strStr);
//             }
//         }
//         printf("\n");
//     }

//     free(rowBuffer);
//     // fclose(file);
//     file.close();
//     printf("==================================================\n");
// }

// ====================================================================
// 9. MULTI-WHERE SELECT DATA
// ====================================================================
// uint8_t selectWhere(const char *tableName, const char *whereColumnsName[], void *whereColumnsData[], const char *whereOperators[]){
//     if (strlen(current_db_path) == 0) {
//         printf("XETA: Evvelce bir verilener bazasina qoshulun!\n");
//         return 0;
//     }

//     char tableFilePath[256];
//     snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

//     FILE *file = fopen(tableFilePath, "rb");
//     if (!file) {
//         printf("Error: '%s' cadvali tapilmadi!\n", tableName);
//         return 0;
//     }

//     DBHeader header;
//     fread(&header, sizeof(DBHeader), 1, file);

//     ColumnConfig configs[MAX_COLUMNS + 1];
//     fread(configs, sizeof(ColumnConfig), header.columnCount, file);

//     // 1. WHERE şərtlərinin sayını hesablayaq
//     int whereCount = 0;
//     while (whereColumnsName[whereCount] != NULL) whereCount++;

//     printf("\n=== FILTERED SELECT: %s ===\n", tableName);

//     // Cədvəlin sütun başlıqlarını ekrana çıxarırıq (Gizli sütun hariç)
//     for (int i = 1; i < header.columnCount; i++) {
//         printf("%-15s\t", configs[i].columnName);
//     }
//     printf("\n--------------------------------------------------\n");

//     // ESP32 optimizasiyası: Dinamik malloc yerinə stack-də sabit ölçülü bufer (Maks 256 byte sətir üçün)
//     uint8_t rowBuffer[256];
//     if (header.rowSize > 256) {
//         printf("XETA: Satir olcusu desteklenen buferden boyukdur!\n");
//         fclose(file);
//         return 0;
//     }

//     uint8_t matchCount = 0;
//     long startPosition = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);

//     // 2. Sətirləri diskdən parça-parça (tək-tək) oxuyuruq
//     for (uint32_t r = 0; r < header.rowCount; r++) {
//         long rowPos = startPosition + (r * header.rowSize);
//         fseek(file, rowPos, SEEK_SET);
//         fread(rowBuffer, header.rowSize, 1, file);

//         if (rowBuffer[0] == 1) continue; // Soft-delete filtri

//         // 3. Bütün şərtlərin ödənib-ödənmədiyini yoxlayırıq (AND məntiqi)
//         bool allConditionsMatch = true;
//         for (int w = 0; w < whereCount; w++) {
//             int currentOffset = 1;
//             int foundIdx = -1;

//             for (int i = 1; i < header.columnCount; i++) {
//                 if (strcmp(configs[i].columnName, whereColumnsName[w]) == 0) {
//                     foundIdx = i;
//                     break;
//                 }
//                 currentOffset += configs[i].dataSize;
//             }

//             if (foundIdx == -1) {
//                 allConditionsMatch = false;
//                 break;
//             }

//             // `compareValues` köməkçi funksiyamız ilə binar müqayisə edirik
//             if (!compareValues(rowBuffer + currentOffset, whereColumnsData[w], whereOperators[w], configs[foundIdx].typeID)) {
//                 allConditionsMatch = false;
//                 break;
//             }
//         }

//         // 4. Əgər bütün WHERE şərtləri ödənirsə, sətri ekrana yazdırırıq
//         if (allConditionsMatch || whereCount == 0) {
//             int offset = 1;
//             for (int i = 1; i < header.columnCount; i++) {
//                 if (configs[i].typeID == TYPE_UINT32 || configs[i].typeID == TYPE_INT) {
//                     uint32_t val;
//                     memcpy(&val, rowBuffer + offset, 4);
//                     printf("%-15u\t", val);
//                     offset += 4;
//                 }
//                 else if (configs[i].typeID == TYPE_UINT8) {
//                     uint8_t val = rowBuffer[offset];
//                     printf("%-15u\t", val);
//                     offset += 1;
//                 }
//                 else if (configs[i].typeID == TYPE_CHAR2) {
//                     int len = configs[i].dataSize;
//                     char strStr[64] = {0}; // Müvəqqəti string buferi
//                     memcpy(strStr, rowBuffer + offset, len);
//                     printf("%-15s\t", strStr);
//                     offset += len;
//                 }
//             }
//             printf("\n");
//             matchCount++;
//         }
//     }

//     fclose(file);
//     printf("--------------------------------------------------\n");
//     printf("Find rows count: %d\n==================================================\n\n", matchCount);
//     return matchCount;
// }

void selectJoin(const char *parentTable, const char *childTable, const char *parentKey, const char *childKey)
{
    if (strlen(current_db_path) == 0)
        return;

    char pPath[256], cPath[256];
    snprintf(pPath, sizeof(pPath), "%s/tables/%s.db", current_db_path, parentTable);
    snprintf(cPath, sizeof(cPath), "%s/tables/%s.db", current_db_path, childTable);

    FILE *pInst = fopen(pPath, "rb");
    FILE *cInst = fopen(cPath, "rb");
    if (!pInst || !cInst)
    {
        if (pInst)
            fclose(pInst);
        if (cInst)
            fclose(cInst);
        return;
    }

    DBHeader pHead, cHead;
    fread(&pHead, sizeof(DBHeader), 1, pInst);
    fread(&cHead, sizeof(DBHeader), 1, cInst);

    ColumnConfig pConfigs[16], cConfigs[16];
    fread(pConfigs, sizeof(ColumnConfig), pHead.columnCount, pInst);
    fread(cConfigs, sizeof(ColumnConfig), cHead.columnCount, cInst);

    // Açarların ofsetlərini təyin edirik
    int pKeyOffset = 1, cKeyOffset = 1; // 0 is_deleted-dir
    for (int i = 1; i < pHead.columnCount; i++)
    {
        if (strcmp(pConfigs[i].columnName, parentKey) == 0)
            break;
        pKeyOffset += pConfigs[i].dataSize;
    }
    for (int i = 1; i < cHead.columnCount; i++)
    {
        if (strcmp(cConfigs[i].columnName, childKey) == 0)
            break;
        cKeyOffset += cConfigs[i].dataSize;
    }

    printf("\n=== JOIN RESULT: %s + %s ===\n", parentTable, childTable);
    printf("%-15s \t %-15s\n", "PARENT DATA", "CHILD DATA");
    printf("-----------------------------------------\n");

    uint8_t pBuffer[128], cBuffer[128];
    long pStart = sizeof(DBHeader) + (sizeof(ColumnConfig) * pHead.columnCount);
    long cStart = sizeof(DBHeader) + (sizeof(ColumnConfig) * cHead.columnCount);

    // SƏTİR-SƏTİR GƏZİNTİ (Stream Cross-Match)
    for (uint32_t pi = 0; pi < pHead.rowCount; pi++)
    {
        fseek(pInst, pStart + (pi * pHead.rowSize), SEEK_SET);
        fread(pBuffer, pHead.rowSize, 1, pInst);
        if (pBuffer[0] == 1)
            continue; // Soft-delete

        uint32_t pKeyValue = *(uint32_t *)(pBuffer + pKeyOffset);

        // Hər bir parent sətir üçün child sətirlərini tək-tək diskdən oxuyub yoxlayırıq (Yaddaşa tam qənaət)
        for (uint32_t ci = 0; ci < cHead.rowCount; ci++)
        {
            fseek(cInst, cStart + (ci * cHead.rowSize), SEEK_SET);
            fread(cBuffer, cHead.rowSize, 1, cInst);
            if (cBuffer[0] == 1)
                continue;

            uint32_t cKeyValue = *(uint32_t *)(cBuffer + cKeyOffset);

            // Əgər ID-lər bərabərdirsə (Match tapıldısa), ekrana çıxarırıq
            if (pKeyValue == cKeyValue)
            {
                // Sadəlik üçün ilk dataları ekrana çıxarırıq
                printf("ID: %-11u \t Match Found!\n", pKeyValue);
            }
        }
    }

    fclose(pInst);
    fclose(cInst);
    printf("=========================================\n\n");
}

void selectJoinData(const char *parentTable, const char *childTable, const char *parentCol, const char *childCol)
{
    if (strlen(current_db_path) == 0)
        return;

    char pPath[256], cPath[256];
    snprintf(pPath, sizeof(pPath), "%s/tables/%s.db", current_db_path, parentTable);
    snprintf(cPath, sizeof(cPath), "%s/tables/%s.db", current_db_path, childTable);

    FILE *pInst = fopen(pPath, "rb");
    FILE *cInst = fopen(cPath, "rb");
    if (!pInst || !cInst)
    {
        if (pInst)
            fclose(pInst);
        if (cInst)
            fclose(cInst);
        return;
    }

    DBHeader pHead, cHead;
    fread(&pHead, sizeof(DBHeader), 1, pInst);
    fread(&cHead, sizeof(DBHeader), 1, cInst);

    uint8_t pTableId = getTableIndexByName(parentTable);
    uint8_t cTableId = getTableIndexByName(childTable);
    uint8_t pColId = getColumnIdByName(pTableId, parentCol);
    uint8_t cColId = getColumnIdByName(cTableId, childCol);

    // Sütunların offsetlərini hesablayaq (Sadəlik üçün qoşulma sütunlarının INT/ID olduğunu fərz edirik)
    int pKeyOffset = 1; // Real layihədə sxem üzrə dövrlə hesablanmalıdır
    int cKeyOffset = 1;

    printf("\n=== INNER JOIN: %s <=> %s ===\n", parentTable, childTable);
    printf("%-20s \t %-20s\n", "PARENT KEY", "CHILD DATA REF");
    printf("-------------------------------------------------------\n");

    uint8_t pBuffer[256], cBuffer[256];
    long pStart = sizeof(DBHeader) + (sizeof(ColumnConfig) * pHead.columnCount);
    long cStart = sizeof(DBHeader) + (sizeof(ColumnConfig) * cHead.columnCount);

    for (uint32_t pi = 0; pi < pHead.rowCount; pi++)
    {
        fseek(pInst, pStart + (pi * pHead.rowSize), SEEK_SET);
        fread(pBuffer, pHead.rowSize, 1, pInst);
        if (pBuffer[0] == 1)
            continue;

        uint32_t pKeyValue = *(uint32_t *)(pBuffer + pKeyOffset);

        // Hər bir parent sətir üçün child sətirlərini tək-tək diskdən yoxlayırıq (RAM-ı qorumaq üçün)
        for (uint32_t ci = 0; ci < cHead.rowCount; ci++)
        {
            fseek(cInst, cStart + (ci * cHead.rowSize), SEEK_SET);
            fread(cBuffer, cHead.rowSize, 1, cInst);
            if (cBuffer[0] == 1)
                continue;

            uint32_t cKeyValue = *(uint32_t *)(cBuffer + cKeyOffset);

            if (pKeyValue == cKeyValue)
            {
                printf("%-20d \t MATCHED (Row %d)\n", pKeyValue, ci);
            }
        }
    }

    fclose(pInst);
    fclose(cInst);
}

uint8_t selectWhere(const char *tableName, const char *whereColumnsName[], void *whereColumnsData[], const char *whereOperators[])
{
    // Cursor cursor;
    // cursor.count = 0;
    // cursor.rowIndices = malloc(sizeof(uint32_t) * 100); // Məsələn, 100 sətir üçün yer ayırırıq
    // cursor.isFinished = true;
    // if (strlen(current_db_path) == 0) {
    //     printf("XETA: Evvelce bir verilener bazasina qoshulun!\n");
    //     return false;
    // }

    // char tableFilePath[256];
    // snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

    // FILE *file = fopen(tableFilePath, "rb");
    // if (!file) {
    //     printf("Error: '%s' cadvali tapilmadi!\n", tableName);
    //     return false;
    // }

    DBHeader header;
#if defined(TARGET_PLATFORM_ESP32)
    // ESP32 üçün Arduino C++ strukturu
    File file = openTable(tableName, "r");
    if (!file)
    {
        // Xəta idarəetməsi
        return false;
    }

    if (file.read((uint8_t *)&header, sizeof(DBHeader)) != sizeof(DBHeader))
    {
        file.close();
        return false;
    }

    ColumnConfig configs[MAX_COLUMNS];
    file.read((uint8_t *)configs, sizeof(ColumnConfig) * header.columnCount);
#else
    // PC (Windows/Linux) üçün Standard C strukturu
    FILE *file = openTable(tableName, "rb");
    if (!file)
    {
        // Xəta idarəetməsi
        return false;
    }

    if (fread(&header, sizeof(DBHeader), 1, file) != 1)
    {
        fclose(file);
        return false;
    }

    ColumnConfig configs[MAX_COLUMNS + 1];
    fread(configs, sizeof(ColumnConfig), header.columnCount, file);
#endif

    int whereCount = 0;
    while (whereColumnsName[whereCount] != NULL)
        whereCount++;

    printf("\n=== FILTERED SELECT: %s (Filtr Sayi: %d) ===\n", tableName, whereCount);
    for (int i = 1; i < header.columnCount; i++)
    {
        printf("%-15s\t", configs[i].columnName);
    }
    printf("\n--------------------------------------------------\n");

    uint8_t rowBuffer[512];
    uint8_t matchCount = 0;
    long startPosition = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);

    for (uint32_t r = 0; r < header.rowCount; r++)
    {
        long rowPos = startPosition + (r * header.rowSize);
// fseek(file, rowPos, SEEK_SET);
// file.seek(rowPos, SeekSet);
#if defined(TARGET_PLATFORM_ESP32)
        file.seek(rowPos, SeekSet);
        file.read((uint8_t *)rowBuffer, header.rowSize);
#else
        fseek(file, rowPos, SEEK_SET);
        fread(rowBuffer, header.rowSize, 1, file);
#endif
        //

        if (rowBuffer[0] == 1)
            continue; // Silinmişləri keç

        bool allConditionsMatch = true;

        for (int w = 0; w < whereCount; w++)
        {
            int offset = getOffsetOfColumn(whereColumnsName[w], configs, header.columnCount);
            if (offset == -1)
            {
                allConditionsMatch = false;
                break;
            }

            uint8_t *dbFieldPtr = rowBuffer + offset;
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

            bool conditionPassed = false;
            // uint8_t *dbFieldPtr = rowBuffer + currentOffset;
            void *userValPtr = whereColumnsData[w];

            // 🌟 İKİ DATANI DA STRING-Ə ÇEVİRİB NORMAL YOXLANILMASI:
            if (configs[foundIdx].typeID == TYPE_CHAR2)
            {
                // Diskdən oxunan sabit ölçülü binar mətni təmiz string edirik
                char dbTemp[64] = {0};
                memcpy(dbTemp, dbFieldPtr, configs[foundIdx].dataSize);
                // std::string dbStr(dbTemp);

                // İstifadəçinin axtardığı mətni string edirik
                // std::string userStr((const char *)userValPtr);

                // İndi tam təhlükəsiz bərabərlik yoxlanışı (C++ string müqayisəsi)
                if (strcmp(whereOperators[w], "=") == 0)
                {
                    if (strcmp(dbTemp, (const char *)userValPtr) == 0)
                    {
                        conditionPassed = true;
                    }
                    else
                    {
                        conditionPassed = false;
                    }
                }
            }
            else if (configs[foundIdx].typeID == TYPE_INT || configs[foundIdx].typeID == TYPE_UINT32)
            {
                uint32_t dbVal = *(uint32_t *)dbFieldPtr;
                uint32_t userVal = *(uint32_t *)userValPtr;

                if (strcmp(whereOperators[w], "=") == 0)
                    conditionPassed = (dbVal == userVal);
                else if (strcmp(whereOperators[w], ">") == 0)
                    conditionPassed = (dbVal > userVal);
                else if (strcmp(whereOperators[w], "<") == 0)
                    conditionPassed = (dbVal < userVal);
            }
            else if (configs[foundIdx].typeID == TYPE_UINT8)
            {
                uint8_t dbVal = *(uint8_t *)dbFieldPtr;
                uint8_t userVal = *(uint8_t *)userValPtr;

                if (strcmp(whereOperators[w], "=") == 0)
                    conditionPassed = (dbVal == userVal);
            }

            // 🌟 Serial.format xətası Serial.printf ilə əvəzləndi:
            if (r == 0)
            {
                // Serial.print("[Diaqnostika] Sütun: ");
                // printf("[Diaqnostika] Sütun: \n");
// Və ya tam cross-platform olması üçün:
#if defined(TARGET_PLATFORM_ESP32)
                Serial.print("[Diaqnostika] Sütun: ");
                Serial.print(whereColumnsName[w]);
                Serial.printf(" | Ofset: %d | TipID: %d | Netice: %s\n",
                              currentOffset, configs[foundIdx].typeID, conditionPassed ? "KECDİ" : "XETA");

#else
                printf("[Diaqnostika] Sütun: ");
                printf(whereColumnsName[w]);
                printf(" | Ofset: %d | TipID: %d | Netice: %s\n", currentOffset, configs[foundIdx].typeID, conditionPassed ? "KECDİ" : "XETA");
#endif
                // }
                //     Serial.print(whereColumnsName[w]);
                //     Serial.printf(" | Ofset: %d | TipID: %d | Netice: %s\n",
                //                   currentOffset, configs[foundIdx].typeID, conditionPassed ? "KECDİ" : "XETA");
            }

            if (!conditionPassed)
            {
                allConditionsMatch = false;
                break;
            }
        }

        if (allConditionsMatch || whereCount == 0)
        {
            int offset = 1;
            for (int i = 1; i < header.columnCount; i++)
            {
                if (configs[i].typeID == TYPE_INT || configs[i].typeID == TYPE_UINT32)
                {
                    uint32_t val = *(uint32_t *)(rowBuffer + offset);
                    printf("%-15d\t", val);
                    offset += 4;
                }
                else if (configs[i].typeID == TYPE_UINT8)
                {
                    uint8_t val = *(uint8_t *)(rowBuffer + offset);
                    printf("%-15d\t", val);
                    offset += 1;
                }
                else if (configs[i].typeID == TYPE_CHAR2)
                {
                    char tempStr[64] = {0};
                    memcpy(tempStr, rowBuffer + offset, configs[i].dataSize);
                    printf("%-15s\t", tempStr);
                    offset += configs[i].dataSize;
                }
                else
                {
                    offset += configs[i].dataSize;
                }
            }
            printf("\n");
            matchCount++;
            // if (allConditionsMatch)
            // {
            // cursor.rowIndices[cursor.count++] = r; // ID-ni yadda saxla
            // }
        }
        // #if defined(TARGET_PLATFORM_ESP32)
        //         file.close();
        // #else
        //         fclose(file);
        // #endif
        //         return true;
    }

// fclose(file);
#if defined(TARGET_PLATFORM_ESP32)
    file.close();
#else
    fclose(file);
#endif
    printf("--------------------------------------------------\n");
    printf("Find rows count: %d\n==================================================\n\n", matchCount);
    return matchCount;
}

Cursor selectWhereCore(const char *tableName, char *whereColumnsName[], void *whereColumnsData[], const char *whereOperators[], int whereCount)
{
    Cursor cursor;
    cursor.count = 0;
    // cursor.rowIndices = malloc(sizeof(uint32_t) * 100); // Məsələn, 100 sətir üçün yer ayırırıq
    cursor.rowIndices = (uint32_t *)malloc(sizeof(uint32_t) * 100);
    cursor.isFinished = true;

    DBHeader header;
#if defined(TARGET_PLATFORM_ESP32)
    // ESP32 üçün Arduino C++ strukturu
    File file = openTable(tableName, "r");
    if (!file)
    {
        // Xəta idarəetməsi
        return cursor;
    }

    if (file.read((uint8_t *)&header, sizeof(DBHeader)) != sizeof(DBHeader))
    {
        file.close();
        return cursor;
    }

    ColumnConfig configs[MAX_COLUMNS];
    file.read((uint8_t *)configs, sizeof(ColumnConfig) * header.columnCount);
#else
    // PC (Windows/Linux) üçün Standard C strukturu
    FILE *file = openTable(tableName, "r");
    if (!file)
    {
        // Xəta idarəetməsi
        printf("File açılmadı");
        return cursor;
    }

    if (fread(&header, sizeof(DBHeader), 1, file) != 1)
    {
        fclose(file);
        printf("Head açılmadı");
        return cursor;
    }

    ColumnConfig configs[MAX_COLUMNS + 1];
    fread(configs, sizeof(ColumnConfig), header.columnCount, file);
#endif

    // int whereCount = 0;
    // while (whereColumnsName[whereCount] != NULL)
    //     whereCount++;

    // printf("\n=== FILTERED SELECT: %s (Filtr Sayi: %d) ===\n", tableName, whereCount);
    for (int i = 1; i < header.columnCount; i++)
    {
        printf("%-15s\t", configs[i].columnName);
    }
    // printf("\n--------------------------------------------------\n");

    uint8_t rowBuffer[512];
    uint8_t matchCount = 0;
    long startPosition = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);

    for (uint32_t r = 0; r < header.rowCount; r++)
    {
        long rowPos = startPosition + (r * header.rowSize);
// fseek(file, rowPos, SEEK_SET);
// file.seek(rowPos, SeekSet);
#if defined(TARGET_PLATFORM_ESP32)
        file.seek(rowPos, SeekSet);
        file.read((uint8_t *)rowBuffer, header.rowSize);
#else
        fseek(file, rowPos, SEEK_SET);
        fread(rowBuffer, header.rowSize, 1, file);
#endif
        //

        if (rowBuffer[0] == 1)
            continue; // Silinmişləri keç

        bool allConditionsMatch = true;

        for (int w = 0; w < whereCount; w++)
        {
            int currentOffset = 1;
            int foundIdx = -1;

            for (int i = 1; i < header.columnCount; i++)
            {
                // printf(" Loop------------------1\n");
                if (strcmp(configs[i].columnName, whereColumnsName[w]) == 0)
                {
                    // printf(" loop------------------2\n");
                    foundIdx = i;
                    break;
                }
                currentOffset += configs[i].dataSize;
                // printf(" loop------------------3\n");
            }

            if (foundIdx == -1)
            {
                // printf(" loop------------------4\n");
                allConditionsMatch = false;
                break;
            }

            bool conditionPassed = false;
            uint8_t *dbFieldPtr = rowBuffer + currentOffset;
            void *userValPtr = whereColumnsData[w];

            // 🌟 İKİ DATANI DA STRING-Ə ÇEVİRİB NORMAL YOXLANILMASI:
            if (configs[foundIdx].typeID == TYPE_CHAR2)
            {
                // Diskdən oxunan sabit ölçülü binar mətni təmiz string edirik
                char dbTemp[64] = {0};
                memcpy(dbTemp, dbFieldPtr, configs[foundIdx].dataSize);
                // std::string dbStr(dbTemp);

                // İstifadəçinin axtardığı mətni string edirik
                // std::string userStr((const char *)userValPtr);

                // İndi tam təhlükəsiz bərabərlik yoxlanışı (C++ string müqayisəsi)
                if (strcmp(whereOperators[w], "=") == 0)
                {
                    if (strcmp(dbTemp, (const char *)userValPtr) == 0)
                    {
                        conditionPassed = true;
                    }
                    else
                    {
                        conditionPassed = false;
                    }
                }
                // printf(" char2 check %d %s %d -> %d", dbTemp, whereOperators[w], userValPtr, conditionPassed);
            }
            else if (configs[foundIdx].typeID == TYPE_INT || configs[foundIdx].typeID == TYPE_UINT32)
            {
                uint32_t dbVal = *(uint32_t *)dbFieldPtr;
                // uint32_t userVal = *(uint32_t *)userValPtr;
                uint32_t userVal = atoi((const char *)userValPtr);

                if (strcmp(whereOperators[w], "=") == 0)
                    conditionPassed = (dbVal == userVal);
                else if (strcmp(whereOperators[w], ">") == 0)
                    conditionPassed = (dbVal > userVal);
                else if (strcmp(whereOperators[w], "<") == 0)
                    conditionPassed = (dbVal < userVal);
                // printf(" INT32 check %s -> %d %s %d -> %d", whereColumnsName[w], dbVal, whereOperators[w], userVal, conditionPassed);
            }
            else if (configs[foundIdx].typeID == TYPE_UINT8)
            {
                // conditionPassed=false;
                uint8_t dbVal = *(uint8_t *)dbFieldPtr;
                // uint8_t userVal = *(uint8_t *)userValPtr;
                uint8_t userVal = atoi((const char *)userValPtr);

                if (strcmp(whereOperators[w], "=") == 0)
                    conditionPassed = (dbVal == userVal);
                // printf(" INT8 column name: %s-> %d %s %d -> %d", whereColumnsName[w], dbVal, whereOperators[w], userVal, conditionPassed);
            }
            // printf(" ------------------1\n");
            // 🌟 Serial.format xətası Serial.printf ilə əvəzləndi:
            if (r == 0)
            {
                // printf(" ------------------2\n");
                // Serial.print("[Diaqnostika] Sütun: ");
                // printf("[Diaqnostika] Sütun: \n");
// Və ya tam cross-platform olması üçün:
#if defined(TARGET_PLATFORM_ESP32)
                Serial.print("[Diaqnostika] Sütun: ");
                Serial.print(whereColumnsName[w]);
                Serial.printf(" | Ofset: %d | TipID: %d | Netice: %s\n",
                              currentOffset, configs[foundIdx].typeID, conditionPassed ? "KECDİ" : "XETA");
                // }
#else
                printf("[Diaqnostika] Sütun: ");
                printf(whereColumnsName[w]);
                printf(" | Ofset: %d | TipID: %d | Netice: %s\n", currentOffset, configs[foundIdx].typeID, conditionPassed ? "KECDİ" : "XETA");
#endif
                //     Serial.print(whereColumnsName[w]);
                //     Serial.printf(" | Ofset: %d | TipID: %d | Netice: %s\n",
                //                   currentOffset, configs[foundIdx].typeID, conditionPassed ? "KECDİ" : "XETA");
            }
            // printf(" ------------------3\n");
            if (!conditionPassed)
            {
                // printf(" ------------------4\n");
                allConditionsMatch = false;
                break;
            }
            // printf(" ------------------5\n");
            printf("\n");
        }
        // printf(" ------------------7\n");
        if (allConditionsMatch)
        {
            // printf(" ------------------6\n");
            cursor.rowIndices[cursor.count++] = r; // ID-ni yadda saxla
            // printf(" id: %d  count: %d\n", r, cursor.count);
            // }
        }
        // printf(" ------------------8\n");
    }
#if defined(TARGET_PLATFORM_ESP32)
    file.close();
#else
    fclose(file);
#endif
    return cursor;
}

// Tapılan sətirlərin sıra nömrələrini outRowIndices massivinə doldurur və ümumi sayı qaytarır
uint8_t selectWhereIndex(const char *tableName, char *whereColumnsName[], void *whereColumnsData[], const char *whereOperators[], uint32_t *outRowIndices, uint8_t maxRowsToReturn)
{
    // if (strlen(current_db_path) == 0) return 0;

    // char tableFilePath[256];
    // snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

    // FILE *file = fopen(tableFilePath, "rb");
    // if (!file) return 0;
    DBHeader header;
#if defined(TARGET_PLATFORM_ESP32)
    // ESP32 üçün Arduino C++ strukturu
    File file = openTable(tableName, "r");
    if (!file)
    {
        // Xəta idarəetməsi
        return 0;
    }

    if (file.read((uint8_t *)&header, sizeof(DBHeader)) != sizeof(DBHeader))
    {
        file.close();
        return 0;
    }

    ColumnConfig configs[MAX_COLUMNS];
    file.read((uint8_t *)configs, sizeof(ColumnConfig) * header.columnCount);
#else
    // PC (Windows/Linux) üçün Standard C strukturu
    FILE *file = openTable(tableName, "rb");
    if (!file)
    {
        // Xəta idarəetməsi
        return 0;
    }

    if (fread(&header, sizeof(DBHeader), 1, file) != 1)
    {
        fclose(file);
        return 0;
    }

    ColumnConfig configs[MAX_COLUMNS + 1];
    fread(configs, sizeof(ColumnConfig), header.columnCount, file);
#endif

    int whereCount = 0;
    while (whereColumnsName[whereCount] != NULL)
        whereCount++;

    uint8_t rowBuffer[512];
    uint8_t matchCount = 0;
    long startPosition = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);

    for (uint32_t r = 0; r < header.rowCount; r++)
    {
        long rowPos = startPosition + (r * header.rowSize);
// fseek(file, rowPos, SEEK_SET);
// file.seek(rowPos, SeekSet);
#if defined(TARGET_PLATFORM_ESP32)
        file.seek(rowPos, SeekSet);
        file.read((uint8_t *)rowBuffer, header.rowSize);
#else
        fseek(file, rowPos, SEEK_SET);
        fread(rowBuffer, header.rowSize, 1, file);
#endif
        //

        if (rowBuffer[0] == 1)
            continue; // Soft-delete olanları keç

        bool allConditionsMatch = true;

        // ... kodun əvvəli eyni qalır ...

        for (int w = 0; w < whereCount; w++)
        {
            // printf(" sel: %d\n",1);
            int currentOffset = 1;
            int foundIdx = -1;

            // Sütun indeksini tap
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

            // YENİ: Köməkçi funksiyanı çağırırıq
            bool conditionPassed = checkCondition(
                configs[foundIdx].typeID,
                configs[foundIdx].dataSize,
                rowBuffer + currentOffset,
                whereColumnsData[w],
                whereOperators[w]);

            if (!conditionPassed)
            {
                allConditionsMatch = false;
                break;
            }
        }

        // ... kodun qalan hissəsi eyni qalır ...

        // Əgər sətir bütün şərtlərə uyğundursa
        if (allConditionsMatch || whereCount == 0)
        {
            // Debug: Uyğun sətir tapıldı
            printf("DEBUG: Match found at Row Index: %u | matchCount: %u\n", r, matchCount);

            // Sıra nömrəsini (r) istifadəçinin massivinə qeyd edirik
            if (outRowIndices != NULL && matchCount < maxRowsToReturn)
            {
                outRowIndices[matchCount] = r;
                printf("DEBUG: Stored row index %u at outRowIndices[%u]\n", r, matchCount);
            }
            else if (matchCount >= maxRowsToReturn)
            {
                printf("DEBUG: Warning: maxRowsToReturn reached, skipping index %u\n", r);
            }

            matchCount++;
        }
    }

// fclose(file);
#if defined(TARGET_PLATFORM_ESP32)
    file.close();
#else
    fclose(file);
#endif
    return matchCount;
}

int32_t selectWhereStep(const char *tableName,
                        const char *returnColumnsName[], void *returnColumnsData[],
                        const char *whereColumnsName[], void *whereColumnsData[], const char *whereOperators[],
                        uint32_t startRowNo)
{
    // if (strlen(current_db_path) == 0) {
    //     printf("XETA: Evvelce bir verilener bazasina qoshulun!\n");
    //     return -1;
    // }

    // char tableFilePath[256];
    // snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", current_db_path, tableName);

    // FILE *file = fopen(tableFilePath, "rb");
    // if (!file) {
    //     printf("Error: '%s' cadvali tapilmadi!\n", tableName);
    //     return -1;
    // }
    DBHeader header;
#if defined(TARGET_PLATFORM_ESP32)
    // ESP32 üçün Arduino C++ strukturu
    File file = openTable(tableName, "r");
    if (!file)
    {
        // Xəta idarəetməsi
        return 0;
    }

    if (file.read((uint8_t *)&header, sizeof(DBHeader)) != sizeof(DBHeader))
    {
        file.close();
        return 0;
    }

    ColumnConfig configs[MAX_COLUMNS];
    file.read((uint8_t *)configs, sizeof(ColumnConfig) * header.columnCount);
#else
    // PC (Windows/Linux) üçün Standard C strukturu
    FILE *file = openTable(tableName, "rb");
    if (!file)
    {
        // Xəta idarəetməsi
        return 0;
    }

    if (fread(&header, sizeof(DBHeader), 1, file) != 1)
    {
        fclose(file);
        return 0;
    }

    ColumnConfig configs[MAX_COLUMNS + 1];
    fread(configs, sizeof(ColumnConfig), header.columnCount, file);
#endif

    // Ötürülən şərtlərin sayını hesablayaq
    int whereCount = 0;
    if (whereColumnsName != NULL)
    {
        while (whereColumnsName[whereCount] != NULL)
            whereCount++;
    }

    // Geri qaytarılacaq sütunların sayını hesablayaq
    int returnCount = 0;
    if (returnColumnsName != NULL)
    {
        while (returnColumnsName[returnCount] != NULL)
            returnCount++;
    }

    // Sətir oxumaq üçün dinamik/stack buferi yaradırıq
    uint8_t rowBuffer[512];
    if (header.rowSize > 512)
    {
        printf("XETA: Satir olcusu 512 baytdan boyukdur!\n");
// fclose(file);
#if defined(TARGET_PLATFORM_ESP32)
        file.close();
#else
        fclose(file);
#endif
        return -1;
    }

    long startPosition = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);
    int32_t foundRowIndex = -1;

    // Şərti ödəyən ilk sətri tapmaq üçün startRowNo indeksindən başlayaraq dövr qururuq
    for (uint32_t r = startRowNo; r < header.rowCount; r++)
    {
        long rowPos = startPosition + (r * header.rowSize);
// fseek(file, rowPos, SEEK_SET);
// file.seek(rowPos, SeekSet);
#if defined(TARGET_PLATFORM_ESP32)
        file.seek(rowPos, SeekSet);
        file.read((uint8_t *)rowBuffer, header.rowSize);
#else
        fseek(file, rowPos, SEEK_SET);
        fread(rowBuffer, header.rowSize, 1, file);
#endif
        //

        if (rowBuffer[0] == 1)
            continue; // Silinmiş (soft-deleted) sətirdirsə keçirik

        // WHERE şərtlərini yoxlayırıq (AND məntiqi ilə)
        bool allConditionsMatch = true;
        for (int w = 0; w < whereCount; w++)
        {
            int currentOffset = 1;
            int foundIdx = -1;

            // Sütunun yerini və rowBuffer daxilindəki offsetini tapaq
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

            // Dəyərləri binar olaraq müqayisə edək
            if (!compareValues(rowBuffer + currentOffset, whereColumnsData[w], whereOperators[w], configs[foundIdx].typeID))
            {
                allConditionsMatch = false;
                break;
            }
        }

        // Əgər bütün WHERE şərtləri ödənirsə (və ya heç şərt yoxdursa)
        if (allConditionsMatch || whereCount == 0)
        {
            foundRowIndex = r; // Tapılan sətirin indeksini yadda saxla

            // İndi isə istifadəçinin tələb etdiyi (returnColumnsName) sütunların datasını onun ötürdüyü buferlərə köçürək
            for (int rc = 0; rc < returnCount; rc++)
            {
                int currentOffset = 1;
                int foundIdx = -1;

                for (int i = 1; i < header.columnCount; i++)
                {
                    if (strcmp(configs[i].columnName, returnColumnsName[rc]) == 0)
                    {
                        foundIdx = i;
                        break;
                    }
                    currentOffset += configs[i].dataSize;
                }

                if (foundIdx != -1 && returnColumnsData[rc] != NULL)
                {
                    // Sütunun binar datasını istifadəçinin göstərdiyi ünvana kopyalayırıq
                    memcpy(returnColumnsData[rc], rowBuffer + currentOffset, configs[foundIdx].dataSize);
                }
            }

            break; // İlk uyğun sətri tapdığımız üçün dövrü tamamilə dayandırırıq!
        }
    }

// fclose(file);
#if defined(TARGET_PLATFORM_ESP32)
    file.close();
#else
    fclose(file);
#endif
    return foundRowIndex; // Tapılmadısa -1, tapıldısa sətir nömrəsini (0, 1, 2...) qaytarır
}

#include <stdio.h>

#include <stdio.h>
#include <string.h>

void printMetadata(const char *dbName)
{
    char path[256];

    // 1. Tables.db oxunması
    snprintf(path, sizeof(path), "%s/%s/metadata/tables.db", "sqlBinDB", dbName);
    FILE *fTable = fopen(path, "rb");
    if (fTable)
    {
        printf("\n--- %s SİSTEMİNDƏKİ CƏDVƏLLƏR ---\n", dbName);
        printf("ID | CƏDVƏL ADI        | ROW_LIMIT | SİLİNİB?\n");
        printf("--------------------------------------------\n");
        CompactTableMeta t;
        int row_counter = 0;
        while (fread(&t, sizeof(CompactTableMeta), 1, fTable))
        {
            row_counter++;
            printf("%-2d | %-16s | %-9d | %s\n",
                   row_counter, t.table_name, t.max_rows,
                   t.is_deleted ? "BƏLİ" : "XEYR");
        }
        fclose(fTable);
    }
    else
    {
        printf("XƏTA: tables.db tapılmadı!\n");
    }

    // 2. Columns.db oxunması
    snprintf(path, sizeof(path), "%s/%s/metadata/columns.db", "sqlBinDB", dbName);
    FILE *fCol = fopen(path, "rb");
    if (fCol)
    {
        printf("\n--- SÜTUN MƏLUMATLARI ---\n");
        printf("TBL_ID | SÜTUN ADI        | TİP | ÖLÇÜ | CONSTRAINT | SİLİNİB?\n");
        printf("-------------------------------------------------------------\n");
        CompactColumnMeta c;
        while (fread(&c, sizeof(CompactColumnMeta), 1, fCol))
        {
            printf("%-6d | %-16s | %-3d | %-4d | %-10d | %s\n",
                   c.table_id, c.column_name, c.type_id, c.data_size,
                   c.constraints, c.is_deleted ? "BƏLİ" : "XEYR");
        }
        fclose(fCol);
    }
}

uint8_t selectAndDelete(const char *tableName, char *whereColumnsName[], void *whereColumnsData[], const char *whereOperators[])
{

    DBHeader header;
#if defined(TARGET_PLATFORM_ESP32)
    // ESP32 üçün Arduino C++ strukturu
    File file = openTable(tableName, "r+");
    if (!file)
    {
        // Xəta idarəetməsi
        printf("OPening error: \n");
        return 0;
    }

    if (file.read((uint8_t *)&header, sizeof(DBHeader)) != sizeof(DBHeader))
    {
        file.close();
        printf("reading error: \n");
        return 0;
    }

    ColumnConfig configs[MAX_COLUMNS];
    file.read((uint8_t *)configs, sizeof(ColumnConfig) * header.columnCount);
    // printf(" select and delete  --------------1: \n");
#else
    // PC (Windows/Linux) üçün Standard C strukturu
    FILE *file = openTable(tableName, "rb+");
    if (!file)
    {
        // Xəta idarəetməsi
        return 0;
    }

    if (fread(&header, sizeof(DBHeader), 1, file) != 1)
    {
        fclose(file);
        return 0;
    }

    ColumnConfig configs[MAX_COLUMNS + 1];
    fread(configs, sizeof(ColumnConfig), header.columnCount, file);
#endif

    int whereCount = 0;
    while (whereColumnsName[whereCount] != NULL)
        whereCount++;
    // printf(" select and delete  --------------2: \n");
    uint8_t rowBuffer[512];
    uint8_t deletedCount = 0;
    uint8_t deleteFlag = 1;
    long startPosition = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);
    // printf(" select and delete  --------------3: \n");
    for (uint32_t r = 0; r < header.rowCount; r++)
    {
        long rowPos = startPosition + (r * header.rowSize);
        // fseek(file, rowPos, SEEK_SET);
        // file.seek(rowPos, SeekSet);
        // #if defined(TARGET_PLATFORM_ESP32)
        //         file.seek(rowPos, SeekSet);
        //         file.read((uint8_t *)rowBuffer, header.rowSize);
        // #else
        //         fseek(file, rowPos, SEEK_SET);
        //         fread(rowBuffer, header.rowSize, 1, file);
        // #endif
        // Sətri oxu
        DB_FILE_SEEK(file, rowPos);
        DB_FILE_READ(file, rowBuffer, header.rowSize);
        //
        // printf(" select and delete  --------------4: \n");
        if (rowBuffer[0] == 1)
            continue; // Soft-delete olanları keç

        bool allConditionsMatch = true;

        // ... kodun əvvəli eyni qalır ...

        for (int w = 0; w < whereCount; w++)
        {
            // printf(" sel: %d\n",1);
            int currentOffset = 1;
            int foundIdx = -1;

            // Sütun indeksini tap
            // printf(" select and delete  --------------4: \n");
            for (int i = 1; i < header.columnCount; i++)
            {
                if (strcmp(configs[i].columnName, whereColumnsName[w]) == 0)
                {
                    foundIdx = i;
                    break;
                }
                currentOffset += configs[i].dataSize;
            }
            // printf(" select and delete  --------------5: \n");

            if (foundIdx == -1)
            {
                allConditionsMatch = false;
                break;
            }
            // printf(" select and delete  --------------6: \n");
            // YENİ: Köməkçi funksiyanı çağırırıq
            bool conditionPassed = checkCondition(
                configs[foundIdx].typeID,
                configs[foundIdx].dataSize,
                rowBuffer + currentOffset,
                whereColumnsData[w],
                whereOperators[w]);

            if (!conditionPassed)
            {
                allConditionsMatch = false;
                break;
            }
        }

        // ... kodun qalan hissəsi eyni qalır ...

        // Əgər sətir bütün şərtlərə uyğundursa
        if (allConditionsMatch || whereCount == 0)
        {
            // printf(" select and delete  --------------7: \n");
            // Faylda həmin sətrin 0-cı baytına (is_deleted) 1 yazırıq
            DB_FILE_SEEK(file, rowPos);          // Sətir başına qayıt
            DB_FILE_WRITE(file, &deleteFlag, 1); // 0-cı baytı 1 et
            DB_FILE_FLUSH(file);                 // Dəyişikliyi diskə yaz

            deletedCount++;
            // printf("DEBUG: Row %u deleted.\n", r);
        }
    }

    // fclose(file);
    // printf(" select and delete  --------------8: \n");
#if defined(TARGET_PLATFORM_ESP32)
    file.close();
#else
    fclose(file);
#endif
    return deletedCount;
}

void listTableData(const char *tableName)
{
    // 1. Cursor-u başlanğıc vəziyyətinə gətir
    Cursor cursor;
    cursor.lastOffset = 0; // Faylın başlanğıcından axtarışa başlayır
    cursor.isFinished = false;
    cursor.rowIndices = NULL; // İlk dəfə null edirik, funksiya özü malloc edəcək

    printf("\n--- '%s' CƏDVƏLİNDƏN MƏLUMATLARIN OXUNMASI ---\n", tableName);

    // 2. Dövr: Fayl bitənə qədər davam et
    do
    {
        // selectData-nı çağırırıq və əvvəlki cursor-u ona ötürürük
        // Funksiya yeni cursor qaytaracaq (yeni batch ilə)
        cursor = selectData(tableName, &cursor);

        // 3. Tapılanları emal et (məsələn, ekrana çap et)
        for (int i = 0; i < cursor.count; i++)
        {
            printf("Sətir indeksi (ofset): %d\n", cursor.rowIndices[i]);
            // Burada həmin indeksə uyğun məlumatı oxuyub çap edə bilərsiniz
        }

        // Yaddaşı təmizləməyi unutmayın!
        if (cursor.rowIndices != NULL)
        {
            free(cursor.rowIndices);
            cursor.rowIndices = NULL;
        }

    } while (!cursor.isFinished); // Əgər faylın sonuna çatmamışıqsa, dövr davam edir

    printf("--- Bütün məlumatlar oxundu. ---\n");
}

bool getValueByColumnName(const char *colName, uint8_t *rowBuffer, ColumnConfig *configs, int colCount, void *outResult)
{
    int currentOffset = 1; // 0-cı bayt 'is_deleted'

    for (int i = 0; i < colCount; i++)
    {
        if (strcmp(configs[i].columnName, colName) == 0)
        {
            // Sütunu tapdıq, tipinə görə buffer-dən kopyalayırıq
            if (configs[i].typeID == TYPE_INT || configs[i].typeID == TYPE_UINT32)
            {
                memcpy(outResult, rowBuffer + currentOffset, 4);
                return true;
            }
            else if (configs[i].typeID == TYPE_CHAR2)
            {
                memcpy(outResult, rowBuffer + currentOffset, configs[i].dataSize);
                return true;
            }
            // Digər tiplər...
        }
        // Offset-i növbəti sütuna sürüşdür
        currentOffset += configs[i].dataSize;
    }
    return false;
}

// Sütun konfiqurasiyasını tapmaq üçün köməkçi
bool getColumnOffsetAndType(const char *tableName, const char *colName, int *offset, int *type, int *dataSize)
{
    // 1. Cədvəlin ColumnConfig-lərini metadata-dan oxuyun (sizdə loadConfigsForTable var)
    ColumnConfig configs[MAX_COLUMNS];
    int colCount = loadConfigsForTable(getTableIndexByName(tableName), configs);

    int currentOffset = 1; // 0-cı bayt 'is_deleted'
    for (int i = 0; i < colCount; i++)
    {
        if (strcmp(configs[i].columnName, colName) == 0)
        {
            *offset = currentOffset;
            *type = configs[i].typeID;
            *dataSize = configs[i].dataSize;
            return true;
        }
        currentOffset += configs[i].dataSize;
    }
    return false;
}

// İstədiyiniz sütunun dəyərini sadəcə adını yazaraq alın
void printColumnValue(const char *tableName, uint32_t rowIndex, const char *colName)
{

    // if (strcmp(colName, "is_deleted") == 0)
    // {
    //     uint8_t statusByte;
    //     // fetchRowData funksiyanızın daxilində row-un 0-cı baytı
    //     // adətən is_deleted statusunu saxlayır.
    //     if (fetchRowStatus(tableName, rowIndex, &statusByte))
    //     {
    //         *(uint8_t*)outValue = statusByte;
    //         // return;
    //     }
    //     return;
    // }

    uint8_t buffer[512];
    if (!fetchRowData(tableName, rowIndex, buffer))
        return;

    int offset, type, size;
    if (getColumnOffsetAndType(tableName, colName, &offset, &type, &size))
    {

        // Burada tipə görə avtomatik çap (Siz bu məntiqi bura bir dəfə yazırsınız)
        if (type == TYPE_INT || type == TYPE_UINT32)
        {
            printf("%s: %u\n", colName, *(uint32_t *)(buffer + offset));
        }
        else if (type == TYPE_CHAR2)
        {
            char str[MAX_CHAR + 1] = {0};
            memcpy(str, buffer + offset, size);
            printf("%s: %s\n", colName, str);
        }
    }
}

bool getColumnData(const char *tableName, uint32_t rowIndex, const char *colName, void *outValue)
{
    uint8_t buffer[512];
    if (!fetchRowData(tableName, rowIndex, buffer))
        return false;
    if (strcmp(colName, "is_deleted") == 0)
    {
        *(uint8_t *)outValue = buffer[0];
        return true;
    }
    int offset, type, size;
    if (getColumnOffsetAndType(tableName, colName, &offset, &type, &size))
    {
        // Tipə görə dəyəri outValue-ya kopyalayırıq (return-in əvəzinə)
        memcpy(outValue, buffer + offset, size);
        return true;
    }
    return false;
}

Cursor selectWhereCursor(const char *tableName, char *whereColumnsName[], void *whereColumnsData[], const char *whereOperators[])
{
    Cursor cursor;
    cursor.count = 0;
    // cursor.rowIndices = malloc(sizeof(uint32_t) * 10); // İlkin 100 sətirlik yer
    cursor.rowIndices = (uint32_t *)malloc(sizeof(uint32_t) * 10);
    cursor.isFinished = true;

    DBHeader header;
#if defined(TARGET_PLATFORM_ESP32)
    File file = openTable(tableName, "r");
    if (!file)
        return cursor;

    if (file.read((uint8_t *)&header, sizeof(DBHeader)) != sizeof(DBHeader))
    {
        file.close();
        return cursor;
    }
    ColumnConfig configs[MAX_COLUMNS];
    file.read((uint8_t *)configs, sizeof(ColumnConfig) * header.columnCount);
#else
    FILE *file = openTable(tableName, "rb");
    if (!file)
        return cursor;

    if (fread(&header, sizeof(DBHeader), 1, file) != 1)
    {
        fclose(file);
        return cursor;
    }
    ColumnConfig configs[MAX_COLUMNS + 1];
    fread(configs, sizeof(ColumnConfig), header.columnCount, file);
#endif

    int whereCount = 0;
    while (whereColumnsName[whereCount] != NULL)
        whereCount++;

    uint8_t rowBuffer[512];
    long startPosition = sizeof(DBHeader) + (sizeof(ColumnConfig) * header.columnCount);

    for (uint32_t r = 0; r < header.rowCount; r++)
    {
        long rowPos = startPosition + (r * header.rowSize);
        DB_FILE_SEEK(file, rowPos);
        DB_FILE_READ(file, rowBuffer, header.rowSize);

        // Silinmiş sətirləri (is_deleted == 1) keç
        if (rowBuffer[0] == 1)
            continue;

        bool allConditionsMatch = true;

        for (int w = 0; w < whereCount; w++)
        {
            int currentOffset = 1;
            int foundIdx = -1;

            // Sütun indeksini və ofsetini tap
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

            // Şərti yoxla
            bool conditionPassed = checkCondition(
                configs[foundIdx].typeID,
                configs[foundIdx].dataSize,
                rowBuffer + currentOffset,
                whereColumnsData[w],
                whereOperators[w]);

            if (!conditionPassed)
            {
                allConditionsMatch = false;
                break;
            }
        }

        // Əgər şərtlər ödənirsə, ID-ni cursor-a yaz
        if (allConditionsMatch)
        {
            // Əgər 100-dən çox sətir tapılsa, realloc etmək olar
            cursor.rowIndices[cursor.count++] = r;
        }
    }

#if defined(TARGET_PLATFORM_ESP32)
    file.close();
#else
    fclose(file);
#endif

    return cursor;
}

#endif