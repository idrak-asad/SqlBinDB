#ifndef ADD_CONTROLS_H
#define ADD_CONTROLS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_COLUMNS 15
#define MAX_CHAR 50
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

// // Tip ID-ləri
// #define TYPE_INT          1
// #define TYPE_UINT32       2
// #define TYPE_UINT8        3
// #define TYPE_CHAR         4
// #define TYPE_CHAR2        5
// #define TYPE_FLOAT        6
// #define TYPE_FIXED_POINT  7
// #define TYPE_DATETIME     8
// #define TYPE_VARCHAR2     9
// #define TYPE_CHAR_N       10

char current_db_path[128] = "";
char current_db_name[18] = "";


int getColumnIndexInConfig(ColumnConfig configs[], int colCount, const char *colName);
int getColumnOffsetInRow(ColumnConfig configs[], int colCount, int colIdx);
bool helperCheckCondition(uint8_t *dataPtr, uint8_t dataType, void *whereData, const char *op);
uint8_t getTableIdByName(const char *tableName);
bool getTableNameById(uint8_t tableId, char *outName);
bool getColumnNameById(uint8_t tableId, uint8_t colId, char *outColName);

// extern char current_db_path[128];
// extern char current_db_name[18];

// uint8_t getTableIdByName(const char *tableName);
// uint8_t getColumnIdByName(uint8_t tableId, const char *colName);
// bool dropTable(const char *tableName, int hardDrop);
// uint8_t deleteRows(const char *tableName, char *whereColumnsName[], void *whereColumnsData[], char *whereOperators[], int hardDelete);

#pragma pack(push, 1)
// .db faylının ən başında duracaq idarəetmə bloku (Metadata)

// typedef struct {
//     char columnName[MAX_NAME_LEN];
//     char columnType[16];
//     uint16_t byteSize;
// } ColumnConfig;

// typedef struct {
//     char columnName[32]; // Sütunun adı
//     char columnType[16]; // SÜTUNUN TİPİ (Bura mütləq columnType olmalıdır!)
//     uint16_t byteSize;   // Bayt ölçüsü
// } ColumnConfig;

// typedef struct {
//     uint32_t signature;       // "SQLB"
//     char tableName[32];
//     uint32_t columnCount;
//     uint32_t rowCount;        // Hazırda bazada olan real sətir sayısı (maxRows-u keçə bilməz)
//     uint32_t rowSize;
//     uint32_t maxRows;         // MƏKSİMUM SƏTİR LİMİTİ (Yeni)
//     uint32_t nextRowIndex;    // NÖVBƏTİ YAZILACAQ İNDEKS (0-dan maxRows-1 kimi fırlanır) (Yeni)
// } DBHeader;

// metadata/columns.db üçün sıxılmış sətir strukturu (Tam 37 Byte)
// typedef struct {
//     uint8_t is_deleted;
//     uint8_t table_id;
//     char column_name[32];
//     uint8_t type_id;
//     uint8_t type_size;
//     uint8_t constraints;
// } CompactColumnMeta;

// metadata/tables.db üçün də limiti qeyd edirik (Cəmi 39 byte olur)
typedef struct {
    uint8_t is_deleted;
    uint8_t table_id;
    char table_name[32];
    uint8_t col_count;
    uint32_t max_rows;    // Yeni
} CompactTableMeta;

// typedef struct {
//     uint8_t is_deleted;       // Soft-delete (1 byte)
//     uint8_t parent_table_id;  // Əsas cədvəl (Məsələn: 'users' cədvəlinin ID-si) (1 byte)
//     uint8_t parent_col_id;    // Əsas sütun (Məsələn: 'id' sütununu ID-si) (1 byte)
//     uint8_t child_table_id;   // Bağlanan cədvəl (Məsələn: 'orders' cədvəlinin ID-si) (1 byte)
//     uint8_t child_col_id;     // Bağlanan sütun (Məsələn: 'user_id' sütunun ID-si) (1 byte)
// } CompactRelation;

// typedef struct {
//     uint8_t is_deleted;
//     uint8_t parent_table_id;
//     uint8_t parent_col_id;
//     uint8_t child_table_id;
//     uint8_t child_col_id;
// } CompactRelation;

