// ====================================================================
// Parserin Köməkçi Funksiyaları (Yüksək Optimizasiyalı Pointer Skanerləri)
// ====================================================================

const char *skipSpaces(const char *str)
{
    while (*str && (isspace((unsigned char)*str) || *str == ';'))
        str++;
    return str;
}

bool matchKeyword(const char **cursor, const char *keyword)
{
    *cursor = skipSpaces(*cursor);
    int len = strlen(keyword);
    if (strncasecmp(*cursor, keyword, len) == 0)
    {
        char nextChar = *(*cursor + len);
        if (isalnum((unsigned char)nextChar) || nextChar == '_')
        {
            return false;
        }
        *cursor += len;
        return true;
    }
    return false;
}

bool extractWord(const char **cursor, char *buffer, int maxLen)
{
    *cursor = skipSpaces(*cursor);
    int i = 0;
    while (**cursor && !isspace((unsigned char)**cursor) && **cursor != ',' &&
           **cursor != ';' && **cursor != '(' && **cursor != ')')
    {
        if (i < maxLen - 1)
        {
            buffer[i++] = **cursor;
        }
        (*cursor)++;
    }
    buffer[i] = '\0';
    return i > 0;
}

bool extractParentheses(const char **cursor, char *buffer, int maxLen)
{
    *cursor = skipSpaces(*cursor);
    if (**cursor != '(')
        return false;
    (*cursor)++;

    int i = 0;
    int openBrackets = 1;
    while (**cursor && openBrackets > 0)
    {
        if (**cursor == '(')
            openBrackets++;
        else if (**cursor == ')')
        {
            openBrackets--;
            if (openBrackets == 0)
            {
                (*cursor)++;
                break;
            }
        }
        if (i < maxLen - 1)
        {
            buffer[i++] = **cursor;
        }
        (*cursor)++;
    }
    buffer[i] = '\0';

    int len = strlen(buffer);
    while (len > 0 && isspace((unsigned char)buffer[len - 1]))
    {
        buffer[--len] = '\0';
    }
    return true;
}

void extractUntilKeywordOrEnd(const char **cursor, const char *keyword, char *buffer, int maxLen)
{
    *cursor = skipSpaces(*cursor);
    int i = 0;
    int kwLen = strlen(keyword);
    while (**cursor && **cursor != ';')
    {
        if (strncasecmp(*cursor, keyword, kwLen) == 0)
        {
            char nextChar = *(*cursor + kwLen);
            if (!isalnum((unsigned char)nextChar) && nextChar != '_')
            {
                break;
            }
        }
        if (i < maxLen - 1)
        {
            buffer[i++] = **cursor;
        }
        (*cursor)++;
    }
    buffer[i] = '\0';

    int len = strlen(buffer);
    while (len > 0 && isspace((unsigned char)buffer[len - 1]))
    {
        buffer[--len] = '\0';
    }
}

