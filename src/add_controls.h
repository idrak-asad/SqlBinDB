#ifndef ADD_CONTROLS_H
#define ADD_CONTROLS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_COLUMNS 15
#define MAX_CHAR 40
#define MAX_NAME_LEN 32

// #ifndef TABLE_CONTROLS_H
// #define TABLE_CONTROLS_H

// Məhdudiyyət (Constraint) Bitmask maskaları
#define FLAG_PRIMARY_KEY (1 << 0)    // 00000001
#define FLAG_NOT_NULL (1 << 1)       // 00000010
#define FLAG_UNIQUE (1 << 2)         // 00000100
#define FLAG_AUTO_INCREMENT (1 << 3) // 00001000 -> Yeni əlavə olundu

// Tip ID-ləri
#define TYPE_INT 1
#define TYPE_UINT32 2
#define TYPE_UINT8 3
#define TYPE_CHAR2 4       // Sabit ölçülü char (Köhnə char)
#define TYPE_FLOAT 5       // Standart Float (4 bayt)
#define TYPE_FIXED_POINT 6 // Sənin dediyin RAM qoruyan tip (2 bayt - int16_t - 100-ə bölünən)
#define TYPE_DATETIME 7    // 8 bayt binar (Y-M-D H:M:S binar paket)
#define TYPE_TIMESTAMP 8   // 4 bayt Unix Epoch saniyəsi (uint32_t)
#define TYPE_VARCHAR2 9    // Dəyişkən sətir üçün bloq pointeri (4 bayt offset)

char current_db_path[70] = "";
char current_db_name[18] = "";

// extern char current_db_path[128];
// extern char current_db_name[18];

#pragma pack(push, 1)
// .db faylının ən başında duracaq idarəetmə bloku (Metadata)

// metadata/tables.db üçün də limiti qeyd edirik (Cəmi 39 byte olur)
typedef struct
{
    uint8_t is_deleted;
    // uint8_t table_id;
    char table_name[32];
    uint8_t col_count;
    uint32_t max_rows; // Yeni
} CompactTableMeta;

typedef struct
{
    uint8_t is_deleted;
    uint8_t table_id;
    char column_name[MAX_NAME_LEN];
    uint8_t type_id;
    uint8_t data_size;
    uint8_t constraints;
} CompactColumnMeta;

typedef struct
{
    uint8_t is_deleted;
    uint8_t table_id;
    uint8_t column_id;
    char index_name[64];
} IndexMeta;

typedef struct
{
    uint8_t is_deleted;
    uint8_t parent_table_id;
    uint8_t parent_col_id;
    uint8_t child_table_id;
    uint8_t child_col_id;
} CompactRelation;

typedef struct
{
    uint32_t signature; // "SQLB"
    char tableName[32];
    uint32_t columnCount;
    uint32_t rowCount; // Hazırda bazada olan real sətir sayısı (maxRows-u keçə bilməz)
    uint32_t rowSize;
    uint32_t maxRows;          // MƏKSİMUM SƏTİR LİMİTİ (Yeni)
    uint32_t nextRowIndex;     // NÖVBƏTİ YAZILACAQ İNDEKS (0-dan maxRows-1 kimi fırlanır) (Yeni)
    uint32_t last_inserted_id; // AUTO_INCREMENT izləmək üçün yeni bölmə
} DBHeader;

typedef struct
{
    char columnName[MAX_NAME_LEN];
    uint8_t typeID;
    uint8_t dataSize;
    uint8_t constraints;
} ColumnConfig;

// Datetime binar strukturu (Cəmi 8 bayt yer tutur)
typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t reserved; // Padding bərabərliyi üçün
} BinaryDateTime;

// İndeks faylında dataların axtarışı üçün binar struktur
typedef struct
{
    uint32_t key_value;   // İndekslənən unikal/id açar dəyəri
    uint32_t file_offset; // Əsas mətndə/faylda yerləşdiyi offset ünvanı
} IndexEntry;

typedef struct
{
    uint8_t is_deleted;
    char db_name[18];
    char db_password[18];
} DBRegistry;

