// ====================================================================
// Parserin Köməkçi Funksiyaları (Yüksək Optimizasiyalı Pointer Skanerləri)
// ====================================================================

const char *globalSQL;

const char *skipSpaces(const char *str)
{
    while (*str && (isspace((unsigned char)*str) || *str == ';'))
        str++;
    return str;
}

// int parseWhereClause(char *whereCond, char *cols[], void *data[], const char *ops[], int *count)
// {
//     char temp[256];
//     strncpy(temp, whereCond, 256);

//     char *token = strtok(temp, " AND");
//     *count = 0;

//     while (token != NULL)
//     {
//         // Operatoru tapmaq üçün pointer
//         char *opPtr = strpbrk(token, "=<>");

//         if (opPtr)
//         {
//             // Operatoru birbaşa pointer ilə tuturuq
//             char opChar = *opPtr; // Məsələn: '=', '>', '<'
//             *opPtr = '\0';        // Sütun adı ilə dəyəri ayırırıq

//             cols[*count] = strdup(token);     // Sütun adı (malloc)
//             data[*count] = strdup(opPtr + 1); // Dəyər (malloc)

//             // Operatoru "const char*" olaraq təyin edirik
//             // Burada biz dinamik yaddaş ayırmırıq, proqramın data segmentindəki sabitləri istifadə edirik
//             if (opChar == '=')
//                 ops[*count] = "=";
//             else if (opChar == '>')
//                 ops[*count] = ">";
//             else if (opChar == '<')
//                 ops[*count] = "<";
//             else
//                 ops[*count] = "="; // Default

//             (*count)++;
//         }
//         token = strtok(NULL, " AND");
//     }
//     return *count;
// }
int parseWhereClause(char *whereCond,
                     char *cols[],
                     void *data[],
                     const char *ops[],
                     int *count)
{
    *count = 0;

    char *token = whereCond;

    while (token && *token)
    {
        char *next = strstr(token, " and ");

        if (next)
        {
            *next = '\0';
        }

        char *op = strpbrk(token, "=<>");

        if (op)
        {
            char opChar = *op;
            *op = '\0';

            char *col = token;
            char *val = op + 1;

            while (isspace((unsigned char)*col))
                col++;

            char *end = col + strlen(col) - 1;
            while (end > col && isspace((unsigned char)*end))
                *end-- = '\0';

            while (isspace((unsigned char)*val))
                val++;

            end = val + strlen(val) - 1;
            while (end > val && isspace((unsigned char)*end))
                *end-- = '\0';

            cols[*count] = strdup(col);
            data[*count] = strdup(val);

            switch (opChar)
            {
            case '=':
                ops[*count] = "=";
                break;
            case '>':
                ops[*count] = ">";
                break;
            case '<':
                ops[*count] = "<";
                break;
            }

            (*count)++;
        }

        token = next ? next + 5 : NULL; /* " and " uzunluğu */
    }

    return *count;
}

int countElements(const char *buffer)
{
    if (buffer == NULL || strlen(buffer) == 0)
        return 0;
    int count = 1; // Ən azı 1 element var
    for (int i = 0; buffer[i] != '\0'; i++)
    {
        if (buffer[i] == ',')
        {
            count++;
        }
    }
    return count;
}

// bool matchKeyword(const char **cursor, const char *keyword)
// {
//     *cursor = skipSpaces(*cursor);
//     int len = strlen(keyword);
//     if (strncasecmp(*cursor, keyword, len) == 0)
//     {
//         char nextChar = *(*cursor + len);
//         if (isalnum((unsigned char)nextChar) || nextChar == '_')
//         {
//             return false;
//         }
//         *cursor += len;
//         return true;
//     }
//     return false;
// }