// // Cəmi 5 Byte! Ultra-sıxılmış əlaqə strukturu
// typedef struct {
//     uint8_t is_deleted;       // 1 byte (Soft delete üçün)
//     uint8_t parent_table_id;  // 1 byte (Əsas cədvəl, məs: users)
//     uint8_t parent_col_id;    // 1 byte (Əsas sütun, məs: id)
//     uint8_t child_table_id;   // 1 byte (Bağlı cədvəl, məs: devices)
//     uint8_t child_col_id;     // 1 byte (Bağlı sütun, məs: user_id)
// } CompactRelation;

// #pragma pack(push, 1)
// typedef struct {
//     uint8_t is_deleted;
//     char db_name[18];
//     char db_psw[18];
// } DBRegistry;
// #pragma pack(pop)

// Mərkəzi indeks qeydiyyat strukturu (metadata/indexes.db)
// typedef struct {
//     uint8_t is_deleted;
//     uint8_t table_id;
//     uint8_t column_id;
//     char index_name[32];
// } IndexMeta;

// Fiziki indeks qeydi (.idx faylının daxili forması)
// typedef struct {
//     uint32_t data_value; // İndekslənən uint32_t dəyər (Məs: id)
//     uint32_t file_pos;   // Sətrin ana .db faylındakı fiziki bayt ünvanı
// } IndexEntry;

// typedef struct
// {
//     uint8_t is_deleted;
//     uint8_t table_id;
//     char table_name[MAX_NAME_LEN];
// } CompactTableMeta;

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

// typedef struct
// {
//     uint32_t maxRows;
//     uint32_t rowCount;
//     uint16_t rowSize;
//     uint8_t columnCount;
//     uint32_t last_inserted_id; // AUTO_INCREMENT izləmək üçün yeni bölmə
// } DBHeader;