typedef struct
{
    char sql[200];
    char tableName[MAX_NAME_LEN];
    uint32_t *rowIndices; // Tapılan sətirlərin ofsetləri
    uint8_t count;        // Hazırkı batch-də tapılanların sayı
    uint32_t lastOffset;  // Axtarışın qaldığı yer (fayl ofseti)
    bool isFinished;      // Bütün axtarış bitdimi?
} Cursor;

typedef struct
{
    char columnName[32];
    uint32_t intValue;
    float floatValue;
    char stringValue[64];
    // Digər tipləri də əlavə edə bilərsiniz
} Field;

typedef struct
{
    Field fields[16]; // Məsələn, 16 sütunluq bir sətir
    int fieldCount;
} DataRow;

#pragma pack(pop)

int getColumnIndexInConfig(ColumnConfig configs[], int colCount, const char *colName);
int getColumnOffsetInRow(ColumnConfig configs[], int colCount, int colIdx);
// bool helperCheckCondition(uint8_t *dataPtr, uint8_t dataType, void *whereData, const char *op);
bool helperCheckCondition(void *colData, int typeID, void *queryData, const char *op);
// uint8_t getTableIdByName(const char *tableName);
bool getTableNameByIndex(uint8_t tableId, char *outName);
bool getColumnNameById(uint8_t tableId, uint8_t colId, char *outColName);

// uint8_t calculate_type_size(const char *typeStr);
uint8_t getTypeId(char *typeStr);
void insertIntoIndexFile(const char *idxName, uint32_t keyValue, uint32_t offsetValue);

#if defined(ESP32) || defined(ARDUINO)
#include <FS.h>
#include <LittleFS.h>
#define PLATFORM_ESP32
// typedef File *FileHandle; // Pointer kimi saxlayaq
typedef fs::File FileHandle;
#else
#include <stdio.h>
#include <windows.h>
#define PLATFORM_WIN
typedef FILE *FileHandle;
#define delay(ms) Sleep(ms)
#endif

size_t myRead(FileHandle f, void *buffer, size_t size)
{
#ifdef PLATFORM_ESP32
    return f.read((uint8_t *)buffer, size);
#else
    return fread(buffer, 1, size, f);
#endif
}

// Seek funksiyası (Universal)
bool mySeek(FileHandle f, long offset)
{
#ifdef PLATFORM_ESP32
    return f.seek(offset);
#else
    return fseek(f, offset, SEEK_SET) == 0;
#endif
}

void myClose(FileHandle f)
{
#ifdef PLATFORM_ESP32
    f.close();
#else
    if (f != NULL)
    {
        fclose(f);
    }
#endif
}

#if defined(TARGET_PLATFORM_ESP32)
// ==================== ESP32 / ARDUINO REJİMİ ====================
File openTable(const char *tableName, const char *openType)
{
    char tableFilePath[256];
    const char *cleanPath = current_db_path;
    if (strncmp(current_db_path, "/littlefs", 9) == 0)
    {
        cleanPath = current_db_path + 9;
    }

    snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", cleanPath, tableName);
    Serial.print("[Diaqnostika] LittleFS ile acilmaga calisilan real yol: ");
    Serial.println(tableFilePath);

    File file = LittleFS.open(tableFilePath, openType);
    if (!file)
    {
        Serial.println("[XƏTA] 'r+' rejimində tapılmadı, 'r' (oxuma) rejimi yoxlanılır...");
        file = LittleFS.open(tableFilePath, "r");
    }
    return file;
}
#else
// ==================== PC (WINDOWS / LINUX) REJİMİ ====================
FILE *openTable(const char *tableName, const char *openType)
{
    char tableFilePath[256];
    const char *cleanPath = current_db_path;
    if (strncmp(current_db_path, "/littlefs", 9) == 0)
    {
        cleanPath = current_db_path + 9;
    }

    snprintf(tableFilePath, sizeof(tableFilePath), "%s/tables/%s.db", cleanPath, tableName);
    printf("[Diaqnostika] PC ile acilmaga calisilan real yol: %s\n", tableFilePath);

    // Windows standard C-də binar fayllar mütləq "rb", "wb" və ya "rb+" rejimləri ilə açılmalıdır
    char mode[8];
    strncpy(mode, openType, sizeof(mode) - 1);
    if (strchr(mode, 'b') == NULL)
    {
        strncat(mode, "b", sizeof(mode) - strlen(mode) - 1);
    }

    FILE *file = fopen(tableFilePath, mode);
    if (!file)
    {
        printf("[XƏTA] '%s' rejimində tapılmadı, 'rb' (oxuma) rejimi yoxlanılır...\n", mode);
        file = fopen(tableFilePath, "rb");
    }

    if (!file)
    {
        printf("[KRİTİK XƏTA] Standard C bu faylı heç bir rejimdə aça bilmədi: %s\n", tableFilePath);
    }
    return file;
}
#endif