// ====================================================================
// Əsas SQL Analiz və İdarəetmə Mərkəzi
// ====================================================================
bool executeSQL(const char *sql)
{
    const char *cursor = sql;
    cursor = skipSpaces(cursor);

    char tableName[64] = {0};
    char dbName[64] = {0};
    printf("\n[DAXİL OLAN SORĞU]: \"%s\"\n", sql);

    // ----------------------------------------------------------------
    // 1. SHOW DATABASES / SHOW DATABASE
    // ----------------------------------------------------------------
    if (matchKeyword(&cursor, "SHOW DATABASES") || matchKeyword(&cursor, "SHOW DATABASE"))
    {
        printf("[PROSES İCRA OLUNUR]: Mərkəzi registrdən ('master_dbs.db') aktiv olan bütün verilənlər bazalarının siyahısı oxunur.\n");
        selectDb("*");
        return true;
    }

    // ----------------------------------------------------------------
    // 2. SHOW TABLES / SHOW TABLE
    // ----------------------------------------------------------------
    if (matchKeyword(&cursor, "SHOW TABLES") || matchKeyword(&cursor, "SHOW TABLE"))
    {
        printf("[PROSES İCRA OLUNUR]: Hazırkı aktiv bazanın 'metadata/tables.db' faylından silinməmiş cədvəllərin adları ekrana çıxarılır.\n");
        selectTables("*");
        return true;
    }

    // ----------------------------------------------------------------
    // 3 & 4. CREATE DATABASE və DROP DATABASE
    // ----------------------------------------------------------------
    if (matchKeyword(&cursor, "CREATE DATABASE"))
    {
        char dbName[64] = {0};
        char dbPsw[32] = {0}; // Parolu saxlamaq üçün bufer
        bool reCreate = false;

        if (extractWord(&cursor, dbName, sizeof(dbName)))
        {
            // RECREATE və PASSWORD açar sözlərini istənilən ardıcıllıqla və təhlükəsiz oxumaq üçün dövr
            while (true)
            {
                if (matchKeyword(&cursor, "RECREATE"))
                {
                    reCreate = true;
                    continue;
                }
                if (matchKeyword(&cursor, "PASSWORD"))
                {
                    if (!extractWord(&cursor, dbPsw, sizeof(dbPsw)))
                    {
                        printf("SİNTAKSİS XƏTASI: PASSWORD açar sözündən sonra parol təyin edilməyib.\n");
                        return false;
                    }
                    continue;
                }
                break; // Başqa tanınan açar söz yoxdursa dövrdən çıx
            }

            // Mühərrikin funksiyasına real verilənlər ötürülür
            createDb(dbName, dbPsw, reCreate);

            if (reCreate)
            {
                printf("[PROSES İCRA OLUNUR]: '%s' bazası (Parol: '%s') köhnə qeydlərdən təmizlənərək YENİDƏN yaradılır (reCreate = true).\n", dbName, dbPsw);
            }
            else
            {
                printf("[PROSES İCRA OLUNUR]: '%s' bazası (Parol: '%s') yoxlanılır, yoxdursa sistemdə yaradılır (reCreate = false).\n", dbName, dbPsw);
            }
             return true;
        }
        else
        {
            printf("SİNTAKSİS XƏTASI: Verilənlər bazasının adı tapılmadı.\n");
             return false;
        }
       return true;
    }

    if (matchKeyword(&cursor, "DROP DATABASE"))
    {
        char dbName[64] = {0};
        char dbPsw[32] = {0}; // Parolu saxlamaq üçün təhlükəsiz bufer

        if (extractWord(&cursor, dbName, sizeof(dbName)))
        {
            // PASSWORD açar sözünü və gələcəkdə əlavə edilə biləcək modifier-ləri təhlükəsiz oxumaq üçün dövr
            while (true)
            {
                if (matchKeyword(&cursor, "PASSWORD"))
                {
                    if (!extractWord(&cursor, dbPsw, sizeof(dbPsw)))
                    {
                        printf("SİNTAKSİS XƏTASI: PASSWORD açar sözündən sonra parol təyin edilməyib.\n");
                        return false;
                    }
                    continue;
                }
                break; // Başqa tanınan açar söz yoxdursa dövrdən çıx
            }

            // Real binar silmə funksiyası yalnız uğurlu skandandan sonra çağırılır
            dropDb(dbName, dbPsw);

            printf("[PROSES İCRA OLUNUR]: '%s' bazası (Parol doğrulaması: '%s'), daxili cədvəlləri və metadatası ilə birlikdə diskdən tamamilə silinir.\n", dbName, dbPsw);
         return true;
        }
        else
        {
            printf("SİNTAKSİS XƏTASI: Silinəcək verilənlər bazasının adı tapılmadı.\n");
             return false;
        }
        return true;
    }

    // ----------------------------------------------------------------
    // 5 & 6. CREATE TABLE və DROP TABLE
    // ----------------------------------------------------------------
    // ----------------------------------------------------------------
    // 5. CREATE TABLE [RECREATE] table_name (col type [constraint], ...) [MAX_ROWS n]
    // ----------------------------------------------------------------
    // ----------------------------------------------------------------
    // AUTO-INJECT CHILD TABLES AS INT COLUMNS PARSER
    // ----------------------------------------------------------------
    // ----------------------------------------------------------------
    // ENHANCED CREATE TABLE PARSER (MAX_ROWS və CHILD_TABLES Dəstəkli)
    // ----------------------------------------------------------------
    // ----------------------------------------------------------------
    // ADVANCED CREATE TABLE PARSER (CHILD_TABLES & PARENT_TABLES AUTO-INJECTION)
    // ----------------------------------------------------------------
    if (matchKeyword(&cursor, "CREATE TABLE"))
    {
        char tableName[64] = {0};

        if (extractWord(&cursor, tableName, sizeof(tableName)))
        {
            char schemaBuf[256] = {0};

            // 1. İSTİFADƏÇİNİN YAZDIĞI ƏSAS SÜTUNLARI OXU
            if (extractParentheses(&cursor, schemaBuf, sizeof(schemaBuf)))
            {

#define MAX_PARSED_COLS 25                                   // Suffix sütunları artacağı üçün limiti 25 etdik
                char colNamesBuf[MAX_PARSED_COLS][64] = {0}; // Şəkilçilər üçün ölçünü 64 etdik
                char colTypesBuf[MAX_PARSED_COLS][32] = {0};
                char colConstraintsBuf[MAX_PARSED_COLS][64] = {0};

                char *columnNames[MAX_PARSED_COLS + 1] = {NULL};
                char *columnTypes[MAX_PARSED_COLS + 1] = {NULL};
                char *constraints[MAX_PARSED_COLS + 1] = {NULL};

                const char *schemaCursor = schemaBuf;
                int colCount = 0;

                // Əsas sütunların parçalanması
                while (*schemaCursor && colCount < MAX_PARSED_COLS)
                {
                    schemaCursor = skipSpaces(schemaCursor);
                    if (*schemaCursor == '\0')
                        break;

                    if (!extractWord(&schemaCursor, colNamesBuf[colCount], 64))
                        break;
                    columnNames[colCount] = colNamesBuf[colCount];

                    char baseType[16] = {0};
                    if (!extractWord(&schemaCursor, baseType, sizeof(baseType)))
                        break;
                    strcpy(colTypesBuf[colCount], baseType);

                    // Ölçü mötərizəsi varsa (Məs: CHAR2(10)) birləşdir
                    schemaCursor = skipSpaces(schemaCursor);
                    if (*schemaCursor == '(')
                    {
                        strcat(colTypesBuf[colCount], "(");
                        schemaCursor++;
                        while (*schemaCursor && isdigit((unsigned char)*schemaCursor))
                        {
                            char digitStr[2] = {*schemaCursor, '\0'};
                            strcat(colTypesBuf[colCount], digitStr);
                            schemaCursor++;
                        }
                        if (*schemaCursor == ')')
                        {
                            strcat(colTypesBuf[colCount], ")");
                            schemaCursor++;
                        }
                    }
                    columnTypes[colCount] = colTypesBuf[colCount];

                    // Constraint-ləri oxu (NOT NULL və s.)
                    schemaCursor = skipSpaces(schemaCursor);
                    int cIdx = 0;
                    while (*schemaCursor && *schemaCursor != ',')
                    {
                        if (cIdx < 63)
                            colConstraintsBuf[colCount][cIdx++] = *schemaCursor;
                        schemaCursor++;
                    }
                    colConstraintsBuf[colCount][cIdx] = '\0';
                    while (cIdx > 0 && isspace((unsigned char)colConstraintsBuf[colCount][cIdx - 1]))
                    {
                        colConstraintsBuf[colCount][--cIdx] = '\0';
                    }
                    constraints[colCount] = colConstraintsBuf[colCount];

                    if (*schemaCursor == ',')
                        schemaCursor++;
                    colCount++;
                }

                // 2. MAX_ROWS PARAMETRİNİN PARÇALANMASI
                uint32_t maxRowsValue = 1000;
                cursor = skipSpaces(cursor);
                if (matchKeyword(&cursor, "MAX_ROWS"))
                {
                    cursor = skipSpaces(cursor);
                    if (*cursor == '=')
                    {
                        cursor++;
                        cursor = skipSpaces(cursor);
                    }
                    char rowBuf[16] = {0};
                    int rIdx = 0;
                    while (*cursor && isdigit((unsigned char)*cursor) && rIdx < 15)
                    {
                        rowBuf[rIdx++] = *cursor++;
                    }
                    if (rIdx > 0)
                        maxRowsValue = (uint32_t)atoi(rowBuf);
                }

                // 🌟 3. CHILD_TABLES BLOKU: [child_adı]_first_id (INT) ENJEKSİYASI
                cursor = skipSpaces(cursor);
                if (matchKeyword(&cursor, "CHILD_TABLES"))
                {
                    char childBuf[256] = {0};
                    if (extractParentheses(&cursor, childBuf, sizeof(childBuf)))
                    {
                        const char *childCursor = childBuf;
                        while (*childCursor && colCount < MAX_PARSED_COLS)
                        {
                            childCursor = skipSpaces(childCursor);
                            if (*childCursor == '\0')
                                break;

                            char tempChildName[32] = {0};
                            if (extractWord(&childCursor, tempChildName, sizeof(tempChildName)))
                            {

                                // Adın formalaşdırılması: [child_table_name]_first_id
                                strcpy(colNamesBuf[colCount], tempChildName);
                                strcat(colNamesBuf[colCount], "_first_id");
                                columnNames[colCount] = colNamesBuf[colCount];

                                strcpy(colTypesBuf[colCount], "INT");
                                columnTypes[colCount] = colTypesBuf[colCount];

                                strcpy(colConstraintsBuf[colCount], "");
                                constraints[colCount] = colConstraintsBuf[colCount];

                                colCount++;
                            }
                            childCursor = skipSpaces(childCursor);
                            if (*childCursor == ',')
                                childCursor++;
                        }
                    }
                }

                // 🌟 4. PARENT_TABLES BLOKU: [parent]_id (INT) və [parent]_next_id (INT) ENJEKSİYASI
                cursor = skipSpaces(cursor);
                if (matchKeyword(&cursor, "PARENT_TABLES"))
                {
                    char parentBuf[256] = {0};
                    if (extractParentheses(&cursor, parentBuf, sizeof(parentBuf)))
                    {
                        const char *parentCursor = parentBuf;
                        while (*parentCursor && colCount < MAX_PARSED_COLS)
                        {
                            parentCursor = skipSpaces(parentCursor);
                            if (*parentCursor == '\0')
                                break;

                            char tempParentName[32] = {0};
                            if (extractWord(&parentCursor, tempParentName, sizeof(tempParentName)))
                            {

                                // Sütun A: [parent_table_name]_id
                                strcpy(colNamesBuf[colCount], tempParentName);
                                strcat(colNamesBuf[colCount], "_id");
                                columnNames[colCount] = colNamesBuf[colCount];
                                strcpy(colTypesBuf[colCount], "INT");
                                columnTypes[colCount] = colTypesBuf[colCount];
                                strcpy(colConstraintsBuf[colCount], "");
                                constraints[colCount] = colConstraintsBuf[colCount];
                                colCount++;

                                // Sütun B: [parent_table_name]_next_id
                                if (colCount < MAX_PARSED_COLS)
                                {
                                    strcpy(colNamesBuf[colCount], tempParentName);
                                    strcat(colNamesBuf[colCount], "_next_id");
                                    columnNames[colCount] = colNamesBuf[colCount];
                                    strcpy(colTypesBuf[colCount], "INT");
                                    columnTypes[colCount] = colTypesBuf[colCount];
                                    strcpy(colConstraintsBuf[colCount], "");
                                    constraints[colCount] = colConstraintsBuf[colCount];
                                    colCount++;
                                }
                            }
                            parentCursor = skipSpaces(parentCursor);
                            if (*parentCursor == ',')
                                parentCursor++;
                        }
                    }
                }

                // Massivlərin sonunun NULL ilə bağlanması
                columnNames[colCount] = NULL;
                columnTypes[colCount] = NULL;
                constraints[colCount] = NULL;

                // 5. MÜHƏRRİKƏ GÖNDƏRİLMƏ
                bool success = createTable(tableName, columnNames, columnTypes, constraints, false, maxRowsValue);

                if (success)
                {
                    printf("\n[SİSTEM TAM ÖDƏYİR]: '%s' cədvəli relyasiya sütunları ilə uğurla yığıldı.\n", tableName);
                    printf(" -> MAX_ROWS Limiti: %d\n", maxRowsValue);
                    printf(" -> Ümumi Sütun Sayı (Orijinal + İnyeksiya): %d\n", colCount);
                    for (int i = 0; i < colCount; i++)
                    {
                        printf("   -> [%d] Sütun: Ad='%s', Tip='%s', Constraint='%s'\n",
                               i + 1, columnNames[i], columnTypes[i], constraints[i]);
                    }
                }
            }
        }
        return true;
    }

    // void parseDropTable(const char *cursor)
    // {

    if (matchKeyword(&cursor, "DROP TABLE")) 
    {
        char tableName[64];
    int hardDrop = 0; // Susmaya görə: 0 (RESTRICT)
        // Cədvəlin adını götürürük
        if (extractWord(&cursor, tableName, sizeof(tableName)))
        {
            // İndi isə sorğunun sonunda rejim açar sözünün olub-olmadığını yoxlayırıq
            if (matchKeyword(&cursor, "CASCADE"))
            {
                hardDrop = 1; // Əlaqəli hər şeyi sil
            }
            else if (matchKeyword(&cursor, "UNLINK"))
            {
                hardDrop = 2; // Əlaqəni kəs və sil
            }
            else if (matchKeyword(&cursor, "RESTRICT") || true)
            {
                // Əgər heç nə yazılmayıbsa və ya RESTRICT yazılıbsa
                hardDrop = 0;
            }

            // İcra olunacaq funksiyaya dəyərləri ötürürük
            printf("[PROSES]: '%s' cədvəli üçün DROP əməliyyatı başladıldı.\n", tableName);
            printf("[REJİM]: hardDrop = %d ", hardDrop);

            // Rejimə uyğun konsol mesajı
            switch (hardDrop)
            {
            case 0:
                printf("(RESTRICT - Əlaqəli data varsa silinməyəcək)\n");
                break;
            case 1:
                printf("(CASCADE - Əlaqəli bütün datalar birlikdə silinəcək)\n");
                break;
            case 2:
                printf("(UNLINK - Əlaqəli dataların əlaqəsi kəsilib cədvəl silinəcək)\n");
                break;
            }

            // Burada sizin dropTable(tableName, hardDrop); funksiyanız çağırılacaq
            return true;
        }
        else{
            printf("SİNTAKSİS XƏTASI: Silinəcək cədvəlin adı tapılmadı.\n");
             return false;
        }
            

        return true;
    }
    // }

    // ----------------------------------------------------------------
    // 7. DELETE FROM ... WHERE
    // ----------------------------------------------------------------
    //    uint8_t parseDeleteRows(const char *cursor)
    // {

    if (matchKeyword(&cursor, "DELETE FROM"))
     
    {
           char tableName[64] = {0};
    int hardDelete = 0; // Susmaya görə: 0 (RESTRICT)
    char whereCond[256] = {0};
        if (extractWord(&cursor, tableName, sizeof(tableName)))
        {
            // 1. WHERE şərtini yoxlayırıq
            if (matchKeyword(&cursor, "WHERE"))
            {
                // Şərti oxuyuruq
                extractUntilKeywordOrEnd(&cursor, ";", whereCond, sizeof(whereCond));

                // İndi isə şərtin daxilində və ya sonunda silinmə rejimini yoxlayırıq.
                // Qeyd: Əgər WHERE daxilində sətirlərin sonunda CASCADE yazılıbsa, onu ayırmalıyıq.
                // Parserinizin gücünə görə cursor üzərindən də matchKeyword edə bilərsiniz.

                // Nümunə yoxlama (Sorğunun sonundakı rejim tənzimlənməsi):
                // `cursor` daxilində mövqeyə görə rejim təyini:
                if (matchKeyword(&cursor, "CASCADE"))
                {
                    hardDelete = 1;
                }
                else if (matchKeyword(&cursor, "UNLINK"))
                {
                    hardDelete = 2;
                }
                else
                {
                    hardDelete = 0; // default və ya RESTRICT
                }

                printf("[PROSES İCRA OLUNUR]: '%s' cədvəlində '%s' şərtini ödəyən sətirlər üçün silinmə başladıldı.\n", tableName, whereCond);
            }
            else
                printf("[PROSES İCRA OLUNUR]: '%s' cədvəlindəki BÜTÜN sətirlər silinməyə gedir.\n", tableName);

            // 2. hardDelete rejiminə uyğun əməliyyat məlumatı
            printf("[REJİM]: hardDelete = %d -> ", hardDelete);
            switch (hardDelete)
            {
            case 0:
                printf("RESTRICT (Əlaqəli sətirlər varsa, silinmə rədd ediləcək)\n");
                break;
            case 1:
                printf("CASCADE (Əlaqəli bütün asılı sətirlər də birlikdə silinəcək/soft-delete ediləcək)\n");
                break;
            case 2:
                printf("UNLINK (Asılı sətirlərin əlaqə sütunları sıfırlanacaq, əsas sətir silinəcək)\n");
                break;
            }

            // Burada sizin deleteRows(tableName, ..., hardDelete); funksiyanız çağırılacaq
        }
        else
            printf("SİNTAKSİS XƏTASI: Cədvəl adı tapılmadı.\n");

        return false;
    }
    return true;
    // }

    // ----------------------------------------------------------------
    // 8. INSERT INTO table_name (cols) VALUES (vals)
    // ----------------------------------------------------------------
    if (matchKeyword(&cursor, "INSERT"))
    {
        int insertMode = 0;
        // Standart "OR REPLACE" və ya "OR IGNORE" sintaksisini yoxlayırıq
        if (matchKeyword(&cursor, "OR REPLACE"))
        {
            insertMode = 1; // Köhnəni sil, yenisini yaz
        }
        else if (matchKeyword(&cursor, "OR IGNORE"))
        {
            insertMode = 2; // Toqquşma olarsa keç (skip)
        }
        else
        {
            // "OR ABORT" yazılsa da, heç nə yazılmasa da default rejim 0-dır
            matchKeyword(&cursor, "OR ABORT");
            insertMode = 0;
        }

        // İndi isə INTO açar sözünü və cədvəli yoxlayırıq
        if (matchKeyword(&cursor, "INTO"))
        {
            char tableName[64] = {0};
            int insertMode = 0; // Susmaya görə: 0 (RESTRICT / ABORT)
            if (extractWord(&cursor, tableName, sizeof(tableName)))
            {
                char colsBuf[256] = {0};
                char valsBuf[256] = {0};

                if (extractParentheses(&cursor, colsBuf, sizeof(colsBuf)))
                {
                    if (matchKeyword(&cursor, "VALUES"))
                    {
                        if (extractParentheses(&cursor, valsBuf, sizeof(valsBuf)))
                        {
                            printf("[PROSES İCRA OLUNUR]: '%s' cədvəlinə məlumat yazılır.\n", tableName);
                            printf("               -> Hədəf Sütunlar: [%s]\n", colsBuf);
                            printf("               -> Binar Dəyərlər : [%s]\n", valsBuf);

                            // Rejimə uyğun log çıxarırıq
                            printf("[REJİM]: insertMode = %d -> ", insertMode);
                            switch (insertMode)
                            {
                            case 0:
                                printf("ABORT/RESTRICT (Xəta olarsa əməliyyat ləğv ediləcək)\n");
                                break;
                            case 1:
                                printf("REPLACE/CASCADE (Eyni unikal data varsa, köhnəni silib yenisini yazacaq)\n");
                                break;
                            case 2:
                                printf("IGNORE/UNLINK (Xəta və ya toqquşma olarsa bu sətir yazılmayacaq, atlanacaq)\n");
                                break;
                            }

                            // Burada sizin insertRows(tableName, ..., insertMode); funksiyanız çağırılacaq
                        }
                        else
                        {
                            printf("SİNTAKSİS XƏTASI: VALUES üçün '(...)' mötərizəsi tapılmadı.\n");
                        }
                    }
                    else
                    {
                        printf("SİNTAKSİS XƏTASI: 'VALUES' açar sözü tapılmadı.\n");
                    }
                }
                else
                {
                    printf("SİNTAKSİS XƏTASI: Sütun siyahısı üçün '(...)' mötərizəsi tapılmadı.\n");
                }
            }
            else
            {
                printf("SİNTAKSİS XƏTASI: Cədvəl adı tapılmadı.\n");
            }
            return true;
        }
        else
        {
            printf("SİNTAKSİS XƏTASI: 'INTO' açar sözü tapılmadı.\n");
            return false;
        }
         return true;
    }

    // ----------------------------------------------------------------
    // 9. SELECT columns FROM t1 JOIN t2 ON cond WHERE cond
    // ----------------------------------------------------------------
    if (matchKeyword(&cursor, "SELECT"))
    {
        char columnsBuf[256] = {0};
        char table1[64] = {0};
        char table2[64] = {0};
        char joinCond[256] = {0};
        char whereCond[256] = {0};
        bool hasJoin = false;
        bool hasWhere = false;
        extractUntilKeywordOrEnd(&cursor, "FROM", columnsBuf, sizeof(columnsBuf));
        skipSpaces(columnsBuf);

        if (matchKeyword(&cursor, "FROM"))
        {
            extractWord(&cursor, table1, sizeof(table1));

            // JOIN yoxlanılması
            if (matchKeyword(&cursor, "JOIN"))
            {
                hasJoin = true;
                extractWord(&cursor, table2, sizeof(table2));

                if (matchKeyword(&cursor, "ON"))
                {
                    // Əgər WHERE varsa, WHERE-ə qədər oxu, yoxdursa sorğunun sonuna qədər
                    extractUntilKeywordOrEnd(&cursor, "WHERE", joinCond, sizeof(joinCond));
                    skipSpaces(joinCond);
                }
                else
                {
                    printf("SİNTAKSİS XƏTASI: JOIN üçün 'ON' şərti tapılmadı.\n");
                    return false;
                }
            }

            // WHERE yoxlanılması
            if (matchKeyword(&cursor, "WHERE"))
            {
                hasWhere = true;
                extractUntilKeywordOrEnd(&cursor, ";", whereCond, sizeof(whereCond));
                skipSpaces(whereCond);
            }

            // ---- FUNKSİYALARIN ÇAĞIRILMA MƏNTİQİ ----

            if (!hasJoin && !hasWhere)
            {
                // Ssenari 1: SELECT * FROM users
                printf("[PROSES]: Sadə select icra olunur.\n");
                selectData(table1);
            }
            else if (!hasJoin && hasWhere)
            {
                // Ssenari 2: SELECT * FROM users WHERE id = 5
                printf("[PROSES]: Şərtli select (WHERE) icra olunur.\n");

                // Qeyd: Burada whereCond stringini parçalayıb massivlərə doldurmalısınız
                // Nümunə təmsili çağırış:
                const char *cols[] = {"id"};
                int val = 5;
                void *vals[] = {&val};
                const char *ops[] = {"="};
                // uint8_t types[] = {1}; // 1 = INT təmsili

                selectWhere(table1, cols, vals, ops);
            }
            else if (hasJoin && !hasWhere)
            {
                // Ssenari 3: SELECT * FROM users JOIN orders ON id = user_id
                printf("[PROSES]: Relyasiyalı select (JOIN) icra olunur. Continue...\n");

                // char parentCol[64] = {0};
                // char childCol[64] = {0};
                // parseJoinCondition(joinCond, parentCol, childCol);

                // selectJoinData(table1, table2, parentCol, childCol);
            }
            else if (hasJoin && hasWhere)
            {
                // Ssenari 4: SELECT * FROM users JOIN orders ON id = user_id WHERE status = 'active'
                printf("[PROSES]: Həm JOIN, həm WHERE olan select icra olunur. Continue...\n");

                // char parentCol[64] = {0};
                // char childCol[64] = {0};
                // parseJoinCondition(joinCond, parentCol, childCol);

                // Həm join, həm şərt funksiyası
                // selectJoinWhereData(table1, table2, parentCol, childCol, ... where massivləri ...);
            }
            return true;
        }
        else
        {
            printf("SİNTAKSİS XƏTASI: 'FROM' açar sözü tapılmadı.\n");
            return false;
        }
         return true;
    }

    // ----------------------------------------------------------------
    // X. CONNECT DATABASE / USE (Verilənlər bazasına qoşulma)
    // ----------------------------------------------------------------
    if (matchKeyword(&cursor, "CONNECT DATABASE") || matchKeyword(&cursor, "CONNECT DB") || matchKeyword(&cursor, "USE"))
    {
        char dbName[64] = {0};
        char dbPsw[32] = {0}; // Parolu təhlükəsiz saxlamaq üçün local bufer

        if (extractWord(&cursor, dbName, sizeof(dbName)))
        {
            // PASSWORD açar sözünü skan etmək üçün dövr
            while (true)
            {
                if (matchKeyword(&cursor, "PASSWORD"))
                {
                    if (!extractWord(&cursor, dbPsw, sizeof(dbPsw)))
                    {
                        printf("SİNTAKSİS XƏTASI: PASSWORD açar sözündən sonra parol daxil edilməyib.\n");
                        return false;
                    }
                    continue;
                }
                break;
            }

            // 🌟 Mühərrikin backend tərəfindəki real binar qoşulma funksiyası çağırılır
            bool success = connectDb(dbName, dbPsw);

            if (success)
            {
                printf("[UĞURLU]: '%s' verilənlər bazasına qoşulma aktivləşdirildi.\n", dbName);
                if (strlen(dbPsw) > 0)
                {
                    printf("               -> Qoşulma növü: Təhlükəsiz (Parol təsdiqləndi).\n");
                }
                else
                {
                    printf("               -> Qoşulma növü: Açıq (Parolsuz baza).\n");
                }
            }
            else
            {
                printf("[XƏTA]: '%s' bazasına qoşulmaq mümkün olmadı! (Baza mövcut deyil və ya parol yalnışdır).\n", dbName);
            }
             return true;
        }
        else
        {
            printf("SİNTAKSİS XƏTASI: Qoşulmaq üçün verilənlər bazasının adı tapılmadı.\n");
             return false;
        }
        return true;
    }

    printf("[XƏTA]: Dəstəklənməyən SQL əmri və ya sintaksis xətası!\n");
}