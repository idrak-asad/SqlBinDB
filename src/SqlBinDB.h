#ifndef SQL_BIN_DB_H
#define SQL_BIN_DB_H


// #ifndef DB_CONTROLS_H
// #define DB_CONTROLS_H


// #include <ctype.h>
// #include <direct.h>  // Windows-da mkdir() funksiyası üçün (Linux/ESP32 üçün: <sys/stat.h>)

#include <stdio.h>   // FILE, fopen, fread, fwrite, snprintf üçün mütləqdir
#include <stdlib.h>  // system(), malloc(), free() üçün
#include <string.h>  // strcmp(), strncpy(), memset() üçün
#include <stdbool.h> // bool, true, false tipləri üçün
// #include <stdint.h>  // uint8_t, uint32_t tipləri üçün
// #include <direct.h>  // Windows-da _mkdir() funksiyası üçün mütləqdir

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <stdbool.h>

// ====================================================================
// 1. PLATFORMA TƏYİNATI VƏ KİTABXANALARININ ÇAĞIRILMASI
// ====================================================================
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
    #define PLATFORM_ESP32
    #include "LittleFS.h"

#elif defined(__linux__) || defined(__raspberry_pi__)
    #define PLATFORM_LINUX
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>

#elif defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
    #include <direct.h>  // Windows-da qovluq yaratmaq üçün (_mkdir)
    #include <io.h>
    #include <ctype.h>
    #include <stdint.h>  // uint8_t, uint32_t tipləri üçün
#endif




// #include "lib\add_controls.h"
// #include "lib\db_controls.h"
// #include "lib\table_controls.h"
// #include "lib\data_controls.h"
// #include "lib\reletion_controls.h"
// #include "lib\select_controls.h"
// #include "lib\index_controls.h"

// C++ (Arduino/PlatformIO) mühitində C kodlarının sıradan çıxmaması üçün qoruyucu
#ifdef __cplusplus
extern "C" {
#endif

// Qlobal verilənlər bazası yolunun elanı
// extern char current_db_path[256];

#include "add_controls.h"
// #include "db_platform.h" // Platforma abstraksiya faylımız
#include "db_controls.h"
#include "table_controls.h"
#include "data_controls.h"
#include "relation_controls.h" // Sizin fayl adındakı "e" hərfi ilə (reletion)
#include "select_controls.h"
#include "index_controls.h"

#ifdef __cplusplus
}
#endif

#endif // SQL_BIN_DB_H