// 1. Sütun adına görə onun konfiqurasiya massivindəki indeksini tapır
int getColumnIndexInConfig(ColumnConfig configs[], int colCount, const char *colName)
{
    for (int i = 0; i < colCount; i++)
    {
        if (strcmp(configs[i].columnName, colName) == 0)
        {
            return i;
        }
    }
    return -1; // Sütun tapılmadı
}

// 2. Sütunun binar sətir daxilində neçənci baytdan (offset) başladığını hesablayır
int getColumnOffsetInRow(ColumnConfig configs[], int colCount, int colIdx)
{
    // İlk bayt silinmə (is_deleted) bayrağı üçün ayrılır (bizim kodda rowBuffer[0])
    int offset = 1;
    for (int i = 0; i < colIdx; i++)
    {
        offset += configs[i].dataSize;
    }
    return offset;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool checkCondition(uint8_t typeID, uint8_t dataSize, uint8_t *dbFieldPtr, void *userValPtr, const char *op)
{
    printf("checkCondition running: \n");
    if (userValPtr == NULL)
        return false;
    // printf("num: %d\n", 2);
    bool result = false;
    if (typeID == TYPE_CHAR2)
    {
        // printf("num: %d\n", 3);
        char dbTemp[64] = {0};
        memcpy(dbTemp, dbFieldPtr, dataSize);
        // if (strcmp(op, "=") == 0)
        return (strcmp(dbTemp, (const char *)userValPtr) == 0);
        // printf("num: %d\n", 4);
    }

    else if (typeID == TYPE_INT || typeID == TYPE_UINT32)
    {
        // printf("num: %d\n", 5);
        uint32_t dbVal = 0;
        // if (typeID == TYPE_INT || typeID == TYPE_UINT32)
        // {
        memcpy(&dbVal, dbFieldPtr, 4);
        // }

        // 2. İstifadəçidən gələn məlumatı (userVal) çeviririk
        // Əgər gələn məlumat string-dirsə, onu ədədə çeviririk
        uint32_t userVal = atoi((const char *)userValPtr);
        // if (strcmp(op, "=") == 0) return (dbVal == userVal);
        // if (strcmp(op, ">") == 0) return (dbVal > userVal);
        // if (strcmp(op, "<") == 0) return (dbVal < userVal);
        if (strcmp(op, "=") == 0)
            result = (dbVal == userVal);
        else if (strcmp(op, ">") == 0)
            result = (dbVal > userVal);
        else if (strcmp(op, "<") == 0)
            result = (dbVal < userVal);

        printf("DEBUG: DB Val: %u, Op: %s, User Val: %u | Result: %s\n",
               dbVal, op, userVal, result ? "TRUE" : "FALSE");
        return result;
    }
    else if (typeID == TYPE_UINT8)
    {
        uint8_t userVal = atoi((const char *)userValPtr);
        // printf("num: %d\n", 7);
        uint8_t dbVal = *dbFieldPtr;
        // uint8_t userVal = *(uint8_t *)userValPtr;
        if (strcmp(op, "=") == 0)
            return (dbVal == userVal);
        // printf("num: %d\n", 8);
    }

    return false;
}

bool helperCheckCondition(void *colData, int typeID, void *queryData, const char *op)
{
    char *valStr = (char *)queryData; // Sorğudan gələn string ("9")

    // Məlumatı tipə görə oxuyuruq
    // printf("tipID: %d\n", typeID);
    switch (typeID)
    {
    case 1: // Məsələn: INT (4 bayt)
    {
        int dbVal = *(int *)colData;
        int qVal = atoi(valStr);

        // Əgər "9" sorğusu gəlibsə və atoi("9") 0 qayıdırsa,
        // string-in həqiqətən ədəd olduğunu yoxlayırıq
        if (qVal == 0 && strcmp(valStr, "0") != 0)
        {
            printf("[XƏTA]: '%s' dəyəri tam ədəd tipinə çevrilə bilmədi!\n", valStr);
            return false;
        }
        printf(": data type 1: %d =  %d\n", dbVal, qVal);
        if (strcmp(op, "=") == 0)
            return dbVal == qVal;
        if (strcmp(op, ">") == 0)
            return dbVal > qVal;
        if (strcmp(op, "<") == 0)
            return dbVal < qVal;
        break;
    }
    case 3: // uint8_t (Sizin qeyd etdiyiniz tip)
    {
        uint8_t dbVal = *(uint8_t *)colData;
        uint8_t qVal = (uint8_t)atoi(valStr);
        printf(": data type3: %d =  %d\n", dbVal, qVal);
        if (strcmp(op, "=") == 0)
            return dbVal == qVal;
        if (strcmp(op, ">") == 0)
            return dbVal > qVal;
        if (strcmp(op, "<") == 0)
            return dbVal < qVal;

        // return true;
    }
    case 4: // Məsələn: STRING (Char array)
    {
        // printf(": data type 4: %s =  %s\n", dbVal, valStr);
        char *dbVal = (char *)colData;
        if (strcmp(op, "=") == 0)
            return strcmp(dbVal, valStr) == 0;
        break;
    }
    default:
        printf("[XƏTA]: Naməlum tipID: %d\n", typeID);
        return false;
    }
    return false;
}

// ====================================================================
// 2. ADAPTİV FAYL SİSTEMİ FUNKSİYALARI
// ====================================================================

// Platformaya uyğun Qovluq (Directory) yaradılması
// bool platform_create_dir(const char *dir_path) {
//     #if defined(PLATFORM_ESP32)
//         // ESP32 (LittleFS) daxilində virtual olaraq qovluq iyerarxiyası avtomatik idarə olunur.
//         // Fiziki olaraq qovluq yaratmağa ehtiyac yoxdur, birbaşa faylı yazmaq kifayətdir.
//         return true;
//     #elif defined(PLATFORM_LINUX)
//         // Linux və Raspberry üçün icazələr (0777 - Oxuma/Yazma/İcra) ilə qovluq açırıq
//         struct stat st = {0};
//         if (stat(dir_path, &st) == -1) {
//             return mkdir(dir_path, 0777) == 0;
//         }
//         return true;
//     #elif defined(PLATFORM_WINDOWS)
//         // Windows üçün _mkdir funksiyası
//         if (_access(dir_path, 0) == -1) {
//             return _mkdir(dir_path) == 0;
//         }
//         return true;
//     #endif
// }

// void platform_create_dir(const char* path) {
//     #if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
//         // ESP32 üçün standart mkdir İŞLƏMİR! Mütləq LittleFS çağırılmalıdır:
//         if (!LittleFS.exists(path)) {
//             if (LittleFS.mkdir(path)) {
//                 Serial.printf("[Sistem] Qovluq ugurla yaradildi: %s\n", path);
//             } else {
//                 Serial.printf("[Sistem] XƏTA: %s qovlugu yaradıla bilmedi!\n", path);
//             }
//         }
//     #elif defined(_WIN32)
//         _mkdir(path);
//     #else
//         mkdir(path, 0777);
//     #endif
// }

void platform_create_dir(const char *path)
{
#if defined(TARGET_PLATFORM_ESP32)
    // Əgər ESP32 seçilibsə, mütləq LittleFS-in öz metodu işləməlidir!
    if (!LittleFS.exists(path))
    {
        if (LittleFS.mkdir(path))
        {
            Serial.printf("[Sistem] Qovluq ugurla yaradildi: %s\n", path);
        }
        else
        {
            Serial.printf("[Sistem] XƏTA: %s qovlugu yaradila bilmedi!\n", path);
        }
    }
#elif defined(TARGET_PLATFORM_WINDOWS)
    _mkdir(path); // Windows üçün
#elif defined(TARGET_PLATFORM_LINUX)
    mkdir(path, 0777); // Linux üçün
#endif
}

// Platformaya uyğun düzgün fayl yolunun (Path) formatlanması
void platform_format_path(char *output_path, size_t max_len, const char *db_name, const char *sub_folder, const char *file_name)
{
#if defined(PLATFORM_ESP32)
    // ESP32-də bütün fayl yolları kök qovluqdan ("/") başlamalıdır və alt qovluqlar sadəcə addır
    if (sub_folder && strlen(sub_folder) > 0)
    {
        snprintf(output_path, max_len, "/%s/%s/%s", db_name, sub_folder, file_name);
    }
    else
    {
        snprintf(output_path, max_len, "/%s/%s", db_name, file_name);
    }
#elif defined(PLATFORM_LINUX)
    // Linux / Raspberry standart "/" seperatoru istifadə edir
    if (sub_folder && strlen(sub_folder) > 0)
    {
        snprintf(output_path, max_len, "%s/%s/%s", db_name, sub_folder, file_name);
    }
    else
    {
        snprintf(output_path, max_len, "%s/%s", db_name, file_name);
    }
#elif defined(PLATFORM_WINDOWS)
    // Windows həm "/" həm də "\\" qəbul edir, qarışıqlıq olmasın deyə standart "/" saxlaya bilərik
    if (sub_folder && strlen(sub_folder) > 0)
    {
        snprintf(output_path, max_len, "%s/%s/%s", db_name, sub_folder, file_name);
    }
    else
    {
        snprintf(output_path, max_len, "%s/%s", db_name, file_name);
    }
#endif
}

// Platformaya uyğun Fiziki Fayl Silinməsi (DROP üçün)
bool platform_delete_file(const char *file_path)
{
    // Standart remove funksiyası hər üç platformada işləyir,
    // lakin ESP32 üçün öncə LittleFS-in aktivliyini yoxlamaq olar.
    return remove(file_path) == 0;
}

// uint8_t calculate_type_size(const char *typeStr)
// {
//     // 1. Təməl tiplərin yoxlanışı
//     if (strcmp(typeStr, "uint8_t") == 0)
//         return 1;
//     if (strcmp(typeStr, "uint16_t") == 0)
//         return 2;
//     if (strcmp(typeStr, "uint32_t") == 0)
//         return 4;
//     if (strcmp(typeStr, "int") == 0)
//         return 4;
//     if (strcmp(typeStr, "float") == 0)
//         return 4;
//     if (strncmp(typeStr, "char(", 5) == 0)
//     {
//         int length = 0;
//         // "char(%d)" formatına əsasən mötərizənin içindəki rəqəmi oxuyuruq
//         if (sscanf(typeStr, "char(%d)", &length) == 1)
//         {

//             if (MAX_CHAR < (uint8_t)length)
//             {
//                 return 0;
//             }
//             return (uint8_t)length; // char 1 byte olduğu üçün birbaşa uzunluğu qaytarırıq
//         }
//     }

//     return 0; // Əgər siyahıda olmayan naməlum bir tip gələrsə
// }

uint8_t getTypeId(char *typeStr)
{
    if (strcmp(typeStr, "int") == 0)
        return TYPE_INT;
    if (strcmp(typeStr, "uint32_t") == 0)
        return TYPE_UINT32;
    if (strcmp(typeStr, "uint8_t") == 0)
        return TYPE_UINT8;
    if (strncmp(typeStr, "char(", 5) == 0)
        return TYPE_CHAR2;
    return 0;
}

// Operatorları yoxlayan daxili köməkçi funksiya
bool compareValues(void *dbValue, void *checkValue, const char *op, uint8_t dataType)
{
    if (dataType == TYPE_UINT32 || dataType == TYPE_INT)
    {
        uint32_t dbVal = *(uint32_t *)dbValue;
        uint32_t chVal = *(uint32_t *)checkValue;
        if (strcmp(op, "=") == 0)
            return dbVal == chVal;
        if (strcmp(op, "!=") == 0)
            return dbVal != chVal;
        if (strcmp(op, ">") == 0)
            return dbVal > chVal;
        if (strcmp(op, "<") == 0)
            return dbVal < chVal;
        if (strcmp(op, ">=") == 0)
            return dbVal >= chVal;
        if (strcmp(op, "<=") == 0)
            return dbVal <= chVal;
    }
    else if (dataType == TYPE_UINT8)
    {
        uint8_t dbVal = *(uint8_t *)dbValue;
        uint8_t chVal = *(uint8_t *)checkValue;
        if (strcmp(op, "=") == 0)
            return dbVal == chVal;
        if (strcmp(op, "!=") == 0)
            return dbVal != chVal;
        if (strcmp(op, ">") == 0)
            return dbVal > chVal;
        if (strcmp(op, "<") == 0)
            return dbVal < chVal;
        if (strcmp(op, ">=") == 0)
            return dbVal >= chVal;
        if (strcmp(op, "<=") == 0)
            return dbVal <= chVal;
    }
    return false;
}

// Cədvəlin adından onun table_id-sini qaytarır
int getTableIndexByName(const char *tableName)
{
    // printf("getTableIndexByName-------------------------------: \n");
    // char tablesMetaPath[256];
    // snprintf(tablesMetaPath, sizeof(tablesMetaPath), "%s/metadata/tables.db", current_db_path);
#if defined(TARGET_PLATFORM_ESP32)
    FILE f = openTable(tableName, "r");
    if (!f){
// printf("cedvel tailmadi---------------------");
        return -;
    }
    
#else
    FILE *f = openTable(tableName, "r");
    // printf("cedvel tapildi-------------------------------0: \n");
    if (!f){
        // printf("cedvel tailmadi----------------");
        return -1;
    }
    
    // printf("cedvel tapildi-------------------------------1: \n");
#endif
//  printf("cedvel tapildi-------------------------------2: \n");

    CompactTableMeta tMeta;
    uint8_t currentIndex = 0; // Faylın başlanğıcında 0-cı indeks
    uint8_t foundIndex = 0;

    // fread hər dövrdə 1 qeyd oxuyur, biz isə currentIndex-i bir-bir artırırıq
    while (fread(&tMeta, sizeof(CompactTableMeta), 1, f))
    {
        currentIndex++; // 1-ci qeyd üçün 1, 2-ci qeyd üçün 2...

        if (tMeta.is_deleted == 0 && strcmp(tMeta.table_name, tableName) == 0)
        {
            foundIndex = currentIndex; // Budur, sıra nömrəsi
            break;
        }
    }
    fclose(f);
    // if (foundIndex==0){
    //     foundIndex++;
    // }
    return foundIndex; // Tapılmasa 0 qaytaracaq
}

// Cədvəl ID-si və sütun adına görə sütunun ID-sini (1-ci indeksdən başlayaraq) qaytarır
uint8_t getColumnIdByName(uint8_t tableId, const char *colName)
{
    char columnsMetaPath[256];
    snprintf(columnsMetaPath, sizeof(columnsMetaPath), "%s/metadata/columns.db", current_db_path);
    FILE *f = fopen(columnsMetaPath, "rb");
    if (!f)
        return 0;
    CompactColumnMeta cMeta;
    uint8_t colId = 1; // 0-cı is_deleted-dir
    bool found = false;
    while (fread(&cMeta, sizeof(CompactColumnMeta), 1, f))
    {
        if (cMeta.table_id == tableId)
        {
            if (cMeta.is_deleted == 0 && strcmp(cMeta.column_name, colName) == 0)
            {
                found = true;
                break;
            }
            colId++;
        }
    }
    fclose(f);
    return found ? colId : 0;
}

// Sütunun indeksli olub-olmadığını yoxlayır
bool isColumnIndexed(uint8_t tableId, uint8_t colId, char *outIndexName)
{
    char idxMetaPath[256];
    snprintf(idxMetaPath, sizeof(idxMetaPath), "%s/metadata/indexes.db", current_db_path);
    FILE *f = fopen(idxMetaPath, "rb");
    if (!f)
        return false;
    IndexMeta meta;
    bool found = false;
    while (fread(&meta, sizeof(IndexMeta), 1, f))
    {
        if (meta.is_deleted == 0 && meta.table_id == tableId && meta.column_id == colId)
        {
            if (outIndexName)
                strcpy(outIndexName, meta.index_name);
            found = true;
            break;
        }
    }
    fclose(f);
    return found;
}

// Helper: Cədvəl ID-sinə görə dinamik real adı tapır (MƏNTİQİ SƏHV BURADA HƏLL OLUNDU)
bool getTableNameByIndex(uint8_t tableIndex, char *outName)
{
    char tablesMetaPath[256];
    snprintf(tablesMetaPath, sizeof(tablesMetaPath), "%s/metadata/tables.db", current_db_path);
    FILE *f = fopen(tablesMetaPath, "rb");
    if (!f)
        return false;

    CompactTableMeta tMeta;
    bool found = false;
    uint8_t currentIndex = 0;

    // Faylı oxuyaraq sıra ilə gedirik
    while (fread(&tMeta, sizeof(CompactTableMeta), 1, f))
    {
        currentIndex++; // Hər qeyddə indeksi artırırıq

        // Əgər indekslərimiz üst-üstə düşürsə və qeyd silinməyibsə
        if (currentIndex == tableIndex && tMeta.is_deleted == 0)
        {
            strncpy(outName, tMeta.table_name, MAX_NAME_LEN);
            found = true;
            break;
        }
    }
    fclose(f);
    return found;
}

// Cədvəl ID-si və Sütun ID-sinə görə sütunun real adını metadatadan tapır
bool getColumnNameById(uint8_t tableId, uint8_t colId, char *outColName)
{
    char columnsMetaPath[256];
    snprintf(columnsMetaPath, sizeof(columnsMetaPath), "%s/metadata/columns.db", current_db_path);
    FILE *f = fopen(columnsMetaPath, "rb");
    if (!f)
        return false;

    CompactColumnMeta cMeta;
    uint8_t currentCharId = 1; // 0-cı indeks is_deleted-dir, real sütunlar 1-dən başlayır
    bool found = false;

    while (fread(&cMeta, sizeof(CompactColumnMeta), 1, f))
    {
        if (cMeta.table_id == tableId && cMeta.is_deleted == 0)
        {
            if (currentCharId == colId)
            {
                strncpy(outColName, cMeta.column_name, MAX_NAME_LEN);
                found = true;
                break;
            }
            currentCharId++;
        }
    }
    fclose(f);
    return found;
}

// uint8_t getTableIndexByName(const char *tableName)
// {
//     char tablesMetaPath[256];
//     snprintf(tablesMetaPath, sizeof(tablesMetaPath), "%s/metadata/tables.db", current_db_path);
//     FILE *f = fopen(tablesMetaPath, "rb");
//     if (!f)
//         return 0;

//     CompactTableMeta tMeta;
//     uint8_t currentIndex = 0; // Faylın başlanğıcında 0-cı indeks
//     uint8_t foundIndex = 0;

//     // fread hər dövrdə 1 qeyd oxuyur, biz isə currentIndex-i bir-bir artırırıq
//     while (fread(&tMeta, sizeof(CompactTableMeta), 1, f))
//     {
//         currentIndex++; // 1-ci qeyd üçün 1, 2-ci qeyd üçün 2...

//         if (tMeta.is_deleted == 0 && strcmp(tMeta.table_name, tableName) == 0)
//         {
//             foundIndex = currentIndex; // Budur, sıra nömrəsi
//             break;
//         }
//     }
//     fclose(f);
//     return foundIndex; // Tapılmasa 0 qaytaracaq
// }

int loadConfigsForTable(uint8_t tableId, ColumnConfig configs[])
{
    int count = 0;
    char path[256];
    snprintf(path, sizeof(path), "%s/metadata/columns.db", current_db_path);

    FILE *f = fopen(path, "rb");
    if (!f)
        return 0;

    CompactColumnMeta cMeta;
    while (fread(&cMeta, sizeof(CompactColumnMeta), 1, f))
    {
        // Yalnız aktiv (is_deleted == 0) və bizim cədvələ aid olanları götür
        if (cMeta.is_deleted == 0 && cMeta.table_id == tableId+1)
        {
            strncpy(configs[count].columnName, cMeta.column_name, MAX_NAME_LEN);
            configs[count].typeID = cMeta.type_id;
            configs[count].dataSize = cMeta.data_size;
            count++;
        }
    }
    fclose(f);
    return count; // Tapılan sütunların ümumi sayını qaytarır
}

int getOffsetOfColumn(const char *colName, ColumnConfig *configs, int colCount)
{
    int offset = 1; // 0-cı bayt 'is_deleted'
    for (int i = 1; i < colCount; i++)
    { // 1-dən başlayırıq, çünki 0-da is_deleted var
        if (strcmp(configs[i].columnName, colName) == 0)
            return offset;
        offset += configs[i].dataSize;
    }
    return -1;
}

#endif