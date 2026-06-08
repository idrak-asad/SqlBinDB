#ifndef SQL_BIN_DB_H
#define SQL_BIN_DB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
// #include <windows.h>



// ====================================================================
// 1. PLATFORMANIN AVTOMATİK TƏYİN EDİLMƏSİ VƏ ABSTRAKSİYA
// ====================================================================
#if defined(ESP32) || defined(ARDUINO)
    #define TARGET_PLATFORM_ESP32
    #define PLATFORM_ESP32
    #include "LittleFS.h"
    #include "Arduino.h"

    // ESP32 üçün fayl tipləri və funksiyaları
    typedef fs::File DBFile;
    #define DB_SEEK_SET SeekSet
    #define DB_PRINTF(...) Serial.printf(__VA_ARGS__)
    #define DB_PRINT(x) Serial.print(x)

#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #define TARGET_PLATFORM_WINDOWS
    #define PLATFORM_WINDOWS
    #include <direct.h>
    #include <io.h>
    #include <ctype.h>
    #include <stdint.h>

    // Windows üçün fayl tipləri və funksiyaları
    typedef FILE* DBFile;
    #define DB_SEEK_SET SEEK_SET
    #define DB_PRINTF(...) printf(__VA_ARGS__)
    #define DB_PRINT(x) printf("%s", x)

#elif defined(__linux__)
    #define TARGET_PLATFORM_LINUX
    #define PLATFORM_LINUX
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <stdint.h>

    typedef FILE* DBFile;
    #define DB_SEEK_SET SEEK_SET
    #define DB_PRINTF(...) printf(__VA_ARGS__)
    #define DB_PRINT(x) printf("%s", x)
#endif


#if defined(TARGET_PLATFORM_ESP32)
    #define FILE_PTR File
    #define FILE_READ(f, buf, size) f.read(buf, size)
    #define FILE_WRITE(f, buf, size) f.write(buf, size)
    #define FILE_SEEK(f, offset) f.seek(offset)
    #define FILE_CLOSE(f) f.close()
    #define FILE_FLUSH(f) f.flush()
    #define LOG_PRINT Serial.printf
#else
    #define FILE_PTR FILE*
    #define FILE_READ(f, buf, size) fread(buf, size, 1, f)
    #define FILE_WRITE(f, buf, size) fwrite(buf, size, 1, f)
    #define FILE_SEEK(f, offset) (fseek(f, offset, SEEK_SET) == 0)
    #define FILE_CLOSE(f) fclose(f)
    #define FILE_FLUSH(f) fflush(f)
    #define LOG_PRINT printf
#endif



#if defined(TARGET_PLATFORM_ESP32)
    // ESP32 üçün
    #define CLOSE_FILE(f) (f).close()
    #define READ_HEADER(f, h) ((f).read((uint8_t *)(h), sizeof(DBHeader)) == sizeof(DBHeader))
#else
    // Windows/Standart C üçün
    #define CLOSE_FILE(f) fclose(f)
    #define READ_HEADER(f, h) (fread((h), sizeof(DBHeader), 1, (f)) == 1)
#endif


// ====================================================================
// 2. KİTABXANALARININ ÇAĞIRILMASI
// ====================================================================
#ifdef __cplusplus
extern "C" {
#endif

#include "add_controls.h"
#include "select_controls.h"
#include "db_controls.h"
#include "table_controls.h"
#include "data_controls.h"
#include "relation_controls.h"
#include "index_controls.h"
#include "sql_pars_controls.h"

#ifdef __cplusplus
}
#endif

#endif // SQL_BIN_DB_H