bool matchKeyword(const char **cursor, const char *keyword)
{
    // Əsl cursoru qorumaq üçün müvəqqəti pointer yaradırıq
    const char *tempCheck = skipSpaces(*cursor);
    int len = strlen(keyword);

    // Case-insensitive müqayisə
    if (strncasecmp(tempCheck, keyword, len) == 0)
    {
        char nextChar = *(tempCheck + len);

        // Simvolun sözün davamı (məs: CREATE_date) olub-olmadığını yoxlayırıq
        if (isalnum((unsigned char)nextChar) || nextChar == '_')
        {
            return false; // Dəyişkən adıdır, açar söz deyil
        }

        // YALNIZ hər şey uğurludursa, əsl cursoru yeni mövqeyə çəkirik
        *cursor = tempCheck + len;
        return true;
    }

    // Uyğun gəlmədisə, false qayıdır və *cursor əvvəlki yerində dəyişmədən qalır
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

void parseWhereClause2(char *whereCond, char *cols[], void *vals[], const char *ops[], int *count)
{
    char *token = strtok(whereCond, "AND"); // "AND" ilə ayırırıq
    *count = 0;

    while (token != NULL)
    {
        char col[64], op[4], valStr[64];
        // SSS (Sütun = Dəyər) formatını parse et
        sscanf(token, "%s %s %s", col, op, valStr);

        cols[*count] = strdup(col); // Sütun adını kopyala
        ops[*count] = strdup(op);   // Operatoru kopyala

        // Dəyəri tiplərinə görə konvertasiya et
        // Bura əlavə funksiya qoymaq olar (isNumeric, isString)
        if (isdigit(valStr[0]))
        {
            uint32_t *v = malloc(sizeof(uint32_t));
            *v = atoi(valStr);
            vals[*count] = v;
        }
        else
        {
            vals[*count] = strdup(valStr); // String kimi
        }

        (*count)++;
        token = strtok(NULL, "AND");
    }
    cols[*count] = NULL; // Sonuncunu NULL et
}

Cursor executeSelect(const char *sql, Cursor cursor)
{

    char columnsBuf[256] = {0};
    char table1[64] = {0};
    char table2[64] = {0};
    char joinCond[256] = {0};
    char whereCond[256] = {0};
    bool hasJoin = false;
    bool hasWhere = false;
    extractUntilKeywordOrEnd(&sql, "FROM", columnsBuf, sizeof(columnsBuf));
    skipSpaces(columnsBuf);

    if (matchKeyword(&sql, "FROM"))
    {
        extractWord(&sql, table1, sizeof(table1));

        // JOIN yoxlanılması
        if (matchKeyword(&sql, "JOIN"))
        {
            hasJoin = true;
            extractWord(&sql, table2, sizeof(table2));

            if (matchKeyword(&sql, "ON"))
            {
                // Əgər WHERE varsa, WHERE-ə qədər oxu, yoxdursa sorğunun sonuna qədər
                extractUntilKeywordOrEnd(&sql, "WHERE", joinCond, sizeof(joinCond));
                skipSpaces(joinCond);
            }
            else
            {
                printf("SİNTAKSİS XƏTASI: JOIN üçün 'ON' şərti tapılmadı.\n");
                return cursor;
            }
        }

        // WHERE yoxlanılması
        if (matchKeyword(&sql, "WHERE"))
        {
            hasWhere = true;
            extractUntilKeywordOrEnd(&sql, ";", whereCond, sizeof(whereCond));
            skipSpaces(whereCond);
        }

        // ---- FUNKSİYALARIN ÇAĞIRILMA MƏNTİQİ ----

        if (!hasJoin && !hasWhere)
        {
            // Ssenari 1: SELECT * FROM users
            printf("[PROSES]: Sadə select icra olunur.\n");
            cursor = selectData(table1, &cursor);
        }
        else if (!hasJoin && hasWhere)
        {
            // Ssenari 2: SELECT * FROM users WHERE id = 5
            printf("[PROSES]: Şərtli select (WHERE) icra olunur.\n");

            // Qeyd: Burada whereCond stringini parçalayıb massivlərə doldurmalısınız
            // Nümunə təmsili çağırış:
            char *cols[10];      // Maksimum 10 şərt
            void *vals[10];      // Dəyərlər
            const char *ops[10]; // Operatorlar
            int whereCount = 0;
            // uint8_t types[] = {1}; // 1 = INT təmsili
            parseWhereClause2(whereCond, cols, vals, ops, &whereCount);
            printf(" col count:  %d \n", whereCount);
            cursor = selectWhereCore(table1, cols, vals, ops, whereCount);
            for (int i = 0; i < whereCount; i++)
            {
                printf(" data: %s %s %d \n", cols[i], ops[i], vals[i]);
                free(cols[i]);
                free((void *)ops[i]);
                free(vals[i]);
            }
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
        // cursor.tableName=table1;
        strncpy(cursor.tableName, (char *)table1, sizeof(cursor.tableName) - 1);
        return cursor;
    }
    else
    {
        printf("SİNTAKSİS XƏTASI: 'FROM' açar sözü tapılmadı.\n");
        return cursor;
    }
    return cursor;
}

void fetch(Cursor *c)
{
    // Əvvəlki yaddaşı azad et ki, memory leak olmasın
    if (c->rowIndices)
    {
        free(c->rowIndices);
        c->rowIndices = NULL;
    }
    printf("DEBUG: Cari SQL hissəsi: %s\n", globalSQL); // SQL-i çap edir
    const char *sqlPtr = c->sql;
    matchKeyword(&globalSQL, "SELECT");
    // Yeni 10-luq batch-i gətir
    *c = executeSelect(globalSQL, *c);
}

// void fetch(Cursor *c) {
//     // Əvvəlki datanı təmizlə
//     if (c->rowIndices) { free(c->rowIndices); c->rowIndices = NULL; }

//     // DİQQƏT: executeSelect-i yox, birbaşa cursor-un
//     // mövcud vəziyyətinə uyğun növbəti batch-i gətirən funksiyanı çağır.
//     // Məsələn: selectNextBatch(c);

//     // Əgər selectData funksiyanız offset qəbul etmirsə, onu mütləq dəyişin:
//     // selectData(c->tableName, c); // Cursor-un içindəki offset-i istifadə etsin

//     // Nümunə:
//     c->offset += 10;
//     *c = loadDataFromTable(c->tableName, c->offset);
// }

Cursor executeSQL(const char *sql)
{
    Cursor retrunCursor;
    // Cursor cursor;
    memset(&retrunCursor, 0, sizeof(Cursor)); // Bütün sahələri 0-a bərabər edir
    retrunCursor.count = 0;
    retrunCursor.isFinished = false;
    retrunCursor.rowIndices = NULL;
    retrunCursor.lastOffset = 0; // Faylın başlanğıcından axtarışa başlayır
    // retrunCursor.isFinished = false;
    // retrunCursor.rowIndices = NULL; // İlk dəfə null edirik, funksiya özü malloc edəcək
    // retrunCursor.sql = sql;
    // strncpy(retrunCursor.sql, sql, sizeof(retrunCursor.sql) - 1);
    strncpy(retrunCursor.sql, (char *)sql, sizeof(retrunCursor.sql) - 1);
    globalSQL = sql;
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
        return retrunCursor;
    }

    // ----------------------------------------------------------------
    // 2. SHOW TABLES / SHOW TABLE
    // ----------------------------------------------------------------
    if (matchKeyword(&cursor, "SHOW TABLES") || matchKeyword(&cursor, "SHOW TABLE"))
    {
        printf("[PROSES İCRA OLUNUR]: Hazırkı aktiv bazanın 'metadata/tables.db' faylından silinməmiş cədvəllərin adları ekrana çıxarılır.\n");
        selectTables("*");
        return retrunCursor;
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
                        return retrunCursor;
                    }
                    continue;
                }
                break; // Başqa tanınan açar söz yoxdursa dövrdən çıx
            }

            // Mühərrikin funksiyasına real verilənlər ötürülür

            if (reCreate)
            {
                dropDb(dbName, dbPsw);
                printf("[PROSES İCRA OLUNUR]: '%s' bazası (Parol: '%s') köhnə qeydlərdən təmizlənərək YENİDƏN yaradılır (reCreate = true).\n", dbName, dbPsw);
            }
            else
            {
                printf("[PROSES İCRA OLUNUR]: '%s' bazası (Parol: '%s') yoxlanılır, yoxdursa sistemdə yaradılır (reCreate = false).\n", dbName, dbPsw);
            }
            createDb(dbName, dbPsw);
            return retrunCursor;
        }
        else
        {
            printf("SİNTAKSİS XƏTASI: Verilənlər bazasının adı tapılmadı.\n");
            return retrunCursor;
        }
        return retrunCursor;
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
                        return retrunCursor;
                    }
                    continue;
                }
                break; // Başqa tanınan açar söz yoxdursa dövrdən çıx
            }

            // Real binar silmə funksiyası yalnız uğurlu skandandan sonra çağırılır
            dropDb(dbName, dbPsw);

            printf("[PROSES İCRA OLUNUR]: '%s' bazası (Parol doğrulaması: '%s'), daxili cədvəlləri və metadatası ilə birlikdə diskdən tamamilə silinir.\n", dbName, dbPsw);
            return retrunCursor;
        }
        else
        {
            printf("SİNTAKSİS XƏTASI: Silinəcək verilənlər bazasının adı tapılmadı.\n");
            return retrunCursor;
        }
        return retrunCursor;
    }

    if (matchKeyword(&cursor, "SELECT METADATA"))
    {
        char dbName[64] = {0};
        // char dbPsw[32] = {0}; // Parolu saxlamaq üçün təhlükəsiz bufer

        if (extractWord(&cursor, dbName, sizeof(dbName)))
        {
            // // PASSWORD açar sözünü və gələcəkdə əlavə edilə biləcək modifier-ləri təhlükəsiz oxumaq üçün dövr
            // while (true)
            // {
            //     if (matchKeyword(&cursor, "PASSWORD"))
            //     {
            //         if (!extractWord(&cursor, dbPsw, sizeof(dbPsw)))
            //         {
            //             printf("SİNTAKSİS XƏTASI: PASSWORD açar sözündən sonra parol təyin edilməyib.\n");
            //             return retrunCursor;
            //         }
            //         continue;
            //     }
            //     break; // Başqa tanınan açar söz yoxdursa dövrdən çıx
            // }

            // Real binar silmə funksiyası yalnız uğurlu skandandan sonra çağırılır
            printMetadata(dbName);

            printf("[PROSES İCRA OLUNUR]: Matadata print edilir");
            return retrunCursor;
        }
        else
        {
            printf("SİNTAKSİS XƏTASI: Silinəcək verilənlər bazasının adı tapılmadı.\n");
            return retrunCursor;
        }
        return retrunCursor;
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
        printf("Createing Table... \n");
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
        return retrunCursor;
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
            dropTableHard(tableName);

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
            return retrunCursor;
        }
        else
        {
            printf("SİNTAKSİS XƏTASI: Silinəcək cədvəlin adı tapılmadı.\n");
            return retrunCursor;
        }

        return retrunCursor;
    }
    // }

    // ----------------------------------------------------------------
    // 7. DELETE FROM ... WHERE
    // ----------------------------------------------------------------
    //    uint8_t parseDeleteRows(const char *cursor)
    // {

    // if (matchKeyword(&cursor, "DELETE FROM"))

    // {
    //     char tableName[64] = {0};
    //     int hardDelete = 0; // Susmaya görə: 0 (RESTRICT)
    //     char whereCond[256] = {0};
    //     if (extractWord(&cursor, tableName, sizeof(tableName)))
    //     {
    //         // 1. WHERE şərtini yoxlayırıq
    //         if (matchKeyword(&cursor, "WHERE"))
    //         {
    //             // Şərti oxuyuruq
    //             extractUntilKeywordOrEnd(&cursor, ";", whereCond, sizeof(whereCond));

    //             // İndi isə şərtin daxilində və ya sonunda silinmə rejimini yoxlayırıq.
    //             // Qeyd: Əgər WHERE daxilində sətirlərin sonunda CASCADE yazılıbsa, onu ayırmalıyıq.
    //             // Parserinizin gücünə görə cursor üzərindən də matchKeyword edə bilərsiniz.

    //             // Nümunə yoxlama (Sorğunun sonundakı rejim tənzimlənməsi):
    //             // `cursor` daxilində mövqeyə görə rejim təyini:
    //             if (matchKeyword(&cursor, "CASCADE"))
    //             {
    //                 hardDelete = 1;
    //             }
    //             else if (matchKeyword(&cursor, "UNLINK"))
    //             {
    //                 hardDelete = 2;
    //             }
    //             else
    //             {
    //                 hardDelete = 0; // default və ya RESTRICT
    //             }
    //             deleteRows(const char *tableName, char *whereColumnsName[], void *whereColumnsData[], const char *whereOperators[], int hardDelete)

    //             printf("[PROSES İCRA OLUNUR]: '%s' cədvəlində '%s' şərtini ödəyən sətirlər üçün silinmə başladıldı.\n", tableName, whereCond);
    //         }
    //         else
    //             printf("[PROSES İCRA OLUNUR]: '%s' cədvəlindəki BÜTÜN sətirlər silinməyə gedir.\n", tableName);

    //         // 2. hardDelete rejiminə uyğun əməliyyat məlumatı
    //         printf("[REJİM]: hardDelete = %d -> ", hardDelete);
    //         switch (hardDelete)
    //         {
    //         case 0:
    //             printf("RESTRICT (Əlaqəli sətirlər varsa, silinmə rədd ediləcək)\n");
    //             break;
    //         case 1:
    //             printf("CASCADE (Əlaqəli bütün asılı sətirlər də birlikdə silinəcək/soft-delete ediləcək)\n");
    //             break;
    //         case 2:
    //             printf("UNLINK (Asılı sətirlərin əlaqə sütunları sıfırlanacaq, əsas sətir silinəcək)\n");
    //             break;
    //         }

    //         // Burada sizin deleteRows(tableName, ..., hardDelete); funksiyanız çağırılacaq
    //     }
    //     else
    //         printf("SİNTAKSİS XƏTASI: Cədvəl adı tapılmadı.\n");

    //     return retrunCursor;
    // }

    if (matchKeyword(&cursor, "DELETE FROM"))
    {
        printf("Delete Tables 1 . . . \n");
        char tableName[64] = {0};
        int hardDelete = 0;
        char whereCond[256] = {0};

        if (extractWord(&cursor, tableName, sizeof(tableName)))
        {
            printf("Delete Tables 2 . . . \n");
            if (matchKeyword(&cursor, "WHERE"))
            {
                printf("Delete Tables WHERE . . . \n");
                // 1. Şərti oxu
                extractUntilKeywordOrEnd(&cursor, ";", whereCond, sizeof(whereCond));

                // 2. BOŞLUQ TƏMİZLƏMƏ: whereCond-in başındakı boşluqları sil (əgər varsa)
                char *trimmedWhere = whereCond;
                while (*trimmedWhere == ' ')
                    trimmedWhere++;

                // 3. CASCADE/UNLINK yoxlamasını təmizlənmiş string üzərində et
                if (strstr(trimmedWhere, "CASCADE"))
                {
                    printf("Delete Tables Cascade . . . \n");
                    hardDelete = 1;
                    *strstr(trimmedWhere, "CASCADE") = '\0';
                }
                else if (strstr(trimmedWhere, "UNLINK"))
                {
                    printf("Delete Tables unlink . . . \n");
                    hardDelete = 2;
                    *strstr(trimmedWhere, "UNLINK") = '\0';
                }

                // Parser vasitəsilə WHERE şərtini massivlərə çevir
                char *cols[10];
                const char *ops[10];
                void *data[10];
                int condCount = 0;
                printf("Check: %s\n", whereCond);
                parseWhereClause(whereCond, cols, data, ops, &condCount);
                printf("Check: %s\n", whereCond);
                // FUNKSİYANIN ÇAĞIRILMASI
                printf("\n--- [DEBUG DELETE: Məlumatlar Yoxlanılır] ---\n");
                printf("Cədvəl Adı: %s\n", tableName);

                // Silinmə rejiminin çapı
                printf("Silinmə Rejimi: %d -> ", hardDelete);
                switch (hardDelete)
                {
                case 0:
                    printf("RESTRICT\n");
                    break;
                case 1:
                    printf("CASCADE\n");
                    break;
                case 2:
                    printf("UNLINK\n");
                    break;
                default:
                    printf("Naməlum\n");
                    break;
                }

                // Şərtlərin çapı (condCount-u burada təyin etmədiyiniz üçün NULL yoxlanışı ilə gedirik)
                printf("WHERE Şərtləri:\n");
                for (int i = 0; i < 10; i++)
                {
                    if (cols[i] == NULL)
                        break;

                    printf("  [%d] Sütun: '%s' | Operator: '%s' | Dəyər: '%s'\n",
                           i,
                           cols[i],
                           ops[i],
                           (char *)data[i]); // void* olduğu üçün char* olaraq cast edirik
                }
                printf("--------------------------------------------\n");
                deleteRows(tableName, cols, data, ops, hardDelete);

                // Yaddaşı təmizlə (malloc etmisinizsə)
                for (int i = 0; i < condCount; i++)
                {
                    free(cols[i]);
                    free(data[i]);
                    // free(ops[i]);
                }
            }
        }
        return retrunCursor;
    }

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
                            int columns_count = countElements(colsBuf);
                            printf("[PROSES İCRA OLUNUR]: '%s' cədvəlinə məlumat yazılır.\n", tableName);
                            printf("               -> Hədəf Sütunlar: [%s]\n", colsBuf);
                            printf("               -> Binar Dəyərlər : [%s]\n", valsBuf);
                            // printf("               -> columns_count : [%s]\n", columns_count);
                            printf("              -> columns_count : [%d]\n", columns_count);

                            // Rejimə uyğun log çıxarırıq
                            printf("[REJİM]: insertMode = %d -> ", insertMode);
                            switch (insertMode)
                            {
                            case 0:
                                printf("ABORT/RESTRICT (Xəta olarsa əməliyyat ləğv ediləcək)\n");
                                insertRowsWithDelete(tableName, colsBuf, valsBuf, columns_count);
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
            return retrunCursor;
        }
        else
        {
            printf("SİNTAKSİS XƏTASI: 'INTO' açar sözü tapılmadı.\n");
            return retrunCursor;
        }
        return retrunCursor;
    }

    // ----------------------------------------------------------------
    // 9. SELECT columns FROM t1 JOIN t2 ON cond WHERE cond
    // ----------------------------------------------------------------
    if (matchKeyword(&cursor, "SELECT"))
    {

        retrunCursor = executeSelect(sql, retrunCursor);
        return retrunCursor;
    }

    // ----------------------------------------------------------------
    // X. CONNECT DATABASE / USE (Verilənlər bazasına qoşulma)
    // ----------------------------------------------------------------
    if (matchKeyword(&cursor, "CONNECT DATABASE") || matchKeyword(&cursor, "CONNECT DB") || matchKeyword(&cursor, "USE"))
    {
        printf("connecting DB....\n");
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
                        return retrunCursor;
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
            return retrunCursor;
        }
        else
        {
            printf("SİNTAKSİS XƏTASI: Qoşulmaq üçün verilənlər bazasının adı tapılmadı.\n");
            return retrunCursor;
        }
        return retrunCursor;
    }
    // SQL sorğusunu emal edən hissə
    if (matchKeyword(&cursor, "DROP SYSTEM"))
    {
        const char dbName[64] = "sqlBinDB";
        // "DROP SYSTEM <dbname>;" formatında adı oxuyur
        // if (readIdentifier(&cursor, dbName, sizeof(dbName)))
        // {
        if (executeDropSystem(dbName))
        {
            printf("Sistem '%s' uğurla silindi.\n", dbName);
        }
        else
        {
            printf("XƏTA: Sistem '%s' silinərkən xəta baş verdi.\n", dbName);
        }
        return retrunCursor;
        // }
    }

    printf("[XƏTA]: Dəstəklənməyən SQL əmri və ya sintaksis xətası!\n");
    return retrunCursor;
}
