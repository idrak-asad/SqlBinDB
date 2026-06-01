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

#define TARGET_PLATFORM_ESP32      // <--- ESP32 üçün bunu aktiv et
// #define TARGET_PLATFORM_WINDOWS   // <--- Windows üçün bunu aktiv et
// #define TARGET_PLATFORM_LINUX     // <--- Linux üçün bunu aktiv et

// ====================================================================
// 1. PLATFORMA TƏYİNATI VƏ KİTABXANALARININ ÇAĞIRILMASI
// ====================================================================
// Müvafiq kitabxanaların qoşulması
#if defined(TARGET_PLATFORM_ESP32)
    #define PLATFORM_ESP32
    #include "LittleFS.h"
#elif defined(TARGET_PLATFORM_LINUX)
    #define PLATFORM_LINUX
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define PLATFORM_WINDOWS
    #include <direct.h>
    #include <io.h>
    #include <ctype.h>
    #include <stdint.h>
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


#include "sql_pars_controls.h"
#include "add_controls.h"
#include "select_controls.h"

// #include "db_platform.h" // Platforma abstraksiya faylımız
#include "db_controls.h"
#include "table_controls.h"
#include "data_controls.h"
#include "relation_controls.h" // Sizin fayl adındakı "e" hərfi ilə (reletion)

#include "index_controls.h"

#ifdef __cplusplus
}
#endif

#endif // SQL_BIN_DB_H