typedef struct {
    uint32_t signature;       // "SQLB"
    char tableName[32];
    uint32_t columnCount;
    uint32_t rowCount;        // Hazırda bazada olan real sətir sayısı (maxRows-u keçə bilməz)
    uint32_t rowSize;
    uint32_t maxRows;         // MƏKSİMUM SƏTİR LİMİTİ (Yeni)
    uint32_t nextRowIndex;    // NÖVBƏTİ YAZILACAQ İNDEKS (0-dan maxRows-1 kimi fırlanır) (Yeni)
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

#pragma pack(pop)

// uint8_t calculate_type_size(const char *typeStr);
uint8_t getTypeId(char *typeStr);
void insertIntoIndexFile(const char *idxName, uint32_t keyValue, uint32_t offsetValue);

// ===================================================================
// KANARDA YARADILMAYAN NÜVƏ FUNKSİYALARININ İCRA KODLARI (GÖVDƏLƏRİ)
// ===================================================================

// 1. Sütun adına görə onun konfiqurasiya massivindəki indeksini tapır
int getColumnIndexInConfig(ColumnConfig configs[], int colCount, const char *colName) {
    for (int i = 0; i < colCount; i++) {
        if (strcmp(configs[i].columnName, colName) == 0) {
            return i;
        }
    }
    return -1; // Sütun tapılmadı
}

// 2. Sütunun binar sətir daxilində neçənci baytdan (offset) başladığını hesablayır
int getColumnOffsetInRow(ColumnConfig configs[], int colCount, int colIdx) {
    // İlk bayt silinmə (is_deleted) bayrağı üçün ayrılır (bizim kodda rowBuffer[0])
    int offset = 1; 
    for (int i = 0; i < colIdx; i++) {
        offset += configs[i].dataSize;
    }
    return offset;
}

// 3. Verilən operatora və tipə görə şərtin ödənilib-ödənilmədiyini yoxlayır
bool helperCheckCondition(uint8_t *dataPtr, uint8_t dataType, void *whereData, const char *op) {
    if (dataPtr == NULL || whereData == NULL || op == NULL) return false;

    if (dataType == TYPE_INT) {
        int32_t valInDb = *(int32_t *)dataPtr;
        int32_t valWhere = *(int32_t *)whereData;

        if (strcmp(op, "=") == 0)   return valInDb == valWhere;
        if (strcmp(op, "!=") == 0)  return valInDb != valWhere;
        if (strcmp(op, ">") == 0)   return valInDb > valWhere;
        if (strcmp(op, "<") == 0)   return valInDb < valWhere;
        if (strcmp(op, ">=") == 0)  return valInDb >= valWhere;
        if (strcmp(op, "<=") == 0)  return valInDb <= valWhere;
    } 
    else if (dataType == TYPE_UINT32) {
        uint32_t valInDb = *(uint32_t *)dataPtr;
        uint32_t valWhere = *(uint32_t *)whereData;

        if (strcmp(op, "=") == 0)   return valInDb == valWhere;
        if (strcmp(op, "!=") == 0)  return valInDb != valWhere;
        if (strcmp(op, ">") == 0)   return valInDb > valWhere;
        if (strcmp(op, "<") == 0)   return valInDb < valWhere;
        if (strcmp(op, ">=") == 0)  return valInDb >= valWhere;
        if (strcmp(op, "<=") == 0)  return valInDb <= valWhere;
    } 
    else if (dataType == TYPE_FLOAT) {
        float valInDb = *(float *)dataPtr;
        float valWhere = *(float *)whereData;

        if (strcmp(op, "=") == 0)   return abs(valInDb - valWhere) < 0.0001; // Float bərabərlik toleransı
        if (strcmp(op, "!=") == 0)  return abs(valInDb - valWhere) >= 0.0001;
        if (strcmp(op, ">") == 0)   return valInDb > valWhere;
        if (strcmp(op, "<") == 0)   return valInDb < valWhere;
        if (strcmp(op, ">=") == 0)  return valInDb >= valWhere;
        if (strcmp(op, "<=") == 0)  return valInDb <= valWhere;
    } 
    else if (dataType == TYPE_CHAR2) {
        // Mətn tipləri üçün əsasən "=" və "!=" müqayisələri keçərlidir
        if (strcmp(op, "=") == 0)   return strcmp((char *)dataPtr, (char *)whereData) == 0;
        if (strcmp(op, "!=") == 0)  return strcmp((char *)dataPtr, (char *)whereData) != 0;
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
uint8_t getTableIdByName(const char *tableName)
{
    char tablesMetaPath[256];
    snprintf(tablesMetaPath, sizeof(tablesMetaPath), "%s/metadata/tables.db", current_db_path);
    FILE *f = fopen(tablesMetaPath, "rb");
    if (!f)
        return 0;
    CompactTableMeta tMeta;
    uint8_t foundId = 0;
    while (fread(&tMeta, sizeof(CompactTableMeta), 1, f))
    {
        if (tMeta.is_deleted == 0 && strcmp(tMeta.table_name, tableName) == 0)
        {
            foundId = tMeta.table_id;
            break;
        }
    }
    fclose(f);
    return foundId;
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
bool getTableNameById(uint8_t tableId, char *outName)
{
    char tablesMetaPath[256];
    snprintf(tablesMetaPath, sizeof(tablesMetaPath), "%s/metadata/tables.db", current_db_path);
    FILE *f = fopen(tablesMetaPath, "rb");
    if (!f)
        return false;
    CompactTableMeta tMeta;
    bool found = false;
    while (fread(&tMeta, sizeof(CompactTableMeta), 1, f))
    {
        if (tMeta.is_deleted == 0 && tMeta.table_id == tableId)
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
bool getColumnNameById(uint8_t tableId, uint8_t colId, char *outColName) {
    char columnsMetaPath[256];
    snprintf(columnsMetaPath, sizeof(columnsMetaPath), "%s/metadata/columns.db", current_db_path);
    FILE *f = fopen(columnsMetaPath, "rb");
    if (!f) return false;

    CompactColumnMeta cMeta;
    uint8_t currentCharId = 1; // 0-cı indeks is_deleted-dir, real sütunlar 1-dən başlayır
    bool found = false;

    while (fread(&cMeta, sizeof(CompactColumnMeta), 1, f)) {
        if (cMeta.table_id == tableId && cMeta.is_deleted == 0) {
            if (currentCharId == colId) {
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

#endif